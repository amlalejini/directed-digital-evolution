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


  // emp::IndexMap weight_map;
  // weight_map.ResizeClear(10);
  // weight_map.Adjust(2, 1);
  // std::cout << "weight map size = " << weight_map.GetSize() << std::endl;
  // std::cout << "Weight map weight = " << weight_map.GetWeight() << std::endl;
  // std::cout << "prob of id 2 = " << weight_map.GetProb(2) << std::endl;
  // std::cout << "prob of id 0 = " << weight_map.GetProb(0) << std::endl;
  // emp::Random rnd(cfg.SEED());
  // size_t total_weight = weight_map.GetWeight();
  // for (size_t i = 0; i < 100; ++i) {
  //   size_t selected = weight_map.Index(rnd.GetDouble() * total_weight);
  //   std::cout << "  - " << selected << std::endl;
  // }

  // dirdevo::ProbabilisticScheduler scheduler(rnd, 10, 30);
  // scheduler.AdjustWeight(2, 10);
  // scheduler.AdjustWeight(3, 5);
  // std::cout << scheduler.UpdateSchedule() << std::endl;
  // std::cout << scheduler.UpdateSchedule() << std::endl;

  return 0;
}
