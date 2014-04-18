#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
#include <idmlib/nec/nec_accuracier.h>
#include <idmlib/util/svm.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <sf1common/ScdParser.h>
#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <am/3rdparty/rde_hash.h>
#include <boost/lexical_cast.hpp>
using namespace idmlib;
using namespace idmlib::kpe;
using namespace idmlib::util;
using namespace boost::filesystem;
namespace po = boost::program_options;

struct svm_node *x;
int max_nr_attr = 64;

struct svm_model* model;
int predict_probability=0;


int predict_line(const std::string& sline)
{
  char* line = (char*)sline.c_str();
  int i = 0;
  double target_label, predict_label;
  char *idx, *val, *label, *endptr;
  int inst_max_index = -1; // strtol gives 0 if wrong format, and precomputed kernel has <index> start from 0

  label = strtok(line," \t");
  target_label = strtod(label,&endptr);
  if(endptr == label)
  {
    std::cout<<"error input line "<<sline<<std::endl;
    return 0;
  }

  while(1)
  {
    if(i>=max_nr_attr-1)  // need one more for index = -1
    {
      max_nr_attr *= 2;
      x = (struct svm_node *) realloc(x,max_nr_attr*sizeof(struct svm_node));
    }

    idx = strtok(NULL,":");
    val = strtok(NULL," \t");

    if(val == NULL)
      break;
    errno = 0;
    x[i].index = (int) strtol(idx,&endptr,10);
    if(endptr == idx || errno != 0 || *endptr != '\0' || x[i].index <= inst_max_index)
    {
      std::cout<<"error input line "<<sline<<std::endl;
      return 0;
    }
    else
      inst_max_index = x[i].index;

    errno = 0;
    x[i].value = strtod(val,&endptr);
    if(endptr == val || errno != 0 || (*endptr != '\0' && !isspace(*endptr)))
    {
      std::cout<<"error input line "<<sline<<std::endl;
      return 0;
    }

    ++i;
  }
  x[i].index = -1;
  predict_label = svm_predict(model,x);
  return (int)predict_label;
}

int main(int ac, char** av)
{
  
  std::string tagged_file = av[1];
  std::string info_file = av[2];
  std::string test_file = av[3];
  std::string model_file = av[4];
  uint32_t num4test = 0;
  uint32_t max_doc = 0;
  izenelib::am::rde_hash<std::string, int> ner_types;
  izenelib::am::rde_hash<int, std::string> info_list;
  NecAccuracier accuracier;
  {
    std::ifstream ifs(tagged_file.c_str());
    std::string line;
    getline ( ifs,line );
    std::vector<std::string> vec_value;
    boost::algorithm::split( vec_value, line, boost::algorithm::is_any_of(",") );
    num4test = boost::lexical_cast<uint32_t>(vec_value[0]);
    max_doc = boost::lexical_cast<uint32_t>(vec_value[1]);
    while ( getline ( ifs,line ) )
    {
      std::vector<std::string> vec;
      boost::algorithm::split( vec, line, boost::algorithm::is_any_of("\t") );
      accuracier.append(vec[0], vec[1]);
//       int type = boost::lexical_cast<int>(vec[1]);
//       ner_types.insert(vec[0], type);
    }
    ifs.close();
  }
  {
    std::ifstream ifs(info_file.c_str());
    std::string line;
    int index = 1;
    while ( getline ( ifs,line ) )
    {
      std::vector<std::string> vec;
      boost::algorithm::split( vec, line, boost::algorithm::is_any_of("\t") );
      info_list.insert(index, vec[1]);
      index++;
    }
    ifs.close();
  }
  //start to predict
  if((model=svm_load_model(model_file.c_str()))==0)
  {
    std::cout<<"load model failed"<<std::endl;
    return -1;
  }
  x = (struct svm_node *) malloc(max_nr_attr*sizeof(struct svm_node));
  int total = 0;
  int correct = 0;
  {
    std::ifstream ifs(test_file.c_str());
    std::string line;
    int index = 0;
    while ( getline ( ifs,line ) )
    {
      index++;
      int result_type = predict_line(line);
      std::string str = *(info_list.find(index));
      int r = accuracier.is_correct(str, result_type);
      std::cout<<str<<","<<result_type<<","<<r<<std::endl;
      if(r>=0)
      {
        total++;
        if(r>0)
        {
          correct++;
        }
      }
    }
    ifs.close();
  }
  std::cout<<"accuracy("<<total<<","<<correct<<") : "<<(double)correct/total<<std::endl;
  
  svm_free_and_destroy_model(&model);
  free(x);
  
  return 0;

  
}
