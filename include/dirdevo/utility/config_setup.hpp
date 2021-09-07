#pragma once

#include <filesystem>

#include "emp/config/ArgManager.hpp"
#include "emp/prefab/ConfigPanel.hpp"
#include "emp/web/UrlParams.hpp"
#include "emp/web/web.hpp"

#include "dirdevo/DirectedDevoConfig.hpp"

void use_existing_config_file(dirdevo::DirectedDevoConfig & config, emp::ArgManager & am) {
  if(std::filesystem::exists("directed-digital-evolution.cfg")) {
    std::cout << "Configuration read from directed-digital-evolution.cfg" << "\n";
    config.Read("directed-digital-evolution.cfg");
  }
  am.UseCallbacks();
  if (am.HasUnused())
    std::exit(EXIT_FAILURE);
}

void setup_config_web(dirdevo::DirectedDevoConfig & config)  {
  auto specs = emp::ArgManager::make_builtin_specs(&config);
  emp::ArgManager am(emp::web::GetUrlParams(), specs);
  use_existing_config_file(config, am);
}

void setup_config_native(dirdevo::DirectedDevoConfig & config, int argc, char* argv[]) {
  auto specs = emp::ArgManager::make_builtin_specs(&config);
  emp::ArgManager am(argc, argv, specs);
  use_existing_config_file(config, am);
}

