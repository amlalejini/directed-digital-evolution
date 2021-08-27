/**
 * @file DirectedDevoExperiment.hpp
 * @brief Defines and manages a directed evolution experiment.
 */

#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <functional>

#include "emp/base/vector.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/tools/string_utils.hpp"

#include "DirectedDevoConfig.hpp"
#include "DirectedDevoWorld.hpp"
#include "Selection/SelectionSchemes.hpp"

namespace dirdevo {

template<typename ORG, typename TASK> // TODO - define org type based on compiler flag?
class DirectedDevoExperiment {
public:
  // --- Publically available types ---
  using this_t = DirectedDevoExperiment<ORG,TASK>;
  using org_t = ORG;
  using task_t = TASK;
  using world_t = DirectedDevoWorld<org_t,task_t>;
  using config_t = DirectedDevoConfig;
  using pop_struct_t = typename world_t::POP_STRUCTURE;
  using mutator_t = typename org_t::mutator_t; // TODO - move the mutator out of the organism? Makes it weird that the task needs to care about mutators....

  const std::unordered_set<std::string> valid_selection_methods={"elite","tournament","lexicase"};

protected:

  const config_t& config;        ///< Experiment configuration (REMINDER: the config object must exist beyond lifetime of this experiment object!)
  emp::Random random;            ///< Random number generators (shared across worlds)
  emp::vector<emp::Ptr<world_t>> worlds;   ///< How many "populations" are we applying directed evolution to?

  pop_struct_t local_pop_struct=pop_struct_t::MIXED;
  mutator_t mutator; ///< Responsible for mutating organisms across worlds. NOTE - currently, mutator is shared; individual worlds cannot tweak settings (i.e., no high/low mutation worlds).

  std::function<void(emp::vector<size_t>&)> do_selection_fun;
  emp::vector<std::function<double(void)>> aggregate_score_funs;

  // TODO - have a criteria vector of functions that access quality criteria (for lexicase, multi obj opt)?
  // TODO - Have a task construct that manages performance criteria, etc?
  //        The task would know about the organism type and encode how to evaluate a population of those organisms
  //        I.e., given a world of a compatible type (e.g., bitorgs), it can evaluate the world's performance
  //        If there's a organism-task mismatch, shit won't compile (which is what we want)!
  // std::function<double(size_t)>
  // emp::vector<std::function<double(size_t)> performance_criteria;

  bool setup=false;
  size_t cur_epoch=0;

  /// Setup the experiment based on the given configuration (called internally).
  void Setup();

  /// Configure local population structure (called internally).
  void SetLocalPopStructure();

  /// Configure population selection (called internally).
  void SetupSelection();
  void SetupEliteSelection();

  /// Output the experiment's configuration as a .csv file.
  void SnapshotConfig(const std::string& filename = "experiment-config.csv");

  /// Return whether configuration is valid.
  bool ValidateConfig();

public:

  DirectedDevoExperiment(
    const config_t& in_config
  ) :
    config(in_config),
    random(config.SEED())
  {
     Setup(); // TODO - should setup be in constructor or controlled externally?
  }

  ~DirectedDevoExperiment() {
    // Clean up worlds
    aggregate_score_funs.clear();
    for (auto world : worlds) {
      if (world != nullptr) world.Delete();
    }
  }

  /// Run experiment for configured number of EPOCHS
  void Run();

  /// Advance each world by one step (advance each world by a single step)
  /// Note that RunStep does not take into account epochs or updates per epoch.
  /// - Used primarily for testing and the web interface. Use Run to run the experiment.
  void RunStep();

};

template <typename ORG, typename TASK>
void DirectedDevoExperiment<ORG, TASK>::Setup() {
  if (setup) return; // Don't let myself run Setup more than once.

  // Validate configuration (even in when compiled outside of debug mode!)
  if(!ValidateConfig()) {
    // todo - report which configs are invalid?
    std::cout << "Invalid configuration, exiting." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // What population structure are we using?
  // TODO - clean this up a bit!
  local_pop_struct = world_t::PopStructureStrToMode(config.LOCAL_POP_STRUCTURE());
  typename world_t::PopStructureDesc pop_struct(local_pop_struct, config.LOCAL_GRID_WIDTH(), config.LOCAL_GRID_HEIGHT(), config.LOCAL_GRID_DEPTH());

  // Configure the mutator
  mutator_t::Configure(mutator, config);

  // Initialize each world.
  worlds.resize(config.NUM_POPS());
  for (size_t i = 0; i < config.NUM_POPS(); ++i) {
    worlds[i] = emp::NewPtr<world_t>(
      random,
      "population_"+emp::to_string(i),
      pop_struct
    );
    worlds[i]->SetAvgOrgStepsPerUpdate(config.AVG_STEPS_PER_ORG());
    // configure world's mutation function
    worlds[i]->SetMutFun([this](org_t & org, emp::Random& rnd) {
      // TODO - add support for mutation tracking!
      return mutator.Mutate(org.GetGenome(), rnd);
    });
  }

  // Seed each world with an initial common ancestor
  auto ancestral_genome = org_t::GenerateAncestralGenome();
  for (auto world_ptr : worlds) {
    // ancestral_genome.Set(0, true); // TODO - this is for testing only!
    world_ptr->InjectAt(ancestral_genome, 0); // TODO - Random location to start?
  }

  // Adjust initial scheduler weights according to initial population!
  for (auto world_ptr : worlds) {
    world_ptr->SyncSchedulerWeights();
  }

  // Setup selection
  SetupSelection();

  // TODO - should config snapshot be here or elsewhere?
  SnapshotConfig();
  setup = true;
}

template <typename ORG, typename TASK>
void DirectedDevoExperiment<ORG, TASK>::SetupSelection() {
  // Wire up aggregate score functions
  for (size_t pop_id = 0; pop_id < config.NUM_POPS(); ++pop_id) {
    aggregate_score_funs.emplace_back(
      [this, pop_id]() {
        return worlds[pop_id]->GetAggregateTaskPerformance();
      }
    );
  }
  // todo - wire up function sets

  if (config.SELECTION_METHOD() == "elite") {
    SetupEliteSelection();
  } else if (config.SELECTION_METHOD() == "tournament") {
    emp_assert(false);
  } else if (config.SELECTION_METHOD() == "lexicase") {
    emp_assert(false);
  } else {
    // code should never reach this else (unless I forget to add a selection scheme here that is in the valid selection method set)
    emp_assert(false, "Unimplemented selection scheme.", config.SELECTION_METHOD());
  }
}

template <typename ORG, typename TASK>
void DirectedDevoExperiment<ORG, TASK>::SetupEliteSelection() {
  // calling the do selected function should population selected with the ids of populations to sample 'propagules' from
  do_selection_fun = [this](emp::vector<size_t>& selected) {
    dirdevo::EliteSelect(selected, aggregate_score_funs, config.ELITE_SEL_NUM_ELITES());
  };
}

template <typename ORG, typename TASK>
void DirectedDevoExperiment<ORG, TASK>::SnapshotConfig(
  const std::string& filename /*= "experiment-config.csv"*/
)
{
  std::cout << "Snapshotting experiment configuration..." << std::endl;

  // TODO - actually snapshot configuration
  for (const auto & entry : config) {
    std::cout << "  * " <<  entry.first << " = " << emp::to_string(entry.second->GetValue()) << std::endl;
  }

  std::cout << "...done snapshotting." << std::endl;
}

template <typename ORG, typename TASK>
bool DirectedDevoExperiment<ORG, TASK>::ValidateConfig() {
  // GLOBAL SETTINGS
  if (config.NUM_POPS() < 1) return false;
  // LOCAL WORLD SETTINGS
  if (!world_t::IsValidPopStructure(config.LOCAL_POP_STRUCTURE())) return false;
  if (config.LOCAL_GRID_WIDTH() < 1) return false;
  if (config.LOCAL_GRID_HEIGHT() < 1) return false;
  if (config.LOCAL_GRID_DEPTH() < 1) return false;
  if (config.AVG_STEPS_PER_ORG() < 1) return false;
  if (!emp::Has(valid_selection_methods,config.SELECTION_METHOD())) return false;
  // TODO - flesh this out!
  return true;
}


template <typename ORG, typename TASK>
void DirectedDevoExperiment<ORG, TASK>::Run() {
  // Create vector to hold the distribution of population ids selected each epoch
  emp::vector<size_t> selected(config.NUM_POPS(), 0);

  for (cur_epoch = 0; cur_epoch <= config.EPOCHS(); ++cur_epoch) {
    std::cout << "==== EPOCH " << cur_epoch << "====" << std::endl;
    // Run worlds forward X updates.
    for (auto world_ptr : worlds) {
      std::cout << "Running world " << world_ptr->GetName() << std::endl;
      world_ptr->Run(config.UPDATES_PER_EPOCH());
    }

    // Do evaluation (could move this into previous loop if I don't add anything else here that requires all worlds to have been run)
    for (auto world_ptr : worlds) {
      world_ptr->Evaluate();
    }

    std::cout << "Evaluation summary: " << std::endl;
    for (auto world_ptr : worlds) {
      std::cout << "  " << world_ptr->GetName() << std::endl;
      std::cout << "    Aggregate performance: " << world_ptr->GetAggregateTaskPerformance() << std::endl;
    }

    // Do selection
    do_selection_fun(selected);

    std::cout << "selected:" << selected << std::endl;

    // selected should hold which populations we should sample from

    // TODO - should sampling individuals from a population remove them from future samples?
    // - Which populations are propagated to the next generation?

    // TODO
    // NOTE - each world should have a 'phenotype' that gets filled out by the world as it goes(?)

    // Report summary information(?)
    // std::cout << "epoch " << cur_epoch << std::endl;
  }

  // todo - Final data dump

}

template <typename ORG, typename TASK>
void DirectedDevoExperiment<ORG, TASK>::RunStep() {
  // Advance each world by one step
  for (auto world_ptr : worlds) {
    std::cout << "-- Stepping " << world_ptr->GetName() << " --" << std::endl;
    world_ptr->RunStep();
  }
  // TODO - update transfer(?) counter
}

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE