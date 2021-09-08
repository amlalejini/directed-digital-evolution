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
#include "BasePeripheral.hpp"

namespace dirdevo {

// TODO - we're using one uniform configuration type, so just hand off configs and let things configure themselves.
// TODO - make communication between experiment and components more consistent (e.g., Configuration; let components configure themselves?)

// PERIPHERAL defines any extra equipment needed to run the experiment (typically something required by the subtasks)
template<typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL=BasePeripheral>
class DirectedDevoExperiment {
public:
  // --- Publically available types ---
  using this_t = DirectedDevoExperiment<WORLD,ORG,MUTATOR,TASK,PERIPHERAL>;
  using org_t = ORG;
  using task_t = TASK;
  using world_t = WORLD; //DirectedDevoWorld<org_t,task_t>;
  using config_t = DirectedDevoConfig;
  using pop_struct_t = typename world_t::POP_STRUCTURE;
  using peripheral_t = PERIPHERAL;

  // using mutator_t = typename org_t::mutator_t; // TODO - move the mutator out of the organism? Makes it weird that the task needs to care about mutators....
  using mutator_t = MUTATOR;
  using genome_t = typename org_t::genome_t;
  using propagule_t = emp::vector<genome_t>;

  const std::unordered_set<std::string> valid_selection_methods={"elite","tournament","lexicase"};

protected:

  const config_t& config;        ///< Experiment configuration (REMINDER: the config object must exist beyond lifetime of this experiment object!)
  emp::Random random;            ///< Random number generators (shared across worlds)
  emp::vector<emp::Ptr<world_t>> worlds;   ///< How many "populations" are we applying directed evolution to?

  pop_struct_t local_pop_struct=pop_struct_t::MIXED;
  mutator_t mutator; ///< Responsible for mutating organisms across worlds. NOTE - currently, mutator is shared; individual worlds cannot tweak settings (i.e., no high/low mutation worlds).
  peripheral_t peripheral; ///< Peripheral components that should exist at the experiment level.

  std::function<void(emp::vector<size_t>&)> do_selection_fun;
  emp::vector<std::function<double(void)>> aggregate_score_funs;

  // TODO - should more state information get passed through the propagule? If so, genome_t => org_t?
  emp::vector< emp::vector<genome_t> > propagules;
  std::unordered_set<size_t> extinct_worlds; /// Set of worlds that are extinct.
  std::unordered_set<size_t> live_worlds;    /// Set of worlds that are not extinct.

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
  // void SetLocalPopStructure();

  /// Configure population selection (called internally).
  void SetupSelection();
  void SetupEliteSelection();

  // TODO - allow for different sampling techniques / ways of forming propagules
  // - e.g., each propagules comes from a single world? each propagule is a mixture of all worlds?
  //        'propagule' crossover?
  propagule_t Sample(world_t& world); // NOTE - should this live in the experiment or the world class?
  void SeedWithPropagule(world_t& world, propagule_t& propagule);

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

  peripheral_t& GetPeripheral() { return peripheral; }
  const peripheral_t& GetPeripheral() const { return peripheral; }

};

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::Setup() {
  if (setup) return; // Don't let myself run Setup more than once.

  // Validate configuration (even in when compiled outside of debug mode!)
  if(!ValidateConfig()) {
    // todo - report which configs are invalid?
    std::cout << "Invalid configuration, exiting." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // What population structure are we using?
  // TODO - clean this up a bit!
  // local_pop_struct = world_t::PopStructureStrToMode(config.LOCAL_POP_STRUCTURE());
  // typename world_t::PopStructureDesc pop_struct(local_pop_struct, config.LOCAL_GRID_WIDTH(), config.LOCAL_GRID_HEIGHT(), config.LOCAL_GRID_DEPTH());

  // Configure the mutator
  mutator_t::Configure(mutator, config);

  // Configure the peripheral components
  peripheral.Setup(config);

  // Initialize each world.
  worlds.resize(config.NUM_POPS());
  for (size_t i = 0; i < config.NUM_POPS(); ++i) {
    worlds[i] = emp::NewPtr<world_t>(
      config,
      random,
      "population_"+emp::to_string(i)
    );
    worlds[i]->SetAvgOrgStepsPerUpdate(config.AVG_STEPS_PER_ORG());
    // configure world's mutation function
    worlds[i]->SetMutFun([this](org_t & org, emp::Random& rnd) {
      // TODO - add support for mutation tracking!
      return mutator.Mutate(org.GetGenome(), rnd);
    });
  }

  // Seed each world with an initial common ancestor
  // PROBLEM - can't do out-of-world injection for genomes
  for (auto world_ptr : worlds) {
    auto ancestral_genome = org_t::GenerateAncestralGenome(*this, *world_ptr);
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

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::SetupSelection() {
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

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::SetupEliteSelection() {
  // calling the do selected function should population selected with the ids of populations to sample 'propagules' from
  do_selection_fun = [this](emp::vector<size_t>& selected) {
    dirdevo::EliteSelect(selected, aggregate_score_funs, config.ELITE_SEL_NUM_ELITES());
  };
}

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
emp::vector<typename ORG::genome_t> DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::Sample(world_t& world) {
  emp_assert(!world.IsExtinct(), "Attempting to sample from an extinct population.");
  // sample randomly (for now)
  propagule_t sample;
  // extinct worlds shouldn't get selected (unless everything went extinct or we're doing random selection...)
  for (size_t i = 0; i < config.POPULATION_SAMPLING_SIZE(); ++i) {
    sample.emplace_back(world.GetRandomOrg().GetGenome());
  }
  return sample;
}

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::SeedWithPropagule(world_t& world, propagule_t& propagule) {
  // TODO - Tweak if we ever complicate how propagules are seeded into the world
  emp_assert(propagule.size() <= world.GetSize(), "Propagule size cannot exceed world size.", propagule.size(), world.GetSize());
  for (size_t i = 0; i < propagule.size(); ++i) {
   const size_t pos = i; // TODO - use a slightly better method of distributing the propagule!
   world.InjectAt(propagule[i], {pos});
  }
}

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::SnapshotConfig(
  const std::string& filename /*= "experiment-config.csv"*/
) {
  std::cout << "Snapshotting experiment configuration..." << std::endl;

  // TODO - actually snapshot configuration
  for (const auto & entry : config) {
    std::cout << "  * " <<  entry.first << " = " << emp::to_string(entry.second->GetValue()) << std::endl;
  }

  std::cout << "...done snapshotting." << std::endl;
}

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
bool DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::ValidateConfig() {
  // GLOBAL SETTINGS
  if (config.NUM_POPS() < 1) return false;
  // LOCAL WORLD SETTINGS
  if (!world_t::IsValidPopStructure(config.LOCAL_POP_STRUCTURE())) return false;
  if (config.LOCAL_GRID_WIDTH() < 1) return false;
  if (config.LOCAL_GRID_HEIGHT() < 1) return false;
  if (config.LOCAL_GRID_DEPTH() < 1) return false;
  if (config.AVG_STEPS_PER_ORG() < 1) return false;
  if (!emp::Has(valid_selection_methods,config.SELECTION_METHOD())) return false;
  if (config.POPULATION_SAMPLING_SIZE() < 1) return false;
  // TODO - flesh this out!
  return true;
}


template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::Run() {
  // Create vector to hold the distribution of population ids selected each epoch
  emp::vector<size_t> selected(config.NUM_POPS(), 0);

  for (cur_epoch = 0; cur_epoch <= config.EPOCHS(); ++cur_epoch) {
    std::cout << "==== EPOCH " << cur_epoch << "====" << std::endl;

    extinct_worlds.clear();
    live_worlds.clear();

    // Run worlds forward X updates.
    for (auto world_ptr : worlds) {
      std::cout << "Running world " << world_ptr->GetName() << std::endl;
      world_ptr->Run(config.UPDATES_PER_EPOCH());
    }

    // Do evaluation (could move this into previous loop if I don't add anything else here that requires all worlds to have been run)
    for (size_t world_id = 0; world_id < worlds.size(); ++world_id) {
      worlds[world_id]->Evaluate();
      (worlds[world_id]->IsExtinct()) ? extinct_worlds.insert(world_id) : live_worlds.insert(world_id);
    }

    if (extinct_worlds.size() == worlds.size()) {
      std::cout << "All of the worlds are extinct." << std::endl;
      // TODO - when exiting because all worlds are extinct, cleanly do snapshot, etc
      break;
    }

    std::cout << "Evaluation summary: " << std::endl;
    for (auto world_ptr : worlds) {
      std::cout << "  " << world_ptr->GetName() << std::endl;
      std::cout << "    Pop size: " << world_ptr->GetNumOrgs() << std::endl;
      std::cout << "    Aggregate performance: " << world_ptr->GetAggregateTaskPerformance() << std::endl;
    }

    // Do selection
    do_selection_fun(selected);

    // std::cout << "selected:" << selected << std::endl;

    // selected should hold which populations we should sample from

    // For each selected world, extract a sample
    propagules.resize(config.NUM_POPS(), {});
    emp_assert(propagules.size()==selected.size());
    for (size_t i = 0; i < selected.size(); ++i) {
      // Sample propagules from each world!
      size_t selected_pop_id = selected[i];
      // Make sure selected pop is isn't extinct (in some weird edge case)
      // Note that we cannot be here if all populations are extinct. Spin until we pull a non-extinct pop.
      while (emp::Has(extinct_worlds, selected_pop_id)) {
        selected_pop_id = (selected_pop_id + 1) % worlds.size();
      }
      // Sample from the selected world to form the propagule.
      // TODO - will need to tweak this code if we complicate propagule formation
      propagules[i] = Sample(*worlds[selected_pop_id]);
    }

    // Reset worlds + inject propagules into them!
    for (size_t i = 0; i < config.NUM_POPS(); ++i) {
      auto& world = *(worlds[i]);
      world.DirectedDevoReset(); // Clear our the world.
      emp_assert(propagules[i].size(), "Propagule is empty.");
      SeedWithPropagule(world, propagules[i]);
    }

    // Report summary information(?)
    // std::cout << "epoch " << cur_epoch << std::endl;
  }

  // todo - Final data dump

}

template <typename WORLD, typename ORG, typename MUTATOR, typename TASK, typename PERIPHERAL>
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::RunStep() {
  // Advance each world by one step
  for (auto world_ptr : worlds) {
    std::cout << "-- Stepping " << world_ptr->GetName() << " --" << std::endl;
    world_ptr->RunStep();
  }
  // TODO - update transfer(?) counter
}

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE