#pragma once
#ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE


#include "emp/hardware/AvidaCPU_InstLib.hpp"
#include "emp/tools/string_utils.hpp"

#include "../BaseTask.hpp"
#include "../DirectedDevoWorld.hpp"
#include "AvidaGPOrganism.hpp"
#include "AvidaGPReplicator.hpp"
#include "L9TaskSet.hpp"
#include "L9EnvironmentBank.hpp"

namespace dirdevo {

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

  void SetupInstLib();

public:
  AvidaGPL9Task(world_t& w) :
    base_t(w),
    org_task_set(),
    env_bank(world.GetRandom(), org_task_set, this_t::ENV_BANK_SIZE)
  { ; }

  inst_lib_t& GetInstLib() { return inst_lib; }
  const inst_lib_t& GetInstLib() const { return inst_lib; }

  // --- WORLD-LEVEL EVENT HOOKS ---

  /// OnWorldSetup called at end of constructor/world setup
  void OnWorldSetup() override {
    // TODO - configure task based on world's configuration

    // Wire up the aggregate task performance function
    aggregate_performance_fun = [this]() { return 0; };

    // Wire up the performance function set (used for multi-objective/-task selection schemes)
    // TODO - fill this out!
    for (size_t i = 0; i < 10; ++i) { // <-- this is a placeholder just to get things to compile
      performance_fun_set.emplace_back(
        [i, this]() {
          // importantly, i is copy-captured
          return 0;
        }
      );
    }

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

  }

  /// Evaluate the world on this task (count ones).
  void Evaluate() override {



    fresh_eval=true; // mark task evaluation
  }

  // --- ORGANISM-LEVEL EVENT HOOKS ---
  // These are always called AFTER the organism's equivalent functions.

  /// Called when parent is about to reproduce, but before an offspring has been constructed.
  void OnBeforeOrgRepro(org_t & parent) override { /*todo*/ }

  /// Called when the offspring has been constructed but has not been placed yet.
  void OnOffspringReady(org_t& offspring, org_t& parent) override {
    // Reset parent and offspring phenotypes
    offspring.GetPhenotype().Reset(org_task_set.GetSize());
    parent.GetPhenotype().Reset(org_task_set.GetSize());
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
    // TODO - process organism output?
    /*todo*/
  }

  /// Called before organism is removed from the world.
  void OnOrgDeath(org_t& org, size_t position) override { /*todo*/ }

  /// Called after two organisms are swapped in the world (new world positions are accurate).
  void AfterOrgSwap(org_t& org1, org_t& org2) override { /*todo*/ }

};

void AvidaGPL9Task::SetupInstLib() {
  std::cout << "Setting up instruction library." << std::endl;

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

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE