/*
 *  Copyright (C) 2006 Dip. Ing. dell'Informazione, University of Pisa, Italy
 *  http://info.iet.unipi.it/~cng/ns2measure/ns2measure.html
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA, USA
 */

/**
   project: measure
   filename: stat.cc
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           body of statistical functions and classes
*/

#include <stat.h>

double Stat::t_table[30][4] = {
    {6.314, 12.706, 25.452, 63.657}, {2.920, 4.303, 6.205, 9.925},
    {2.353, 3.182, 4.177, 5.841},    {2.132, 2.776, 3.495, 4.604},
    {2.015, 2.571, 3.163, 4.032},    {1.943, 2.447, 2.969, 3.707},
    {1.895, 2.365, 2.841, 3.499},    {1.860, 2.306, 2.752, 3.355},
    {1.833, 2.262, 2.685, 3.250},    {1.812, 2.228, 2.634, 3.169},
    {1.796, 2.201, 2.593, 3.106},    {1.782, 2.179, 2.560, 3.055},
    {1.771, 2.160, 2.533, 3.012},    {1.768, 2.145, 2.510, 2.977},
    {1.753, 2.131, 2.490, 2.947},    {1.746, 2.120, 2.473, 2.921},
    {1.740, 2.110, 2.458, 2.898},    {1.734, 2.101, 2.445, 2.878},
    {1.729, 2.093, 2.433, 2.861},    {1.725, 2.086, 2.423, 2.845},
    {1.721, 2.080, 2.414, 2.831},    {1.717, 2.074, 2.405, 2.819},
    {1.714, 2.069, 2.398, 2.807},    {1.711, 2.064, 2.391, 2.797},
    {1.708, 2.060, 2.385, 2.787},    {1.706, 2.056, 2.379, 2.779},
    {1.703, 2.052, 2.373, 2.771},    {1.701, 2.048, 2.368, 2.763},
    {1.699, 2.045, 2.364, 2.756},    {1.697, 2.042, 2.360, 2.750}};

double Stat::t_student(double cl, int df) {
  if (cl <= .9) {
    if (df > 30)
      return 1.65;
    else
      return t_table[df - 1][0];
  } else if (cl <= .95) {
    if (df > 30)
      return 1.96;
    else
      return t_table[df - 1][1];
  } else if (cl <= .975) {
    if (df > 30)
      return -1; // TODO: check this value for df = infty
    else
      return t_table[df - 1][2];
  } else {
    if (df > 30)
      return 2.58;
    else
      return t_table[df - 1][3];
  }
}

double Stat::mean(bool& valid, const std::vector<sample_t>& samples) {
  double             avg = 0.0;
  const unsigned int n   = samples.size(); // alias for the number of samples

  // validate input
  if (n < 1) {
    valid = false;
    return -1.0;
  }
  valid = true;

  // compute the mean
  for (unsigned int i = 0; i < n; i++) {
    avg += samples[i];
  }
  avg /= double(n);

  return avg;
}

double Stat::confInterval(bool&                        valid,
                          const std::vector<sample_t>& samples,
                          double                       cl) {
  double             avg      = 0.0;
  double             variance = 0.0;
  const unsigned int n = samples.size(); // alias for the number of samples

  // validate input
  if ((cl > 0 && cl < 1 && n <= 1) || (cl == 2 && n <= 1)) {
    valid = false;
    return -1.0;
  }
  valid = true;

  avg = mean(valid, samples);

  // compute the sample variance
  for (unsigned int i = 0; i < n; i++) {
    variance += (samples[i] - avg) * (samples[i] - avg);
  }
  variance /= n - 1.0;

  // return the half confidence interval
  if (cl == 2)
    return sqrt(variance) / 2.0;
  return t_student(cl, n - 1) * sqrt(variance / double(n));
}
