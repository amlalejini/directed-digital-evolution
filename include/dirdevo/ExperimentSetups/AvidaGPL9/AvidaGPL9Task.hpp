#pragma once
#ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE

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

/// TODO - allow merit to be proportional to genome size (if we allow for variable-length genomes)
/// TODO - move events into proper place once things are more settled (task vs organism vs world)
class AvidaGPL9Task : public BaseTask<AvidaGPL9Task, AvidaGPOrganism> {

public:

  using org_t = AvidaGPOrganism;
  using this_t = AvidaGPL9Task;
  using base_t = BaseTask<this_t,org_t>;
  using world_t = DirectedDevoWorld<org_t, this_t>;

  using hardware_t = AvidaGPReplicator;
  using inst_lib_t = typename hardware_t::inst_lib_t;
  using org_task_set_t = L9TaskSet;

  static constexpr size_t ENV_BANK_SIZE = 100000;

  /// Attaches data file functions to summary file. Updated at configured world update interval.
  static void AttachWorldUpdateDataFileFunctions(
    WorldAwareDataFile<world_t>& summary_file
  ) {
    // Output task performance profile
    summary_file.AddFun<std::string>(
      [&summary_file]() {
        const this_t& task = summary_file.GetCurWorld().GetTask();
        std::ostringstream stream;
        stream << "\"{";
        for (size_t i = 0; i < task.org_task_set.GetSize(); ++i) {
          if (i) stream << ",";
          stream << task.org_task_set.GetName(i) << ":" << task.l9_world_task_performance[i];
        }
        stream << "}\"";
        return stream.str();
      },
      "task_performance"
    );
    // Average generation
    summary_file.AddFun<double>(
      [&summary_file]() {
        double total_generation=0;
        size_t num_orgs=0;
        world_t& world = summary_file.GetCurWorld();
        for (size_t pop_id = 0; pop_id < world.GetSize(); ++pop_id) {
          if (!world.IsOccupied({pop_id,0})) continue;
          num_orgs += 1;
          total_generation += world.GetOrg(pop_id).GetGeneration();
        }
        return (num_orgs > 0) ? total_generation / (double)num_orgs : 0;
      },
      "avg_generation"
    );

  }

protected:

  using base_t::aggregate_performance_fun;
  using base_t::performance_fun_set;
  using base_t::fresh_eval;
  using base_t::world;

  // Shared instruction set
  inst_lib_t inst_lib;

  // Environment/logic task information
  org_task_set_t org_task_set;
  L9EnvironmentBank env_bank;

  emp::vector<double> l9_indiv_task_values;    ///< Configured value of each logic function (for individuals).
  emp::vector<double> l9_world_task_values;    ///< Configured value of each logic function (for a world)
  emp::vector<size_t> l9_indiv_task_ids;       ///< Which logic functions (as task ids) confer bonuses for individual reproduction?
  emp::vector<size_t> l9_world_task_ids;       ///< Which logic functions (as task ids) confer bonuses for world selection?

  emp::vector<size_t> l9_world_task_performance; ///< Counts of how many times each logic task has been performed in the world. Used to calculate world scores.

  emp::vector<double> world_scores; ///< Set during evaluation. Score for each world objective (corresponds to l9_world_task_ids)
  double world_agg_score=0;         ///< Set during evaluation. World's aggregate score (sum of objective scores).

  std::function<double(const org_t&)> calc_merit_fun;

  void SetupInstLib();
  void SetupTaskValues();
  void SetupMeritCalcFun();
  void SetupWorldTaskPerformanceFun();

public:
  AvidaGPL9Task(world_t& w) :
    base_t(w),
    org_task_set(),
    env_bank(world.GetRandom(), org_task_set),
    l9_indiv_task_values(),
    l9_world_task_values(),
    l9_indiv_task_ids(),
    l9_world_task_ids(),
    l9_world_task_performance()
  { ; }

  inst_lib_t& GetInstLib() { return inst_lib; }
  const inst_lib_t& GetInstLib() const { return inst_lib; }

  // --- WORLD-LEVEL EVENT HOOKS ---

  emp::vector<ConfigSnapshotEntry> GetConfigSnapshotEntries() override {
     emp::vector<ConfigSnapshotEntry> entries;
     const std::string source("world__" + world.GetName() + "__task");
     entries.emplace_back(
       "org_task_set_size",
       emp::to_string(org_task_set.GetSize()),
       source
     );
     entries.emplace_back(
       "env_bank_size",
       emp::to_string(env_bank.GetSize()),
       source
     );
     entries.emplace_back(
       "inst_set_size",
       emp::to_string(inst_lib.GetSize()),
       source
     );
    // Individual tasks
    std::ostringstream stream;
    stream << "\"[";
    for (size_t i = 0; i < l9_indiv_task_ids.size(); ++i) {
      if (i) stream << ",";
      stream << org_task_set.GetName(l9_indiv_task_ids[i]);
    }
    stream << "]\"";
    entries.emplace_back(
      "indiv_tasks",
      stream.str(),
      source
    );

    // World-level tasks
    stream.str("");
    stream << "\"[";
    for (size_t i = 0; i < l9_world_task_ids.size(); ++i) {
      if (i) stream << ",";
      stream << org_task_set.GetName(l9_world_task_ids[i]);
    }
    stream << "]\"";
    entries.emplace_back(
      "world_tasks",
      stream.str(),
      source
    );
    return entries;
  }

  /// OnWorldSetup called at end of constructor/world setup
  void OnWorldSetup() override {
    // TODO - configure task based on world's configuration
    // Configure individual and world logic tasks.
    SetupTaskValues();
    // Configure merit calculation
    SetupMeritCalcFun();
    // Wire up the aggregate task performance function
    SetupWorldTaskPerformanceFun();

    fresh_eval=false;
    SetupInstLib();
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
    std::fill(
      l9_world_task_performance.begin(),
      l9_world_task_performance.end(),
      0
    );

    std::fill(
      world_scores.begin(),
      world_scores.end(),
      0.0
    );

    world_agg_score=0;
  }

  /// Evaluate the world on this task.
  void Evaluate() override {

    #ifndef EMP_NDEBUG
    std::cout << world.GetName() << " tasks:";
    for (size_t i =0; i < org_task_set.GetSize(); ++i) {
      std::cout << " " << org_task_set.GetName(i) << ":" << l9_world_task_performance[i];
    }
    std::cout << std::endl;
    #endif

    emp_assert(world_scores.size() == l9_world_task_ids.size());
    emp_assert(l9_world_task_performance.size() == l9_world_task_values.size());
    for (size_t i = 0; i < l9_world_task_ids.size(); ++i) {
      const size_t task_id = l9_world_task_ids[i];
      world_scores[i] = l9_world_task_performance[task_id] * l9_world_task_values[task_id];
    }
    world_agg_score = emp::Sum(world_scores);

    fresh_eval=true; // mark task evaluation
  }

  // --- ORGANISM-LEVEL EVENT HOOKS ---
  // These are always called AFTER the organism's equivalent functions.
  void OnOrgInjectReady(org_t& org) override {
    // anything that happens OnOffspringReady might also need to happen here (injected organisms are never offspring)
    org.GetPhenotype().Reset(org_task_set.GetSize());
    org.SetMerit(1.0); // Injected organisms have merit set to 1
  }

  /// Called when parent is about to reproduce, but before an offspring has been constructed.
  void OnBeforeOrgRepro(org_t & parent) override { /*todo*/ }

  /// Called when the offspring has been constructed but has not been placed yet.
  void OnOffspringReady(org_t& offspring, org_t& parent) override {
    // Calculate merit based on parent's phenotype.
    const double merit = calc_merit_fun(parent);

    // Reset parent and offspring phenotypes
    offspring.GetPhenotype().Reset(org_task_set.GetSize());
    parent.GetPhenotype().Reset(org_task_set.GetSize());

    // Set offspring and parent's merit to be a function of the parent's phenotype
    offspring.SetMerit(merit);
    parent.SetMerit(merit);

    // Parent gets reset, but doesn't get re-placed. Need to give it a new environment and reset its input buffer.
    // TODO - move this into a function
    const size_t parent_env_id = world.GetRandom().GetUInt(env_bank.GetSize());
    parent.GetHardware().SetEnvID(parent_env_id);
    parent.GetHardware().GetInputBuffer() = env_bank.GetEnvironment(parent_env_id).input_buffer;

  }

  /// Called when org is being placed (@ position) in the world
  void OnOrgPlacement(org_t& org, size_t position) override {
    // Assign organism an environment ID
    const size_t env_id = world.GetRandom().GetUInt(env_bank.GetSize());
    org.GetHardware().SetEnvID(env_id);
    // Configure organism's intput buffer
    org.GetHardware().GetInputBuffer() = env_bank.GetEnvironment(env_id).input_buffer;
    emp_assert(org.GetHardware().GetInputBuffer() == env_bank.GetEnvironment(env_id).input_buffer);
  }

  /// Called just before the organism's process step function is called.
  void BeforeOrgProcessStep(org_t& org) override { /*todo*/ }

  /// Called just after the organism's process step function is called.
  void AfterOrgProcessStep(org_t& org) override {
    // Analyze organism output buffer
    auto& output_buffer = org.GetHardware().GetOutputBuffer();
    for (auto value : output_buffer) {
      // Is this value the correct output to any of the tasks?
      const auto& env = env_bank.GetEnvironment(org.GetHardware().GetEnvID());
      if (emp::Has(env.valid_outputs, value)) {
        emp_assert(env.task_lookup.find(value)->second.size() == 1, "Environment should guarantee unique output for each logic operation");
        const size_t task_id = env.task_lookup.find(value)->second[0];
        org.GetPhenotype().org_task_performances[task_id] += 1; // hypothesis => injected organism's phenotype hasn't been reset to right size
        // TODO - allow configuration of when tasks count toward world performance
        // for now, just let each time the task is performed count for the world
        l9_world_task_performance[task_id] += 1;
      }
    }
    output_buffer.clear(); // Clear the output buffer after processing

    // Is organism still alive?
    const size_t age_limit = org.GetGenome().GetSize()*world.config.AVIDAGP_ORG_AGE_LIMIT();
    org.SetDead(org.GetAge() >= age_limit);
  }

  /// Called before organism is removed from the world.
  void OnOrgDeath(org_t& org, size_t position) override { /*todo*/ }

  /// Called after two organisms are swapped in the world (new world positions are accurate).
  void AfterOrgSwap(org_t& org1, org_t& org2) override { /*todo*/ }

};

void AvidaGPL9Task::SetupInstLib() {

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

  // Input
  inst_lib.AddInst(
    "Input",
    [](hardware_t& hw, const hardware_t::inst_t& inst) {
      emp_assert(hw.GetInputBuffer().size(), "Input buffer should contain at least one element");
      const size_t input_ptr = hw.AdvanceInputPointer();    // Returns the current input pointer value and then advances the pointer.
      emp_assert(input_ptr < hw.GetInputBuffer().size(), input_ptr, hw.GetInputBuffer().size());
      const auto input_val = hw.GetInputBuffer()[input_ptr];
      hw.regs[inst.args[0]] = input_val;
    },
    1,
    "REG[ARG0]=NextInput"
  );

  // Output
  inst_lib.AddInst(
    "Output",
    [](hardware_t& hw, const hardware_t::inst_t& inst) {
      hw.GetOutputBuffer().emplace_back(hw.regs[inst.args[0]]);
    },
    1,
    "Push REG[ARG0] to output buffer"
  );

}

void AvidaGPL9Task::SetupTaskValues() {
  // todo - add tasks to org task set
  // (1) Parse environment file
  // std::cout << world.GetConfig().AVIDAGP_ENV_FILE() << std::endl;
  // std::cout << std::filesystem::exists(world.GetConfig().AVIDAGP_ENV_FILE()) << std::endl;
  const bool env_file_exists = std::filesystem::exists(world.GetConfig().AVIDAGP_ENV_FILE());
  if (!env_file_exists) {
    std::cout << "Environment file does not exist. " << world.GetConfig().AVIDAGP_ENV_FILE() << std::endl;
    std::exit(EXIT_FAILURE);
  }
  std::ifstream env_ifstream(world.GetConfig().AVIDAGP_ENV_FILE());
  nlohmann::json env_json;
  env_ifstream >> env_json;
  emp_assert(env_json.contains("organism"), "Improperly configured environment file. Failed to find 'organism' key.");
  emp_assert(env_json.contains("world"), "Improperly configured environment file. Failed to find 'world' key.");

  // Get the set of tasks (world and )
  std::unordered_set<std::string> loaded_env_task_set; // Keep track of which tasks we should add to the org_task_set
  emp::vector<std::string> loaded_env_task_order;      // Would prefer to order tasks in the order they appeared in the environment file.
  // Start with organism tasks, then world tasks
  emp_assert(env_json["organism"].contains("tasks"));
  emp_assert(env_json["world"].contains("tasks"));
  auto& org_task_json = env_json["organism"]["tasks"];
  auto& world_task_json = env_json["world"]["tasks"];
  for (auto& task : org_task_json) {
    emp_assert(task.contains("name"));
    if (emp::Has(loaded_env_task_set, task["name"])) continue;
    loaded_env_task_set.emplace(task["name"]);
    loaded_env_task_order.emplace_back(task["name"]);
  }
  for (auto& task : world_task_json) {
    emp_assert(task.contains("name"));
    if (emp::Has(loaded_env_task_set, task["name"])) continue;
    loaded_env_task_set.emplace(task["name"]);
    loaded_env_task_order.emplace_back(task["name"]);
  }

  // std::cout << loaded_env_task_order << std::endl;
  // Build the task set
  org_task_set.AddTasksByName(loaded_env_task_order);
  emp_assert(org_task_set.GetSize() == loaded_env_task_set.size());
  // Generate the environment bank
  env_bank.GenerateBank(this_t::ENV_BANK_SIZE);
  // Setup task performance tracking vector
  l9_world_task_performance.resize(org_task_set.GetSize());
  std::fill(
    l9_world_task_performance.begin(),
    l9_world_task_performance.end(),
    0
  );

  // Assign to each task (at organism and at world level)
  // FIRST, assign values to organism-level tasks.
  l9_indiv_task_values.resize(org_task_set.GetSize(), 0.0);
  std::fill(
    l9_indiv_task_values.begin(),
    l9_indiv_task_values.end(),
    0.0
  );
  l9_indiv_task_ids.clear();
  for (auto& task : org_task_json) {
    emp_assert(task.contains("value"));
    const std::string task_name = task["name"];
    const double task_value = task["value"];
    const size_t task_id = org_task_set.GetID(task_name);
    l9_indiv_task_ids.emplace_back(task_id);
    l9_indiv_task_values[task_id] = task_value;
  }

  // Next, assign values to world-level tasks.
  l9_world_task_values.resize(org_task_set.GetSize(), 0.0);
  std::fill(
    l9_world_task_values.begin(),
    l9_world_task_values.end(),
    0.0
  );
  l9_world_task_ids.clear();
  for (auto& task : world_task_json) {
    emp_assert(task.contains("value"));
    const std::string task_name = task["name"];
    const double task_value = task["value"];
    const size_t task_id = org_task_set.GetID(task_name);
    l9_world_task_ids.emplace_back(task_id);
    l9_world_task_values[task_id] = task_value;
  }

  // Configure the world score tracking based on world task configuration
  world_scores.resize(l9_world_task_ids.size(), 0.0);
  world_agg_score=0;

}

void AvidaGPL9Task::SetupMeritCalcFun() {
  // TODO - this is where we could implement options for different merit calculations
  calc_merit_fun = [this](const org_t& org) {
    double merit = 1.0; // Base merit = 1.0
    // for each indiv task performed (>= 1), multiple base merit by 2^{indiv task value}
    for (auto task_id : l9_indiv_task_ids) {
      emp_assert(task_id < l9_indiv_task_values.size());
      // Get credit if organism performed indiv task >= 1 time (only get credit for each task once)
      if (org.GetPhenotype().org_task_performances[task_id] >= 1) {
        merit *= emp::Pow2(l9_indiv_task_values[task_id]) ;
      }
    }
    // todo - test this function
    return merit;
  };
}

void AvidaGPL9Task::SetupWorldTaskPerformanceFun() {

  aggregate_performance_fun = [this]() {
    return world_agg_score;
  };

  // Wire up the performance function set (used for multi-objective/-task selection schemes)
  // TODO - fill this out!
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

#endif // #ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE