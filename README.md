**Student submission** for the [PACE 2024][PACE2024] challenge.

`crossy` is an exact (and parameterized) solver for One-Sided Crossing Minimization (OCM)
based on [UWrMaxSat](https://github.com/marekpiotrow/UWrMaxSat) with
[CaDiCaL](https://github.com/arminbiere/cadical) and [SCIP Optimization Suite](https://scip.zib.de/).
Refer to the [challenge description][PACE2024] for more details about PACE and an overview of OCM.

For build instructions, see [BUILD.md](BUILD.md).

**Brief solver description:**

After applying a basic set of reduction rules, our solver reduces the problem to Weighted Directed Feedback Arc
Set (WDFAS) by constructing the penalty graph.
We employ the cycle propagation technique for solving WDFAS, proposed by the [winning team][PACE2022Winner] of the
[PACE 2022][PACE2022] challenge about Directed Feedback Vertex Set (DFVS).
To incorporate cycle propagation, we modify UWrMaxSat to connect a User Propagator to its internal SAT solver, CaDiCal,
using the recently introduced [IPASIR-UP][IPASIRUP] interface.

For the exact track, SCIP is run upfront for 600 seconds and extended by another 600 seconds if the integrality gap is
small.

**Solver Performance:**

All benchmarks were run on an Intel Core i7-11370H CPU.

- The exact solver is able to solve 91 out of 100 instances from the PACE 2024 public exact benchmark set within the time limit
of 1800 seconds per instance.
- The parameterized solver is able to solve all 125 instances from the PACE 2024 public parameterized benchmark set within
the time limit of 1800 seconds per instance. The average runtime is < 1 second.

[IPASIRUP]: https://doi.org/10.4230/LIPIcs.SAT.2023.8
[PACE2022Winner]: https://doi.org/10.1137/1.9781611977561.ch4
[PACE2022]: https://pacechallenge.org/2022/
[PACE2024]: https://pacechallenge.org/2024/
