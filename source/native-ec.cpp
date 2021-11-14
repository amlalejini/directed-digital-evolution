//  This file is part of directed-digital-evolution
//  Copyright (C) Alexander Lalejini, 2021.
//  Released under MIT license; see LICENSE

#include <iostream>

#include "emp/base/vector.hpp"
#include "emp/config/ArgManager.hpp"
#include "emp/config/command_line.hpp"

// AVIDAGP
#include "dirdevo/ExperimentSetups/AvidaGP/AvidaGPEC.hpp"

int main(int argc, char* argv[])
{

  std::string config_fname = "avidagp-ec-config.cfg";
  dirdevo::AvidaGPEvoCompConfig config;
  auto args = emp::cl::ArgManager(argc, argv);
  config.Read(config_fname);
  if (args.ProcessConfigOptions(config, std::cout, config_fname, "config-macros.h") == false) exit(0);
  if (args.TestUnknown() == false) exit(0); // If there are leftover args, throw an error.

  // Write to screen how the experiment is configured
  std::cout << "==============================" << std::endl;
  std::cout << "|    How am I configured?    |" << std::endl;
  std::cout << "==============================" << std::endl;
  config.Write(std::cout);
  std::cout << "==============================\n" << std::endl;

  dirdevo::AvidaGPEvoCompWorld world(config);
  world.Run();

  return 0;
}
