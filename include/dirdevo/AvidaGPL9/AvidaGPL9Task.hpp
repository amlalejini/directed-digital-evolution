#pragma once
#ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE

#include "../BaseTask.hpp"
#include "AvidaGPOrganism.hpp"
#include "AvidaGPReplicator.hpp"
#include "L9TaskSet.hpp"
#include "L9EnvironmentBank.hpp"

namespace dirdevo {

/// todo ORG_T should really be OneMaxOrganism, but that requires extra fiddling with more template parameters to declare a one max task
/**
 * OneMaxTask is simple task that is used only to test all of the pieces of this digital directed evolution framework.
 * Aggregate performance = average num_ones
 * Multiple criteria = average genome value for each site (works because fixed length genomes)
 */
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
    org.GetHardware().SetEnvID(world.GetRandom().GetUInt(org_task_set.GetSize()));
  }

  /// Called just before the organism's process step function is called.
  void BeforeOrgProcessStep(org_t& org) override { /*todo*/ }

  /// Called just after the organism's process step function is called.
  void AfterOrgProcessStep(org_t& org) override { /*todo*/ }

  /// Called before organism is removed from the world.
  void OnOrgDeath(org_t& org, size_t position) override { /*todo*/ }

  /// Called after two organisms are swapped in the world (new world positions are accurate).
  void AfterOrgSwap(org_t& org1, org_t& org2) override { /*todo*/ }

};

void AvidaGPL9Task::SetupInstLib() {
  std::cout << "Setting up instruction library." << std::endl;

  inst_lib = inst_lib_t::DefaultInstLib();

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

  // TODO - input
  // inst_lib.AddInst(

  // );


  // TODO - output
  // inst_lib.AddInst(

  // );
}

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE