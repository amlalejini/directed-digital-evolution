# OneMax

Initial testing setup to scaffold the system.

Relevant types:
```{c++}
using org_t = dirdevo::OneMaxOrganism<256>;
using task_t = dirdevo::OneMaxTask<org_t>;
using mutator_t = dirdevo::BitSetMutator;
using world_t = dirdevo::DirectedDevoWorld<org_t,task_t>;
using experiment_t = dirdevo::DirectedDevoExperiment<world_t, org_t, mutator_t, task_t>;
```