#pragma once
#ifndef DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE
#define DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE

#include "emp/config/config.hpp"

namespace dirdevo {

EMP_BUILD_CONFIG(DirectedDevoConfig,
  GROUP(GLOBAL_SETTINGS, "Global settings"),
  VALUE(SEED, int, -1, "Seed for a simulation"),
  VALUE(NUM_POPS, size_t, 1, "Number of populations. Must be > 0"),
  VALUE(EPOCHS, size_t, 100, "Number of iterations of population-level selection to perform."),
  GROUP(LOCAL_WORLD_SETTINGS, "Settings for each local population (world)"),
  VALUE(AVG_STEPS_PER_ORG, size_t, 30, "On average, how many steps per organism do we allot on each world update? Must be >= 1."),
  VALUE(UPDATES_PER_EPOCH, size_t, 100, "How many updates should we run each local population for during an period of evolution?"),
  VALUE(LOCAL_POP_STRUCTURE, std::string, "mixed", "Options: mixed, grid, grid3d"),
  VALUE(LOCAL_GRID_WIDTH, size_t, 10, "Grid width"),
  VALUE(LOCAL_GRID_HEIGHT, size_t, 10, "Grid height"),
  VALUE(LOCAL_GRID_DEPTH, size_t, 10, "Grid depth (only used in grid3d mode)"),
  // GROUP(ONEMAX_ORG_SETTINGS, "Settings specific to the onemax organism"),
  GROUP(BITSET_GENOME_SETTINGS, "Settings specific to bitset genomes"),
  VALUE(BITSET_MUTATOR_PER_SITE_SUBSTITUTION_RATE, double, 0.01, "Per-site substitution rate for bitset genomes")
);

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE