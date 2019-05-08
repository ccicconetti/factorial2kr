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
   filename: stat.h
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           statistical classes and functions
*/

#ifndef __MEASURE_STAT_H
#define __MEASURE_STAT_H

#include <config.h>
#include <object.h>

#include <cmath>
#include <vector>

//! Utility static class containing statistical functions.
class Stat : public Object
{
  //! Static table containing the t-student values.
  static double t_table[30][4];
  //! Function to access the t-student table.
  /*!
    Access to the t-student table through the number of degrees of
    freedom and the confidence level.
    Return -1.0 if the the number of the degrees of freedom is smaller
    than or equal to 1, or if the confidence level is outside [0, 1].
    */
  static double t_student(double cl, int df);

 public:
  //! Default constructor. Invoked once. Does nothing.
  Stat()
      : Object("Stat") {
  }
  //! Distructor. Does nothing.
  ~Stat() {
  }

  //! Return the confidence interval of a set of samples.
  /*!
    The validity bit is false if the number of samples is smaller
    than or equal to 1, or if the confidence level is outside [0, 1].
    */
  static double
  confInterval(bool& valid, const std::vector<sample_t>& samples, double cl);
  //! Return the mean of a set of samples.
  /*!
    The validity bit is false if the number of samples is zero.
    */
  static double mean(bool& valid, const std::vector<sample_t>& samples);
};

#endif // __MEASURE_STAT_H
