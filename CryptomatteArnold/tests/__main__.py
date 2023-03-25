import argparse
import sys
parser = argparse.ArgumentParser()

parser.add_argument(
    "-f",
    "--filter",
    dest="filter",
    default="",
    type=str,
    help="Wildcard enabled filter for test names (class or method names). Example: Cryptomatte*")

args = parser.parse_args()

if __name__ == '__main__':
    import tests
    if tests.run_arnold_tests(args.filter):  # means it returned the results, i.e. failure
        sys.exit()
