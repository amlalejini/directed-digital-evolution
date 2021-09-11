#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_BASE_TASK_HPP_INCLUDE

#include <cstddef>
#include <functional>

#include "emp/base/vector.hpp"

#include "DirectedDevoWorld.hpp"
#include "utility/ConfigSnapshotEntry.hpp"

namespace dirdevo {

// TODO - at compile time, depending on the task strip out unused function calls from the world?

/// Tasks describe the world-level task. Organism-level "task" (i.e., how organisms reproduce/compete within a world is defined by the organism).
/// This BaseTask design intends for derived tasks to ONLY work with a dirdevo::DirectedDevoWorld!
template<typename DERIVED_T, typename ORG_T>
class BaseTask {

public:

  using org_t = ORG_T;
  using world_t = DirectedDevoWorld<org_t, DERIVED_T>;
  using perf_fun_t = std::function<double(void)>;

protected:

  world_t& world; ///< Reference back to the world that own's this task object. WARNING - must be used responsibly! It would be very easy to create some nasty memory errors by moving things around (e.g., world assumes a reference is still valid but task-driven manipulation invalidated it).
  bool fresh_eval=true;

  // TODO - think more about whether this is the cleanest way to do this...
  perf_fun_t aggregate_performance_fun;
  emp::vector<perf_fun_t> performance_fun_set;

  BaseTask(world_t& w) : world(w) { ; }

public:

  perf_fun_t GetAggregatePerformanceFun() { return aggregate_performance_fun; }

  emp::vector<perf_fun_t> & GetPerformanceFunSet() { return performance_fun_set; }

  bool IsEvalFresh() const { return fresh_eval; }

  // --- WORLD-LEVEL EVENT HOOKS ---

  virtual emp::vector<ConfigSnapshotEntry> GetConfigSnapshotEntries() { return {}; } // By default, return an empty vector.

  /// OnWorldSetup called at end of constructor/world setup
  virtual void OnWorldSetup() { emp_assert(false, "Derived task class must implement this function."); }

  /// OnBeforeWorldUpdate is called at the beginning of running the world update
  virtual void OnBeforeWorldUpdate(size_t update) { emp_assert(false, "Derived task class must implement this function."); }

  /// OnWorldUpdate is called when the OnUpdate signal is triggered (at the end of a world update)
  virtual void OnWorldUpdate(size_t update) { emp_assert(false, "Derived task class must implement this function."); }

  /// OnWorldReset is called when the DirectedDevoWorld's DirectedDevoReset function is called.
  virtual void OnWorldReset() { emp_assert(false, "Derived task class must implement this function."); }

  // TODO - any selection based hooks!
  /// Ensure that task performance is up-to-date. Might not do anything if performance is updated as the world updates.
  virtual void Evaluate() { emp_assert(false, "Derived task class must implement this function."); }

  // --- ORGANISM-LEVEL EVENT HOOKS ---
  // These are always called AFTER the organism's equivalent functions.

  /// Called before an organism is injected into the population
  virtual void OnOrgInjectReady(org_t& org) { emp_assert(false, "Derived task class must implement this function."); }

  // NOTE - Asserts are a lazy way to indicate to future me that these functions need to be implemented.
  /// Called when parent is about to reproduce, but before an offspring has been constructed.
  virtual void OnBeforeOrgRepro(org_t & parent) { emp_assert(false, "Derived task class must implement this function."); }

  /// Called when the offspring has been constructed but has not been placed yet.
  virtual void OnOffspringReady(org_t& offspring, org_t& parent) { emp_assert(false, "Derived task class must implement this function."); }

  /// Called when org is being placed (@ position) in the world
  virtual void OnOrgPlacement(org_t& org, size_t position) { emp_assert(false, "Derived task class must implement this function."); }

  /// Called just before the organism's process step function is called.
  virtual void BeforeOrgProcessStep(org_t& org) { emp_assert(false, "Derived task class must implement this function."); }

  /// Called just after the organism's process step function is called.
  virtual void AfterOrgProcessStep(org_t& org) { emp_assert(false, "Derived task class must implement this function."); }

  /// Called before organism is removed from the world.
  virtual void OnOrgDeath(org_t& org, size_t position) { emp_assert(false, "Derived task class must implement this function."); }

  /// Called after two organisms are swapped in the world (new world positions are accurate).
  virtual void AfterOrgSwap(org_t& org1, org_t& org2) { emp_assert(false, "Derived task class must implement this function."); }

};


} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_BASE_TASK_HPP_INCLUDE