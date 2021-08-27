#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE

#include "BaseTask.hpp"

namespace dirdevo {

template<typename ORG_T>
class OneMaxTask : BaseTask<OneMaxTask<ORG_T>, ORG_T> {

public:

  using org_t = ORG_T;
  using this_t = OneMaxTask<org_t>;
  using base_t = BaseTask<this_t,org_t>;
  using world_t = DirectedDevoWorld<org_t,this_t>;

protected:
public:
  OneMaxTask(world_t& w) : base_t(w) { ; }

  // --- WORLD-LEVEL EVENT HOOKS ---

  /// OnWorldSetup called at end of constructor/world setup
  void OnWorldSetup() override { /*todo*/ }

  /// OnBeforeWorldUpdate is called at the beginning of running the world update
  void OnBeforeWorldUpdate(size_t update) override { /*todo*/ }

  /// OnWorldUpdate is called when the OnUpdate signal is triggered (at the end of a world update)
  void OnWorldUpdate(size_t update) override { /*todo*/ }

  // TODO - any selection based hooks!

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


} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE