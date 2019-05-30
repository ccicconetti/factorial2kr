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
import random
import sys

from scipy.stats import t, norm

class Observations:
    """
    Collect observations from experiment, compute effects and confidence intervals
    """

    def __init__(self, input_file, verbose, confidence):
        "Load data from file"

        self.verbose    = verbose
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

        # residual errors as a function of the predicted response
        self.residuals = {}

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
            ret += Observations.number_to_letter(1 << pos, k)
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
    def number_to_letter(number, k):
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

    @staticmethod
    def approx(x):
        "Return a string version of the input number with 4 significant digits"

        return str.format('{:.3}', x)

    @staticmethod
    def percent(x, latex_mode = False):
        "Return a string version of the input number suitable for percentages"

        if latex_mode:
            return str.format('{:.2}\\%', x * 100.0)
        else:
            return str.format('{:.2%}', x)

    def analyze(self):
        "Perform analysis on data"

        estimated_response = {}
        for ndx, values in self.data.items():
            total = 0.0
            for v in values:
                total += v
            estimated_response[ndx] = total / self.r

        # compute the SSE and save the residuals
        for ndx, values in self.data.items():
            for v in values:
                y = estimated_response[ndx]
                self.sse += (v - y) ** 2
                if y not in self.residuals:
                    self.residuals[y] = []
                self.residuals[y].append(v - y)

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

    def print_summary(self, brief):
        "Print to output a summary of the analysis"

        print('SSY {}\nSST {}\nSSE {} {}\nstd dev {}'.format(
            self.approx(self.ssy),
            self.approx(self.sst),
            self.approx(self.sse),
            self.percent(self.sse / self.sst),
            self.approx(self.std_dev)))
        for ndx in range(0, 2 ** self.k):
            letters = Observations.number_to_letter(ndx, self.k)
            if not letters:
                letters = '0'
            relative_importance = \
                self.ss_effects[ndx] / self.sst if ndx > 0 \
                else 0.0
            ci = t.interval(self.confidence, (2 ** self.k) * (self.r - 1), self.effects[ndx], self.std_dev)
            assert len(ci) == 2
            zero_cross_warning = '***' if ci[0] < 0.0 < ci[1] else ''
            if not brief or ndx == 0 or relative_importance >= 0.1:
                print('q{letter} {} SS{letter} {} {} ({}, {}) {}'.format(
                    self.approx(self.effects[ndx]),
                    self.approx(self.ss_effects[ndx]),
                    self.percent(relative_importance) if ndx > 0 else '',
                    self.approx(ci[0]),
                    self.approx(ci[1]),
                    zero_cross_warning,
                    letter=letters
                    ))

    def print_latex(self, brief):
        "Print to output a summary of the analysis to be included in a LaTeX document"

        print('\\begin{tabular}{|ll|lll|l|}\n\\hline')
        print(('SSY & {} & SST & {} & & \multirow{{2}}{{*}}{{conf intervals}} \\\\\n'
               'std dev & {} & SSE & {} & {} & \\\\').format(
            self.approx(self.ssy),
            self.approx(self.sst),
            self.approx(self.std_dev),
            self.approx(self.sse),
            self.percent(self.sse / self.sst, True)))
        print('\\hline')
        for ndx in range(0, 2 ** self.k):
            letters = Observations.number_to_letter(ndx, self.k)
            if not letters:
                letters = '0'
            relative_importance = \
                self.ss_effects[ndx] / self.sst if ndx > 0 \
                else 0.0
            ci = t.interval(self.confidence, (2 ** self.k) * (self.r - 1), self.effects[ndx], self.std_dev)
            assert len(ci) == 2
            zero_cross_warning = '*' if ci[0] < 0.0 < ci[1] else ''
            if not brief or ndx == 0 or relative_importance >= 0.1:
                print('q{letter} & {} & SS{letter} & {} & {} & ({}, {}) {} \\\\'.format(
                    self.approx(self.effects[ndx]),
                    self.approx(self.ss_effects[ndx]),
                    self.percent(relative_importance, True) if ndx > 0 else '',
                    self.approx(ci[0]),
                    self.approx(ci[1]),
                    zero_cross_warning,
                    letter=letters
                    ))
        print('\\hline\n\\end{tabular}')

    def save_residuals(self, filename):
        "Save the scatter plot of residuals to the given file"

        with open(filename, 'w') as outfile:
            for y, residuals in self.residuals.items():
                for err in residuals:
                    outfile.write('{} {}\n'.format(y, err))

    def save_qqnorm(self, filename):
        """
        Save the Q-Q normal plot of the residuals to the given file.

        We use the formula from the 'stats' R package, as also mentioned in Wikipedia:
        https://en.wikipedia.org/wiki/Normal_probability_plot
        """

        with open(filename, 'w') as outfile:
            residuals = []
            for values in self.residuals.values():
                for value in values:
                    residuals.append(value)
            residuals.sort()

            n = len(residuals)
            a = 3.0/8 if n <= 10 else 0.5
            quantiles = []
            for i in range(1, n + 1):
                quantiles.append(norm.ppf((i - a) / (n + 1 - 2 * a)))

            for x,y in zip(residuals, quantiles):
                outfile.write('{} {}\n'.format(x, y))

    @staticmethod
    def print_random(k, r, real_effects):
        "Print a sample input file generated with random observations"

        random.seed()

        for values in Observations.sign_matrix(k).values():
            coeffs = []
            for col in range(0, k):
                if Observations.number_to_letter(2 ** col, k) in real_effects \
                   and values[2 ** col] > 0:
                    coeffs.append(1.0)
                else:
                    coeffs.append(0.0)

            line = ''
            for _ in range(0, r):
                value = 0.0
                for c in coeffs:
                    value += c * 10  + random.random()

                line += '{} '.format(value)
            print(line)

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
        "--residuals", type=str, default='',
        help=("Name of the file where to save the scatter plot of residuals. "
              "Ignore trend if the magnitude of residuals is smaller than 1/10 "
              "of the magnitude of responses. If the trend goes up or down "
              "then there are other factors or side effects that are not "
              "accounted by the current analysis."))
parser.add_argument(
        "--qqnorm", type=str, default='',
        help=("Name of the file where to save the normal Q-Q plot of errors. "
              "The analysis is statistically significant if the plot is "
              "approximately linear."))
parser.add_argument(
        "--sign_matrix", action="store_true", default=False,
        help="Print the sign matrix and quit")
parser.add_argument(
        "--confidence", type=float, default=0.9,
        help="Confidence level")
parser.add_argument(
        "--verbose", action="store_true", default=False,
        help="Verbose output")
parser.add_argument(
        "--brief", action="store_true", default=False,
        help='Only show the factors with sufficient relative importance')
parser.add_argument(
        "--output_type", type=str, default='text',
        help='Select the output type, one of {text, latex, none}.')
parser.add_argument(
        "--random", type=str, default='',
        help=("Generate a random input file whose observations depend on "
              "the list of the given effects, e.g. --random AC means that "
              "only A and C parameters have an effect on the metric of interest"))
parser.add_argument(
        "infile", nargs="?", help="input file with observations")
args = parser.parse_args()

assert 0 < args.confidence < 1
assert args.k <= 26

if args.sign_matrix and args.random:
    raise Exception("Cannot specify both --sign_matrix and --random at the same time")

if args.output_type not in [ 'text', 'latex', 'none' ]:
    raise Exception("Choose one of {text, latex, none} as argument of --output-type")

if args.sign_matrix:
    print(Observations.sign_table(args.k))
    sys.exit(0)
elif args.random:
    Observations.print_random(args.k, 10, args.random)
    sys.exit(0)

if not args.infile:
    raise Exception("Missing input file with observations")

if not os.path.isfile(args.infile):
    raise Exception("Input file does not exist: " + args.infile)

observations = Observations(args.infile, verbose=args.verbose, confidence=args.confidence)
observations.analyze()

if args.output_type == 'text':
    observations.print_summary(args.brief)
elif args.output_type == 'latex':
    observations.print_latex(args.brief)

if args.residuals:
    observations.save_residuals(args.residuals)

if args.qqnorm:
    observations.save_qqnorm(args.qqnorm)

sys.exit(0)
