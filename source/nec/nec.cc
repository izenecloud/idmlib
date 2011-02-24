#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
#include <idmlib/nec/nec.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>
#include <util/scd_parser.h>
#include <util/ustring/UString.h>
#include <boost/algorithm/string.hpp>
#include <am/3rdparty/rde_hash.h>
#include <boost/lexical_cast.hpp>
using namespace idmlib;
using namespace idmlib::nec;
using namespace idmlib::util;


NEC::NEC():model_(0)
{
}
NEC::~NEC()
{
  if(model_!=0)
  {
    svm_free_and_destroy_model(&model_);
  }
}

bool NEC::Load(const std::string& dir)
{
  std::string index_file = dir+"/feature_index";
  if(!boost::filesystem::exists(index_file))
  {
    std::cout<<"can not find "<<index_file<<std::endl;
    return false;
  }
  std::ifstream ifs(index_file.c_str());
  std::string line;
  int index = 1;
  while(getline(ifs, line))
  {
    feature_index_.insert(line, index);
    index++;
  }
  len_index_ = index;
  ifs.close();
  std::string model_file = dir+"/model";
  if( (model_=svm_load_model(model_file.c_str()))==0 )
  {
    std::cout<<"load model failed for nec"<<std::endl;
    return false;
  }
  std::cout<<model_file<<" loaded!"<<std::endl;
  return true;
}

int NEC::Predict(const NECItem& item)
{
  std::vector<std::pair<int, double> > vx;
  std::vector<std::pair<std::string, double> > features;
  item.get_all_feature_values(features);
  if(features.empty()) return 0;
  for(uint32_t i=0;i<features.size();i++)
  {
    std::string label = features[i].first;
    int* index = feature_index_.find(label);
    if(index!=NULL)
    {
      vx.push_back(std::make_pair(*index, features[i].second));
    }
  }
  std::sort(vx.begin(), vx.end());
  struct svm_node *x = (struct svm_node *) malloc((vx.size()+1)*sizeof(struct svm_node));
  for(uint32_t i=0;i<vx.size();i++)
  {
    x[i].index = vx[i].first;
    x[i].value = vx[i].second;
  }
  x[vx.size()].index = -1;
  double predict_label = svm_predict(model_,x);
  return (int)predict_label;
}

