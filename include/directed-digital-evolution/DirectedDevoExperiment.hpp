/**
 * @file DirectedDevoExperiment.hpp
 * @brief Defines and manages a directed evolution experiment.
 */

#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE

#include "emp/Evolve/World.hpp"

#include "DirectedDevoConfig.hpp"

namespace dirdevo {

template<typename ORG>
class DirectedDevoExperiment {
public:
  // --- Publically available types ---
  using this_t = DirectedDevoExperiment<ORG>;
  using org_t = ORG;
  using world_t = emp::World<org_t>;
  using config_t = DirectedDevoConfig;

protected:


  config_t& config;             ///< Experiment configuration

  emp::vector<world_t> worlds;   ///< How many "populations" are we applying directed evolution to?

  void Setup();

public:

  DirectedDevoExperiment(
    config_t& exp_config
  ) :
    config(exp_config)
  { ; }

  void Run();
  void RunStep();

};

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_EXPERIMENT_HPP_INCLUDE