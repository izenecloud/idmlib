#include <idmlib/keyphrase-extraction/kpe_knowledge.h>
#include <idmlib/util/resource_util.h>
#include <idmlib/util/idm_term_tag.h>
using namespace idmlib::kpe;

KpeKnowledge::KpeKnowledge(idmlib::util::IDMAnalyzer* analyzer)
:analyzer_(analyzer)
{
    std::vector<StringType> vec;
    vec.push_back(StringType("的", StringType::UTF_8));
    vec.push_back(StringType("是", StringType::UTF_8));
    vec.push_back(StringType("和", StringType::UTF_8));
    vec.push_back(StringType("与", StringType::UTF_8));
    
    for(uint32_t i=0;i<vec.size();i++)
    {
        std::string str;
        vec[i].convertString(str, StringType::UTF_8);
        a_set_.insert(str);
    }
}
KpeKnowledge::~KpeKnowledge()
{
}
bool KpeKnowledge::Load(const std::string& res_dir)
{
//   std::cout<<"[KPEScorer] loading KPE resources..."<<std::endl;
//   namespace bfs = boost::filesystem;
//   swContainer_ = new idmlib::util::StopWordContainer();
//   {
//       std::istream* ifs = idmlib::util::getResourceStream(resPath+"/stop_words");
//       if( ifs== NULL)
//       {
//         std::cerr<<"[KPEScorer] load stop_words failed."<<std::endl;
//         return false;
//       }
//       std::string word;
//       while ( getline ( *ifs,word ) )
//       {
//           boost::to_lower(word);
//           if ( word.length() >0 )
//           {
// //                       std::cout<<"add stop "<<word<<std::endl;
//               std::vector<uint32_t> termIdList;
//               analyzer_->GetIdList(izenelib::util::UString(word,izenelib::util::UString::UTF_8), termIdList);
// 
//               if( termIdList.size() > 0 )
//               {
//                   swContainer_->insert(termIdList);
//               }
//               
//           }
//       }
//       delete ifs;
//   }
//     
//   std::istream* ifs = idmlib::util::getResourceStream(resPath+"/invalid_chn_bigram");
//   if( ifs== NULL)
//   {
//     std::cerr<<"[KPEScorer] load invalid_chn_bigram failed."<<std::endl;
//     return false;
//   }
//   std::string word;
//   while ( getline ( *ifs,word ) )
//   {
//       if ( word.length() >0 )
//       {
// //                     std::cout<<"[KPE] "<<word<<std::endl;
//           if( word[0] == '@' ) break;
//           izenelib::util::UString ustr(word, izenelib::util::UString::UTF_8);
//           if( ustr.length()!= 2 ) continue;
//           if( !ustr.isChineseChar(0) ) continue;
//           if( !ustr.isChineseChar(1) ) continue;
//           izenelib::util::UString ustr1;
//           izenelib::util::UString ustr2;
//           ustr.substr(ustr1, 0,1);
//           ustr.substr(ustr2, 1,1);
//           uint32_t id1 = idmlib::util::IDMIdConverter::GetId(ustr1, idmlib::util::IDMTermTag::CHN);
//           uint32_t id2 = idmlib::util::IDMIdConverter::GetId(ustr2, idmlib::util::IDMTermTag::CHN);
//           uint64_t key = idmlib::util::make64UInt(id1, id2);
//           invalidChnBigram_->insert(key, 0);
//       }
//   }
//   delete ifs;
//   
//   
//   if(!ub_info_.load(resPath+"/ub_info"))
//   {
//     std::cerr<<"[KPEScorer] ubinfo load failed."<<std::endl;
//     return false;
//   }
//   std::cout<<"[KPEScorer] loading KPE resources finished."<<std::endl;
  return true;
}


bool KpeKnowledge::IsSplitTerm(const idmlib::util::IDMTerm& term)
{
    if(term.tag == idmlib::util::IDMTermTag::SYMBOL)
    {
        return true;
    }
    else if(term.tag == idmlib::util::IDMTermTag::OTHER)
    {
        return true;
    }
    return false;
}

//only care about SI more than 1 length
bool KpeKnowledge::Test(
const std::vector<TermInNgram>& term_list, uint32_t freq, const std::vector<id2count_t>& docitem_list, uint32_t total_doc_count
, const std::vector<std::pair<TermInNgram, uint32_t> >& left_list, const std::vector<std::pair<TermInNgram, uint32_t> >& right_list)
{
    return TestContext_(left_list, right_list);
}

bool KpeKnowledge::TestContext_(const std::vector<std::pair<TermInNgram, uint32_t> >& left_list, const std::vector<std::pair<TermInNgram, uint32_t> >& right_list)
{
  double max_ratio = 0.9;
  double multi_max_ratio = 0.7;
  double left_ratio = TestContextScore_(left_list);
  double right_ratio = TestContextScore_(right_list);
  if(left_ratio>=max_ratio) return false;
  if(right_ratio>=max_ratio) return false;
  if(left_ratio*right_ratio>=multi_max_ratio) return false;
  
  return true;
}

double KpeKnowledge::TestContextScore_(const std::vector<std::pair<TermInNgram, uint32_t> >& context_list)
{
    std::vector<uint32_t> real_list;
    uint32_t num_count = 0;
    for(uint32_t i=0;i<context_list.size();i++)
    {
        if(context_list[i].first!= idmlib::util::IDMTermTag::NUM)
        {
            real_list.push_back(context_list[i].second);
        }
        else
        {
            num_count += context_list[i].second;
        }
        
    }
    if(num_count>0)
    {
        real_list.push_back(num_count);
    }
    uint32_t sum = 0;
    uint32_t max = 0;
    for(uint32_t i=0;i<real_list.size();i++)
    {
        if(real_list[i]>max) max = real_list[i];
        sum += real_list[i];
    }
    double result = 0.0;
    if(sum>0)
    {
        result = (double)max/sum;
    }
    return result;
}

double KpeKnowledge::TestContextScore2_(const std::vector<std::pair<TermInNgram, uint32_t> >& context_list)
{
    uint32_t sum = 0;
    uint32_t max = 0;
    for(uint32_t i=0;i<context_list.size();i++)
    {
        if(context_list[i].second>max) max = context_list[i].second;
        sum += context_list[i].second;
    }
    double result = 0.0;
    if(sum>0)
    {
        result = (double)max/sum;
    }
    return result;
}


void KpeKnowledge::Flush()
{

}

void KpeKnowledge::PostProcess_(const std::vector<std::pair<StringType, double> >& input, std::vector<std::pair<StringType, double> >& output)
{
//     output.assign(input.begin(), input.end());
//     return;
    
    uint32_t max_diff = 1;
    for(uint32_t i=0;i<input.size();i++)
    {
        const StringType& input_text = input[i].first;
        std::string input_str;
        input_text.convertString(input_str, StringType::UTF_8);
        std::vector<std::pair<StringType, double> >::iterator out_it = output.begin();
        ///type<0 means del input_text, type>0 means del out_del_it in output;
        int type = 0;
        std::vector<std::pair<StringType, double> >::iterator out_del_it;
//         std::cout<<"[input-str] "<<input_str<<std::endl;
        while(out_it!=output.end())
        {
            int input_longer = 0;
            StringType addition;
            StringType& output_text = out_it->first;
            std::string output_str;
            output_text.convertString(output_str, StringType::UTF_8);
//             std::cout<<"[output-str] "<<output_str<<std::endl;
            if(input_text.length() > output_text.length())
            {
                uint32_t diff = input_text.length() - output_text.length();
                if(diff<=max_diff)
                {
                    if(boost::algorithm::starts_with(input_str, output_str))
                    {
                        addition = input_text.substr(output_text.length(), diff);
                    }
                    else if(boost::algorithm::ends_with(input_str, output_str))
                    {
                        addition = input_text.substr(0, diff);
                    }
                    input_longer = 1;
                }
            }
            else if(input_text.length() < output_text.length())
            {
                uint32_t diff = output_text.length() - input_text.length();
                if(diff<=max_diff)
                {
                    if(boost::algorithm::starts_with(output_str, input_str))
                    {
                        addition = output_text.substr(input_text.length(), diff);
                    }
                    else if(boost::algorithm::ends_with(output_str, input_str))
                    {
                        addition = output_text.substr(0, diff);
                    }
                    input_longer = -1;
                }
            }
//             std::cout<<"[input_longer] "<<input_longer<<std::endl;
            if(input_longer!=0 && addition.length()>0)
            {
                std::string addition_str;
                addition.convertString(addition_str, StringType::UTF_8);
                boost::unordered_set<std::string>::iterator a_it = a_set_.find(addition_str);
                if(a_it!=a_set_.end())
                {
                    //it's a invalid char
                    if(input_longer>0)
                    {
                        type = -1;
                    }
                    else
                    {
                        type = 1;
                        out_del_it = out_it;
                    }
                }
                else
                {
                    if(input_longer>0)
                    {
                        type = 1;
                        out_del_it = out_it;
                    }
                    else
                    {
                        type = -1;
                    }
                }
                break;
            }
            ++out_it;
        }
        if(type<0)
        {
            //do nothing
//             std::cout<<"[input-deleted] "<<input_str<<std::endl;
        }
        else if(type==0)
        {
            output.push_back(input[i]);
        }
        else
        {
//             StringType& output_text = out_del_it->first;
//             std::string output_str;
//             output_text.convertString(output_str, StringType::UTF_8);
//             std::cout<<"[output-deleted] "<<output_str<<std::endl;
            out_del_it->first = input_text;
//             output.erase(out_del_it);
//             output.push_back(input[i]);
        }
    }
}
    
      
        
