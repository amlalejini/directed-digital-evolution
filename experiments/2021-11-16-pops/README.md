# 2021-11-16 - Number of populations in the metapopulation

This experiment asks how much the number of populations in the metapopulation matters.

Population counts:

- 6
- 12
- 24
- 48
- 96
- 384
- 1536(?)

Notes

- Local memory demand:
  - 1,000 env bank size:
    - 96 pops: 1.5gb
    - 384 pops: 4GB (req 8)
  - 500 size
    - 1536: 12gb
  - 100 size
    - 1536: 10gb

Conditions:

- 0: {'NUM_POPS': '6', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 1: {'NUM_POPS': '6', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'lexicase', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 2: {'NUM_POPS': '6', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'non-dominated-elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 3: {'NUM_POPS': '6', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'none', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 4: {'NUM_POPS': '12', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 5: {'NUM_POPS': '12', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'lexicase', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 6: {'NUM_POPS': '12', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'non-dominated-elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 7: {'NUM_POPS': '12', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'none', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 8: {'NUM_POPS': '24', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 9: {'NUM_POPS': '24', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'lexicase', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 10: {'NUM_POPS': '24', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'non-dominated-elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 11: {'NUM_POPS': '24', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'none', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 12: {'NUM_POPS': '48', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 13: {'NUM_POPS': '48', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'lexicase', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 14: {'NUM_POPS': '48', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'non-dominated-elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 15: {'NUM_POPS': '48', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'none', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 16: {'NUM_POPS': '96', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 17: {'NUM_POPS': '96', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'lexicase', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 18: {'NUM_POPS': '96', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'non-dominated-elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 19: {'NUM_POPS': '96', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'none', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 20: {'NUM_POPS': '384', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 21: {'NUM_POPS': '384', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'lexicase', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 22: {'NUM_POPS': '384', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'non-dominated-elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 23: {'NUM_POPS': '384', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'none', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 24: {'NUM_POPS': '1536', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 25: {'NUM_POPS': '1536', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'lexicase', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 26: {'NUM_POPS': '1536', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'non-dominated-elite', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}
- 27: {'NUM_POPS': '1536', '_MULTI_PARAM_TIMING': '-OUTPUT_SUMMARY_UPDATE_RESOLUTION 200 -UPDATES_PER_EPOCH 200 -EPOCHS 2000', 'LOCAL_GRID_WIDTH': '25', 'LOCAL_GRID_HEIGHT': '40', 'SELECTION_METHOD': 'none', 'POPULATION_SAMPLING_SIZE': '10', 'AVIDAGP_ENV_FILE': 'environment-big.json', 'TOURNAMENT_SEL_TOURN_SIZE': '4', 'ANCESTOR_FILE': 'ancestor-100.gen'}