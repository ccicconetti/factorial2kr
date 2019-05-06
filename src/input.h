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
   filename: input.h
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           definition of input classes and functions
*/

/*
        communication protocol NS2 -> measure program

        unsigned int = UIN
        double = DBL
        char = CHR

        type  data
        UIN   run identifier
        UIN   no. of averaged metrics
 |-UIN   no. of indices of metric j = nj
 |	UIN   length of the name of the metric = lenj (including '\0')
j|	CHR   name of the metric, of length lenj
 |	UIN   index of the i-th sample of metric j (i=0,1,..,nj-1)  -| nj times
 |-DBL   i-th sample of metric j                               -|
   UIN   no. of distribution metrics
 |-UIN   no. of indices of the metric = mj
 |	UIN   length of the name of the metric = lenj (including '\0')
 |	CHR   name of the metric, of length lenj
 | DBL   bin size
 | DBL   lower bound of the distribution
j| UIN   number of bins bj
 | UIN   index of the i-th distribution of metric j (i=0,1,..,mj-1)  -| mj times
 | DBL   first sample 0                                               |
 | DBL   second sample 1                                              |
 | DBL   ..                                                           |
 |-DBL   last sample bj-1                                            -|

*/

#ifndef __MEASURE_INPUT_H
#define __MEASURE_INPUT_H

#include <config.h>
#include <configuration.h>
#include <measure.h>
#include <object.h>

#include <map>
#include <set>
#include <vector>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

//! Class for reading the input file according to the configuration.
class Input : public Object
{
  //! Configuration object used to parse input data.
  Configuration& configuration;
  //! Metrics database.
  Metrics& metrics;
  //! Set of run identifiers.
  std::set<unsigned int> runIdentifiers;

 public:
  //! Read a single run from an input file.
  /*!
    If fileOut != 0, the input is copied to fileOut. The recover flag
    is set to true if you want to gather all metrics for debugging or
    recovering purposes.
    If onlyAvg == true, the distribution metrics are not loaded in memory.
    If oneMetr != NULL the function loads in memory only the metric
    specified in this string.
    */
  bool readSingleRun(std::istream& fileIn,
                     std::ostream* fileOut = 0,
                     bool          recover = false,
                     bool          onlyAvg = false,
                     const char*   oneMetr = NULL);

  //! Create an empty Input object.
  Input(Configuration& c, Metrics& m)
      : Object("Input")
      , configuration(c)
      , metrics(m) {
  }
  //! Do nothing.
  ~Input() {
  }

  //! Reads data from a client.
  /*!
    fileIn and fileOut are the unix descriptors of the input and ouput
    files, respectively, which must have been already opened for
    reading and writing, respectively.

    This function returns when one of the following conditions becomes true:
    - the maximum number of replics has been reached
    - all the relevant metrics with check == true
      have a confidence interval below the threshold

Provided that:
    - the minimum number of replics has been reached

    This function also appends data read from fileIn to the outputfile
    specified in the configuration file.
    */
  void loadData(std::string fileIn, std::string fileOut);
  //! Load saved data and return true if the confidence level is reached.
  bool checkSavedData();
  //! Recover a (possibly damaged) save data file.
  bool recoverData(std::string saveFile,
                   bool        onlyAvg = false,
                   const char* oneMetr = NULL);
  //! Check whether the confidence level is reached. If so, return true.
  bool checkConfidence();
  //! Check if no more simulations are needed. If so, return true.
  bool check();
  //! Get the set of run identifiers.
  const std::set<unsigned int>& getRunIdentifiers() const {
    return runIdentifiers;
  }
};

#endif // __MEASURE_INPUT_H
