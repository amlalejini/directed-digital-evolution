//  This file is part of directed-digital-evolution
//  Copyright (C) Alexander Lalejini, 2021.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/base/vector.hpp"

#include "directed-digital-evolution/config_setup.hpp"
#include "directed-digital-evolution/DirectedDevoConfig.hpp"
#include "directed-digital-evolution/DirectedDevoExperiment.hpp"

// This is the main function for the NATIVE version of directed-digital-evolution.

dirdevo::DirectedDevoConfig cfg;

int main(int argc, char* argv[])
{
  // Set up a configuration panel for native application
  setup_config_native(cfg, argc, argv);
  cfg.Write(std::cout);

  std::cout << "Hello, world!" << "\n";

  return 0;
}
