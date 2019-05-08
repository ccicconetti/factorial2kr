#!/usr/bin/env python3
"""
Compute effects and confidence intervals of a factorial 2kr design.
"""

__author__ = "Claudio Cicconetti"
__copyright__ = "Copyright 2019"
__credits__ = ["Claudio Cicconetti"]
__license__ = "GPL"
__version__ = "1.0.1"
__maintainer__ = "Claudio Cicconetti"
__status__ = "Prototype"

import argparse
import math
import os
import sys

import statsmodels.stats.api as sms


class Observations:
    """
    Collect observations from experiment, compute effects and confidence intervals
    """

    def __init__(self, input_file):
        "Load data from file"

        self.data = dict()
        self.k = 0
        self.r = 0
        with open(input_file, 'r') as infile:
            cnt = 0
            for line in infile:
                values = line.rstrip().split()
                self.data[cnt] = []
                for v in values:
                    self.data[cnt].append(float(v))
                if self.r == 0:
                    self.r = len(values)
                else:
                    if self.r != len(values):
                        raise Exception("Invalid input at line {}".format(cnt+1))
                cnt += 1

        self.k = int(round(math.log2(cnt)))

        if self.r == 0 or self.k == 0 or (2 ** self.k) != cnt:
            raise Exception("Invalid observations from " + input_file)

        print("observation file valid: k = {}, r = {}".format(self.k, self.r))

    @staticmethod
    def sign_table(k):
        "Save a sign table for k parameters in a multi-line string"

        ret = ''
        for pos in range(0, k):
            ret += chr(ord('A') + pos)
        ret += '\n'

        num_entries = 2 ** k - 1
        for ndx in range(0, num_entries):
            for pos in range(0, k):
                if (ndx >> (k - pos - 1) & 1) == 1:
                    ret += '+'
                else:
                    ret += '-'
            if ndx < (num_entries - 1):
                ret += '\n'

        return ret

    def analyze(self):
        "Perform analysis on data"

        estimated_response = dict()
        for ndx, values in self.data.items():
            total = 0.0
            for v in values:
                total += v
            estimated_response[ndx] = total / self.r

        # sum of squared errors
        sse = 0.0 
        for ndx, values in self.data.items():
            for v in values:
                sse += (v - estimated_response[ndx]) ** 2

        print(sse)

parser = argparse.ArgumentParser(
        "Compute effects and confidence intervals of a factorial 2kr design",
        formatter_class=argparse.ArgumentDefaultsHelpFormatter)
parser.add_argument(
        "--k", type=int, default=2,
        help="Number of parameters")
parser.add_argument(
        "--sign_matrix", action="store_true", default=False,
        help="Only print the sign matrix")
parser.add_argument(
        "--alpha", type=float, default=0.05,
        help="Confidence level")
parser.add_argument(
        "infile", nargs="?", help="input file with observations")
args = parser.parse_args()

assert 0 < args.alpha < 1
assert args.k <= 26

if args.sign_matrix:
    print(Observations.sign_table(args.k))
    sys.exit(0)

if not args.infile:
    raise Exception("Missing input file with observations")

if not os.path.isfile(args.infile):
    raise Exception("Input file does not exist: " + args.infile)

observations = Observations(args.infile)
observations.analyze()

sys.exit(0)
