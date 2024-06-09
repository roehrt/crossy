# Build instructions

You need `gmp` to build the solver.
On Debian, you can install it with `sudo apt install libgmp-dev`.

## Setup the modified version of UWrMaxSat
1. Download [SCIP Optimization Suite 9.0.1](https://scipopt.org/download.php?fname=scipoptsuite-9.0.1.tgz) and extract it
   to `external/scipoptsuite-9.0.1`.
2. Run `setup-uwrmaxsat.sh` to download and build UWrMaxSat and its dependencies.

## Build the exact solver

1. Run `cmake -DSCIP_CPU=600 -DSCIP_CPU_ADD=600 -B build-exact -S .` to generate the build files.
2. Run `cmake --build build-exact` to build `crossy`.

## Build the parameterized solver

1. Run `cmake -DSCIP_CPU=0 -B build-parameterized -S .` to generate the build files.
2. Run `cmake --build build-parameterized` to build `crossy`.
