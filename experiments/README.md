# Experiments

## Dependencies

Python dependencies for running job scripts + data aggregation/management scripts:

```bash
python3 -m venv pyenv
source pyenv/bin/activate
pip3 install -r requirements.txt
```

## 2021-09-22 - Initial Parameter Exploration

Fix total number of per-organism steps across conditions

- updates * epochs
  - 300 * 1000 = 300000 updates
  - 100 * 3000 = 300000 updates
  - 1000 * 300 = 300000 updates
- Timing estimates (laptop)
  - 48 pops, 30x30, 300updates per epoch: (504.275869sec / 10 epochs)

Knobs

- Replicates: x30(?)
- Axes:
  - Selection schemes [x7]: none (no select), random, elite, tournament, lexicase, non-dominated elite, non-dominated tournament
  - number of populations [x2]: 48, 96
    - 24, 48, 96 (well plate sizes)
  - updates per epoch [x3]: 100, 300, 1000
    - ~how many generations per round of selection makes sense?
    - updates 100, 300, 1000
  - world size [x2]: 30x30, 60x60
    - ~how many organisms per population?
    - 32x32?
    - 30x30? - clean proportions on sampling
  - average steps per organism: 30 (to match avida)
  - tournament size (where relevant): 8
  - number of elites: 1
  - population sampling size [x2]: 30%, 10%, 1%
    - what makes sense for a bottleneck size? (look at the LTEE?)
  - population structure: well-mixed
  - age limit: 20
- Backbone: [x7] selection schemes, 48 well plate, 300 updates per epoch, 10% sampling, 30x30

- Intuition-building:
  - 1000-update epochs, 32x32 world:
    - fills the grid, avg gens: 130 - 155
  - 500-update epochs, 32x32 world:
    - fills the grid, 60 - 80 generations
  - 300-update epochs, 32x32 world:
    - fills grid, avg gens = 30 to 50
  - 100-update epochs, 32x32 world
    - does not fill grid (700 - 950), avg gens = 10 - 15

For this, I'm going to just run the 7 different selection schemes at the backbone settings.