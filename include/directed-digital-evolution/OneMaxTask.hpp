#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE

#include "BaseTask.hpp"

namespace dirdevo {

/// todo ORG_T should really be OneMaxOrganism, but that requires extra fiddling with more template parameters to declare a one max task
/**
 * OneMaxTask is simple task that is used only to test all of the pieces of this digital directed evolution framework.
 * Aggregate performance = average num_ones
 * Multiple criteria = average genome value for each site (works because fixed length genomes)
 */
template<typename ORG_T>
class OneMaxTask : public BaseTask<OneMaxTask<ORG_T>, ORG_T> {

public:

  using org_t = ORG_T;
  using this_t = OneMaxTask<org_t>;
  using base_t = BaseTask<this_t,org_t>;
  using world_t = DirectedDevoWorld<org_t,this_t>;

protected:

  using base_t::aggregate_performance_fun;
  using base_t::performance_fun_set;
  using base_t::fresh_eval;
  using base_t::world;

  emp::vector<double> ones_per_position;
  double total_num_ones=0;

public:
  OneMaxTask(world_t& w) : base_t(w), ones_per_position(org_t::GENOME_SIZE, 0) { ; }

  // --- WORLD-LEVEL EVENT HOOKS ---

  /// OnWorldSetup called at end of constructor/world setup
  void OnWorldSetup() override {
    // TODO - configure task based on world's configuration

    // Wire up the aggregate task performance function
    aggregate_performance_fun = [this]() { return total_num_ones; };

    // Wire up the performance function set (used for multi-objective/-task selection schemes)
    for (size_t i = 0; i < org_t::GENOME_SIZE; ++i) {
      performance_fun_set.emplace_back(
        [i, this]() {
          // importantly, i is copy-captured
          return ones_per_position[i];
        }
      );
    }

    fresh_eval=false;

  }

  /// OnBeforeWorldUpdate is called at the beginning of running the world update
  void OnBeforeWorldUpdate(size_t update) override {
    // as soon as the world has updated, evaluation is no longer guaranteed to be fresh
    fresh_eval=false;
  }

  /// OnWorldUpdate is called when the OnUpdate signal is triggered (at the end of a world update)
  void OnWorldUpdate(size_t update) override { /*todo*/ }

  // TODO - any selection based hooks!

  /// Evaluate the world on this task (count ones).
  void Evaluate() override {

    // Reset internal performance state
    total_num_ones = 0.0;
    std::fill(
      ones_per_position.begin(),
      ones_per_position.end(),
      0.0
    );

    // Count ones!
    for (size_t org_id = 0; org_id < world.GetSize(); ++org_id) {
      if (!world.IsOccupied({org_id})) continue;
      auto& org = world.GetOrg(org_id);
      for (size_t site_i=0; site_i < org.GetGenome().GetSize(); ++site_i) {
        ones_per_position[site_i] += (double)org.GetGenome().Get(site_i);
      }
      total_num_ones += org.GetPhenotype().num_ones;
    }

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


} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_ONE_MAX_TASK_HPP_INCLUDE