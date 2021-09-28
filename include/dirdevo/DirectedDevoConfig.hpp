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
  VALUE(LOAD_ANCESTOR_FROM_FILE, bool, false, "Should the ancestral genome be loaded from file? NOTE - the experiment setup must implement this functionality."),
  VALUE(ANCESTOR_FILE, std::string, "ancestor.gen", "Path to file containing ancestor genome to be loaded"),

  GROUP(OUTPUT_SETTINGS, "Settings specific to experiment output"),
  VALUE(OUTPUT_DIR, std::string, "output", "Where should the experiment dump output?"),
  VALUE(OUTPUT_COLLECT_WORLD_UPDATE_SUMMARY, bool, true, "Collect world update summary data?"),
  VALUE(OUTPUT_SUMMARY_EPOCH_RESOLUTION, size_t, 1, "Epoch resolution for recording summary data"),
  VALUE(OUTPUT_SUMMARY_UPDATE_RESOLUTION, size_t, 100, "Output resolution for recording summary data"),
  VALUE(OUTPUT_PHYLOGENY_SNAPSHOT_EPOCH_RESOLUTION, size_t, 10, "How often to output a snapshot of the phylogeny?"),
  VALUE(OUTPUT_SYSTEMATICS_EPOCH_RESOLUTION, size_t, 1, "Interval (in epochs) to output to systematics file"),
  VALUE(TRACK_SYSTEMATICS, bool, true, "Should we enable systematics tracking?"),

  GROUP(LOCAL_WORLD_SETTINGS, "Settings for each local population (world)"),
  VALUE(AVG_STEPS_PER_ORG, size_t, 30, "On average, how many steps per organism do we allot on each world update? Must be >= 1."),
  VALUE(UPDATES_PER_EPOCH, size_t, 100, "How many updates should we run each local population for during an period of evolution?"),
  VALUE(LOCAL_POP_STRUCTURE, std::string, "mixed", "Options: mixed, grid, grid3d"),
  VALUE(LOCAL_GRID_WIDTH, size_t, 10, "Grid width"),
  VALUE(LOCAL_GRID_HEIGHT, size_t, 10, "Grid height"),
  VALUE(LOCAL_GRID_DEPTH, size_t, 10, "Grid depth (only used in grid3d mode)"),

  GROUP(POPULATION_SELECTION_SETTINGS, "Settings for selecting populations to propagate"),
  VALUE(SELECTION_METHOD, std::string, "elite", "Which algorithm should be used to select populations to propagate? Options: elite, tournament"),
  VALUE(ELITE_SEL_NUM_ELITES, size_t, 1, "(elite selection) The top ELITE_SEL_NUM_ELITES populations are propagated"),
  VALUE(TOURNAMENT_SEL_TOURN_SIZE, size_t, 4, "(tournament selection) How large are tournaments?"),
  VALUE(POPULATION_SAMPLING_SIZE, size_t, 1, "How many genomes to sample from each population when forming propagules (after population selection)?"),

  GROUP(BITSET_GENOME_SETTINGS, "Settings specific to bitset genomes"),
  VALUE(BITSET_MUTATOR_PER_SITE_SUBSTITUTION_RATE, double, 0.01, "Per-site substitution rate for bitset genomes"),
  // GROUP(ONEMAX_ORG_SETTINGS, "Settings specific to the onemax organism"),

  GROUP(AVIDAGP_ORG_SETTINGS, "Settings specific to the AvidaGP organisms "),
  VALUE(AVIDAGP_ORG_AGE_LIMIT, size_t, 20, "Organisms die when instructions executed = AGE_LIMIT*length"),

  GROUP(AVIDAGP_MUTATION_SETTINGS, "Settings specific to AvidaGP mutation"),
  VALUE(AVIDAGP_MUT_RATE_INST_SUB, double, 0.01, "Instruction substitution rate (applied per-instruction)"),
  VALUE(AVIDAGP_MUT_RATE_ARG_SUB, double, 0.025, "Instruction argument substitution rate (applied per-argument)"),

  GROUP(AVIDAGP_ENV_SETTINGS, "Settings specific to AvidaGP environment/task"),
  VALUE(AVIDAGP_ENV_FILE, std::string, "environment.json", "Path to the environment file that specifies which tasks are rewarded at organism and world level")

);

} // namespace dirdevo

#endif // #ifndef DIRECTED_DEVO_DIRECTED_DEVO_CONFIG_HPP_INCLUDE