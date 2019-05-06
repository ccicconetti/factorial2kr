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
   filename: configuration.h
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           definition of Configuration class
*/

#ifndef __MEASURE_CONFIGURATION_H
#define __MEASURE_CONFIGURATION_H

#include <config.h>
#include <measure.h>
#include <object.h>

#include <map>
#include <set>
#include <vector>

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

//! Descriptor for averaged metrics.
struct MetricDescAvg {
 public:
  //! False if this metric should be ignored.
  bool relevant;
  //! True if this metric generates an output sample.
  bool output;
  //! True if the confidence interval is checked for simulation termination.
  bool check;
  //! Output confidence level. Only meaningful if output == true.
  double outCL;
  //! Confidence level. Only meaningful if check == true.
  double CL;
  //! Confidence threshold. Only meaningful if check == true.
  double threshold;

  //! Create by default a non-relevant metric descriptor.
  MetricDescAvg()
      : relevant(false)
      , output(false)
      , check(false) {
  }
  //! Return true if this metric is relevant.
  bool isRelevant() const {
    return relevant;
  }
};

//! Descriptor for distribution metrics.
struct MetricDescDst {
 public:
  //! Descriptor for the Probability Mass Function.
  MetricDescAvg pmf;
  //! Descriptor for the Cumulative Distribution Function.
  MetricDescAvg cdf;
  //! Descriptor for the mean value.
  MetricDescAvg mean;
  //! Descriptor for he median value.
  MetricDescAvg median;
  //! Descriptor for the 95th percentile value.
  MetricDescAvg percentile95;
  //! Descriptor for the 99th percentile value.
  MetricDescAvg percentile99;

  //! Create by default a non-relevent metric descriptor.
  MetricDescDst() {
  }
  //! Return true if this metric is relevant.
  bool isRelevant() const {
    if (pmf.relevant == true || cdf.relevant == true || mean.relevant == true ||
        median.relevant == true || percentile95.relevant == true ||
        percentile99.relevant == true)
      return true;
    return false;
  }
};

//! One instance of this class stores information from the configuration file.
class Configuration : public Object
{
  //! Output file name.
  std::string outputFileName;
  //! Minimum number of replics. No minimum => 0.
  unsigned int minReplics;
  //! Maximum number of replics. No maximum => 0.
  unsigned int maxReplics;
  //! Header file name.
  std::string headerName;
  //! Trailer file name.
  std::string trailerName;
  //! Descriptors for averaged metrics.
  std::map<std::string, std::vector<MetricDescAvg>> avg;
  //! Descriptors for distribution metrics.
  std::map<std::string, std::vector<MetricDescDst>> dst;

  //! Insert an averaged metric descriptor.
  /*!
    The function allows a metric descriptor to be overriden.
    */
  void insert(std::string s, unsigned int id, const MetricDescAvg& dsc);
  //! Insert a distribution metric descriptor.
  /*!
    The function allows a metric descriptor to be overriden.
    The what argument specifies what submetric is to updated.
    */
  void insert(std::string          s,
              unsigned int         id,
              std::string          what,
              const MetricDescAvg& dsc);

  //! Get the next word from configuration file.
  /*!
    If a comment character '#' is found, the entire line is skipped.
    If the end of file is reached, std::string::npos is returned.
    If the next word is required, the an exception is thown if
    the end of file is reached.
    */
  std::string getNextWord(std::istream& is, bool required = false);

 public:
  //! Create an empty Configuration object.
  Configuration()
      : Object("Configuration")
      , minReplics(0)
      , maxReplics(0) {
  }
  //! Do nothing.
  ~Configuration() {
  }

  //! Parse the configuration file, and close it immediately after.
  void parse(std::string inputFileName);

  //! Get the output file name.
  std::string getOutputFileName() const {
    return outputFileName;
  }
  //! Get the minimum number of replics.
  unsigned int getMinReplics() const {
    return minReplics;
  }
  //! Get the maximum number of replics.
  unsigned int getMaxReplics() const {
    return maxReplics;
  }
  //! Get the header file name information.
  std::string getHeaderName() const {
    return headerName;
  }
  //! Get the trailer file name information.
  std::string getTrailerName() const {
    return trailerName;
  }
  //! Get the descriptor of an averaged metric.
  void getDescAvg(bool&          valid,
                  MetricDescAvg& dsc, // output
                  std::string    s,
                  unsigned int   id) { // input
    if (avg.count(s) == 0 || id >= avg[s].size())
      valid = false;
    else {
      valid = true;
      dsc   = avg[s][id];
    }
  }
  //! Get the descriptor of a distribution metric.
  void getDescDst(bool&          valid,
                  MetricDescDst& dsc, // output
                  std::string    s,
                  unsigned int   id) { // input
    if (dst.count(s) == 0 || id >= dst[s].size())
      valid = false;
    else {
      valid = true;
      dsc   = dst[s][id];
    }
  }

  //! Debug function to dump to an ostream the database content.
  void dump(std::ostream& os);
};

#endif // __MEASURE_CONFIGURATION_H
