# 2021-11-12 - Time

This experiments looks at trading off time between population-level selection events (i.e., time for evolution) and total number of selection events.
For each condition, we fix the total number of updates.
A greater number of selection events (with fewer updates between) should shift overall selection pressures toward the group level, whereas fewer selection events with more time between should shift overall selection pressures toward the individual level (but also allow for more genetic variation to build up in each population; i.e., more differentiation between selection events).

Selection schemes

- Elite
- Lexicase
- NDE
- None

Timing

| Updates / epoch | total epochs |
| --- | --- |
|     20          |   20,000 |
|     50          |   8,000 |
|     100         |   4,000 |
|     200         |   2,000 |
|     500         |   800 |
|     1000        |   400 |
|     2000        |   200 |