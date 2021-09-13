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
#include <filesystem>
#include <sys/stat.h>

#include "emp/base/vector.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/tools/string_utils.hpp"
#include "emp/data/DataFile.hpp"

#include "DirectedDevoConfig.hpp"
#include "DirectedDevoWorld.hpp"
#include "BasePeripheral.hpp"             /// TODO - fully integrate the peripheral component!
#include "selection/SelectionSchemes.hpp"
#include "utility/ConfigSnapshotEntry.hpp"

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
  using world_container_file_t = emp::ContainerDataFile<emp::vector<emp::Ptr<world_t>>>;


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
  bool record_epoch=false;
  emp::Ptr<world_t> cur_world=nullptr; ///< NON-OWNING. Used internally for data tracking.


  emp::Ptr<emp::DataFile> world_summary_file; ///< Manages world summary output. (is updated during world updates)
  std::string output_dir;                     ///< Formatted output directory

  /// Setup the experiment based on the given configuration (called internally).
  void Setup();

  /// Configure local population structure (called internally).
  // void SetLocalPopStructure();

  /// Configure population selection (called internally).
  void SetupSelection();
  void SetupEliteSelection();
  void SetupDataCollection();

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
    if (world_summary_file) world_summary_file.Delete();
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
      "world_"+emp::to_string(i),
      i
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
    std::function<genome_t(this_t&, world_t&)> get_ancestor_genome;
    // TODO - Make ancestor loading a little smarter? E.g., allow different ancestors for different worlds? Can hack into the load function (using the world name to differentiate).
    if (config.LOAD_ANCESTOR_FROM_FILE()) {
      // check that file exists
      if (!std::filesystem::exists(config.ANCESTOR_FILE())) {
        std::cout << "Ancestor file does not exist. " << config.ANCESTOR_FILE() << std::endl;
        std::exit(EXIT_FAILURE);
      }
      get_ancestor_genome = [](this_t& experiment, world_t& world) {
        return org_t::LoadAncestralGenome(experiment, world);
      };
    } else {
      get_ancestor_genome = [](this_t& experiment, world_t& world) {
        return org_t::GenerateAncestralGenome(experiment, world);
      };
    }
    // auto ancestral_genome = ;
    world_ptr->InjectAt(get_ancestor_genome(*this, *world_ptr), 0); // TODO - Random location to start?
  }

  // Adjust initial scheduler weights according to initial population!
  for (auto world_ptr : worlds) {
    world_ptr->SyncSchedulerWeights();
  }

  // Setup selection
  SetupSelection();

  // Setup data collection
  SetupDataCollection();

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
void DirectedDevoExperiment<WORLD, ORG, MUTATOR, TASK, PERIPHERAL>::SetupDataCollection() {
  output_dir = config.OUTPUT_DIR();
  if (setup) {
    // anything we need to do if this function is called post-setup
    if (world_summary_file) world_summary_file.Delete();
  } else {
    mkdir(output_dir.c_str(), ACCESSPERMS);
    if(output_dir.back() != '/') {
      output_dir += '/';
    }
  }

  // TODO - Configure data collection!

  // Generally useful functions
  std::function<size_t(void)> get_epoch = [this]() { return cur_epoch; };

  // World update summary information
  if (config.OUTPUT_COLLECT_UPDATE_SUMMARY()) {
    // Attach data file update to world on update signals
    for (size_t i = 0; i < worlds.size(); ++i) {
      // Trigger world summary on correct updates
      worlds[i]->OnUpdate(
        [this](size_t u){
          if (record_epoch) {
            // record this update if final or at recording interval
            const bool record_update = !(u % config.OUTPUT_SUMMARY_UPDATE_RESOLUTION()) || (u == config.UPDATES_PER_EPOCH());
            if (!record_update) return;
            // TODO - update current world (somehow switch between which world is providing information)
            world_summary_file->Update();
          }
        }
      );
    }
    // TODO - rename world_summary file and associated functions?
    world_summary_file = emp::NewPtr<emp::DataFile>(output_dir + "world_summary.csv");
    // Experiment level functions
    world_summary_file->AddFun<size_t>(get_epoch,"epoch");
    // World-level functions
    world_t::AttachWorldUpdateDataFileFunctions(*world_summary_file, [this](){ return cur_world; });
    world_summary_file->PrintHeaderKeys();
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

  emp::DataFile snapshot_file(output_dir + "/run_config.csv");
  std::function<std::string(void)> get_param;
  std::function<std::string(void)> get_value;
  std::function<std::string(void)> get_source;
  snapshot_file.AddFun<std::string>(
    [&get_param]() { return get_param(); },
    "parameter"
  );
  snapshot_file.AddFun<std::string>(
    [&get_value]() { return get_value(); },
    "value"
  );
  snapshot_file.AddFun<std::string>(
    [&get_source]() { return get_source(); },
    "source"
  );

  snapshot_file.PrintHeaderKeys();

  // CUSTOM STUFF would go here!

  // Snapshot config
  get_source = []() { return "experiment"; };
  for (const auto & entry : config) {
    get_param = [&entry]() { return entry.first; };
    get_value = [&entry]() { return emp::to_string(entry.second->GetValue()); };
    snapshot_file.Update();
  }

  // Collect and snapshot downstream configuration
  emp::vector<ConfigSnapshotEntry> entries;
  for (auto world_ptr : worlds) {
    auto world_entries = world_ptr->GetConfigSnapshotEntries();
    std::copy(world_entries.begin(), world_entries.end(), std::back_inserter(entries));
  }
  for (const auto& entry : entries) {
    get_param = [&entry]() { return entry.param; };
    get_value = [&entry]() { return entry.value; };
    get_source = [&entry]() { return entry.source; };
    snapshot_file.Update();
  }

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

    // Refresh epoch-level bookkeeping
    extinct_worlds.clear();
    live_worlds.clear();

    // Is this an epoch that we want to record data for?
    // - Either correct interval or final epoch.
    record_epoch = !(cur_epoch % config.OUTPUT_SUMMARY_EPOCH_RESOLUTION()) || (cur_epoch == config.EPOCHS());
    // NOTE, should use onupdate to trigger update signals?

    // Run worlds forward X updates.
    for (auto world_ptr : worlds) {
      std::cout << "Running world " << world_ptr->GetName() << std::endl;
      cur_world = world_ptr;

      for (size_t u = 0; u <= config.UPDATES_PER_EPOCH(); u++) {
        world_ptr->RunStep();
      }

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
      // world_ptr->GetRandomOrg().GetHardware().PrintGenome(std::cout);
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