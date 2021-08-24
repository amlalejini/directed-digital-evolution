#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE

#include "emp/config/config.hpp"

namespace dirdevo {

EMP_BUILD_CONFIG(DirectedDevoConfig,
  GROUP(GLOBAL_SETTINGS, "Global settings"),
  VALUE(SEED, int, -1, "Seed for a simulation"),
  VALUE(NUM_POPS, size_t, 1, "Number of populations. Must be > 0"),
  GROUP(LOCAL_WORLD_SETTINGS, "Settings for each local population (world)"),
  VALUE(LOCAL_POP_STRUCTURE, std::string, "mixed", "Options: mixed, grid, grid3d"),
  VALUE(LOCAL_GRID_WIDTH, size_t, 10, "Grid width"),
  VALUE(LOCAL_GRID_HEIGHT, size_t, 10, "Grid height"),
  VALUE(LOCAL_GRID_DEPTH, size_t, 10, "Grid depth (only used in grid3d mode)"),
);

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE