#pragma once
#ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE

#include "../BaseTask.hpp"
#include "AvidaGPOrganism.hpp"

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

  using hardware_t = emp::AvidaGP;
  using inst_lib_t = typename hardware_t::inst_lib_t;

protected:

  using base_t::aggregate_performance_fun;
  using base_t::performance_fun_set;
  using base_t::fresh_eval;
  using base_t::world;

  inst_lib_t inst_lib;

  void SetupInstLib();

public:
  // TODO - fix this => task is initialized by the base obj
  AvidaGPL9Task(world_t& w) : base_t(w) { ; }

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

    // TODO

    fresh_eval=true; // mark task evaluation
  }

  // --- ORGANISM-LEVEL EVENT HOOKS ---
  // These are always called AFTER the organism's equivalent functions.

  /// Called when parent is about to reproduce, but before an offspring has been constructed.
  void OnBeforeOrgRepro(org_t & parent) override { /*todo*/ }

  /// Called when the offspring has been constructed but has not been placed yet.
  void OnOffspringReady(org_t& offspring, org_t& parent) override { /*todo*/ }

  /// Called when org is being placed (@ position) in the world
  void OnOrgPlacement(org_t& org, size_t position) override { /*todo*/ }

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

  inst_lib.AddInst(
    "Nop",
    [](hardware_t& hw, const hardware_t::inst_t & inst) {
      return;
    },
    0,
    "No operation"
  );

}

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_AVIDAGP_L9_TASK_HPP_INCLUDE