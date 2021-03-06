/** AvidaGPEC.hpp
 * An evolutionary computing version of the AvidaGP directed evolution experiment.
 * Used to investigate whether EC results are predictive of the directed evolution results.
 *
 */


#pragma once
#ifndef AVIDAGP_EC_WORLD_HPP_INCLUDE
#define AVIDAGP_EC_WORLD_HPP_INCLUDE

#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <filesystem>
#include <sys/stat.h>

#ifdef DIRDEVO_THREADING
#include <thread>
#include <mutex>
#endif // DIRDEVO_THREADED

// json
#include "json/json.hpp"

// empirical includes
#include "emp/Evolve/World.hpp"
#include "emp/config/config.hpp"
#include "emp/control/Signal.hpp"

// local includes
#include "AvidaGPOrganism.hpp"
#include "AvidaGPReplicator.hpp"
#include "AvidaGPTaskSet.hpp"
#include "AvidaGPMutator.hpp"
#include "AvidaGPEnvironmentBank.hpp"

#include "../../utility/pareto.hpp"

namespace dirdevo {

EMP_BUILD_CONFIG(AvidaGPEvoCompConfig,
  GROUP(GLOBAL_SETTINGS, "Global settings"),
  VALUE(SEED, int, -1, "Seed for a simulation"),
  VALUE(POP_SIZE, size_t, 1, "Number of populations. Must be > 0"),
  VALUE(GENS, size_t, 10, "Number of generations to run"),
  VALUE(LOAD_ANCESTOR_FROM_FILE, bool, false, "Should the ancestral genome be loaded from file? NOTE - the experiment setup must implement this functionality."),
  VALUE(ANCESTOR_FILE, std::string, "ancestor.gen", "Path to file containing ancestor genome to be loaded"),
  VALUE(STOP_ON_SOLUTION, bool, true, "Stop running if a solution is found?"),
  VALUE(NUM_THREADS, size_t, 4, "How many threads to use when evaluating population? (only used when compiled with threading flag)"),

  GROUP(OUTPUT_SETTINGS, "Settings specific to experiment output"),
  VALUE(OUTPUT_DIR, std::string, "output", "Where should the experiment dump output?"),
  VALUE(OUTPUT_RESOLUTION, size_t, 10, "How often should we output non-snapshot files?"),
  VALUE(SNAPSHOT_RESOLUTION, size_t, 10, "How often should we snapshot the population?"),

  GROUP(EVALUATION_SETTINGS, "Settings related to program evaluation"),
  VALUE(EVAL_STEPS, size_t, 30, "How many CPU cycles do programs get per evaluation?"),

  GROUP(SELECTION_SETTINGS, "Settings for selecting individuals as parents"),
  VALUE(SELECTION_METHOD, std::string, "elite", "Which algorithm should be used to select populations to propagate? Options: elite, tournament"),
  VALUE(ELITE_SEL_NUM_ELITES, size_t, 1, "(elite selection) The top ELITE_SEL_NUM_ELITES populations are propagated"),
  VALUE(TOURNAMENT_SEL_TOURN_SIZE, size_t, 4, "(tournament selection) How large are tournaments?"),

  GROUP(AVIDAGP_MUTATION_SETTINGS, "Settings specific to AvidaGP mutation"),
  VALUE(AVIDAGP_MUT_RATE_INST_SUB, double, 0.01, "Instruction substitution rate (applied per-instruction)"),
  VALUE(AVIDAGP_MUT_RATE_ARG_SUB, double, 0.025, "Instruction argument substitution rate (applied per-argument)"),

  GROUP(AVIDAGP_ENV_SETTINGS, "Settings specific to AvidaGP environment/task"),
  VALUE(AVIDAGP_UNIQUE_ENV_OUTPUT, bool, true, "Should each environment input buffer result in unique output for all environment tasks?"),
  VALUE(AVIDAGP_ENV_FILE, std::string, "environment-ec.json", "Path to the environment file that specifies which tasks are rewarded at organism and world level")

)

/// Select everything to serve as parents.
template<typename ORG>
void NoSelect(emp::World<ORG>& world) {
  for (size_t i = 0; i < world.GetSize(); ++i) {
    if (world.IsOccupied(i)) {
      world.DoBirth(world.GetGenomeAt(i), i, 1);
    }
  }
}


/// ==ELITE== Selection picks a set of the most fit individuals from the population to move to
/// the next generation.  Find top e_count individuals and make copy_count copies of each.
/// @param world The emp::World object with the organisms to be selected.
/// @param e_count How many distinct organisms should be chosen, starting from the most fit.
/// @param repro_count How many total reproduction events to carry out?
template<typename ORG>
void EliteSelect(emp::World<ORG> & world, size_t e_count=1, size_t repro_count=1) {
  emp_assert(e_count > 0 && e_count <= world.GetNumOrgs(), e_count);
  emp_assert(repro_count > 0);

  // Load the population into a multimap, sorted by fitness.
  std::multimap<double, size_t> fit_map;
  for (size_t id = 0; id < world.GetSize(); id++) {
    if (world.IsOccupied(id)) {
      const double cur_fit = world.CalcFitnessID(id);
      fit_map.insert( std::make_pair(cur_fit, id) );
    }
  }

  emp::vector<size_t> elites(e_count, 0);
  // Grab the organisms with the top fitnesses.
  auto m = fit_map.rbegin();
  for (size_t i = 0; i < e_count; i++) {
    const size_t repro_id = m->second;
    elites[i] = repro_id;
    ++m;
  }
  // Reproduce elites up to repro_count.
  for (size_t i = 0; i < repro_count; ++i) {
    const size_t selected_id = elites[i%e_count];
    world.DoBirth(world.GetGenomeAt(selected_id), selected_id);
  }

}


template<typename ORG>
void NonDominatedEliteSelect(
  emp::World<ORG>& world,
  const emp::vector< std::function<double(const ORG &)> > & fit_funs,
  size_t repro_count=1
) {
  emp_assert(world.GetSize() > 0);
  emp_assert(fit_funs.size() > 0);

  const size_t num_candidates = world.GetSize();
  emp_assert(num_candidates > 0);
  const size_t obj_count = fit_funs.size();

  // Build a score table.
  emp::vector< emp::vector<double> > score_table(num_candidates, emp::vector<double>(fit_funs.size(), -1.0));
  for (size_t cand_i = 0; cand_i < num_candidates; ++cand_i) {
    score_table[cand_i].resize(obj_count);
    if (!world.IsOccupied(cand_i)) continue;
    for (size_t fun_i = 0; fun_i < obj_count; ++fun_i) {
      emp_assert(obj_count == fit_funs.size());
      score_table[cand_i][fun_i] = fit_funs[fun_i](world.GetOrg(cand_i));
    }
  }

  // Find the pareto front
  emp::vector<size_t> front(find_pareto_front(score_table));
  // std::cout << "----" << std::endl;
  // std::cout << "Pareto front size: " << front.size() << std::endl;
  // for (size_t i = 0; i < front.size(); ++i) {
  //   std::cout << "  " << world.GetOrg(front[i]).GetPhenotype().org_task_performances << std::endl;
  // }
  emp::Shuffle(world.GetRandom(), front);
  const size_t front_size = front.size();
  for (size_t i = 0; i < repro_count; ++i) {
    const size_t selected_id = front[i%front_size];
    world.DoBirth(world.GetGenomeAt(selected_id), selected_id);
  }

}


/// Note that this class is not necessarily optimized (in a run time sense) for an evolutionary computing setup.
/// Instead, I am trying to reuse as many components from the AvidaGP directed evolution experiment as possible because:
/// (1) that's fewer things I need to implement and (2) ensures that as many things as possible are the same across this and the directed evolution setups.
class AvidaGPEvoCompWorld : public emp::World<AvidaGPOrganism> {
public:

  using org_t = AvidaGPOrganism;
  using this_t = AvidaGPEvoCompWorld;
  using base_t = emp::World<org_t>;

  using config_t = AvidaGPEvoCompConfig;

  using mutator_t = AvidaGPMutator;
  using hardware_t = AvidaGPReplicator;
  using inst_lib_t = typename hardware_t::inst_lib_t;

  using org_task_set_t = AvidaGPTaskSet;
  using env_bank_t = AvidaGPEnvironmentBank;

  static constexpr size_t ENV_BANK_SIZE = 10000;

  // Environment/logic task information
  struct MetabolicPathway {
    size_t id=0;                                ///< Pathway id
    emp::vector<size_t> global_task_id_lookup;  ///< Lookup global-level task id given pathway-level task id
    org_task_set_t task_set;                    ///< Which tasks are part of this pathway?
    emp::Ptr<env_bank_t> env_bank=nullptr;      ///< lookup table of IO examples

    ~MetabolicPathway() {
      if (env_bank) env_bank.Delete();
    }
  };

  /// Used to track information about tasks
  struct TaskInfo {
    size_t pathway=0;       ///< Which pathway is this task a part of?
    size_t local_id=0;      ///< What is that local within-pathway id of this task?
    bool repeatable=false;  ///< Can organisms get individual-level credit for this task multiple times?
    double value=0;         ///< What is the value of this task?
  };

protected:

  const config_t& config;

  inst_lib_t inst_lib;
  mutator_t mutator;

  bool found_solution;
  size_t max_fit_org_id=0; // gets set to solution id when solution found
  // size_t solution_id=0;

  emp::Signal<void(void)> end_setup_sig;    ///< Triggered at end of world setup.
  emp::Signal<void(void)> do_selection_sig; ///< Triggered when it's time to do selection!

  emp::vector< emp::Range<size_t> > thread_org_ranges;
  std::function<void(size_t)> eval_thread;

  size_t total_tasks=0;
  emp::vector<TaskInfo> task_info;
  emp::vector<MetabolicPathway> task_pathways;

  emp::vector<double> org_aggregate_scores;
  emp::vector< std::function<double(const org_t&)> > fit_fun_set;  ///< Manages fitness functions if we're doing multi-obj selection.
  emp::vector<bool> population_task_coverage;

  std::string output_dir;
  // emp::Ptr<emp::DataFile> max_fit_file=nullptr;
  emp::Ptr<emp::DataFile> world_summary_file=nullptr; // TODO - combine these?
  struct PopSnapshotter {
    emp::Ptr<emp::DataFile> file=nullptr;
    size_t cur_org_id=0;
    ~PopSnapshotter() {
      if (file) file.Delete();
    }
  } population_snapshotter;

  // emp::Ptr<emp::DataFile> population_snapshot_file=nullptr;

  void Setup();
  void SetupThreading();
  void SetupTasks();
  void SetupSelection();
  void SetupInstLib();
  void SetupMutator();
  void SetupDataCollection();

  void SetupEliteSelection();
  void SetupTournamentSelection();
  void SetupLexicaseSelection();
  void SetupNonDominatedEliteSelection();
  void SetupNonDominatedTournamentSelection();
  void SetupRandomSelection();
  void SetupNoSelection();


  void InitPop();

  void DoEvaluation();
  void DoSelection();
  void DoUpdate();

  void DoConfigSnapshot();
  void DoPopSnapshot();

  void RunOrg(size_t org_id);

public:
  AvidaGPEvoCompWorld(
    const config_t& in_config
  ) :
    base_t("AvidaGP EC World", true),
    config(in_config)
  {
    NewRandom(config.SEED()); // Set the random number seed in world's rng.
    Setup();
  }

  ~AvidaGPEvoCompWorld() {
    // if (max_fit_file) max_fit_file.Delete();
    // if (population_snapshot_file) population_snapshot_file.Delete();
    if (world_summary_file) world_summary_file.Delete();
  }

  void RunStep();
  void Run();

};


void AvidaGPEvoCompWorld::Setup() {
  std::cout << "--- Setting up AvidaGP EvoComp World ---" << std::endl;

  Reset(); // Reset the world
  found_solution = false;

  #ifdef DIRDEVO_THREADING
  SetupThreading();
  #endif //DIRDEVO_THREADING


  // Init tasks
  SetupTasks();

  // Initialize selection
  SetupSelection();

  // Create instruction set
  SetupInstLib();

  // Initialize mutator
  SetupMutator();

  // Setup population initialization
  end_setup_sig.AddAction(
    [this]() {
      std::cout << "Initializing the population..." << std::endl;
      InitPop();
      std::cout << "  ...Done." << std::endl;
      // SetAutoMutate();
    }
  );

  // Setup population structure
  SetPopStruct_Mixed(true);

  // Setup data collection
  SetupDataCollection();

  // Setup internal birth signals
  OnInjectReady(
    [this](org_t& org) {
      org.OnInjectReady();
      org.GetPhenotype().Reset(total_tasks);
      // org.SetMerit(1.0); // merit not used by EC world
    }
  );

  OnPlacement(
    [this](size_t pos) {
      auto& org = GetOrg(pos);
      org.OnPlacement(pos);
      const size_t num_pathways = task_pathways.size();
      org.SetNumPathways(num_pathways);
      // Assign organism an environment ID for each pathway
      for (size_t pathway_id = 0; pathway_id < num_pathways; ++pathway_id) {
        auto& pathway = task_pathways[pathway_id];
        auto& env_bank = *(pathway.env_bank);
        const size_t env_id = random_ptr->GetUInt(env_bank.GetSize());
        org.GetHardware().SetEnvID(pathway_id, env_id);
        org.GetHardware().GetInputBuffer(pathway_id) = env_bank.GetEnvironment(env_id).input_buffer;
      }
    }
  );

  // auto on_death_key = OnOrgDeath(
  //   [this](size_t pos) {
  //     GetOrg(pos).OnDeath(pos);
  //   }
  // );

  // OnWorldDestruct(
  //   [this,on_death_key]() {
  //     on_death_sig.Remove(on_death_key);
  //   }
  // );

  OnBeforeRepro(
    [this](size_t parent_pos) {
      auto& parent = GetOrg(parent_pos);
      parent.OnBeforeRepro();
    }
  );

  OnOffspringReady(
    [this](org_t& offspring, size_t parent_pos) {
      DoMutationsOrg(offspring);
      auto& parent = GetOrg(parent_pos);
      offspring.OnBirth(parent);
      parent.OnOffspringReady(offspring);
      // Reset offspring phenotype
      offspring.GetPhenotype().Reset(total_tasks);
    }
  );

  // todo - task.onworldsetup
  end_setup_sig.Trigger();
}

/// Only called when running in threading mode.
void AvidaGPEvoCompWorld::SetupThreading() {

  #ifdef DIRDEVO_THREADING
  std::cout << "Compiled with threading enabled." << std::endl;
  emp_assert(config.NUM_THREADS(), "NUM_THREADS cannot be set to 0 when compiled with DIRDEVO_THREADING flag.");
  // Determine which organisms should be run on each thread every generation.
  const size_t orgs_per_thread = config.POP_SIZE() / config.NUM_THREADS();
  thread_org_ranges.resize(config.NUM_THREADS());
  for (size_t thread_i = 0; thread_i < config.NUM_THREADS(); ++thread_i) {
    thread_org_ranges[thread_i].Set((thread_i*orgs_per_thread), (thread_i*orgs_per_thread)+orgs_per_thread);
  }
  thread_org_ranges.back().SetUpper(config.POP_SIZE());

  #ifndef EMP_NDEBUG
  std::cout << "GetSize(): " << GetSize() << std::endl;
  for (auto& range : thread_org_ranges) {
    std::cout << "[" << range.GetLower() << "," << range.GetUpper() << ")" << std::endl;
  }
  #endif // EMP_NDEBUG

  // Setup eval thread function
  eval_thread = [this](size_t thread_id) {
    // std::cout << "Thread id: " << thread_id << "[" << thread_org_ranges[thread_id].GetLower() << "," << thread_org_ranges[thread_id].GetUpper() << ")" << std::endl;
    for (size_t org_id = thread_org_ranges[thread_id].GetLower(); org_id < thread_org_ranges[thread_id].GetUpper(); ++org_id) {
      emp_assert(IsOccupied(org_id));
      RunOrg(org_id);
    }
  };

  #endif // DIRDEVO_THREADING
}

/// Analogous to `SetupTasks` in AvidaGPMultiPathwayTask.hpp
void AvidaGPEvoCompWorld::SetupTasks() {
  // === Parse environment file ===
  // Check to see if the environment file exists
  const bool env_file_exists = std::filesystem::exists(config.AVIDAGP_ENV_FILE());
  if (!env_file_exists) {
    std::cout << "Environment file does not exist. " << config.AVIDAGP_ENV_FILE() << std::endl;
    std::exit(EXIT_FAILURE);
  }
  // If it does exist, read it.
  std::ifstream env_ifstream(config.AVIDAGP_ENV_FILE());
  nlohmann::json env_json;
  env_ifstream >> env_json;

  emp_assert(env_json.contains("tasks"), "Improperly configured environment file. Failed to find 'tasks'.");
  emp_assert(env_json.contains("pathways"), "Improperly configured environment file. Failed to find 'pathways'.");

  // How many pathways?
  const size_t num_pathways = env_json["pathways"];

  // Create metabolic pathways.
  task_pathways.resize(num_pathways);
  emp::vector< std::unordered_set<std::string> > pathway_task_set(num_pathways); // Keep track of which tasks have been requested for each pathway's task set.
  emp::vector< emp::vector<std::string> > pathway_task_order(num_pathways); // Keep track of the order we should add tasks
  emp::vector< std::unordered_map<std::string, nlohmann::json> > pathway_task_info(num_pathways);

  // Initialize each pathway
  for (size_t pathway_id=0; pathway_id < task_pathways.size(); ++pathway_id) {
    auto& pathway = task_pathways[pathway_id];
    pathway.id = 0;
    pathway.env_bank = emp::NewPtr<env_bank_t>(*random_ptr, pathway.task_set);
  }

  // Configure tasks
  auto& tasks_json = env_json["tasks"];
  for (auto& task : tasks_json) {
    emp_assert(task.contains("name"));
    const size_t task_pathway_id = (task.contains("pathway")) ? (size_t)task["pathway"] : 0;
    emp_assert(task_pathway_id < num_pathways, "Invalid task pathway id.", task_pathway_id, num_pathways);

    auto& task_set = pathway_task_set[task_pathway_id];
    auto& task_order = pathway_task_order[task_pathway_id];
    auto& task_info_map = pathway_task_info[task_pathway_id];

    // If this is the first time we've seen this task for this pathway, make note.
    if (!emp::Has(task_set, task["name"])) {
      task_set.emplace(task["name"]);
      task_order.emplace_back(task["name"]);
      task_info_map.emplace(
        task["name"],
        task
      );
    }
  }

  // Update pathways with tasks
  total_tasks = 0;
  for (size_t pathway_id=0; pathway_id < task_pathways.size(); ++pathway_id) {
    // Convenient shortcuts
    auto& pathway = task_pathways[pathway_id];
    auto& task_set = pathway_task_set[pathway_id];
    auto& task_order = pathway_task_order[pathway_id];
    auto& pathway_info = pathway_task_info[pathway_id];

    emp_assert(task_set.size() == task_order.size());

    // How many tasks in this pathway?
    const size_t num_tasks = task_set.size();

    // Add tasks to pathway's task set
    pathway.task_set.AddTasksByName(task_order);
    // Fill out global task information
    pathway.global_task_id_lookup.resize(num_tasks);
    for (size_t i = 0; i < num_tasks; ++i) {
      const size_t global_task_id = total_tasks + i;
      const size_t local_task_id = i;
      const std::string& task_name = pathway.task_set.GetName(local_task_id);
      task_info.emplace_back();
      task_info.back().pathway = pathway_id;
      task_info.back().local_id = local_task_id;
      task_info.back().value = pathway_info[task_name]["value"];
      task_info.back().repeatable = (pathway_info[task_name].contains("repeatable")) ? (bool)((int)pathway_info[task_name]["repeatable"]) : false;
      pathway.global_task_id_lookup[local_task_id] = global_task_id;
    }
    pathway.env_bank->GenerateBank(this_t::ENV_BANK_SIZE, config.AVIDAGP_UNIQUE_ENV_OUTPUT());
    total_tasks += num_tasks;
  }

  // org_task_scores.resize(config.POP_SIZE(), emp::vector<double>(total_tasks, 0.0));
  org_aggregate_scores.resize(config.POP_SIZE(), 0.0);
  population_task_coverage.resize(total_tasks, false);

  #ifndef EMP_NDEBUG
  // tasks per pathway
  for (size_t i = 0; i < num_pathways; ++i) {
    auto& pathway = task_pathways[i];
    std::cout << "== PATHWAY " << i << " INFO ==" << std::endl;
    std::cout << "Task Order: " << pathway_task_order[i] << std::endl;
    std::cout << "Global task ids: " << pathway.global_task_id_lookup << std::endl;
    std::cout << "Task Info: " << std::endl;
    for (const auto& pair : pathway_task_info[i]) {
      const size_t id = pathway.task_set.GetID(pair.first);
      std::cout << "  " << pair.first << ": " << pair.second;
      std::cout << ";  local id: " << id << "; global id: " << pathway.global_task_id_lookup[id];
      std::cout << "; repeatable: " << task_info[pathway.global_task_id_lookup[id]].repeatable;
      std::cout << std::endl;
    }
  }
  #endif // end EMP_NDEBUG
}

void AvidaGPEvoCompWorld::SetupSelection() {

  // Configure world's fitness function (based on aggregate scores)
  SetFitFun(
    [this](org_t& org) {
      // This is a little awkward because I don't want to modify AvidaGPOrganism on this class's behalf.
      const size_t org_id = org.GetHardware().GetWorldID();
      return org_aggregate_scores[org_id];
    }
  );

  // Configure fitness function set (for multi-objective selection schemes)
  fit_fun_set.clear();
  for (size_t fun_id = 0; fun_id < total_tasks; ++fun_id) {
    fit_fun_set.emplace_back(
      [fun_id, this](const org_t& org) {
        emp_assert(fun_id < org.GetPhenotype().org_task_performances.size());
        const double score = org.GetPhenotype().org_task_performances[fun_id];
        return score;
      }
    );
  }

  if (config.SELECTION_METHOD() == "elite") {
    SetupEliteSelection();
  } else if (config.SELECTION_METHOD() == "tournament") {
    SetupTournamentSelection();
  } else if (config.SELECTION_METHOD() == "lexicase") {
    SetupLexicaseSelection();
  } else if (config.SELECTION_METHOD() == "non-dominated-elite") {
    SetupNonDominatedEliteSelection();
  } else if (config.SELECTION_METHOD() == "non-dominated-tournament") {
    SetupNonDominatedTournamentSelection();
  } else if (config.SELECTION_METHOD() == "random") {
    SetupRandomSelection();
  } else if (config.SELECTION_METHOD() == "none") {
    SetupNoSelection();
  } else {
    // code should never reach this else (unless I forget to add a selection scheme here that is in the valid selection method set)
    emp_assert(false, "Unimplemented selection scheme.", config.SELECTION_METHOD());
  }

}

void AvidaGPEvoCompWorld::SetupEliteSelection() {
  do_selection_sig.AddAction(
    [this]() {
      dirdevo::EliteSelect(*this, config.ELITE_SEL_NUM_ELITES(), config.POP_SIZE());
    }
  );
}

void AvidaGPEvoCompWorld::SetupTournamentSelection() {
  do_selection_sig.AddAction(
    [this]() {
      emp::TournamentSelect(*this, config.TOURNAMENT_SEL_TOURN_SIZE(), config.POP_SIZE());
    }
  );
}

void AvidaGPEvoCompWorld::SetupLexicaseSelection() {
  do_selection_sig.AddAction(
    [this]() {
      emp::LexicaseSelect(*this, fit_fun_set, config.POP_SIZE());
    }
  );
}

void AvidaGPEvoCompWorld::SetupNonDominatedEliteSelection() {
  do_selection_sig.AddAction(
    [this]() {
      NonDominatedEliteSelect(*this, fit_fun_set, config.POP_SIZE());
    }
  );
}

void AvidaGPEvoCompWorld::SetupNonDominatedTournamentSelection() {
  // todo
  emp_assert(false, "NDT not implemented!");
}

void AvidaGPEvoCompWorld::SetupRandomSelection() {
  do_selection_sig.AddAction(
    [this]() {
      emp::RandomSelect(*this, config.POP_SIZE());
    }
  );
}

void AvidaGPEvoCompWorld::SetupNoSelection() {
  do_selection_sig.AddAction(
    [this]() {
      NoSelect(*this);
    }
  );
}

void AvidaGPEvoCompWorld::SetupInstLib() {
  ///////////////////////////////////////////////////////////////////////////////////
  // Add default instructions
  // - Default instructions not used: Input (replaced), Output (replaced)
  inst_lib.AddInst("Inc", inst_lib_t::Inst_Inc, 1, "Increment value in reg Arg1");
  inst_lib.AddInst("Dec", inst_lib_t::Inst_Dec, 1, "Decrement value in reg Arg1");
  inst_lib.AddInst("Not", inst_lib_t::Inst_Not, 1, "Logically toggle value in reg Arg1");
  inst_lib.AddInst("SetReg", inst_lib_t::Inst_SetReg, 2, "Set reg Arg1 to numerical value Arg2");
  inst_lib.AddInst("Add", inst_lib_t::Inst_Add, 3, "regs: Arg3 = Arg1 + Arg2");
  inst_lib.AddInst("Sub", inst_lib_t::Inst_Sub, 3, "regs: Arg3 = Arg1 - Arg2");
  inst_lib.AddInst("Mult", inst_lib_t::Inst_Mult, 3, "regs: Arg3 = Arg1 * Arg2");
  inst_lib.AddInst("Div", inst_lib_t::Inst_Div, 3, "regs: Arg3 = Arg1 / Arg2");
  inst_lib.AddInst("Mod", inst_lib_t::Inst_Mod, 3, "regs: Arg3 = Arg1 % Arg2");
  inst_lib.AddInst("TestEqu", inst_lib_t::Inst_TestEqu, 3, "regs: Arg3 = (Arg1 == Arg2)");
  inst_lib.AddInst("TestNEqu", inst_lib_t::Inst_TestNEqu, 3, "regs: Arg3 = (Arg1 != Arg2)");
  inst_lib.AddInst("TestLess", inst_lib_t::Inst_TestLess, 3, "regs: Arg3 = (Arg1 < Arg2)");
  inst_lib.AddInst("If", inst_lib_t::Inst_If, 2, "If reg Arg1 != 0, scope -> Arg2; else skip scope", emp::ScopeType::BASIC, 1);
  inst_lib.AddInst("While", inst_lib_t::Inst_While, 2, "Until reg Arg1 != 0, repeat scope Arg2; else skip", emp::ScopeType::LOOP, 1);
  inst_lib.AddInst("Countdown", inst_lib_t::Inst_Countdown, 2, "Countdown reg Arg1 to zero; scope to Arg2", emp::ScopeType::LOOP, 1);
  inst_lib.AddInst("Break", inst_lib_t::Inst_Break, 1, "Break out of scope Arg1");
  inst_lib.AddInst("Scope", inst_lib_t::Inst_Scope, 1, "Enter scope Arg1", emp::ScopeType::BASIC, 0);
  inst_lib.AddInst("Define", inst_lib_t::Inst_Define, 2, "Build function Arg1 in scope Arg2", emp::ScopeType::FUNCTION, 1);
  inst_lib.AddInst("Call", inst_lib_t::Inst_Call, 1, "Call previously defined function Arg1");
  inst_lib.AddInst("Push", inst_lib_t::Inst_Push, 2, "Push reg Arg1 onto stack Arg2");
  inst_lib.AddInst("Pop", inst_lib_t::Inst_Pop, 2, "Pop stack Arg1 into reg Arg2");
  inst_lib.AddInst("CopyVal", inst_lib_t::Inst_CopyVal, 2, "Copy reg Arg1 into reg Arg2");
  inst_lib.AddInst("ScopeReg", inst_lib_t::Inst_ScopeReg, 1, "Backup reg Arg1; restore at end of scope");

  for (size_t i = 0; i < hardware_t::CPU_SIZE; i++) {
    inst_lib.AddArg(emp::to_string((int)i), i);                   // Args can be called by value
    inst_lib.AddArg(emp::to_string("Reg", 'A'+(char)i), i);  // ...or as a register.
  }
  ///////////////////////////////////////////////////////////////////////////////////

 // Add instruction: Nop
  inst_lib.AddInst(
    "Nop",
    [](hardware_t& hw, const hardware_t::inst_t& inst) {
      return;
    },
    0,
    "No operation"
  );

  // Add instruction: GetLen
  inst_lib.AddInst(
    "GetLen",
    [](hardware_t& hw, const hardware_t::inst_t& inst) {
      hw.regs[inst.args[0]] = hw.GetSize();
    },
    1,
    "REG[ARG0]=ProgramSize"
  );

  // Add nand instruction
  inst_lib.AddInst(
    "Nand",
    [](hardware_t& hw, const hardware_t::inst_t& inst) {
      hw.regs[inst.args[2]] = ~((uint32_t)hw.regs[inst.args[0]]&(uint32_t)hw.regs[inst.args[1]]);
    },
    3,
    "REG[ARG3]=~(REG[ARG1]&REG[ARG2])"
  );

  // Add IO channel for each pathway
  for (size_t pathway_id = 0; pathway_id < task_pathways.size(); ++pathway_id) {
    // Input
    inst_lib.AddInst(
      emp::to_string("Input-", pathway_id),
      [pathway_id](hardware_t& hw, const hardware_t::inst_t& inst) {
        const auto& input_buffer = hw.GetInputBuffer(pathway_id);
        emp_assert(input_buffer.size(), "Input buffer should contain at least one element", pathway_id, input_buffer.size());
        const size_t input_ptr = hw.AdvanceInputPointer(pathway_id);    // Returns the current input pointer value and then advances the pointer.
        emp_assert(input_ptr < input_buffer.size(), input_ptr, input_buffer.size());
        const auto input_val = input_buffer[input_ptr];
        hw.regs[inst.args[0]] = input_val;
      },
      1,
      "REG[ARG0]=NextInput"
    );

    // Output
    inst_lib.AddInst(
      emp::to_string("Output-", pathway_id),
      [pathway_id](hardware_t& hw, const hardware_t::inst_t& inst) {
        hw.GetOutputBuffer(pathway_id).emplace_back(hw.regs[inst.args[0]]);
      },
      1,
      "Push REG[ARG0] to output buffer"
    );
  }

}

void AvidaGPEvoCompWorld::SetupMutator() {
  mutator_t::Configure(mutator, config);
  SetMutFun(
    [this](org_t& org, emp::Random& rnd) {
      const size_t mut_cnt = mutator.Mutate(org.GetGenome(), *random_ptr);
      return mut_cnt;
    }
  );
}

void AvidaGPEvoCompWorld::SetupDataCollection() {
  // create output directory if it doesn't exist yet
  output_dir = config.OUTPUT_DIR();
  mkdir(output_dir.c_str(), ACCESSPERMS);
  if(output_dir.back() != '/') {
    output_dir += '/';
  }

  // Wire config snapshot to end of setup signal.
  end_setup_sig.AddAction(
    [this]() {
      DoConfigSnapshot();
    }
  );

  // Collect population profiles, collect population-level coverage
  // Setup fitness file
  SetupFitnessFile(output_dir+"fitness.csv").SetTimingRepeat(config.OUTPUT_RESOLUTION());

  // Setup population snapshot file
  population_snapshotter.file = emp::NewPtr<emp::DataFile>(output_dir+"population_snapshot.csv");
  population_snapshotter.file->AddFun<size_t>(
    [this]() { return GetUpdate(); },
    "update"
  );
  // -- organism id --
  population_snapshotter.file->AddFun<size_t>(
    [this]() { return population_snapshotter.cur_org_id; },
    "org_id"
  );
  // -- aggregate fitness --
  population_snapshotter.file->AddFun<double>(
    [this]() { return CalcFitnessID(population_snapshotter.cur_org_id); },
    "aggregate_score"
  );
  population_snapshotter.file->AddFun<std::string>(
    [this]() {
      std::ostringstream stream;
      const auto& phen = GetOrg(population_snapshotter.cur_org_id).GetPhenotype();
      stream << "\"[";
      for (size_t i = 0; i < phen.org_task_performances.size(); ++i) {
        if (i) stream << ",";
        stream << phen.org_task_performances[i];
      }
      stream << "]\"";
      return stream.str();
    },
    "scores_by_task"
  );
  population_snapshotter.file->PrintHeaderKeys();

  // Setup world summary file
  world_summary_file = emp::NewPtr<emp::DataFile>(output_dir+"world_summary.csv");
  // -- update --
  world_summary_file->AddFun<size_t>(
    [this]() { return GetUpdate(); },
    "update"
  );
  // -- organism id --
  world_summary_file->AddFun<size_t>(
    [this]() { return max_fit_org_id; },
    "max_fit_org_id"
  );
  // -- is solution? --
  world_summary_file->AddFun<bool>(
    [this]() { return found_solution; },
    "max_fit_is_solution"
  );
  // -- aggregate fitness --
  world_summary_file->AddFun<double>(
    [this]() { return CalcFitnessID(max_fit_org_id); },
    "max_fit_aggregate_score"
  );
  // -- scores by task --
  world_summary_file->AddFun<std::string>(
    [this]() {
      std::ostringstream stream;
      const auto& phen = GetOrg(max_fit_org_id).GetPhenotype();
      stream << "\"[";
      for (size_t i = 0; i < phen.org_task_performances.size(); ++i) {
        if (i) stream << ",";
        stream << phen.org_task_performances[i];
      }
      stream << "]\"";
      return stream.str();
    },
    "max_fit_scores_by_task"
  );
  // -- genome length --
  world_summary_file->AddFun<size_t>(
    [this]() {
      return GetOrg(max_fit_org_id).GetHardware().GetSize();
    },
    "max_fit_genome_size"
  );
  // -- population-level task coverage --
  world_summary_file->AddFun<std::string>(
    [this]() {
      std::ostringstream stream;
      stream << "\"[";
      for (size_t i = 0; i < population_task_coverage.size(); ++i) {
        if (i) stream << ",";
        stream << population_task_coverage[i];
      }
      stream << "]\"";
      return stream.str();
    },
    "population_task_coverage"
  );
  world_summary_file->AddFun<size_t>(
    [this]() {
      size_t num_covered=0;
      for (bool cov : population_task_coverage) num_covered += (size_t)cov;
      return num_covered;
    },
    "population_num_tasks_covered"
  );

  // -- total tasks evaluated --
  world_summary_file->AddFun<size_t>(
    [this]() { return GetOrg(max_fit_org_id).GetPhenotype().org_task_performances.size(); },
    "num_tasks"
  );
  world_summary_file->PrintHeaderKeys();

}

void AvidaGPEvoCompWorld::InitPop() {
  if (config.LOAD_ANCESTOR_FROM_FILE()) {
    hardware_t hw(inst_lib); // Use this dummy hardware because of they wonky way AvidaGP is implemented.
    hw.Load(config.ANCESTOR_FILE());
    Inject(hw.GetGenome(), config.POP_SIZE());
  } else {
    // TODO - generate random
    emp_assert(false, "Generating ancestor randomly not implemented!");
    std::exit(EXIT_FAILURE);
  }
}

void AvidaGPEvoCompWorld::DoEvaluation() {

  #ifdef DIRDEVO_THREADING
  // Thread evaluation!
  emp::vector<std::thread> threads;
  for (size_t thread_id = 1; thread_id < config.NUM_THREADS(); ++thread_id) {
    threads.emplace_back(
      eval_thread,
      thread_id
    );
  }
  eval_thread(0); // use the main thread to run world 0
  // Join threads
  for (auto& thread : threads) {
    thread.join();
  }

  #else
  // THREADING DISABLED

  for (size_t org_id = 0; org_id < GetSize(); ++org_id) {
    emp_assert(IsOccupied(org_id));
    RunOrg(org_id);
  }

  #endif // DIRDEVO_THREADING


  // Analyze each organism's output buffers! (do this here to make it super easy to thread program evaluation)
  std::fill(
    population_task_coverage.begin(),
    population_task_coverage.end(),
    false
  );
  max_fit_org_id = 0;
  for (size_t org_id = 0; org_id < GetSize(); ++org_id) {
    const size_t num_pathways = task_pathways.size();
    auto& org = GetOrg(org_id);
    for (size_t pathway_id = 0; pathway_id < num_pathways; ++pathway_id) {
      auto& output_buffer = org.GetHardware().GetOutputBuffer(pathway_id);
      auto& pathway = task_pathways[pathway_id];
      const auto& env = pathway.env_bank->GetEnvironment(org.GetHardware().GetEnvID(pathway_id));
      for (auto value : output_buffer) {
        // Is this value the correct output to any tasks?
        if (emp::Has(env.valid_outputs, value)) {
          emp_assert(env.task_lookup.find(value)->second.size() == 1, "Environment should guarantee unique output for each operation");
          const size_t local_task_id = env.task_lookup.find(value)->second[0];
          const size_t global_task_id = pathway.global_task_id_lookup[local_task_id];
          // IF REPEATABLE: Increase world level task performance no matter what.
          // IF NOT REPEATABLE: If this is the first time an organism is performing this task, increase population-level task performance counter.
          //                    I.e., limit each organism to one contribution per task.
          if (task_info[global_task_id].repeatable) {
            org.GetPhenotype().org_task_performances[global_task_id] += 1;
          } else if (!org.GetPhenotype().org_task_performances[global_task_id]) {
            org.GetPhenotype().org_task_performances[global_task_id] += 1;
          }
        }
      }
      output_buffer.clear(); // Clear the output buffer after processing
    }
    org_aggregate_scores[org_id] = emp::Sum(GetOrg(org_id).GetPhenotype().org_task_performances);
    if (CalcFitnessID(org_id) > CalcFitnessID(max_fit_org_id)) {
      max_fit_org_id = org_id;
    }
  }

  for (size_t org_id=0; org_id < GetSize(); ++org_id) {
    auto& org_task_performances = GetOrg(org_id).GetPhenotype().org_task_performances;
    size_t coverage = 0;
    for (size_t task_i=0; task_i < org_task_performances.size(); ++task_i) {
      population_task_coverage[task_i] = population_task_coverage[task_i] || (org_task_performances[task_i] > 0);
      coverage += (int)(org_task_performances[task_i] > 0);
    }
    if (coverage == total_tasks) {
      found_solution = true;
      max_fit_org_id = org_id;
      break;
    }
  }
}

void AvidaGPEvoCompWorld::DoSelection() {
  do_selection_sig.Trigger();
}

void AvidaGPEvoCompWorld::DoUpdate() {
  const double max_score = CalcFitnessID(max_fit_org_id);
  const size_t cur_update = GetUpdate();

  std::cout << "update: " << cur_update << "; ";
  std::cout << "best score (" << max_fit_org_id << "): " << max_score << "; ";
  std::cout << "solution? " << found_solution << std::endl;

  const bool output = (config.OUTPUT_RESOLUTION() > 0) && ( !(cur_update % config.OUTPUT_RESOLUTION()) || (cur_update == config.GENS()) || (config.STOP_ON_SOLUTION() & found_solution) );
  if (output) {
    world_summary_file->Update();
  }

  const bool snapshot = (config.SNAPSHOT_RESOLUTION() > 0) && ( !(cur_update % config.SNAPSHOT_RESOLUTION()) || (cur_update == config.GENS()) || (config.STOP_ON_SOLUTION() & found_solution) );
  if (snapshot) {
    DoPopSnapshot();
  }

  Update();
  ClearCache();
}

void AvidaGPEvoCompWorld::DoConfigSnapshot() {
  emp::DataFile snapshot_file(output_dir + "/run_config.csv");
  std::function<std::string(void)> get_param;
  std::function<std::string(void)> get_value;
  std::ostringstream stream;
  snapshot_file.AddFun<std::string>(
    [&get_param]() { return get_param(); },
    "parameter"
  );
  snapshot_file.AddFun<std::string>(
    [&get_value]() { return get_value(); },
    "value"
  );
  snapshot_file.PrintHeaderKeys();

  // -- Custom configuration information --
  std::string param_name;
  std::string param_value;
  get_param = [&param_name]() { return param_name; };
  get_value = [&param_value]() { return param_value; };

  param_name = "threading_enabled";
  #ifdef DIRDEVO_THREADING
    param_value = "1";
  #else
    param_value = "0";
  #endif // DIRDEVO_THREADING
  snapshot_file.Update();

  // -- Num pathways --
  param_name = "num_pathways";
  param_value = emp::to_string(task_pathways.size());
  snapshot_file.Update();

  // -- Total tasks --
  param_name = "total_tasks";
  param_value = emp::to_string(total_tasks);
  snapshot_file.Update();

  // -- Task set size by pathway --
  stream.str("");
  stream << "\"[";
  for (size_t i = 0; i < task_pathways.size(); ++i) {
    if (i) stream << ",";
    stream << task_pathways[i].task_set.GetSize();
  }
  stream << "]\"";
  param_name = "task_set_sizes";
  param_value = stream.str();
  snapshot_file.Update();

  // -- Environment bank size by pathway --
  stream.str("");
  stream << "\"[";
  for (size_t i = 0; i < task_pathways.size(); ++i) {
    if (i) stream << ",";
    stream << task_pathways[i].env_bank->GetSize();
  }
  stream << "]\"";
  param_name = "env_bank_sizes";
  param_value = stream.str();
  snapshot_file.Update();

  // -- Instruction set size --
  param_name =  "inst_set_size";
  param_value = emp::to_string(inst_lib.GetSize());
  snapshot_file.Update();

  // -- Individual tasks --
  stream.str("");
  stream << "\"[";
  for (size_t i = 0; i < task_info.size(); ++i) {
    if (i) stream << ",";
    const auto& info = task_info[i];
    const auto& pathway = task_pathways[info.pathway];
    stream << "(" << pathway.task_set.GetName(info.local_id) << "," << info.pathway << ")";
  }
  stream << "]\"";
  param_name = "indiv_tasks";
  param_value = stream.str();
  snapshot_file.Update();

  // -- Configuration file information --
  for (const auto & entry : config) {
    get_param = [&entry]() { return entry.first; };
    get_value = [&entry]() { return emp::to_string(entry.second->GetValue()); };
    snapshot_file.Update();
  }

}

void AvidaGPEvoCompWorld::DoPopSnapshot() {
  for (size_t org_id = 0; org_id < GetSize(); ++org_id) {
    emp_assert(IsOccupied(org_id));
    population_snapshotter.cur_org_id = org_id;
    population_snapshotter.file->Update();
  }
}

void AvidaGPEvoCompWorld::RunOrg(size_t org_id) {
  emp_assert(IsOccupied(org_id));
  // Phenotype should be reset from inject/offspring ready signal
  auto& org = GetOrg(org_id);
  // Run organism for N cpu cycles
  for (size_t step = 0; step < config.EVAL_STEPS(); ++step) {
    org.ProcessStep(*this);
  }
}

void AvidaGPEvoCompWorld::RunStep() {
  DoEvaluation();
  DoSelection();
  DoUpdate();
}

void AvidaGPEvoCompWorld::Run() {
  for (size_t u = 0; u <= config.GENS(); ++u) {
    RunStep();
    if (config.STOP_ON_SOLUTION() & found_solution) break;
  }
}

}


#endif // AVIDAGP_EC_WORLD_HPP_INCLUDE