import argparse
import pathlib

parser = argparse.ArgumentParser()
parser.add_argument("src", type=pathlib.Path)
parser.add_argument("dst", type=pathlib.Path)
args = parser.parse_args()

args.dst.mkdir(parents=True, exist_ok=True)
for testcase in args.src.glob("*.gr"):
    with open(testcase) as f:
        lines = f.readlines()

    for i, line in enumerate(lines):
        if line.startswith("p ocr"):
            n0, n1, m = map(int, line.split()[2:5])
            break
    else:
        raise ValueError("p ocr not found")

    with open(args.dst / testcase.name, "w") as f:
        f.write(f"p ocr {n0} {n1} {m}\n")
        f.writelines(lines[i+1+n0+n1:])
