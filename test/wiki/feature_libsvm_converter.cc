#include <idmlib/wiki/feature_item.h>
#include <am/3rdparty/rde_hash.h>
#include <util/functional.h>
using namespace idmlib::wiki;


void print_stat(const std::string& file)
{
  std::ifstream ifs(file.c_str());
  std::string line;
  int a[4] = {0,0,0,0};
  while( getline(ifs, line))
  {
    FeatureItem item;
    item.Parse(line);
    a[item.type]++;
  }
  ifs.close();
  std::cout<<"a[0] "<<a[0]<<std::endl;
  std::cout<<"a[1] "<<a[1]<<std::endl;
  std::cout<<"a[2] "<<a[2]<<std::endl;
  std::cout<<"a[3] "<<a[3]<<std::endl;
}

void gen_feature_index(const std::string& input, const std::string& output)
{
  std::ifstream ifs(input.c_str());
  std::string line;
  std::ofstream ofs(output.c_str());
  izenelib::am::rde_hash<std::string, int> apps;
  int id = 0;
  std::vector<std::pair<std::string, double> > features;
  while( getline(ifs, line))
  {
    FeatureItem item;
    item.Parse(line);
    item.get_all_feature_values(features);
    for(uint32_t i=0;i<features.size();i++)
    {
      std::string label = features[i].first;
      if(apps.find(label)!=NULL) continue;
      id++;
      apps.insert(label, id);
      ofs<<label<<std::endl;
    }
  }
  ifs.close();
  ofs.close();
}

void gen_libsvm(const std::string& input, const std::string& feature_index_file, const std::string& output, int type)
{
  izenelib::am::rde_hash<std::string, int> feature_index;
  int len_index = 0;
  {
    std::ifstream ifs(feature_index_file.c_str());
    std::string line;
    int index = 1;
    while(getline(ifs, line))
    {
      feature_index.insert(line, index);
      index++;
    }
    len_index = index;
    ifs.close();
  }
  std::ifstream ifs(input.c_str());
  std::ofstream ofs(output.c_str());
  std::vector<std::pair<std::string, double> > features;
  std::string line;
  while( getline(ifs, line))
  {
    FeatureItem item;
    item.Parse(line);
    if(type>0 && item.type==0) continue;
    if(type<0 && item.type!=0) continue;
    item.get_all_feature_values(features);
    if(features.size()==0) continue;
    ofs<<item.type;
    std::vector<std::pair<int, double> > feature_list;
    for(uint32_t i=0;i<features.size();i++)
    {
      std::string label = features[i].first;
      int index = *(feature_index.find(label));
      feature_list.push_back(std::make_pair(index, features[i].second));
    }
    std::sort(feature_list.begin(), feature_list.end());
    for(uint32_t i=0;i<feature_list.size();i++)
    {
      ofs<<" "<<feature_list[i].first<<":"<<feature_list[i].second;
    }
    ofs<<std::endl;
  }
  ifs.close();
  ofs.close();
}

bool is_bigram_label(const std::string& label)
{
  if( boost::starts_with(label, "PB") || boost::starts_with(label, "SB") ) return true;
  return false;
}

void gen_libsvm_ai(const std::string& input, const std::string& feature_index_file, const std::string& output, uint32_t max)
{
  typedef std::vector<std::pair<std::string, double> > FeaturesType;
  std::cout<<"[gen_libsvm_ai] max : "<<max<<std::endl;
  izenelib::am::rde_hash<std::string, int> feature_index;
  {
    std::ifstream ifs(feature_index_file.c_str());
    std::string line;
    int index = 1;
    while(getline(ifs, line))
    {
      feature_index.insert(line, index);
      index++;
    }
    ifs.close();
  }
  izenelib::am::rde_hash<std::string, int> feature_app_in_ne;
  std::ifstream ifs(input.c_str());
  std::ofstream ofs(output.c_str());
  std::ofstream info_ofs( (output+".info").c_str());
  std::vector<std::pair<std::string, double> > features;
  std::vector<std::pair<int, FeatureItem> > other_list;
  std::string line;
  while( getline(ifs, line))
  {
    FeatureItem item;
    item.Parse(line);
    if(item.surface.length()<2) continue;
    item.get_all_feature_values(features);
    if(features.size()==0) continue;
    if(item.type>0)
    {
      info_ofs<<item.key()<<std::endl;
      ofs<<item.type;
      std::vector<std::pair<int, double> > feature_list;
      for(uint32_t i=0;i<features.size();i++)
      {
        std::string label = features[i].first;
        int index = *(feature_index.find(label));
        feature_list.push_back(std::make_pair(index, features[i].second));
      }
      std::sort(feature_list.begin(), feature_list.end());
      for(uint32_t i=0;i<feature_list.size();i++)
      {
        ofs<<" "<<feature_list[i].first<<":"<<feature_list[i].second;
      }
      ofs<<std::endl;
    }
    else
    {
      bool has_english_char = false;
      for(uint32_t i=0;i<item.surface.length();i++)
      {
        if(item.surface.isAlphaChar(i))
        {
          has_english_char = true;
          break;
        }
      }
      if(has_english_char) continue;
      other_list.push_back(std::make_pair(features.size(), item));
    }
  }
  ifs.close();
  std::cout<<"start to sort"<<std::endl;
  std::sort( other_list.begin(), other_list.end(), izenelib::util::first_greater<std::pair<int, FeatureItem> >());
  uint32_t num = other_list.size()>max?max:other_list.size();
  for(uint32_t i=0;i<num;i++)
  {
    FeatureItem item = other_list[i].second;
    item.get_all_feature_values(features);
    info_ofs<<item.key()<<std::endl;
    ofs<<item.type;
    std::vector<std::pair<int, double> > feature_list;
    for(uint32_t i=0;i<features.size();i++)
    {
      std::string label = features[i].first;
      int index = *(feature_index.find(label));
      feature_list.push_back(std::make_pair(index, features[i].second));
    }
    std::sort(feature_list.begin(), feature_list.end());
    for(uint32_t i=0;i<feature_list.size();i++)
    {
      ofs<<" "<<feature_list[i].first<<":"<<feature_list[i].second;
    }
    ofs<<std::endl;
  }
  ofs.close();
  info_ofs.close();
}

int main(int argc, char** argv)
{
  std::string cmd(argv[1]);
  if(cmd=="-p")
  {
    print_stat(argv[2]);
  }
  else if(cmd=="-i")
  {
    gen_feature_index(argv[2], argv[3]);
  }
  else if(cmd=="-sn")
  {
    gen_libsvm(argv[2], argv[3], argv[4], 1);
  }
  else if(cmd=="-sa")
  {
    gen_libsvm(argv[2], argv[3], argv[4], 0);
  }
  else if(cmd=="-so")
  {
    gen_libsvm(argv[2], argv[3], argv[4], -1);
  }
  else if(cmd=="-sai")
  {
    std::string max_str = argv[5];
    uint32_t max = boost::lexical_cast<uint32_t>(max_str);
    gen_libsvm_ai(argv[2], argv[3], argv[4], max);
  }
}
