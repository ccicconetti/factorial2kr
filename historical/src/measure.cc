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
   filename: measure.cc
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           body of the main project classes
*/

#include <fstream>
#include <measure.h>
#include <sstream>

//
// class Population
//

void Population::addSample(sample_t x) {
  if (population.size() == population.capacity())
    population.reserve(population.size() + VECTOR_CHUNK_SIZE);
  population.push_back(x);
}

sample_t Population::getSample(bool& valid, unsigned int i) {
  if (i >= population.size()) {
    valid = false;
    return 0.0;
  }
  valid = true;
  return population[i];
}

void Population::dump(std::ostream& os) {
  for (unsigned int i = 0; i < population.size(); i++) {
    os << population[i];
    if (i < population.size() - 1)
      os << ", ";
  }
}

//
// class AvgMeasure
//

void AvgMeasure::addSample(sample_t x, unsigned int id) {
  populations[id].addSample(x);
}

Population& AvgMeasure::getPopulation(unsigned int id) {
  if (populations.find(id) == populations.end())
    throw *this;
  return populations[id];
}

Population& AvgMeasure::getPopulation() {
  return it->second;
}

unsigned int AvgMeasure::getPopulationId() {
  return it->first;
}

void AvgMeasure::nextPopulation() {
  ++it;
}

void AvgMeasure::restartPopulation() {
  it = populations.begin();
}

bool AvgMeasure::getValid(unsigned int id) {
  return populations.find(id) != populations.end();
}

//
// class DstMeasure
//

void DstMeasure::addSample(sample_t x, unsigned int id, unsigned int bin) {
  // static variable used to compute the cumulative distribution
  // this value is reset to 0.0 whenever the first bin (bin == 0)
  // is added to this DstMeasure object
  static sample_t cumulative = 0.0;
  if (bin == 0)
    cumulative = 0;

  // update the static variable cumulative
  cumulative += x;

  // check whether more entries must be added to the array of populations
  if (id >= populations.size()) {
    if (populations.size() == populations.capacity()) {
      const unsigned int n = populations.size();
      populations.reserve(n + VECTOR_CHUNK_SIZE);
      populationsCDF.reserve(n + VECTOR_CHUNK_SIZE);
      valid.reserve(n + VECTOR_CHUNK_SIZE);
    }
    populations.resize(id + 1);
    populationsCDF.resize(id + 1);
    valid.resize(id + 1);
  }

  if (bin >= populations[id].size()) {
    unsigned int oldSize = populations[id].size();
    if (populations[id].size() == populations[id].capacity()) {
      const unsigned int n = populations[id].size();
      populations[id].reserve(n + VECTOR_CHUNK_SIZE);
      populationsCDF[id].reserve(n + VECTOR_CHUNK_SIZE);
      valid[id].reserve(n + VECTOR_CHUNK_SIZE);
    }
    populations[id].resize(bin + 1);
    populationsCDF[id].resize(bin + 1);
    valid[id].resize(bin + 1);
    for (unsigned int i = oldSize; i < populations[id].size(); i++) {
      valid[id][i] = false;
    }
  }
  valid[id][bin] = true;
  populations[id][bin].addSample(x);
  populationsCDF[id][bin].addSample(cumulative);
}

Population& DstMeasure::getPopulation(unsigned int id, unsigned int bin) {
  if (id >= populations.size() || bin >= populations[id].size() ||
      valid[id][bin] == false)
    throw *this;
  return populations[id][bin];
}

Population& DstMeasure::getPopulationCDF(unsigned int id, unsigned int bin) {
  if (id >= populationsCDF.size() || bin >= populationsCDF[id].size() ||
      valid[id][bin] == false)
    throw *this;
  return populationsCDF[id][bin];
}

bool DstMeasure::getValid(unsigned int id, unsigned int bin) {
  if (id >= populations.size() || bin >= populations[id].size())
    throw *this;
  return valid[id][bin];
}

unsigned int DstMeasure::getSize(unsigned int id) {
  if (id >= populations.size())
    throw *this;
  return populations[id].size();
}

Population& DstMeasure::getMeanPopulation(unsigned int id) {
  if (id >= populations.size())
    throw *this;

  computeDerivedStatistics(id);
  return meanPopulations[id];
}

Population& DstMeasure::getMedianPopulation(unsigned int id) {
  if (id >= populations.size())
    throw *this;

  computeDerivedStatistics(id);
  return medianPopulations[id];
}

Population& DstMeasure::getPercentile95Population(unsigned int id) {
  if (id >= populations.size())
    throw *this;

  computeDerivedStatistics(id);
  return percentile95Populations[id];
}

Population& DstMeasure::getPercentile99Population(unsigned int id) {
  if (id >= populations.size())
    throw *this;

  computeDerivedStatistics(id);
  return percentile99Populations[id];
}

void DstMeasure::computeDerivedStatistics(unsigned int id) {
  // consistency check
  if (id >= populations.size() || !binSizeSet || !distLowerSet)
    throw *this;

  // check if we need to reallocate the derived metrics
  if (id >= derivedLast.size()) {
    unsigned int oldSize = derivedLast.size();
    if (derivedLast.size() == derivedLast.capacity()) {
      const unsigned int n = derivedLast.size();
      derivedLast.reserve(n + VECTOR_CHUNK_SIZE);
      meanPopulations.reserve(n + VECTOR_CHUNK_SIZE);
      medianPopulations.reserve(n + VECTOR_CHUNK_SIZE);
      percentile95Populations.reserve(n + VECTOR_CHUNK_SIZE);
      percentile99Populations.reserve(n + VECTOR_CHUNK_SIZE);
    }
    derivedLast.resize(id + 1);
    meanPopulations.resize(id + 1);
    medianPopulations.resize(id + 1);
    percentile95Populations.resize(id + 1);
    percentile99Populations.resize(id + 1);
    for (unsigned int i = oldSize; i < derivedLast.size(); i++) {
      derivedLast[i] = 0;
    }
  }

  // we do not compute the derived metrics whether one of the following
  // conditions is true:
  // - there are no bins with index id
  // - the derived metrics are up to date
  if (populations[id].size() == 0 ||
      populations[id][0].getSize() == derivedLast[id])
    return;

  //
  // otherwise, let us compute them all
  //

  // in the loop below, i is the run index
  for (unsigned int i = derivedLast[id]; i < populations[id][0].getSize();
       i++) {

    // mean
    sample_t mean  = 0.0;
    bool     valid = true;
    for (unsigned int j = 0; j < populations[id].size(); j++) {
      mean += populations[id][j].getSample(valid, i) *
              (distLower + binSize * (j + 1));
      if (valid == false)
        throw *this;
    }

    // quantiles
    double median       = 0.0;
    double percentile95 = 0.0;
    double percentile99 = 0.0;
    for (int j = populationsCDF[id].size() - 1; j >= 0; j--) {
      const sample_t x = populationsCDF[id][j].getSample(valid, i); // alias

      if (x > 0.50)
        median = distLower + binSize * (j + 1);
      if (x > 0.95)
        percentile95 = distLower + binSize * (j + 1);
      if (x > 0.99)
        percentile99 = distLower + binSize * (j + 1);
    }

    // push back the derived values
    meanPopulations[id].addSample(mean);
    medianPopulations[id].addSample(median);
    percentile95Populations[id].addSample(percentile95);
    percentile99Populations[id].addSample(percentile99);
  }

  // update the last population size for the derived metrics computation
  derivedLast[id] = populations[id][0].getSize();
}

//
// class Metrics
//

void Metrics::addSample(std::string m, sample_t x, unsigned int id) {
  avgMeasures[m].addSample(x, id);
}

void Metrics::addSample(std::string  m,
                        sample_t     x,
                        unsigned int id,
                        unsigned int bin) {
  dstMeasures[m].addSample(x, id, bin);
}

void Metrics::setBinSize(std::string m, sample_t binSize) {
  // if ( dstMeasures.count(m) == 0 ) throw *this;  // XXX check
  dstMeasures[m].setBinSize(binSize);
}

void Metrics::setDistLower(std::string m, sample_t distLower) {
  // if ( dstMeasures.count(m) == 0 ) throw *this;  // XXX check
  dstMeasures[m].setDistLower(distLower);
}

bool Metrics::checkConfidence(std::set<std::string>& metrics,
                              double                 cl,
                              double                 th) {
  // iterate over all the averaged measures
  std::map<std::string, AvgMeasure>::iterator it = avgMeasures.begin();
  for (; it != avgMeasures.end(); it++) {

    // skip non-relevant metrics
    if (metrics.count(it->first) == 0)
      continue;

    AvgMeasure& m = it->second; // alias
    m.restartPopulation();
    for (unsigned int i = 0; i < m.getSize(); i++) {
      bool        valid; // unused
      Population& p     = m.getPopulation();
      double      value = p.mean(valid);
      double      conf  = p.confInterval(valid, cl);
      if (p.getSize() == 1 || (value > 0 && (2.0 * conf) / value > th))
        return false;
      m.nextPopulation();
    }
  }
  return true;
}

void Metrics::dump(std::string savedir, std::string hdr, double cl, bool dist) {
  bool valid;

  // print all the averaged measures
  std::map<std::string, AvgMeasure>::iterator it = avgMeasures.begin();
  for (; it != avgMeasures.end(); it++) {

    // open the output file
    std::ofstream     os;
    std::string       filename;
    std::stringstream buf;
    buf << savedir << it->first << ".dat";
    getline(buf, filename);
    os.open(filename.c_str(), std::ios::out | std::ios::app);
    if (!os.is_open())
      throw *this;

    // print the values
    AvgMeasure& m = it->second; // alias
    m.restartPopulation();
    for (unsigned int i = 0; i < m.getSize(); i++) {
      Population& p = m.getPopulation();
      os << hdr << "," << m.getPopulationId() << "," << p.mean(valid) << ","
         << p.confInterval(valid, cl) << '\n';
      m.nextPopulation();
    }

    // close the output file
    os.close();
  }

  if (!dist)
    return;

  // print all the distribution measures
  std::map<std::string, DstMeasure>::iterator jt = dstMeasures.begin();
  for (; jt != dstMeasures.end(); jt++) {
    DstMeasure& m = jt->second; // alias

    for (unsigned int i = 0; i < m.getSize(); i++) {

      {
        // open the output file
        std::ofstream     os;
        std::string       filename;
        std::stringstream buf;
        buf << savedir << jt->first << "_pmf.dat";
        getline(buf, filename);
        os.open(filename.c_str(), std::ios::out | std::ios::app);
        if (!os.is_open())
          throw *this;

        // print the p.m.f.
        for (unsigned int j = 0; j < m.getSize(i); j++) {
          if (!m.getValid(i, j))
            continue;
          sample_t x = m.getDistLower() + (j + 1.0) * m.getBinSize();
          os << hdr << "," << i << "," << x << ","
             << m.getPopulation(i, j).mean(valid) << ","
             << m.getPopulation(i, j).confInterval(valid, cl) << '\n';
        }

        // close the output file
        os.close();
      }

      {
        // open the output file
        std::ofstream     os;
        std::string       filename;
        std::stringstream buf;
        buf << savedir << jt->first << "_cdf.dat";
        getline(buf, filename);
        os.open(filename.c_str(), std::ios::out | std::ios::app);
        if (!os.is_open())
          throw *this;

        // print the c.d.f.
        for (unsigned int j = 0; j < m.getSize(i); j++) {
          if (!m.getValid(i, j))
            continue;
          sample_t x = m.getDistLower() + (j + 1.0) * m.getBinSize();
          os << hdr << "," << i << "," << x << ","
             << m.getPopulationCDF(i, j).mean(valid) << ","
             << m.getPopulationCDF(i, j).confInterval(valid, cl) << '\n';
        }

        // close the output file
        os.close();
      }

      // median
      if (m.getMedianPopulation(i).getSize() > 0) {
        // open the output file
        std::ofstream     os;
        std::string       filename;
        std::stringstream buf;
        buf << savedir << jt->first << "_q50.dat";
        getline(buf, filename);
        os.open(filename.c_str(), std::ios::out | std::ios::app);
        if (!os.is_open())
          throw *this;

        // print the median value
        os << hdr << "," << i << "," << m.getMedianPopulation(i).mean(valid)
           << "," << m.getMedianPopulation(i).confInterval(valid, cl) << '\n';

        // close the output file
        os.close();
      }

      // mean
      if (m.getMeanPopulation(i).getSize() > 0) {
        // open the output file
        std::ofstream     os;
        std::string       filename;
        std::stringstream buf;
        buf << savedir << jt->first << "_avg.dat";
        getline(buf, filename);
        os.open(filename.c_str(), std::ios::out | std::ios::app);
        if (!os.is_open())
          throw *this;

        // print the median value
        os << hdr << "," << i << "," << m.getMeanPopulation(i).mean(valid)
           << "," << m.getMeanPopulation(i).confInterval(valid, cl) << '\n';

        // close the output file
        os.close();
      }

      // 95th percentile
      if (m.getPercentile95Population(i).getSize() > 0) {
        // open the output file
        std::ofstream     os;
        std::string       filename;
        std::stringstream buf;
        buf << savedir << jt->first << "_95.dat";
        getline(buf, filename);
        os.open(filename.c_str(), std::ios::out | std::ios::app);
        if (!os.is_open())
          throw *this;

        // print the median value
        os << hdr << "," << i << ","
           << m.getPercentile95Population(i).mean(valid) << ","
           << m.getPercentile95Population(i).confInterval(valid, cl) << '\n';

        // close the output file
        os.close();
      }

      // 99th percentile
      if (m.getPercentile99Population(i).getSize() > 0) {
        // open the output file
        std::ofstream     os;
        std::string       filename;
        std::stringstream buf;
        buf << savedir << jt->first << "_99.dat";
        getline(buf, filename);
        os.open(filename.c_str(), std::ios::out | std::ios::app);
        if (!os.is_open())
          throw *this;

        // print the median value
        os << hdr << "," << i << ","
           << m.getPercentile99Population(i).mean(valid) << ","
           << m.getPercentile99Population(i).confInterval(valid, cl) << '\n';

        // close the output file
        os.close();
      }
    }
  }
}

void Metrics::dump(std::ostream& os, double cl, bool dist) {
  bool valid;

  // print all the averaged measures
  std::map<std::string, AvgMeasure>::iterator it = avgMeasures.begin();
  for (; it != avgMeasures.end(); it++) {
    // print the measure name
    os << "averaged measure = " << it->first << '\n';

    // print the measure values
    AvgMeasure& m = it->second; // alias
    m.restartPopulation();
    for (unsigned int i = 0; i < m.getSize(); i++) {
      Population& p = m.getPopulation();
      os << "(" << m.getPopulationId() << ") = ";
      p.dump(os);
      os << " [" << p.mean(valid) << ", " << p.confInterval(valid, cl) << "]"
         << '\n';
      m.nextPopulation();
    }
  }

  if (!dist)
    return;

  // print all the distribution measures
  std::map<std::string, DstMeasure>::iterator jt = dstMeasures.begin();
  for (; jt != dstMeasures.end(); jt++) {
    DstMeasure& m = jt->second; // alias

    os << "distribution measure = " << jt->first << '\n';
    for (unsigned int i = 0; i < m.getSize(); i++) {
      // print the p.m.f.
      for (unsigned int j = 0; j < m.getSize(i); j++) {
        if (m.getValid(i, j)) {
          os << "(" << i << ", " << j << ") = ";
          m.getPopulation(i, j).dump(os);
          os << " [" << m.getPopulation(i, j).mean(valid) << ", "
             << m.getPopulation(i, j).confInterval(valid, cl) << "]" << '\n';
        }
      }

      // print the c.d.f.
      for (unsigned int j = 0; j < m.getSize(i); j++) {
        if (m.getValid(i, j)) {
          os << "(" << i << ", " << j << ") = ";
          m.getPopulationCDF(i, j).dump(os);
          os << " [" << m.getPopulationCDF(i, j).mean(valid) << ", "
             << m.getPopulationCDF(i, j).confInterval(valid, cl) << "]" << '\n';
        }
      }

      if (m.getMedianPopulation(i).getSize() > 0) {
        os << "(" << i << ") "
           << "median = ";
        m.getMedianPopulation(i).dump(os);
        os << " [" << m.getMedianPopulation(i).mean(valid) << ", "
           << m.getMedianPopulation(i).confInterval(valid, cl) << "]" << '\n';
      }

      if (m.getMeanPopulation(i).getSize() > 0) {
        os << "(" << i << ") "
           << "mean   = ";
        m.getMeanPopulation(i).dump(os);
        os << " [" << m.getMeanPopulation(i).mean(valid) << ", "
           << m.getMeanPopulation(i).confInterval(valid, cl) << "]" << '\n';
      }

      if (m.getPercentile95Population(i).getSize() > 0) {
        os << "(" << i << ") "
           << "q95    = ";
        m.getPercentile95Population(i).dump(os);
        os << " [" << m.getPercentile95Population(i).mean(valid) << ", "
           << m.getPercentile95Population(i).confInterval(valid, cl) << "]"
           << '\n';
      }

      if (m.getPercentile99Population(i).getSize() > 0) {
        os << "(" << i << ") "
           << "q99    = ";
        m.getPercentile99Population(i).dump(os);
        os << " [" << m.getPercentile99Population(i).mean(valid) << ", "
           << m.getPercentile99Population(i).confInterval(valid, cl) << "]"
           << '\n';
      }
    }
  }
}
