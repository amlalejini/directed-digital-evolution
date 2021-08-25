//  This file is part of directed-digital-evolution
//  Copyright (C) Alexander Lalejini, 2021.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/base/vector.hpp"

#include "directed-digital-evolution/config_setup.hpp"
#include "directed-digital-evolution/DirectedDevoConfig.hpp"
#include "directed-digital-evolution/DirectedDevoExperiment.hpp"
#include "directed-digital-evolution/OneMaxOrganism.hpp"

// This is the main function for the NATIVE version of directed-digital-evolution.

dirdevo::DirectedDevoConfig cfg;

int main(int argc, char* argv[])
{
  using experiment_t = dirdevo::DirectedDevoExperiment<dirdevo::OneMaxOrganism<128>>;

  // Set up a configuration panel for native application
  setup_config_native(cfg, argc, argv);
  cfg.Write(std::cout);

  experiment_t experiment(cfg);

  for (size_t i = 0; i < 5; ++i) {
    std::cout << "======= experiment step " << i << "=======" << std::endl;
    experiment.RunStep();
  }


  // dirdevo::ProbabilisticScheduler scheduler(rnd, 10, 30);
  // scheduler.AdjustWeight(2, 10);
  // scheduler.AdjustWeight(3, 5);
  // std::cout << scheduler.UpdateSchedule() << std::endl;
  // std::cout << scheduler.UpdateSchedule() << std::endl;

  return 0;
}
