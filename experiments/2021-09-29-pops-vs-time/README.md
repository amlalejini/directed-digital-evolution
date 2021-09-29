# 2021-09-29 - Number of populations vs time per selection round

Conditions:

- Number of populations: 24, 48, 96
- Updates/epoch (time between selection events): 100, 300, 1000
  - Total epochs (maintain constant total time): 1800, 600, 100
- Selection schemes: elite, lexicase, pde, none

Commit hash: `d006f08a0d59d5b55e275d80ca045fbf8b9128d9`

## Notes

Using threaded version of the software to run this. As such, these experiments won't have phylogeny tracking!

## Job details

- mem=8gb
- cores=24 cores
- time=
  - 24: ?, 48: ?, 96: ?
-