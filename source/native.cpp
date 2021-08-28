//  This file is part of directed-digital-evolution
//  Copyright (C) Alexander Lalejini, 2021.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/base/vector.hpp"

#include "directed-digital-evolution/config_setup.hpp"
#include "directed-digital-evolution/DirectedDevoConfig.hpp"
#include "directed-digital-evolution/DirectedDevoExperiment.hpp"
#include "directed-digital-evolution/OneMaxOrganism.hpp"
#include "directed-digital-evolution/BitSetMutator.hpp"
#include "directed-digital-evolution/OneMaxTask.hpp"

// This is the main function for the NATIVE version of directed-digital-evolution.

dirdevo::DirectedDevoConfig cfg;

int main(int argc, char* argv[])
{

  // Things that need to be configured at compile time:
  // - Organism type
  // - Mutator?
  using org_t = dirdevo::OneMaxOrganism<256,dirdevo::BitSetMutator>;
  using task_t = dirdevo::OneMaxTask<org_t>; // TODO
  using experiment_t = dirdevo::DirectedDevoExperiment<org_t, task_t>;

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
