/**
 * @file DirectedDevoExperiment.hpp
 * @brief Defines and manages a directed evolution experiment.
 */

#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE

#include "emp/Evolve/World.hpp"

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

protected:

  const config_t& config;        ///< Experiment configuration (REMINDER: the config object must exist beyond lifetime of this experiment object!)
  emp::Random random;            ///< Random number generators (shared across worlds)
  emp::vector<world_t> worlds;   ///< How many "populations" are we applying directed evolution to?

  void Setup();

  void SnapshotConfig(const std::string& filename = "experiment-config.csv");


public:

  DirectedDevoExperiment(
    const config_t& exp_config
  ) :
    config(exp_config),
    random(config.SEED())
  {
     Setup(); // TODO - should setup be in constructor or controlled externally?
  }

  void Run();
  void RunStep();

};

template <typename ORG>
void DirectedDevoExperiment<ORG>::Setup() {

  // TODO - should config snapshot be here or elsewhere?
  SnapshotConfig();
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


} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE