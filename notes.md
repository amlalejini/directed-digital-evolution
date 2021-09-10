# Notes

## Questions / ideas

- allow for different sampling techniques / ways of forming propagules?
  - each propagules comes from a single population?
  - each propagule is a mixture of all selected populations?
  - somewhere in the middle where we do 'propagule' crossover?
    - HYPOTHESIS: this won't work all that well usually => within-population competition will probably wipe out one of the clades; but, if a stable ecology formed, population-level selection would

## TODO

- Simulation
  - Clean up task terminology
  - implement data tracking
  - implement more interesting organisms / tasks
  - allow for EC-style evolution (sync gens, individual evaluation), have different ECWorld that experiment configures?
- AvidaGP L9
  - Add more tasks? (e.g., arithmetic that uses different 'resources' and io receptors)
    - e.g., allow for: does rewarding different tasks (that we don't necessarily care about) improve actual objective performance (e.g., equ)?
  - Allow for task reward configuration
  - Allow for ancestor load
  - Allow for instruction
  - Rebrand brom AvidaGPL9 to just AvidaGP?
- Nice things
  - Isolate bug in IndexMap where you provide an initial weight(?)
  - Write tests!
  - Turn on documentation
  - Design a web interface?
  - Allow configurations to be merged? => Inherit from one another? e.g., have a base dirdevo config that all experiment setup configs inherit from?