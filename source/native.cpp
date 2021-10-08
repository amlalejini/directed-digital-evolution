//  This file is part of directed-digital-evolution
//  Copyright (C) Alexander Lalejini, 2021.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/base/vector.hpp"

#include "dirdevo/utility/config_setup.hpp"
#include "dirdevo/DirectedDevoConfig.hpp"
#include "dirdevo/DirectedDevoExperiment.hpp"

// ONE MAX
#include "dirdevo/mutator/BitSetMutator.hpp"
#include "dirdevo/ExperimentSetups/OneMax/OneMaxOrganism.hpp"
#include "dirdevo/ExperimentSetups/OneMax/OneMaxTask.hpp"

// SGP-LITE
// #include "directed-digital-evolution/Organism/SGPLiteOrganism.hpp"

// AVIDAGP
#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPOrganism.hpp"
#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPMutator.hpp"
#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPMultiPathwayTask.hpp"

// This is the main function for the NATIVE version of directed-digital-evolution.

dirdevo::DirectedDevoConfig cfg;

int main(int argc, char* argv[])
{

  // Things that need to be configured at compile time:
  // - Organism type
  // - Mutator?

  // TODO - configure compile time flags to switch between configurations (or just have multiple .cpp?)

  ///////////////////////////////////////////////////////
  // OneMax
  ///////////////////////////////////////////////////////
  // using org_t = dirdevo::OneMaxOrganism<256>;
  // using task_t = dirdevo::OneMaxTask<org_t>;
  // using mutator_t = dirdevo::BitSetMutator;
  // using world_t = dirdevo::DirectedDevoWorld<org_t,task_t>;
  // using experiment_t = dirdevo::DirectedDevoExperiment<world_t, org_t, mutator_t, task_t>;
  ///////////////////////////////////////////////////////

  ///////////////////////////////////////////////////////
  // AvidaGP Multipathway
  ///////////////////////////////////////////////////////
  using org_t = dirdevo::AvidaGPOrganism;
  using task_t = dirdevo::AvidaGPMultiPathwayTask;
  using mutator_t = dirdevo::AvidaGPMutator;
  using world_t = dirdevo::DirectedDevoWorld<org_t,task_t>;
  using experiment_t = dirdevo::DirectedDevoExperiment<world_t, org_t, mutator_t, task_t>;
  ///////////////////////////////////////////////////////

  // Set up a configuration panel for native application
  setup_config_native(cfg, argc, argv);
  cfg.Write(std::cout);

  experiment_t experiment(cfg);
  experiment.Run();

  return 0;
}
