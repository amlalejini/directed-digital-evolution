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
    - world population snapshots
    - world summary
    - systematics
  - implement more interesting organisms / tasks
  - allow for EC-style evolution (sync gens, individual evaluation), have different ECWorld that experiment configures?
- AvidaGP L9
  - Add more tasks? (e.g., arithmetic that uses different 'resources' and io receptors)
    - e.g., allow for: does rewarding different tasks (that we don't necessarily care about) improve actual objective performance (e.g., equ)?
  - Allow for instruction
  - Rebrand brom AvidaGPL9 to just AvidaGP?
  - Add support for multiple IO channels
    - Will need a task set, environment bank for each channel; need io buffers for each channel; need to track performances by channel
- Nice things
  - Refactor each components responsibilities => now that things are more fleshed out, does everyone's roles make sense? e.g., does it make sense for the organism to be responsible for generating ancestral genotypes? should the world's task be renamed something else (it's more than just a task => it manages all the specifics of the particular setup).
  - Isolate bug in IndexMap where you provide an initial weight(?)
  - Write tests!
  - Turn on documentation
  - Design a web interface?
  - Allow configurations to be merged? => Inherit from one another? e.g., have a base dirdevo config that all experiment setup configs inherit from?