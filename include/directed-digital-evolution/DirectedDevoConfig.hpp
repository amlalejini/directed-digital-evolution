#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE

#include "emp/config/config.hpp"

namespace dirdevo {

EMP_BUILD_CONFIG(DirectedDevoConfig,
  GROUP(GLOBAL_SETTINGS, "Global settings"),
  VALUE(SEED, int, -1, "Seed for a simulation")
);

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE