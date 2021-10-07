#pragma once
#ifndef DIRECTED_DEVO_AVIDAGP_MULTIPATHWAY_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_AVIDAGP_MULTIPATHWAY_TASK_HPP_INCLUDE

#include <algorithm>
#include <filesystem>

#include "emp/hardware/AvidaCPU_InstLib.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/base/vector.hpp"
#include "emp/datastructs/vector_utils.hpp"

#include "json/json.hpp"

#include "../../BaseTask.hpp"
#include "../../DirectedDevoWorld.hpp"

#include "AvidaGPOrganism.hpp"
#include "AvidaGPReplicator.hpp"
#include "L9TaskSet.hpp"
#include "L9EnvironmentBank.hpp"

namespace dirdevo {

/// TODO - move events into proper place once things are more settled (task vs organism vs world)
class AvidaGPMultiPathwayTask : public BaseTask<AvidaGPMultiPathwayTask, AvidaGPOrganism> {

public:

  using org_t = AvidaGPOrganism;
  using this_t = AvidaGPMultiPathwayTask;
  using base_t = BaseTask<this_t,org_t>;
  using world_t = DirectedDevoWorld<org_t, this_t>;

  using hardware_t = AvidaGPReplicator;
  using inst_lib_t = typename hardware_t::inst_lib_t;
  using org_task_set_t = L9TaskSet;

  using env_bank_t = L9EnvironmentBank;

  static constexpr size_t ENV_BANK_SIZE = 10000;

  /// Attaches data file functions to summary file. Updated at configured world update interval.
  static void AttachWorldUpdateDataFileFunctions(
    WorldAwareDataFile<world_t>& summary_file
  ) {
    // TODO - OVERHAUL
    // Output task performance profile
    // summary_file.AddFun<std::string>(
    //   [&summary_file]() {
    //     const this_t& task = summary_file.GetCurWorld().GetTask();
    //     std::ostringstream stream;
    //     stream << "\"{";
    //     for (size_t i = 0; i < task.org_task_set.GetSize(); ++i) {
    //       if (i) stream << ",";
    //       stream << task.org_task_set.GetName(i) << ":" << task.l9_world_task_performance[i];
    //     }
    //     stream << "}\"";
    //     return stream.str();
    //   },
    //   "task_performance"
    // );
    // // Average generation
    // summary_file.AddFun<double>(
    //   [&summary_file]() {
    //     double total_generation=0;
    //     size_t num_orgs=0;
    //     world_t& world = summary_file.GetCurWorld();
    //     for (size_t pop_id = 0; pop_id < world.GetSize(); ++pop_id) {
    //       if (!world.IsOccupied({pop_id,0})) continue;
    //       num_orgs += 1;
    //       total_generation += world.GetOrg(pop_id).GetGeneration();
    //     }
    //     return (num_orgs > 0) ? total_generation / (double)num_orgs : 0;
    //   },
    //   "avg_generation"
    // );
  }

protected:

  using base_t::aggregate_performance_fun;
  using base_t::performance_fun_set;
  using base_t::fresh_eval;
  using base_t::world;

  // Shared instruction set
  inst_lib_t inst_lib;

  // Environment/logic task information
  struct MetabolicPathway {
    size_t id=0; ///< Pathway id

    emp::vector<size_t> global_task_id_lookup; ///< Lookup global-level task id given pathway-level task id

    org_task_set_t task_set;                ///< Which tasks are part of this pathway?
    emp::Ptr<env_bank_t> env_bank=nullptr;                 ///< lookup table of IO examples

    ~MetabolicPathway() {
      if (env_bank) env_bank.Delete();
    }
  };

  /// Used to track information about tasks
  struct TaskInfo {
    size_t pathway=0;     ///< Which pathway is this task a part of?
    size_t local_id=0;    ///< What is that local within-pathway id of this task?

    double org_value=0;   ///< What is the organism-level value of this task?
    double world_value=0; ///< What is the world-level value of this task?

    // TODO - here's where I can mark task by-products, dependencies, etc
  };

  size_t total_tasks=0; ///< Number of tasks across all pathways
  emp::vector<size_t> org_task_ids;
  emp::vector<size_t> world_task_ids;
  emp::vector<TaskInfo> task_info;
  emp::vector<MetabolicPathway> task_pathways;
  emp::vector<size_t> task_performance;

  emp::vector<double> world_scores; ///< Set during evaluation. Score for each world objective (corresponds to l9_world_task_ids)
  double world_agg_score=0;         ///< Set during evaluation. World's aggregate score (sum of objective scores).

  std::function<double(const org_t&)> calc_merit_fun;

  void SetupInstLib();
  void SetupTasks();
  void SetupMeritCalcFun();
  void SetupWorldTaskPerformanceFun();

public:
  AvidaGPMultiPathwayTask(world_t& w) :
    base_t(w)
  { ; }

  inst_lib_t& GetInstLib() { return inst_lib; }
  const inst_lib_t& GetInstLib() const { return inst_lib; }

  // --- WORLD-LEVEL EVENT HOOKS ---

  emp::vector<ConfigSnapshotEntry> GetConfigSnapshotEntries() override {
     emp::vector<ConfigSnapshotEntry> entries;
     const std::string source("world__" + world.GetName() + "__task");
     // TODO - FIX!
     //  entries.emplace_back(
     //    "org_task_set_size",
     //    emp::to_string(org_task_set.GetSize()),
     //    source
     //  );
     // TODO - FIX
     //  entries.emplace_back(
     //    "env_bank_size",
     //    emp::to_string(env_bank.GetSize()),
     //    source
     //  );
     entries.emplace_back(
       "inst_set_size",
       emp::to_string(inst_lib.GetSize()),
       source
     );
    // Individual tasks
    // TODO - FIX
    // std::ostringstream stream;
    // stream << "\"[";
    // for (size_t i = 0; i < l9_indiv_task_ids.size(); ++i) {
    //   if (i) stream << ",";
    //   stream << org_task_set.GetName(l9_indiv_task_ids[i]);
    // }
    // stream << "]\"";
    // entries.emplace_back(
    //   "indiv_tasks",
    //   stream.str(),
    //   source
    // );

    // // World-level tasks
    // stream.str("");
    // stream << "\"[";
    // for (size_t i = 0; i < l9_world_task_ids.size(); ++i) {
    //   if (i) stream << ",";
    //   stream << org_task_set.GetName(l9_world_task_ids[i]);
    // }
    // stream << "]\"";
    // entries.emplace_back(
    //   "world_tasks",
    //   stream.str(),
    //   source
    // );
    return entries;
  }

  /// OnWorldSetup called at end of constructor/world setup
  void OnWorldSetup() override {
    // TODO - configure task based on world's configuration
    // Configure individual and world logic tasks.
    SetupTasks();
    // Configure merit calculation
    SetupMeritCalcFun();
    // Wire up the aggregate task performance function
    SetupWorldTaskPerformanceFun();
    // Call Evaluate to refresh eval status
    fresh_eval=false;
    // Configure the instruction library
    SetupInstLib();
    exit(1);
  }

  /// OnBeforeWorldUpdate is called at the beginning of running the world update
  void OnBeforeWorldUpdate(size_t update) override {
    // as soon as the world has updated, evaluation is no longer guaranteed to be fresh
    fresh_eval=false;
  }

  /// OnWorldUpdate is called when the OnUpdate signal is triggered (at the end of a world update)
  void OnWorldUpdate(size_t update) override { /*todo*/ }

  void OnWorldReset() override {
    // Reset world task performance counts
    // TODO - Fix
    // std::fill(
    //   l9_world_task_performance.begin(),
    //   l9_world_task_performance.end(),
    //   0
    // );

    std::fill(
      world_scores.begin(),
      world_scores.end(),
      0.0
    );

    world_agg_score=0;
  }

  /// Evaluate the world on this task.
  void Evaluate() override {
    // TODO - fix

    #ifndef EMP_NDEBUG
    std::cout << world.GetName() << " tasks:";
    // for (size_t i =0; i < org_task_set.GetSize(); ++i) {
    //   std::cout << " " << org_task_set.GetName(i) << ":" << l9_world_task_performance[i];
    // }
    std::cout << std::endl;
    #endif

    // emp_assert(world_scores.size() == l9_world_task_ids.size());
    // emp_assert(l9_world_task_performance.size() == l9_world_task_values.size());
    // for (size_t i = 0; i < l9_world_task_ids.size(); ++i) {
    //   const size_t task_id = l9_world_task_ids[i];
    //   world_scores[i] = l9_world_task_performance[task_id] * l9_world_task_values[task_id];
    // }
    // world_agg_score = emp::Sum(world_scores);

    fresh_eval=true; // mark task evaluation
  }

  // --- ORGANISM-LEVEL EVENT HOOKS ---
  // These are always called AFTER the organism's equivalent functions.
  void OnOrgInjectReady(org_t& org) override {
    // TODO - fix
    // anything that happens OnOffspringReady might also need to happen here (injected organisms are never offspring)
    // org.GetPhenotype().Reset(org_task_set.GetSize());
    org.SetMerit(1.0); // Injected organisms have merit set to 1
  }

  /// Called when parent is about to reproduce, but before an offspring has been constructed.
  void OnBeforeOrgRepro(org_t & parent) override { /*todo*/ }

  /// Called when the offspring has been constructed but has not been placed yet.
  void OnOffspringReady(org_t& offspring, org_t& parent) override {
    // TODO - fix
    // // Calculate merit based on parent's phenotype.
    // const double merit = calc_merit_fun(parent);
    // emp_assert(merit > 0, merit, parent.GetMerit());

    // // Reset parent and offspring phenotypes
    // offspring.GetPhenotype().Reset(org_task_set.GetSize());
    // parent.GetPhenotype().Reset(org_task_set.GetSize());

    // // Set offspring and parent's merit to be a function of the parent's phenotype
    // offspring.SetMerit(merit);
    // parent.SetMerit(merit);

    // // Parent gets reset, but doesn't get re-placed. Need to give it a new environment and reset its input buffer.
    // // TODO - move this into a function
    // const size_t parent_env_id = world.GetRandom().GetUInt(env_bank.GetSize());
    // parent.GetHardware().SetEnvID(parent_env_id);
    // parent.GetHardware().GetInputBuffer() = env_bank.GetEnvironment(parent_env_id).input_buffer;

  }

  /// Called when org is being placed (@ position) in the world
  void OnOrgPlacement(org_t& org, size_t position) override {
    // TODO - fix
    // // Assign organism an environment ID
    // const size_t env_id = world.GetRandom().GetUInt(env_bank.GetSize());
    // org.GetHardware().SetEnvID(env_id);
    // // Configure organism's intput buffer
    // org.GetHardware().GetInputBuffer() = env_bank.GetEnvironment(env_id).input_buffer;
    // emp_assert(org.GetHardware().GetInputBuffer() == env_bank.GetEnvironment(env_id).input_buffer);
  }

  /// Called just before the organism's process step function is called.
  void BeforeOrgProcessStep(org_t& org) override { /*todo*/ }

  /// Called just after the organism's process step function is called.
  void AfterOrgProcessStep(org_t& org) override {
    // Analyze organism output buffer
    // TODO - fix
    // auto& output_buffer = org.GetHardware().GetOutputBuffer();
    // for (auto value : output_buffer) {
    //   // Is this value the correct output to any of the tasks?
    //   const auto& env = env_bank.GetEnvironment(org.GetHardware().GetEnvID());
    //   if (emp::Has(env.valid_outputs, value)) {
    //     emp_assert(env.task_lookup.find(value)->second.size() == 1, "Environment should guarantee unique output for each logic operation");
    //     const size_t task_id = env.task_lookup.find(value)->second[0];
    //     org.GetPhenotype().org_task_performances[task_id] += 1; // hypothesis => injected organism's phenotype hasn't been reset to right size
    //     // TODO - allow configuration of when tasks count toward world performance
    //     // for now, just let each time the task is performed count for the world
    //     l9_world_task_performance[task_id] += 1;
    //   }
    // }
    // output_buffer.clear(); // Clear the output buffer after processing

    // // Is organism still alive?
    // const size_t age_limit = org.GetGenome().GetSize()*world.config.AVIDAGP_ORG_AGE_LIMIT();
    // org.SetDead(org.GetAge() >= age_limit);
  }

  /// Called before organism is removed from the world.
  void OnOrgDeath(org_t& org, size_t position) override { /*todo*/ }

  /// Called after two organisms are swapped in the world (new world positions are accurate).
  void AfterOrgSwap(org_t& org1, org_t& org2) override { /*todo*/ }

};

void AvidaGPMultiPathwayTask::SetupInstLib() {

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

  // Add instruction: CopyInst
  inst_lib.AddInst(
    "CopyInst",
    [](hardware_t& hw, const hardware_t::inst_t& inst) {
      if (hw.IsDoneCopying()) return; // Don't over-copy.
      hw.IncSitesCopied();            // 'Copy' an instruction.
    },
    0,
    "Copy next instrution"
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

  // Add instruction: IsDoneCopying

  // Add divide instruction
  inst_lib.AddInst(
    "DivideSelf",
    [](hardware_t& hw, const hardware_t::inst_t& inst) {
      hw.SetDividing(hw.IsDoneCopying());
      hw.IncFailedSelfDivisions((size_t)!hw.IsDividing());
    },
    0,
    "Mark hardware unit for self-replication"
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

void AvidaGPMultiPathwayTask::SetupTasks() {

  // === Parse environment file ===
  // Check to see if environment file exists.
  const bool env_file_exists = std::filesystem::exists(world.GetConfig().AVIDAGP_ENV_FILE());
  if (!env_file_exists) {
    std::cout << "Environment file does not exist. " << world.GetConfig().AVIDAGP_ENV_FILE() << std::endl;
    std::exit(EXIT_FAILURE);
  }
  // If it does, read it.
  std::ifstream env_ifstream(world.GetConfig().AVIDAGP_ENV_FILE());
  nlohmann::json env_json;
  env_ifstream >> env_json;
  emp_assert(env_json.contains("organism"), "Improperly configured environment file. Failed to find 'organism' key.");
  emp_assert(env_json.contains("world"), "Improperly configured environment file. Failed to find 'world' key.");
  emp_assert(env_json.contains("pathways"), "Improperly configured environment file. Failed to find 'pathways' key.");

  // How many pathways are there?
  const size_t num_pathways = env_json["pathways"];
  std::cout << num_pathways << std::endl;

  // Create metabolic pathways
  task_pathways.resize(num_pathways);
  emp::vector< std::unordered_set<std::string> > pathway_task_set(num_pathways); // Keep track of which tasks have been requested for each pathway's task set.
  emp::vector< emp::vector<std::string> > pathway_task_order(num_pathways); // Keep track of the order we should add tasks

  emp::vector< std::unordered_map<std::string, nlohmann::json> > pathway_org_task_info(num_pathways);
  emp::vector< std::unordered_map<std::string, nlohmann::json> > pathway_world_task_info(num_pathways);

  // Initialize each pathway
  for (size_t pathway_id=0; pathway_id < task_pathways.size(); ++pathway_id) {
    auto& pathway = task_pathways[pathway_id];
    pathway.id = 0;
    pathway.env_bank = emp::NewPtr<env_bank_t>(world.GetRandom(), pathway.task_set);
  }

  // Configure organism-level tasks
  emp_assert(env_json["organism"].contains("tasks"));
  auto& org_task_json = env_json["organism"]["tasks"];
  for (auto& task : org_task_json) {
    emp_assert(task.contains("name"));
    const size_t task_pathway_id = (task.contains("pathway")) ? (size_t)task["pathway"] : 0;
    emp_assert(task_pathway_id < num_pathways, "Invalid task pathway id.", task_pathway_id, num_pathways);

    auto& task_set = pathway_task_set[task_pathway_id];
    auto& task_order = pathway_task_order[task_pathway_id];
    auto& task_info_map = pathway_org_task_info[task_pathway_id];

    // If this is the first time we've seen this task for this pathway, make note.
    if (!emp::Has(task_set, task["name"])) {
      task_set.emplace(task["name"]);
      task_order.emplace_back(task["name"]);
    }
    // If this is the first time we've seen this task as an organism task, make note.
    if (!emp::Has(task_info_map, task["name"])) {
      task_info_map.emplace(
        task["name"],
        task
      );
    }
  }

  // Configure world-level tasks
  emp_assert(env_json["world"].contains("tasks"));
  auto& world_task_json = env_json["world"]["tasks"];
  for (auto& task : world_task_json) {
    emp_assert(task.contains("name"));
    const size_t task_pathway_id = (task.contains("pathway")) ? (size_t)task["pathway"] : 0;
    emp_assert(task_pathway_id < num_pathways, "Invalid task pathway id.", task_pathway_id, num_pathways);

    auto& task_set = pathway_task_set[task_pathway_id];
    auto& task_order = pathway_task_order[task_pathway_id];
    auto& task_info_map = pathway_world_task_info[task_pathway_id];

    // If this is the first time we've seen this task for this pathway, make note.
    if (!emp::Has(task_set, task["name"])) {
      task_set.emplace(task["name"]);
      task_order.emplace_back(task["name"]);
    }
    // If this is the first time we've seen this task as an world task, make note.
    if (!emp::Has(task_info_map, task["name"])) {
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
    auto& org_task_info = pathway_org_task_info[pathway_id];
    auto& world_task_info = pathway_world_task_info[pathway_id];

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
      if (emp::Has(org_task_info, task_name)) {
        org_task_ids.emplace_back(global_task_id);
        task_info.back().org_value = org_task_info[task_name]["value"];
      }
      if (emp::Has(world_task_info, task_name)) {
        world_task_ids.emplace_back(global_task_id);
        task_info.back().world_value = world_task_info[task_name]["value"];
      }
      pathway.global_task_id_lookup[local_task_id] = global_task_id;
    }
    total_tasks += num_tasks;
  }

  task_performance.resize(total_tasks, 0);
  emp_assert(task_info.size() == task_performance.size());

  #ifndef EMP_NDEBUG
  // tasks per pathway
  for (size_t i = 0; i < num_pathways; ++i) {
    auto& pathway = task_pathways[i];
    std::cout << "== PATHWAY " << i << " INFO ==" << std::endl;
    std::cout << "Task Order: " << pathway_task_order[i] << std::endl;
    std::cout << "Global task ids: " << pathway.global_task_id_lookup << std::endl;
    std::cout << "Organism Task Info: " << std::endl;
    for (const auto& pair : pathway_org_task_info[i]) {
      const size_t id = pathway.task_set.GetID(pair.first);
      std::cout << "  " << pair.first << ": " << pair.second;
      std::cout << ";  local id: " << id << "; global id: " << pathway.global_task_id_lookup[id];
      std::cout << std::endl;
    }
    std::cout << "World Task Info: " << std::endl;
    for (const auto& pair : pathway_world_task_info[i]) {
      const size_t id = pathway.task_set.GetID(pair.first);
      std::cout << "  " << pair.first << ": " << pair.second;
      std::cout << ";  local id: " << id << "; global id: " << pathway.global_task_id_lookup[id];
      std::cout << std::endl;
    }
  }
  #endif // end EMP_NDEBUG

  world_scores.resize(world_task_ids.size(), 0.0);
  world_agg_score = 0;

}

void AvidaGPMultiPathwayTask::SetupMeritCalcFun() {
  // TODO - this is where we would implement options for different merit calculations
  calc_merit_fun = [this](const org_t& org) {
    double merit = 1.0; // Base merit = 1.0
    // For each organism-level task performed, multiply base merit by 2^{org_task_value}
    for (auto task_id : org_task_ids) {
      // Note that task_id is the global id for this task.
      emp_assert(task_id < task_info.size());
      emp_assert(task_id < org.GetPhenotype().org_task_performances.size());
      // Get credit if the organism performed this at least once (only get credit for each task once).
      if (org.GetPhenotype().org_task_performances[task_id] >= 1) {
        const double value = task_info[task_id].org_value;
        merit *= emp::Pow2(value);
      }
    }
    emp_assert(merit > 0, "If all organisms have 0 merit, then the scheduler will crash.");
    return merit;
  };
}

void AvidaGPMultiPathwayTask::SetupWorldTaskPerformanceFun() {

  aggregate_performance_fun = [this]() {
    return world_agg_score;
  };

  // Wire up the performance function set (used for multi-objective/-task selection schemes)
  for (size_t i = 0; i < world_scores.size(); ++i) { // <-- this is a placeholder just to get things to compile
    performance_fun_set.emplace_back(
      [i, this]() {
        emp_assert(i < world_scores.size());
        // importantly, i is copy-captured
        return world_scores[i];
      }
    );
  }
}

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_AVIDAGP_MULTIPATHWAY_TASK_HPP_INCLUDE