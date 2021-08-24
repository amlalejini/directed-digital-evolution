/**
 * @file DirectedDevoExperiment.hpp
 * @brief Defines and manages a directed evolution experiment.
 */

#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE

#include <string>
#include <unordered_map>

#include "emp/base/vector.hpp"
#include "emp/datastructs/vector_utils.hpp"
#include "emp/Evolve/World.hpp"
#include "emp/tools/string_utils.hpp"

#include "DirectedDevoConfig.hpp"
#include "DirectedDevoWorld.hpp"

namespace dirdevo {

template<typename ORG> // TODO - define org type based on compiler flag?
class DirectedDevoExperiment {
public:
  // --- Publically available types ---
  using this_t = DirectedDevoExperiment<ORG>;
  using org_t = ORG;
  using world_t = DirectedDevoWorld<org_t>;
  using config_t = DirectedDevoConfig;
  using pop_struct_t = typename world_t::POP_STRUCTURE;

protected:

  const config_t& config;        ///< Experiment configuration (REMINDER: the config object must exist beyond lifetime of this experiment object!)
  emp::Random random;            ///< Random number generators (shared across worlds)
  emp::vector<emp::Ptr<world_t>> worlds;   ///< How many "populations" are we applying directed evolution to?

  pop_struct_t local_pop_struct=pop_struct_t::MIXED;
  bool setup=false;

  /// Setup the experiment based on the given configuration.
  void Setup();
  void SetLocalPopStructure();

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
    for (auto world : worlds) {
      if (world != nullptr) world.Delete();
    }
  }

  void Run();
  void RunStep();

};

template <typename ORG>
void DirectedDevoExperiment<ORG>::Setup() {
  if (setup) return; // Don't let myself run Setup more than once.

  // Validate configuration (even in when compiled outside of debug mode!)
  if(!ValidateConfig()) {
    // todo - report which configs are invalid?
    std::cout << "Invalid configuration, exiting." << std::endl;
    std::exit(EXIT_FAILURE);
  }

  // What population structure are we using?
  local_pop_struct = world_t::PopStructureStrToMode(config.LOCAL_POP_STRUCTURE());

  // Initialize each world.
  worlds.resize(config.NUM_POPS());
  for (size_t i = 0; i < config.NUM_POPS(); ++i) {
    worlds[i] = emp::NewPtr<world_t>(random, "population_"+emp::to_string(i));
    worlds[i]->SetAvgOrgStepsPerUpdate(config.AVG_STEPS_PER_ORG());
    worlds[i]->SetPopStructure(
      local_pop_struct,
      config.LOCAL_GRID_WIDTH(),
      config.LOCAL_GRID_HEIGHT(),
      config.LOCAL_GRID_DEPTH()
    );
  }

  // Seed each world with an initial common ancestor
  for (auto world_ptr : worlds) {
    world_ptr->Inject(org_t::GenerateAncestralGenome());
  }

  // Adjust initial scheduler weights according to initial population!
  for (auto world_ptr : worlds) {

  }


  // TODO - should config snapshot be here or elsewhere?
  SnapshotConfig();
  setup = true;
}

template <typename ORG>
void DirectedDevoExperiment<ORG>::SnapshotConfig(
  const std::string& filename /*= "experiment-config.csv"*/
) {
  std::cout << "Snapshotting experiment configuration..." << std::endl;

  // TODO - actually snapshot configuration
  for (const auto & entry : config) {
    std::cout << "  * " <<  entry.first << " = " << emp::to_string(entry.second->GetValue()) << std::endl;
  }

  std::cout << "...done snapshotting." << std::endl;
}

template <typename ORG>
bool DirectedDevoExperiment<ORG>::ValidateConfig() {
  // GLOBAL SETTINGS
  if (config.NUM_POPS() < 1) return false;
  // LOCAL WORLD SETTINGS
  if (!world_t::IsValidPopStructure(config.LOCAL_POP_STRUCTURE())) return false;
  if (config.LOCAL_GRID_WIDTH() < 1) return false;
  if (config.LOCAL_GRID_HEIGHT() < 1) return false;
  if (config.LOCAL_GRID_DEPTH() < 1) return false;
  if (config.AVG_STEPS_PER_ORG() < 1) return false;
  return true;
}


} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE