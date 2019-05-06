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
   filename: configuration.cc
   author: C. Cicconetti <c.cicconetti@iet.unipi.it>
   year: 2006
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
      University of Pisa, Italy
   description:
      body of the Configuration class
*/

#include <configuration.h>

void Configuration::insert(std::string          s,
                           unsigned int         id,
                           const MetricDescAvg& dsc) {
  std::vector<MetricDescAvg>& v = avg[s]; // alias

  if (id >= v.size())
    v.resize(id + 1);

  v[id] = dsc;
}

void Configuration::insert(std::string          s,
                           unsigned int         id,
                           std::string          what,
                           const MetricDescAvg& dsc) {
  std::vector<MetricDescDst>& v = dst[s]; // alias

  if (id >= v.size())
    v.resize(id + 1);

  if (what == "pmf")
    v[id].pmf = dsc;
  else if (what == "cdf")
    v[id].cdf = dsc;
  else if (what == "mean")
    v[id].mean = dsc;
  else if (what == "median")
    v[id].median = dsc;
  else if (what == "q95")
    v[id].percentile95 = dsc;
  else if (what == "q99")
    v[id].percentile99 = dsc;
  else
    throw *this;
}

std::string Configuration::getNextWord(std::istream& is, bool required) {
  std::string word; // buffer

  // read the next non-comment word
  while (!is.eof()) {
    // read the next word
    is >> word;

    // if the first character of the word is a '#' skip this line
    if (word[0] == '#')
      is.ignore(MAX_LINE, '\n');
    else
      return word;
  }
  if (required)
    throw *this;
  return std::string();
}

void Configuration::parse(std::string inputFileName) {
  std::string   word;   // buffer
  std::string   metric; // metric name
  MetricDescDst dst;    // descriptor for a distribution metric
  unsigned int  id;     // metric ID

  // open the configuration file for reading
  std::ifstream is;
  is.open(inputFileName.c_str(), std::ios::in);
  // check if the file can be actually read
  if (!is.is_open())
    throw *this;

  // read the first word
  word = getNextWord(is);

  // parse the rest of the file
  for (;;) {
    if (word.empty())
      break;

    if (word == "save") {
      outputFileName = getNextWord(is, true);
    } else if (word == "header") {
      headerName = getNextWord(is, true);
    } else if (word == "trailer") {
      trailerName = getNextWord(is, true);
    } else if (word == "minruns") {
      word       = getNextWord(is, true);
      minReplics = atoi(word.c_str());
    } else if (word == "maxruns") {
      word       = getNextWord(is, true);
      maxReplics = atoi(word.c_str());
    } else if (word == "s") {
      MetricDescAvg avg; // hides Configuration::avg
      avg.relevant = true;
      metric       = getNextWord(is, true);
      word         = getNextWord(is, true);
      id           = atoi(word.c_str());
      word         = getNextWord(is, true);
      if (word == "out") {
        avg.output = true;
        word       = getNextWord(is, true);
        avg.outCL  = atof(word.c_str());
        word       = getNextWord(is, false);
      }
      if (word == "check") {
        avg.check     = true;
        word          = getNextWord(is, true);
        avg.CL        = atof(word.c_str());
        word          = getNextWord(is, true);
        avg.threshold = atof(word.c_str());
        word          = getNextWord(is, false);
      }
      // parse error if neither out nor check words are found
      if (avg.output == false && avg.check == false)
        throw *this;
      // insert this metric descriptor into the configuration database
      insert(metric, id, avg);
      // since the next word has been already read, restart the loop
      continue;
    } else if (word == "d") {
      MetricDescDst dst; // hides Configuration::dst
      MetricDescAvg dsc; // buffer
      dsc.relevant = true;

      metric = getNextWord(is, true);
      word   = getNextWord(is, true);
      id     = atoi(word.c_str());
      // distribution submetric (mean, quantile, ...)
      std::string what = getNextWord(is, true);

      word = getNextWord(is, true);
      if (word == "out") {
        dsc.output = true;
        word       = getNextWord(is, true);
        dsc.outCL  = atof(word.c_str());
        word       = getNextWord(is, false);
      }
      if (word == "check") {
        dsc.check     = true;
        word          = getNextWord(is, true);
        dsc.CL        = atof(word.c_str());
        word          = getNextWord(is, true);
        dsc.threshold = atof(word.c_str());
        word          = getNextWord(is, false);
      }
      // parse error if neither out nor check words are found
      if (dsc.output == false && dsc.check == false)
        throw *this;
      // insert this metric descriptor into the configuration database
      insert(metric, id, what, dsc);
      // since the next word has been already read, restart the loop
      continue;
    } else {
      // unknown command
      throw *this;
    }

    // if we reach this point, then the loop must be restarted
    // with a new word (the end of line may be reached)
    word = getNextWord(is, false);
  }

  // close the configuration file
  is.close();
}

void Configuration::dump(std::ostream& os) {
  // print general configuration
  os << "save:    " << outputFileName << '\n';
  os << "header:  " << headerName << '\n';
  os << "trailer: " << trailerName << '\n';
  os << "minruns: " << minReplics << '\n';
  os << "maxruns: " << maxReplics << '\n';

  // print averaged metrics configuration
  std::map<std::string, std::vector<MetricDescAvg>>::iterator it;

  for (it = avg.begin(); it != avg.end(); it++) {
    for (unsigned int i = 0; i < it->second.size(); i++) {
      const MetricDescAvg& m = it->second[i];
      if (m.relevant == true) {
        os << "s;" << it->first << ';' << i << ';'
           << ((m.output == true) ? '1' : '0') << ';';
        if (m.output == true)
          os << m.outCL << ';';
        else
          os << ';';
        os << ((m.check == true) ? '1' : '0') << ';';
        if (m.check == true)
          os << m.CL << ';' << m.threshold;
        else
          os << ';';
        os << '\n';
      }
    }
  }

  // print distribution metrics configuration
  std::map<std::string, std::vector<MetricDescDst>>::iterator jt;

  for (jt = dst.begin(); jt != dst.end(); jt++) {
    for (unsigned int i = 0; i < jt->second.size(); i++) {
      for (unsigned int j = 0; j < 6; j++) {
        MetricDescAvg* m = nullptr;

        // demux the current submetric
        if (j == 0)
          m = &jt->second[i].pmf;
        else if (j == 1)
          m = &jt->second[i].cdf;
        else if (j == 2)
          m = &jt->second[i].mean;
        else if (j == 3)
          m = &jt->second[i].median;
        else if (j == 4)
          m = &jt->second[i].percentile95;
        else if (j == 5)
          m = &jt->second[i].percentile99;

        if (m != nullptr and m->relevant == true) {
          os << "d;" << jt->first << ';' << i << ';'
             << ((j == 0) ?
                     "pmf" :
                     (j == 1) ?
                     "cdf" :
                     (j == 2) ?
                     "mean" :
                     (j == 3) ? "median" :
                                (j == 4) ? "q95" : (j == 5) ? "q99" : "unknown")
             << ';' << ((m->output == true) ? '1' : '0') << ';';
          if (m->output == true)
            os << m->outCL << ';';
          else
            os << ';';
          os << ((m->check == true) ? '1' : '0') << ';';
          if (m->check == true)
            os << m->CL << ';' << m->threshold;
          else
            os << ';';
          os << '\n';
        }
      }
    }
  }
}
