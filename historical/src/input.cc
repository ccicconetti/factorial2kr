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
   filename: input.cc
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
        year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
           body of the Input class
*/

#include <input.h>
#include <string.h>

bool Input::readSingleRun(std::istream& is,
                          std::ostream* os,
                          bool          recover,
                          bool          onlyAvg,
                          const char*   oneMetr) {
  //
  // TODO: write a function to have a single line for read+check+write
  //

  // both averaged and distribution metrics
  unsigned int id;     // run identifier
  unsigned int len;    // length of the strings (including trailing '\0')
  unsigned int ndx;    // number of indices
  unsigned int mid;    // metric ID
  sample_t     sample; // sample
  bool         valid;  // check validity of a descriptor

  // averaged metrics only
  unsigned int  avg;    // number of averaged metrics
  MetricDescAvg avgDsc; // metric descriptor

  // distribution metrics only
  unsigned int  dst;       // number of distribution metrics
  unsigned int  bin;       // number of bins of distribution metrics
  sample_t      binSize;   // bin size of distribution metrics
  sample_t      distLower; // lower bound of distribution metrics
  MetricDescDst dstDsc;    // metric descriptor

  char metricName[MAX_METRIC_NAME];

  // read the run ID
  is.read((char*)&id, sizeof(id));

  // if we reached the eof of file, return
  // this is the only point in which an end-of-file does not
  // mean that the input file is damaged
  if (is.eof())
    return false;

  // if a run with the same ID has been alread read => skip this run
  if (runIdentifiers.count(id) == 1) {
    // skip averaged metrics
    is.read((char*)&avg, sizeof(avg));
    for (unsigned int i = 0; i < avg; i++) {
      is.read((char*)&ndx, sizeof(ndx));
      is.read((char*)&len, sizeof(len));
      if (len > MAX_METRIC_NAME)
        throw *this;
      // move the get pointer to the end of the current averaged metric
      is.seekg(len + ndx * (sizeof(unsigned int) + sizeof(sample_t)),
               std::ios::cur);
    }

    // skip distribution metrics
    is.read((char*)&dst, sizeof(dst));
    for (unsigned int i = 0; i < dst; i++) {
      is.read((char*)&ndx, sizeof(ndx));
      is.read((char*)&len, sizeof(len));
      if (len > MAX_METRIC_NAME)
        throw *this;
      is.seekg(len + sizeof(binSize) + sizeof(distLower), std::ios::cur);
      is.read((char*)&bin, sizeof(bin));
      // move the get pointer to the end of the current distribution metric
      is.seekg(ndx * (sizeof(unsigned int) + bin * sizeof(sample_t)),
               std::ios::cur);
    }
    return true;
  }

  // insert this run ID into the set of run identifiers
  runIdentifiers.insert(id);

  // write to the save file
  if (os != 0)
    os->write((char*)&id, sizeof(id));

  //
  // averaged metrics
  //

  is.read((char*)&avg, sizeof(avg)); // number of averaged metrics
  if (is.eof())
    throw *this; // check for premature end of file
  if (os != 0)
    os->write((char*)&avg, sizeof(avg));

  for (unsigned int i = 0; i < avg; i++) { // for each averaged metric
    is.read((char*)&ndx, sizeof(ndx));     // number of indices
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write((char*)&ndx, sizeof(ndx));
    is.read((char*)&len, sizeof(len)); // length of the metric's name
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write((char*)&len, sizeof(len));
    if (len > MAX_METRIC_NAME)
      throw *this;
    is.read(metricName, len); // metric's name
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write(metricName, len);
    for (unsigned int j = 0; j < ndx; j++) { // for each index
      is.read((char*)&mid, sizeof(mid));     // metric ID
      if (is.eof())
        throw *this; // check for premature end of file
      if (os != 0)
        os->write((char*)&mid, sizeof(mid));
      is.read((char*)&sample, sizeof(sample)); // sample
      if (is.eof())
        throw *this; // check for premature end of file
      if (os != 0)
        os->write((char*)&sample, sizeof(sample));

      bool rel = true;
      // check if i need to load only one metric
      // if( oneMetr!=NULL && strcmp(metricName,oneMetr)==0 )
      //	rel==false;

      // add sample if needed ('out' or 'check' in the configuration file)
      if (recover == false)
        // TODO: this may cause SEGMENTATION FAULT, which is not good
        configuration.getDescAvg(valid, avgDsc, metricName, mid);

      if ((recover == true || (valid && avgDsc.isRelevant() == true)) &&
          rel == true)
        metrics.addSample(metricName, sample, mid);
    } // end - for each index
  }   // end - for each averaged metric

  //
  // distribution metrics
  //

  is.read((char*)&dst, sizeof(dst)); // number of distribution metrics
  if (is.eof())
    throw *this; // check for premature end of file
  if (os != 0)
    os->write((char*)&dst, sizeof(dst));

  for (unsigned int i = 0; i < dst; i++) { // for each distribution metric
    is.read((char*)&ndx, sizeof(ndx));     // number of indices
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write((char*)&ndx, sizeof(ndx));
    is.read((char*)&len, sizeof(len)); // length of the metric's name
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write((char*)&len, sizeof(len));
    if (len > MAX_METRIC_NAME)
      throw *this;
    is.read(metricName, len); // metric's name
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write(metricName, len);
    is.read((char*)&binSize, sizeof(binSize)); // get bin size
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write((char*)&binSize, sizeof(binSize));
    is.read((char*)&distLower, sizeof(distLower)); // get lower bound
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write((char*)&distLower, sizeof(distLower));
    is.read((char*)&bin, sizeof(bin)); // number of bins
    if (is.eof())
      throw *this; // check for premature end of file
    if (os != 0)
      os->write((char*)&bin, sizeof(bin));
    for (unsigned int j = 0; j < ndx; j++) { // for each index
      is.read((char*)&mid, sizeof(mid));     // metric ID
      if (is.eof())
        throw *this; // check for premature end of file
      if (os != 0)
        os->write((char*)&mid, sizeof(mid));

      // check if this metric is relevant
      // that is, 'out' or 'check' set in the configuration file
      // if so, then valid is set to true; otherwise, to false
      if (recover == false)
        configuration.getDescDst(valid, dstDsc, metricName, mid);
      if (recover == true || (valid && dstDsc.isRelevant() == true))
        valid = true;
      else
        valid = false;

      // get all bin samples
      for (unsigned int k = 0; k < bin; k++) {   // for each bin
        is.read((char*)&sample, sizeof(sample)); // get sample
        if (os != 0)
          os->write((char*)&sample, sizeof(sample));
        // add sample only if i need for the distributions
        if (valid && !onlyAvg)
          metrics.addSample(metricName, sample, mid, k); // set sample
      }                                          // end - for each sample
    }                                            // end - for each index
    metrics.setDistLower(metricName, distLower); // set lower bound
    metrics.setBinSize(metricName, binSize);     // set bin size
  } // end - for each distribution metric

  // flush the buffer of save file
  if (os != 0)
    os->flush();
  return true;
}

bool Input::recoverData(std::string saveFile,
                        bool        onlyAvg,
                        const char* oneMetr) {
  std::ifstream save;
  save.open(saveFile.c_str(), std::ios::in);
  if (!save.is_open())
    throw *this;

  try {
    while (!save.eof())
      readSingleRun(save, 0, true, onlyAvg, oneMetr);
  } catch (const Object& obj) {
    // if an exception is raised, then the save data file is damaged
    // we recover it now by saving the old save data file into an
    // '.old' file, and substituting the old file with a recovered one
    // we assume that the first n-1 runs are safe

    // copy the old file
    std::ifstream inSave;
    std::ofstream outSave;
    char          buf[VECTOR_CHUNK_SIZE];

    std::string outFileName = saveFile + ".old";

    inSave.open(saveFile.c_str(), std::ios::in);
    outSave.open(outFileName.c_str(), std::ios::out);

    if (!outSave.is_open() || !inSave.is_open())
      throw *this;
    while (!inSave.eof()) {
      inSave.read(buf, sizeof(buf));
      outSave.write(buf, inSave.gcount());
    }
    inSave.close();
    outSave.close();

    std::ifstream savedFile;
    std::ofstream repairedSave;

    unsigned int goodRuns = runIdentifiers.size() - 1;
    runIdentifiers.clear();

    savedFile.open(outFileName.c_str(), std::ios::in);  // old save file
    repairedSave.open(saveFile.c_str(), std::ios::out); // new save file
    if (!savedFile.is_open() || !repairedSave.is_open())
      throw *this;
    for (unsigned int i = 0; i < goodRuns - 1; i++) {
      readSingleRun(savedFile, &repairedSave, true);
    }
    savedFile.close();
    repairedSave.close();
    return false;
  }
  save.close();
  return true;
}

bool Input::checkSavedData() {
  std::ifstream save;
  save.open(configuration.getOutputFileName().c_str(), std::ios::in);
  if (save.is_open())
    while (!save.eof())
      readSingleRun(save);
  save.close();

  unsigned int n = runIdentifiers.size(); // number of runs

  return (n > 1) ? checkConfidence() : false;
}

void Input::loadData(std::string fileIn, std::string fileOut) {
  //
  // before reading from fileIn, load saved data, if any
  //

  std::ifstream save;
  save.open(configuration.getOutputFileName().c_str(), std::ios::in);
  if (save.is_open())
    while (!save.eof())
      readSingleRun(save);
  save.close();

  // open the output file
  std::ofstream os; // output file stream
  os.open(fileOut.c_str(), std::ios::out);
  if (!os.is_open())
    throw *this;

  // write the number of saved run identifiers
  unsigned int command = runIdentifiers.size();
  os.write((char*)&command, sizeof(command));
  // print to the output file the list of saved run identifiers
  std::set<unsigned int>::iterator runIterator;
  for (runIterator = runIdentifiers.begin();
       runIterator != runIdentifiers.end();
       runIterator++) {
    command = *runIterator;
    os.write((char*)&command, sizeof(command));
  }

  // if the saved data fulfills the confidence requirements, exit immediately
  if (check() == true) {
    command = 0; // stop
    os.write((char*)&command, sizeof(command));
    os.close();
    return;
  }

  // otherwise, read data from new simulations
  command = 1; // go
  os.write((char*)&command, sizeof(command));
  os.flush();

  // open the save file
  std::ofstream saveFile;
  saveFile.open(configuration.getOutputFileName().c_str(),
                std::ios::out | std::ios::app);
  if (!saveFile.is_open())
    throw *this;

  // cycle until collected data does not fulfill the confidence requirements
  for (;;) { // infinite loop

    // open the input file
    std::ifstream is; // input file stream
    is.open(fileIn.c_str(), std::ios::in);
    if (!is.is_open())
      throw *this;
    if (!readSingleRun(is, &saveFile)) {
      is.close();
      continue;
    }
    is.close();
    // check whether the simulation should stop
    if (check() == true)
      break;
    // if not, then restart simulation and load new samples
    command = 1; // go
    os.write((char*)&command, sizeof(command));
    os.flush();
  }

  command = 0; // stop
  os.write((char*)&command, sizeof(command));
  os.close();
  saveFile.close();
}

bool Input::check() {
  unsigned int n = runIdentifiers.size(); // number of runs

  if (n >= configuration.getMaxReplics() ||
      (n >= configuration.getMinReplics() && n > 1 &&
       checkConfidence() == true))
    return true;

  return false;
}

bool Input::checkConfidence() {
  // aliases
  std::map<std::string, AvgMeasure>& avg = metrics.getAvgMeasures();
  std::map<std::string, DstMeasure>& dst = metrics.getDstMeasures();

  // utility variables
  bool          valid;
  MetricDescAvg avgDsc;
  MetricDescDst dstDsc;

  //
  // averaged measures
  //
  std::map<std::string, AvgMeasure>::iterator it = avg.begin();
  for (; it != avg.end(); it++) {
    const std::string& name = it->first;  // alias
    AvgMeasure&        m    = it->second; // alias

    m.restartPopulation();
    int id = 0;
    for (unsigned int j = 0; j < m.getSize(); j++) {
      id = m.getPopulationId();
      configuration.getDescAvg(valid, avgDsc, name, id);
      if (m.getValid(id) && valid && avgDsc.check == true) {
        Population& p = m.getPopulation(id);
        if (p.mean(valid) > 0.0) {
          sample_t x = 2.0 * p.confInterval(valid, avgDsc.CL) / p.mean(valid);
          if (x > avgDsc.threshold)
            return false;
        } // TODO: what if the mean of a set of samples is 0.0 ?
      }
      m.nextPopulation();
    }
    m.restartPopulation();
  }

  //
  // distribution measures
  //
  std::map<std::string, DstMeasure>::iterator jt = dst.begin();
  for (; jt != dst.end(); jt++) {
    const std::string& name = jt->first;  // alias
    DstMeasure&        m    = jt->second; // alias

    for (unsigned int i = 0; i < m.getSize(); i++) {
      configuration.getDescDst(valid, dstDsc, name, i);

      // if the descriptor is not valid, restart the loop
      if (!valid)
        continue;

      // otherwise, check the derived metrics and the pdf/cdf

      // 0 = pmf
      // 1 = cdf
      // 2 = mean
      // 3 = median
      // 4 = percentile95
      // 5 = percentile99
      for (unsigned k = 0; k < 6; k++) {

        // demultiplex the submetric descriptor
        MetricDescAvg* dsc = nullptr;

        if (k == 0)
          dsc = &dstDsc.pmf;
        else if (k == 1)
          dsc = &dstDsc.cdf;
        else if (k == 2)
          dsc = &dstDsc.mean;
        else if (k == 3)
          dsc = &dstDsc.median;
        else if (k == 4)
          dsc = &dstDsc.percentile95;
        else if (k == 5)
          dsc = &dstDsc.percentile99;

        // if the check of this submetric is not required
        // then restart this loop
        if (dsc != nullptr or dsc->check == false)
          continue;

        Population* p;       // population under consideration
        double      x = 0.0; // ratio between conf interval and mean

        if (k == 0 || k == 1) {
          for (unsigned int h = 0; h < m.getSize(i); h++) {
            if (k == 0)
              p = &m.getPopulation(i, h);
            else
              p = &m.getPopulationCDF(i, h);

            if (p->mean(valid) > 0.0) {
              x = 2.0 * p->confInterval(valid, dsc->CL) / p->mean(valid);
              if (x > dsc->threshold)
                return false;
            }
          }
        } else {
          if (k == 2)
            p = &m.getMeanPopulation(i);
          else if (k == 3)
            p = &m.getMedianPopulation(i);
          else if (k == 4)
            p = &m.getPercentile95Population(i);
          else
            p = &m.getPercentile99Population(i);

          if (p->mean(valid) > 0.0) {
            x = 2.0 * p->confInterval(valid, dsc->CL) / p->mean(valid);
            if (x > dsc->threshold)
              return false;
          }
        }
      }
    }
  }

  return true;
}
