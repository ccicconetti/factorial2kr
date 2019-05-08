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

from scipy.stats import t

class Observations:
    """
    Collect observations from experiment, compute effects and confidence intervals
    """

    def __init__(self, input_file, verbose, confidence):
        "Load data from file"

        self.verbose = verbose
        self.confidence = confidence

        # sum of squared errors
        self.sse = 0.0

        # effects, indexed by their appearance in the input file
        self.effects = {}

        # squared sum of effects
        self.ss_effects = {}

        # total variation, i.e. SST
        self.sst = 0.0

        # SSY
        self.ssy = 0.0

        # standard deviation of effects
        self.std_dev = 0.0

        # observations, loaded from file
        self.data = {}
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

        if self.verbose:
            print("observation file valid: k = {}, r = {}".format(self.k, self.r))

    @staticmethod
    def sign_table(k):
        "Return a sign table for k parameters in a multi-line string"

        ret = ''
        for pos in range(0, k):
            ret += chr(ord('A') + pos)
        ret += '\n'

        for values in Observations.sign_matrix(k).values():
            for col in range(0, k):
                ret += '+' if values[2 ** col] > 0 else '-'
            ret += '\n'

        return ret[:-1]

    @staticmethod
    def parallel_swar(number):
        "Return the parity of a 32-bit integer"

        number = number - ((number >> 1) & 0x55555555)
        number = (number & 0x33333333) + ((number >> 2) & 0x33333333)
        number = (((number + (number >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24
        return int(number % 2)

    @staticmethod
    def number_to_letters(number, k):
        "Return a string representing which combination of effects is considered"

        ret = ''
        for pos in range(0, k):
            if (number & (1 << pos)) != 0:
                ret += chr(ord('A') + pos)
        return ret

    @staticmethod
    def sign_matrix(k):
        "Return a sign table for k parameters as a square matrix with size 2^k"

        ret = {}

        for row in range(0, 2 ** k):
            ret[row] = {}

            for col in range(0, 2 ** k):
                if Observations.parallel_swar(row & col) == 0:
                    ret[row][col] = 1.0
                else:
                    ret[row][col] = -1.0

        return ret

    def analyze(self):
        "Perform analysis on data"

        estimated_response = {}
        for ndx, values in self.data.items():
            total = 0.0
            for v in values:
                total += v
            estimated_response[ndx] = total / self.r

        # compute the SSE
        for ndx, values in self.data.items():
            for v in values:
                self.sse += (v - estimated_response[ndx]) ** 2

        if self.verbose:
            for row, values in Observations.sign_matrix(self.k).items():
                line = '{}: '.format(row)
                for col, sign in values.items():
                    line += '({}){} '.format(col,sign)
                print(line)

        # compute the effects
        signs = Observations.sign_matrix(self.k)
        for col in range(0, 2 ** self.k):
            self.effects[col] = 0.0
            for row in range(0, 2 ** self.k):
                if self.verbose:
                    print('{} {} {}'.format(
                        col,
                        estimated_response[row],
                        signs[row][col]))
                self.effects[col] += estimated_response[row] * signs[row][col]
            self.effects[col] /= 2 ** self.k


        # compute the squared sum of effects, i.e., SS0, SSA, SSB, SSAB witk k = 2
        for ndx, effect in self.effects.items():
            self.ss_effects[ndx] = (2 ** self.k) * self.r * (effect ** 2)
            self.ssy += self.ss_effects[ndx]
        self.ssy += self.sse

        # compute SST = SSY - SS0
        self.sst = self.ssy - self.ss_effects[0]

        # compute the standard deviation of effects
        self.std_dev = math.sqrt(self.sse / (self.r * (self.r - 1))) / (2 ** self.k)

    def print_summary(self):
        "Print to output a summary of the analysis"

        print('SSY {}\nSST {}\nSSE {} {}%\nstd dev {}'.format(
            self.ssy,
            self.sst,
            self.sse,
            round(100.0 * self.sse / self.sst, 2),
            self.std_dev))
        for ndx in range(0, 2 ** self.k):
            letters = Observations.number_to_letters(ndx, self.k)
            if not letters:
                letters = '0'
            relative_importance = \
                100.0 * self.ss_effects[ndx] / self.sst if ndx > 0 \
                else 0.0
            ci = t.interval(self.confidence, (2 ** self.k) * (self.r - 1), self.effects[ndx])
            assert len(ci) == 2
            zero_cross_warning = ' ***' if ci[0] < 0.0 else ''
            print('q{} {} SS{} {} {}% {}{}'.format(
                letters,
                self.effects[ndx],
                letters,
                self.ss_effects[ndx],
                round(relative_importance, 2),
                (round(ci[0], 2), round(ci[1], 2)),
                zero_cross_warning
                ))

################################################################################
# Main body
#

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
        "--confidence", type=float, default=0.9,
        help="Confidence level")
parser.add_argument(
        "--verbose", action="store_true", default=False,
        help="Verbose output")
parser.add_argument(
        "infile", nargs="?", help="input file with observations")
args = parser.parse_args()

assert 0 < args.confidence < 1
assert args.k <= 26

if args.sign_matrix:
    print(Observations.sign_table(args.k))
    sys.exit(0)

if not args.infile:
    raise Exception("Missing input file with observations")

if not os.path.isfile(args.infile):
    raise Exception("Input file does not exist: " + args.infile)

observations = Observations(args.infile, verbose=args.verbose, confidence=args.confidence)
observations.analyze()
observations.print_summary()

sys.exit(0)
