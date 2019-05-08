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
   filename: factorial2kr.cc
        author: C. Cicconetti <c.cicconetti@iet.unipi.it>
                C. Vallati <carlo.vallati@iet.unipi.it>
        year: 2008
   affiliation:
      Dipartimento di Ingegneria dell'Informazione
           University of Pisa, Italy
   description:
                load saved data and perform factorial2kr analysis according to a
   config file
 */

/*
        this program wants as input a config file with this structure:

        savefile_dir "s_dir_value"
        response_var "response_variable_name_value"
        num_pr_factors "num_value"
        primary_factor_name_value
        primary_factor_name_value
        primary_factor_name_value
        primary_factor_name_value
        primary_factor_name_value
        primary_factor_name_value
        ...
        ...
        ...
        name_savefile_value parameter_name_value {1-0} parameter_name_value
   {1-0} ....
        ...
        ...
        ...

*/

/*
        The factorial matrix (sign table) is has this structure:
        -----------------------------------------------------------------------
                I		Pr1	Pr2	Pr1*Pr2		RespVar
        -----------------------------------------------------------------------
                1		-1	-1	1		tot
                1		1	-1	-1		tot
                1		-1	1	-1		tot
                1		1	1	1		tot
        -----------------------------------------------------------------------
                tot0		tot1	tot2	tot1*2		Total
                "		"	"	"		Total/4

        This matrix is stored into the structure config.
        Every row is stored into a instance of class savefile under the map
        "valPrFa". In every row the values of the respVar is stored into the
        instance of Metrics called "data"
*/

#include <config.h>
#include <input.h>
#include <measure.h>
//#include <object.h>

#include <cstdlib>
#include <list>
#include <unistd.h>

//#include <set>
#include <iostream>
#include <string>

using namespace std;

double t_table[30][4] = {
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

double t_student(double cl, int df) {
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

//! this structure represent a savefile
struct savefile {
 public:
  //! Name of savefile
  string saveFileName;
  //! Stores data of savefile
  Metrics data;
  //! Indicates the values of primary factors (low=-1, high=1)
  std::map<std::string, int> valPrFa;
};

//! this structure stores the config data
class config
{
  //! contains the effects
  std::map<std::string, double> risp;
  //! contains the sum of squares
  std::map<std::string, double> squares;
  //! Number of repetitions
  int runs;
  //! utility function for parsing config file
  std::string getNextWord(std::istream& is, bool required);

 public:
  ~config() {
    delete namePrFac;
    delete save;
  };

  //! Number of primary factors
  unsigned int numPrFac;
  //! Name of response var
  string respVar;
  //! Name of savefile directory
  string saveDir;
  //! Names of primary factors
  string* namePrFac;
  //! Savefiles
  savefile* save;
  //! Store the value of sum of squares errors
  double sse;
  //! Store the value of sum of squares total
  double sst;
  //! Parse the config file and initialize the values of class
  void parseConfigFile(const string confFile, string rVar, string dataDir);
  //! Load data from files
  void loadData();
  //! Complete the design matrix with interactions
  void completeMatrix();
  //! Calculate the effects
  void compEffects(bool molModel, bool id_valid, unsigned int id);
  //! Calculate the sum of squares
  void compSquares(bool id_valid, unsigned int id);
  //! Print result on std out
  void printOutput(double);
  //! Save data for visual test
  void saveVerifyData(string, string, bool id_valid, unsigned int id);
};

std::string config::getNextWord(std::istream& is, bool required) {
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

void config::parseConfigFile(const string confFile,
                             string       rVar,
                             string       dataDir) {
  std::string word; // buffer
  // open the configuration file for reading
  std::ifstream is;
  is.open(confFile.c_str(), std::ios::in);
  // check if the file can be actually read
  if (!is.is_open())
    throw *this;

  // read the first word
  word = getNextWord(is, true);

  respVar = rVar;

  // parse the rest of the file
  for (;;) {
    if (word.empty())
      break;

    if (word == "savefile_dir") {
      if (dataDir == "")
        saveDir = getNextWord(is, true);
      // printf("%s",saveDir.c_str());
    } else if (word == "response_var") {
      if (rVar == "")
        respVar = getNextWord(is, true);
      // printf("%s",respVar.c_str());
    } else if (word == "num_pr_factors") {
      string num = getNextWord(is, true);
      // printf("%s",num.c_str());
      numPrFac  = atoi(num.c_str());
      namePrFac = new string[numPrFac];
      // i must find the names of primary factors
      for (unsigned int i = 0; i < numPrFac; i++) {
        namePrFac[i] = getNextWord(is, true);
        // printf("%s",namePrFac[i].c_str());
      }
      int numSaveFiles = (int)exp2(numPrFac);
      save             = new savefile[numSaveFiles];
      for (int i = 0; i < numSaveFiles; i++) {
        // i must find the names of savefiles and relative parameters
        save[i].saveFileName     = getNextWord(is, true);
        save[i].valPrFa[respVar] = 1;
        for (unsigned int j = 0; j < numPrFac; j++) {
          bool   valid = false;
          string name  = getNextWord(is, true);
          for (unsigned int f = 0; f < numPrFac; f++)
            if (name == namePrFac[f])
              valid = true;
          if (valid == false) {
            perror("config file error\n");
            throw *this;
          }

          // printf("%s",name.c_str());
          string level = getNextWord(is, true);
          // printf("%s",level.c_str());

          int val = atoi(level.c_str());
          if (val == 0)
            save[i].valPrFa[name] = -1;
          else
            save[i].valPrFa[name] = 1;
        }
      }
    }

    // if we reach this point, then the loop must be restarted
    // with a new word (the end of line may be reached)
    word = getNextWord(is, false);
  }

  // close the configuration file
  is.close();
}

void config::loadData() {
  Configuration conf; // empty configuration
  int           numSavefiles = (int)exp2(numPrFac);
  for (int i = 0; i < numSavefiles; i++) {
    Input input(conf, save[i].data);
    bool  good = true;
    good       = input.recoverData(
        saveDir + "/" + save[i].saveFileName, true, respVar.c_str());
    if (good == false) {
      cerr << "One savefile is bad!\n";
      throw *this;
    }
  }
}

void config::completeMatrix() {
  for (int savef = 0; savef < (int)exp2(numPrFac); savef++) {
    for (unsigned int pass = 1; pass <= numPrFac; pass++) {
      std::map<string, int>::iterator it = save[savef].valPrFa.begin();
      for (; it != save[savef].valPrFa.end(); it++) {
        unsigned int many = 1;
        if (it->first != respVar) {
          for (unsigned int i = 0; i < (it->first.length()); i++) {
            if (it->first.c_str()[i] == '*')
              many++;
          }
          if (many == (pass - 1)) {
            int num = it->first.rfind('*');
            if (num == -1)
              num = 0;
            string last = it->first.substr(num, it->first.length() - num);
            int    pos  = 0;
            for (unsigned int i = 0; i < numPrFac; i++) {
              if (last.find(namePrFac[i]) != string::npos)
                pos = i + 1;
            }
            for (unsigned int i = pos; i < numPrFac; i++) {
              string add = it->first + "*" + namePrFac[i];
              save[savef].valPrFa[add] =
                  it->second * save[savef].valPrFa[namePrFac[i]];
              // printf("\n%s value=%d\n",add.c_str(),save[savef].valPrFa[add]);
            }
          }
        }
      }
    }
  }
}

void config::compEffects(bool molModel, bool id_valid, unsigned int id) {
  int    numEffects = (int)exp2(numPrFac);
  bool   valid      = true;
  double mean;
  int    index = 0;
  // init the structure
  std::map<string, int>::iterator iti = save[0].valPrFa.begin();
  for (; iti != save[0].valPrFa.end(); iti++) {
    index++;
    risp[iti->first] = 0;
  }
  std::map<string, double>::iterator it;
  // printf("index:%d,numEff:%d\n",index,numEffects);
  if (index != numEffects)
    throw *this;

  for (int i = 0; i < numEffects; i++) {
    it = risp.begin();
    // mean effect
    AvgMeasure& m = save[i].data.getAvgMeasures()[respVar];
    m.restartPopulation();
    Population& p = m.getPopulation();
    if (id_valid)
      p = m.getPopulation(id);
    mean = p.mean(valid);
    if (molModel)
      mean = log10(mean);
    for (; it != risp.end(); it++) {
      risp[it->first] += save[i].valPrFa[it->first] * mean;
    }
  }
  if (valid == false)
    throw *this;
  it = risp.begin();
  for (; it != risp.end(); it++) {
    (it->second) /= numEffects;
    // if(molModel)
    //	it->second=pow(10,it->second);
  }
}

void config::compSquares(bool id_valid, unsigned int id) {
  int    numEffects = (int)exp2(numPrFac);
  double mean       = 0;
  double ssy        = 0;
  double err        = 0;
  double errTot     = 0;
  int    many       = 0;
  bool   valid      = true;
  // compute the total sum of squares
  for (int i = 0; i < numEffects; i++) {
    AvgMeasure& m = save[i].data.getAvgMeasures()[respVar];
    m.restartPopulation();
    Population& p = m.getPopulation();
    if (id_valid)
      p = m.getPopulation(id);
    mean = p.mean(valid);
    many = p.getSize();
    for (unsigned int j = 0; j < p.getSize(); j++) {
      double run = p.getSample(valid, j);
      ssy += pow(run, 2);
      errTot += pow((run - mean), 2);
    }
  }
  if (valid == false)
    throw *this;
  // compute the others sum of square
  std::map<string, double>::iterator it;
  it = risp.begin();
  for (; it != risp.end(); it++) {
    if (it->first != respVar) {
      squares[it->first] = numEffects * many * pow(it->second, 2);
      err += squares[it->first];
    } else {
      squares[it->first] = numEffects * many * pow(it->second, 2);
    }
  }
  sst = ssy - squares[respVar];
  // compute sum of square errors
  sse = sst - err;
  // Bad hack : if the sse is negative (is possible due to rounding little
  // values) i must use the classic method
  if (sse <= 0)
    sse = errTot;
  runs = many;
}

void config::printOutput(double cl) {
  std::map<string, double>::iterator it;
  double                             variance   = 0;
  int                                numEffects = (int)exp2(numPrFac);
  int                                n          = runs;
  double                             confInt    = 0;
  variance = sqrtf(sse / (numEffects * (n - 1))) / (sqrtf(numEffects * n));
  confInt  = t_student(cl, (n - 1) * numEffects) * variance;
  printf("%s:%f[+-%f]\n", respVar.c_str(), risp[respVar], confInt);
  for (unsigned int pass = 1; pass <= numPrFac; pass++) {
    it = risp.begin();
    for (; it != risp.end(); it++) {
      unsigned int many = 1;
      if (it->first != respVar) {
        for (unsigned int i = 0; i < (it->first.length()); i++) {
          if (it->first.c_str()[i] == '*')
            many++;
        }
        if (many == pass) {
          printf("%s:%f [+-%f],per=%f%%\n",
                 it->first.c_str(),
                 it->second,
                 confInt,
                 squares[it->first] / sst * 100);
        }
      }
    }
  }
  printf("errors per:%f%%\n", sse / sst * 100);
}

void config::saveVerifyData(string       name1,
                            string       name2,
                            bool         id_valid,
                            unsigned int id) {
  int           numEffects = (int)exp2(numPrFac);
  double        mean;
  bool          valid = false;
  std::ofstream os;
  unlink(name1.c_str());
  os.open(name1.c_str(), std::ios::out | std::ios::app);
  if (!os.is_open())
    throw *this;
  // predicted response VS residuals
  // CONDITION : the residual must be an order smaller than th responses
  for (int i = 0; i < numEffects; i++) {
    AvgMeasure& m = save[i].data.getAvgMeasures()[respVar];
    m.restartPopulation();
    Population& p = m.getPopulation();
    if (id_valid)
      p = m.getPopulation(id);
    mean = p.mean(valid);
    for (unsigned int j = 0; j < p.getSize(); j++) {
      double run = p.getSample(valid, j);
      os << mean << " " << (run - mean) << "\n";
    }
  }
  os.close();
  unlink(name2.c_str());
  os.open(name2.c_str(), std::ios::out | std::ios::app);
  // normal quantile VS quantile
  // CONDITION : the residuals appear to be approximately normally distribuited
  list<double> residuals;
  for (int i = 0; i < numEffects; i++) {
    AvgMeasure& m = save[i].data.getAvgMeasures()[respVar];
    m.restartPopulation();
    Population& p = m.getPopulation();
    if (id_valid)
      p = m.getPopulation(id);
    mean = p.mean(valid);
    for (unsigned int j = 0; j < p.getSize(); j++) {
      double run = p.getSample(valid, j);
      residuals.push_back(run - mean);
    }
  }
  residuals.sort();
  int                    size = residuals.size();
  list<double>::iterator it;
  it = residuals.begin();
  for (int i = 1; i <= size; i++) {
    double q = (i - 0.5) / size;
    double x = 4.91 * (pow(q, 0.14) - (pow(1 - q, 0.14)));
    os << x << " " << *it << "\n";
    it++;
  }
  os.close();
}

void printUsage() {
  printf("usage: factorial2kr:\n");
  printf("factorial2kr path_config_file\n");
  printf("-c conf     use confidence level 'conf' (default = 0.90)\n");
  printf("-r name     save data for residual visual test\n");
  printf("-q name     save data for quantile visual test\n");
  printf("-o name     specify the response variable for analysis\n");
  printf("-d name     specify the directory of savefiles\n");
  printf("-m          use a moltiplicative model for analisys\n");
  printf("-n id	    id run to use\n");
  exit(0);
}

int main(int argc, char* argv[]) {
  int          ch; // for parsing arguments
  string       configFileName;
  config       cfg;
  double       cl           = 0.90;
  string       residualFile = "response.dat";
  string       quantileFile = "quantile.dat";
  string       rVar         = "";
  string       dataDir      = "";
  bool         molModel     = false;
  bool         verbose      = false;
  bool         id_valid     = false;
  unsigned int id_run       = 0;
  // parse command-line arguments
  while ((ch = getopt(argc, argv, "hc:q:r:o:mn:")) != -1) {
    switch (ch) {
      case 'h':
        printUsage();
        break;
      case 'c':
        cl = atof(optarg);
        break;
      case 'q':
        quantileFile = optarg;
        break;
      case 'r':
        residualFile = optarg;
        break;
      case 'o':
        rVar = optarg;
        break;
      case 'd':
        dataDir = optarg;
        break;
      case 'm':
        molModel = true;
        break;
      case 'v':
        verbose = true;
        break;
      case 'n':
        id_run   = atoi(optarg);
        id_valid = true;
        break;
      default:
        printUsage();
        break;
    }
  }

  argc -= optind;
  argv += optind;

  if (argc > 1 || argc == 0)
    printUsage(); // does not return

  if (argc == 1)
    configFileName = argv[0];

  try {
    if (verbose == true)
      printf("Parsing config file...\n");
    // parse config file
    cfg.parseConfigFile(configFileName, rVar, dataDir);
    if (verbose == true)
      printf("Loadnig data...\n");
    // load data
    cfg.loadData();
    if (verbose == true)
      printf("Complete matrix...\n");
    // complete matrix
    cfg.completeMatrix();
    if (verbose == true)
      printf("Comp effects...\n");
    // comp data
    cfg.compEffects(molModel, id_valid, id_run);
    if (verbose == true)
      printf("Comp squares...\n");
    // comp sum of squares
    cfg.compSquares(id_valid, id_run);
    if (verbose == true)
      printf("Print data...\n");
    // print data on stdout
    cfg.printOutput(cl);
    if (verbose == true)
      printf("Saving data for visual tests...\n");
    // save data for visual test
    cfg.saveVerifyData(residualFile, quantileFile, id_valid, id_run);

  } catch (Object& obj) {
    printf("Exception raised by the instance #%d of class %s. ",
           obj.getId(),
           obj.getName().c_str());
    perror("Terminated\n");
  }

  exit(1);
}
