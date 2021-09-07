//  This file is part of directed-digital-evolution
//  Copyright (C) Alexander Lalejini, 2021.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/base/vector.hpp"

#include "dirdevo/utility/config_setup.hpp"
#include "dirdevo/DirectedDevoConfig.hpp"
#include "dirdevo/DirectedDevoExperiment.hpp"

// ONE MAX
#include "dirdevo/OneMax/OneMaxOrganism.hpp"
#include "dirdevo/Mutator/BitSetMutator.hpp"
#include "dirdevo/OneMax/OneMaxTask.hpp"

// SGP-LITE
// #include "directed-digital-evolution/Organism/SGPLiteOrganism.hpp"

// AVIDAGP-L9
#include "dirdevo/AvidaGPL9/AvidaGPOrganism.hpp"
#include "dirdevo/AvidaGPL9/AvidaGPL9Task.hpp"
#include "dirdevo/Mutator/AvidaGPMutator.hpp"

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
  // AvidaGP-L9
  ///////////////////////////////////////////////////////
  using org_t = dirdevo::AvidaGPOrganism;
  using task_t = dirdevo::AvidaGPL9Task;
  using mutator_t = dirdevo::AvidaGPMutator;
  using world_t = dirdevo::DirectedDevoWorld<org_t,task_t>;
  using experiment_t = dirdevo::DirectedDevoExperiment<world_t, org_t, mutator_t, task_t>;
  ///////////////////////////////////////////////////////


  // Set up a configuration panel for native application
  setup_config_native(cfg, argc, argv);
  cfg.Write(std::cout);

  experiment_t experiment(cfg);
  experiment.Run();
  // for (size_t i = 0; i < 100; ++i) {
  //   std::cout << "======= experiment step " << i << "=======" << std::endl;
  //   experiment.RunStep();
  // }


  // dirdevo::ProbabilisticScheduler scheduler(rnd, 10, 30);
  // scheduler.AdjustWeight(2, 10);
  // scheduler.AdjustWeight(3, 5);
  // std::cout << scheduler.UpdateSchedule() << std::endl;
  // std::cout << scheduler.UpdateSchedule() << std::endl;

  return 0;
}
