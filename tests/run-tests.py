import argparse
import asyncio
import os
import pathlib
import resource
import subprocess
import tempfile
import time

import emoji
import natsort
import termcolor
import tqdm
import tqdm.asyncio

GB = 1024 ** 3


def set_limits(heuristic=False):
    timeout = 300 if heuristic else 1800

    def limits():
        resource.setrlimit(resource.RLIMIT_STACK, (resource.RLIM_INFINITY, resource.RLIM_INFINITY))
        resource.setrlimit(resource.RLIMIT_AS, (8 * GB, 8 * GB))
        resource.setrlimit(resource.RLIMIT_CPU, (timeout, timeout))

    return limits


class MyLogger:
    def __init__(self, prefix: str, verbose: bool):
        self.verbose = verbose
        self.prefix = prefix

    def info(self, message: str):
        self.print(termcolor.colored(self.prefix + message, 'cyan'))

    def error(self, message: str):
        self.print(termcolor.colored(self.prefix + message, 'red'))

    def warning(self, message: str):
        self.print(termcolor.colored(self.prefix + message, 'yellow'))

    def success(self, message: str):
        self.print(termcolor.colored(self.prefix + message, 'green'))

    def debug(self, message: str):
        if self.verbose:
            self.print(termcolor.colored(self.prefix + message, 'magenta'))

    @staticmethod
    def print(message: str):
        tqdm.tqdm.write(message)


def count_crossings(graph: pathlib.Path, solution: pathlib.Path, logger: MyLogger):
    result = subprocess.run(['pace2024verifier', '-c', graph, solution], capture_output=True, text=True)
    if result.returncode != 0:
        logger.debug(f"Verifier failed: {result.stderr.strip()}")
        return -1
    return int(result.stdout.strip())


async def run_single(executable: pathlib.Path, testcase: pathlib.Path, solution: pathlib.Path,
                     heuristic: bool = False,
                     no_verifier: bool = False,
                     verbose: bool = False):
    start_time = time.time()
    logger = MyLogger(f"[{testcase.stem:>3}] ", verbose)
    logger.info(f"{emoji.emojize(':rocket:')} Running testcase")

    solver_solution = tempfile.NamedTemporaryFile(mode='w', delete=False)
    runner = await asyncio.create_subprocess_shell(str(executable), preexec_fn=set_limits(heuristic),
                                                   stdin=open(testcase), stdout=solver_solution,
                                                   stderr=asyncio.subprocess.DEVNULL)
    rt_code = await runner.wait()
    time_taken = time.time() - start_time
    if rt_code != 0:
        logger.error(
            f"{emoji.emojize(':cross_mark:')} Solver exited with non-zero exit code: {rt_code} (took {time_taken:.2f}s)")
        return False

    if no_verifier:
        logger.info(f"{emoji.emojize(':sparkles:')} No verifier enabled, skipping verification")
        return True
    solver_crossings = count_crossings(testcase, pathlib.Path(solver_solution.name), logger)
    if solver_crossings == -1:
        logger.error(f"{emoji.emojize(':cross_mark:')} Verifier failed")
        return False

    if not solution.exists():
        logger.warning(f"{emoji.emojize(':party_popper:')} No solution file found, skipping comparison")
        logger.success(
            f"{emoji.emojize(':writing_hand:')} Writing slvr solution ({solver_crossings} crossings) to {solution} (took {time_taken:.2f}s)")
        os.rename(solver_solution.name, solution)
        return True
    jury_crossings = count_crossings(testcase, solution, logger)
    if solver_crossings > jury_crossings:
        method = logger.warning if heuristic else logger.error
        em = ':warning:' if heuristic else ':cross_mark:'
        method(
            f"{emoji.emojize(em)} Suboptimal solution found: slvr={solver_crossings}, jury={jury_crossings} (took {time_taken:.2f}s)")
        return False
    if solver_crossings < jury_crossings:
        method = logger.success if heuristic else logger.warning
        em = ':check_mark_button:' if heuristic else ':warning:'
        method(
            f"{emoji.emojize(em)} Better solution found: slvr={solver_crossings}, jury={jury_crossings} (took {time_taken:.2f}s)")
        os.rename(solver_solution.name, solution)
        return True

    method = logger.info if heuristic else logger.success
    method(
        f"{emoji.emojize(':check_mark_button:')} Solution verified: {solver_crossings} crossings (took {time_taken:.2f}s)")
    os.unlink(solver_solution.name)
    return True


async def run_testcases(n, *jobs):
    semaphore = asyncio.Semaphore(n)
    pbar = tqdm.tqdm(total=len(jobs), leave=False, dynamic_ncols=True)
    active = set()

    def update_running():
        pbar.set_postfix_str(''.join(f"[{name}]" for name in sorted(active)))

    async def sem(*args):
        async with semaphore:
            name = args[1].stem
            active.add(name)
            update_running()
            result = await run_single(*args)
            active.remove(name)
            update_running()
            pbar.update(1)
            return result

    return await asyncio.gather(*(sem(*j) for j in jobs))


async def run(executable: pathlib.Path,
              test_dir: pathlib.Path,
              only_failed: bool = False,
              threads: int = 1,
              heuristic: bool = False,
              no_verifier: bool = False,
              verbose: bool = False):
    logger = MyLogger("", verbose)
    if not executable.exists() or not executable.is_file():
        logger.error(f"Executable {executable} not found")
        return
    if not test_dir.exists():
        logger.error(f"Test directory {test_dir} not found")
        return
    if test_dir.is_file():
        await run_single(executable, test_dir, test_dir.with_suffix('.sol'), heuristic, no_verifier, verbose)
        return
    queue = []
    for testfile in test_dir.glob('*.gr'):
        solution = testfile.with_suffix('.sol')
        if only_failed and solution.exists():
            continue
        queue.append((testfile, solution))
    queue = natsort.natsorted(queue)
    logger.info(f"Running {len(queue)} testcases")

    jobs = [(executable, testcase, solution, heuristic, no_verifier, verbose) for testcase, solution in queue]
    results = await run_testcases(threads, *jobs)
    passed = results.count(True)
    failed = results.count(False)
    if failed == 0:
        logger.success(f"All ({passed}) testcases passed! {emoji.emojize(':party_popper:')}")
    else:
        logger.error(f"{failed}/{passed + failed} ({failed / (passed + failed) * 100:.2f}%) testcases failed!")


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Run a pace executable')
    parser.add_argument('executable', type=pathlib.Path, help='The executable to run')
    parser.add_argument('test_dir', type=pathlib.Path, help='The directory with testcases')
    parser.add_argument('-f', '--only-failed', action='store_true', help='Only run testcases without a solution file')
    parser.add_argument('-j', '--jobs', type=int, default=1, help='Number of threads to use')
    parser.add_argument('-v', '--verbose', action='store_true', help='Print more information')
    parser.add_argument('--heuristic', action='store_true', help='Solver is a heuristic one')
    parser.add_argument('--no-verifier', action='store_true', help='Skip verification of solutions')
    args = parser.parse_args()
    asyncio.run(run(args.executable, args.test_dir, args.only_failed, args.jobs, args.heuristic,
                    args.no_verifier, args.verbose))
