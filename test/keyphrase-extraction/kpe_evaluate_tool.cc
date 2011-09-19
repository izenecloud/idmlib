#include <iostream>
#include <fstream>
#include <idmlib/keyphrase-extraction/kpe_evaluate.h>
#include <boost/algorithm/string.hpp>


#include "../TestResources.h"
using namespace idmlib::kpe;

int main(int ac, char** av)
{
    std::string evaluation_file(av[1]);
    std::string output_file(av[2]);
    KpeEvaluate eva;
    {
        std::ifstream ifs(evaluation_file.c_str());
        while(true)
        {
            std::string str_docid;
            std::string must_kp;
            std::string may_kp;
            if(!getline(ifs, str_docid)) break;
            if(!getline(ifs, must_kp)) break;
            if(!getline(ifs, may_kp)) break;
            boost::algorithm::trim(str_docid);
            boost::algorithm::trim(must_kp);
            boost::algorithm::trim(may_kp);
            std::vector<std::string> may_kp_list;
            boost::algorithm::split( may_kp_list, may_kp, boost::algorithm::is_any_of(",") );
            
            std::vector<std::string> must_kp_list;
            boost::algorithm::split( must_kp_list, must_kp, boost::algorithm::is_any_of(",") );
            uint32_t must_count = must_kp_list.size();
            std::vector<std::string> new_must_kp_list;
            for(uint32_t i=0;i<must_kp_list.size();i++)
            {
                std::vector<std::string> tmp_must_kp_list;
                boost::algorithm::split( tmp_must_kp_list, must_kp_list[i], boost::algorithm::is_any_of("|") );
                new_must_kp_list.insert(new_must_kp_list.end(), tmp_must_kp_list.begin(), tmp_must_kp_list.end());
            }
            
            eva.AddTaggedResult(str_docid, new_must_kp_list, must_count, may_kp_list);
        }
        ifs.close();
    }

    {
        std::ifstream ifs(output_file.c_str());
        while(true)
        {
            std::string str_docid;
            std::string kp;
            if(!getline(ifs, str_docid)) break;
            if(!getline(ifs, kp)) break;
            boost::algorithm::trim(str_docid);
            boost::algorithm::trim(kp);
            std::vector<std::string> kp_list;
            boost::algorithm::split( kp_list, kp, boost::algorithm::is_any_of(",") );
            
            eva.FindDocResult(str_docid, kp_list);
        }
        ifs.close();
    }
    
    for(uint32_t max = 5;max<=15;++max)
    {
        std::string result = eva.Finish(max);
        std::cout<<"[MAX="<<max<<"] "<<result<<std::endl;
    }
    
  
}
