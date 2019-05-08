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
   filename: measure.h
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           definition of the main project classes
*/

#ifndef __MEASURE_MEASURE_H
#define __MEASURE_MEASURE_H

#include <config.h>
#include <stat.h>

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

//! A Population is a set of samples collected from the same sensor.
class Population
{
  //! Vector of samples. It is the population itself.
  std::vector<sample_t> population;

 public:
  //! Create an empty population.
  Population() {
  }
  //! The destructor does nothing.
  ~Population() {
  }

  //! Return the number of elements.
  unsigned int getSize() const {
    return population.size();
  }
  //! Add a sample to the population.
  void addSample(sample_t x);
  //! Return the i-th sample.
  sample_t getSample(bool& valid, unsigned int i);
  //! Return the mean of the population.
  double mean(bool& valid) {
    return Stat::mean(valid, population);
  }
  //! Return the confidence interval of the population.
  double confInterval(bool& valid, double cl) {
    return Stat::confInterval(valid, population, cl);
  }

  //! Debug function to print the values to an output stream.
  void dump(std::ostream& os);
};

//! An AvgMeasure is a set of populations for averaged metrics.
class AvgMeasure : public Object
{
  //! Array of populations.
  std::map<unsigned int, Population> populations;
  //! TODO
  std::map<unsigned int, Population>::iterator it;

 public:
  //! Create an emptry AvgMeasure.
  AvgMeasure()
      : Object("AvgMeasure") {
  }
  //! Do nothing.
  ~AvgMeasure() {
  }

  //! Add a sample to a population.
  void addSample(sample_t x, unsigned int id);
  //! Return the population of a given index.
  Population& getPopulation(unsigned int id);

  Population&  getPopulation();
  void         nextPopulation();
  void         restartPopulation();
  unsigned int getPopulationId();
  //! Return true if the population with a given index exists.
  bool getValid(unsigned int id);
  //! Return the number of populations in this measure.
  unsigned int getSize() const {
    return populations.size();
  }
};

//! A DstMeasure is a set of populations for distribution metrics.
/*!
  Each element in a DstMeasure is the probability that a given sample
  fits in a given bin. The size of the bin and the minimum
  value of the distribution are part of the DstMeasure
  data structure. In addition to the cumulative distribution function
  and the probability mass function, the mean value and quantiles
  are derived from the DstMeasure.

  The bin size and the mininmum data structure must be set before
  computing the quantile values. Also, samples must be added in
  bin order. If they are not, then the cumulative values will not
  be meaningful.
  */
class DstMeasure : public Object
{
  //! Array of array of populations.
  std::vector<std::vector<Population>> populations;
  //! Array of array of populations. Cumulative wrt the previous bins.
  /*!
    Note that the populations and populationsCDF always have the
    same size and structure. Thus, the valid structure is meaningful
    for both these data structures.
    */
  std::vector<std::vector<Population>> populationsCDF;
  //! Bit array to check if the i-th entry of populations is valid.
  std::vector<std::vector<bool>> valid;
  //! Bin size.
  sample_t binSize;
  //! Distribution lower bound.
  sample_t distLower;
  //! True if the bin size has been set.
  bool binSizeSet;
  //! True if the distribution lower bound has been set.
  bool distLowerSet;

  //! Populations of average values.
  std::vector<Population> meanPopulations;
  //! Populations of median values.
  std::vector<Population> medianPopulations;
  //! Populations of 95th percentile values.
  std::vector<Population> percentile95Populations;
  //! Populations of 99th percentile values.
  std::vector<Population> percentile99Populations;
  //! Record the last size of the bins populations.
  std::vector<unsigned int> derivedLast;

 public:
  //! Create an emptry DstMeasure.
  DstMeasure()
      : Object("DstMeasure") {
  }
  //! Do nothing.
  ~DstMeasure() {
  }

  //! Add a sample to a population bin.
  void addSample(sample_t x, unsigned int id, unsigned int bin);
  //! Return the population of a given index/bin.
  Population& getPopulation(unsigned int id, unsigned int bin);
  //! Return the CDF population of a given index/bin.
  Population& getPopulationCDF(unsigned int id, unsigned int bin);
  //! Return the mean population.
  Population& getMeanPopulation(unsigned int id);
  //! Return the mediam population.
  Population& getMedianPopulation(unsigned int id);
  //! Return the 95th percentile population.
  Population& getPercentile95Population(unsigned int id);
  //! Return the 99th percentile population.
  Population& getPercentile99Population(unsigned int id);

  //! Compute the derived statistics (mean, quantiles) if not already done.
  void computeDerivedStatistics(unsigned int id);

  //! Return true if the population with a given index exists.
  bool getValid(unsigned int id, unsigned int bin);
  //! Return the number of populations in this measure.
  unsigned int getSize() const {
    return populations.size();
  }
  //! Return the number of bins in the given index.
  unsigned int getSize(unsigned int id);
  //! Set the bin size.
  void setBinSize(sample_t s) {
    binSize    = s;
    binSizeSet = true;
  }
  //! Set the distribution lower bound.
  void setDistLower(sample_t s) {
    distLower    = s;
    distLowerSet = true;
  }
  //! Get the bin size.
  sample_t getBinSize() const {
    return binSize;
  }
  //! Get the distribution lower bound.
  sample_t getDistLower() const {
    return distLower;
  }
};

//! A Metrics object contain all the AvgMeasure and DstMeasure objects.
class Metrics : public Object
{
  std::map<std::string, AvgMeasure> avgMeasures;
  std::map<std::string, DstMeasure> dstMeasures;

 public:
  //! Create an empty Metrics object.
  Metrics()
      : Object("Metrics") {
  }
  //! Do nothing.
  ~Metrics() {
  }

  //! Add a sample to an averaged measure.
  void addSample(std::string m, sample_t x, unsigned int id);
  //! Add a sample to a distribution measure.
  void addSample(std::string m, sample_t x, unsigned int id, unsigned int bin);
  //! Set the bin size of a distribution measure.
  void setBinSize(std::string m, sample_t binSize);
  //! Set the lower bound of a distribution measure.
  void setDistLower(std::string m, sample_t distLower);

  //! Return the set of average measures.
  std::map<std::string, AvgMeasure>& getAvgMeasures() {
    return avgMeasures;
  }
  //! Return the set of distribution measures.
  std::map<std::string, DstMeasure>& getDstMeasures() {
    return dstMeasures;
  }

  //! Check the confidence level of a set of averaged metrics.
  bool checkConfidence(std::set<std::string>& metrics, double cl, double th);

  //! Print the content of the Metrics object into files on a directory.
  void dump(std::string savedir, std::string hdr, double cl, bool dist);

  //! Debug function to dump the content of the Metrics object into a stream.
  void dump(std::ostream& os, double cl, bool dist);
};

#endif // __MEASURE_MEASURE_H
