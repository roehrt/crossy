import asyncio
import itertools
import pathlib
import random
import resource
import shutil
import subprocess
import sys
import tempfile
import time

import networkx as nx

GB = 1024 ** 3


def limits():
    resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))
    resource.setrlimit(resource.RLIMIT_AS, (GB, GB))
    resource.setrlimit(resource.RLIMIT_CPU, (60, 60))


def count_crossings(graph: pathlib.Path, solution: pathlib.Path):
    result = subprocess.run(['pace2024verifier', '-c', graph, solution], capture_output=True, text=True)
    if result.returncode != 0:
        print(f"Verifier failed: {result.stderr.strip()}", file=sys.stderr)
        return -1
    return int(result.stdout.strip())


def brute_force(graph: pathlib.Path):
    with open(graph) as f:
        _p, _ocr, n0, n1, m = f.readline().split()

    n0 = int(n0)
    n1 = int(n1)
    best = float('inf')

    with tempfile.TemporaryDirectory() as tmpdir:
        tmpdir = pathlib.Path(tmpdir)
        for p in itertools.permutations(range(n0 + 1, n0 + n1 + 1)):
            solution = tmpdir / 'solution'
            with open(solution, 'w') as f:
                f.write('\n'.join(map(str, p)))
            crossings = count_crossings(graph, solution)
            best = min(best, crossings)

    return best


def random_bipartite_graph(n0, n1, p, seed=None):
    G = nx.bipartite.random_graph(n0, n1, p, seed=seed)
    G = nx.relabel_nodes(G, {node: node + 1 for node in G.nodes})
    return G


async def fuzz(executable: pathlib.Path, timeout=float('inf'), big=False):
    n0_hi = 10000 if big else 50
    n1_hi = 100 if big else 5

    fuzzed = 0

    start = time.time()
    while time.time() - start < timeout:
        seed = random.randint(0, 2 ** 32 - 1)
        random.seed(seed)
        n0 = random.randint(1, n0_hi)
        n1 = random.randint(1, n1_hi)
        p = random.gauss(0.1, 0.3)
        G = random_bipartite_graph(n0, n1, p, seed=seed)
        with tempfile.TemporaryDirectory() as tmpdir:
            tmpdir = pathlib.Path(tmpdir)
            graph = tmpdir / 'graph'
            with open(graph, 'w') as f:
                f.write(f'p ocr {n0} {n1} {len(G.edges)}\n')
                f.write('\n'.join(f'{u} {v}' for u, v in G.edges))
            solver_solution = tempfile.NamedTemporaryFile(mode='w', delete=False)
            runner = await asyncio.create_subprocess_shell(str(executable), preexec_fn=limits,
                                                           stdin=open(graph), stdout=solver_solution,
                                                           stderr=asyncio.subprocess.DEVNULL)
            rt_code = await runner.wait()
            if rt_code != 0:
                print(f"Solver failed with code {rt_code}", file=sys.stderr)
                shutil.copy(graph, f'failed_testcase-{seed}.gr')
            if not big:
                jury = brute_force(graph)
                solver = count_crossings(graph, pathlib.Path(solver_solution.name))
                if jury != solver:
                    print(f"Jury: {jury}, Solver: {solver}")
                    shutil.copy(graph, f'failed_testcase-{seed}.gr')
                    shutil.copy(pathlib.Path(solver_solution.name), f'failed_testcase-{seed}.sol')
            fuzzed += 1
            print(f"Fuzzed {fuzzed} testcases")


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='fuzz')
    parser.add_argument('executable', type=pathlib.Path)
    parser.add_argument('-t', '--timeout', type=int, default=float('inf'), help='Timeout in seconds')
    parser.add_argument('--big', action='store_true', help='Run big testcases')
    args = parser.parse_args()

    asyncio.run(fuzz(args.executable, args.timeout, args.big))
