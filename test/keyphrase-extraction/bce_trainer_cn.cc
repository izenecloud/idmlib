#include <idmlib/keyphrase-extraction/kpe_algorithm.h>
#include <boost/algorithm/string/trim.hpp>
#include "../TestResources.h"
using namespace idmlib;
using namespace idmlib::kpe;
using namespace idmlib::util;
using namespace boost::filesystem;

struct TUnigramInfo
{
  TUnigramInfo()
  :text(), total_count(0), p_count(0), a_count(0), b_count(0), d_count(0), e_count(0), s_count(0)
  {
  }
  TUnigramInfo(const izenelib::util::UString& s)
  :text(s), total_count(0), p_count(0), a_count(0), b_count(0), d_count(0), e_count(0), s_count(0)
  {
  }
  izenelib::util::UString text;
  uint64_t total_count;
  //for example prefix, abcde, suffix
  uint64_t p_count;
  uint64_t a_count;
  uint64_t b_count;
  uint64_t d_count;
  uint64_t e_count;
  uint64_t s_count;
};

struct TBigramInfo
{
  TBigramInfo()
  :text(), total_count(0), pa_count(0), ab_count(0), de_count(0), es_count(0)
  {
  }
  TBigramInfo(const izenelib::util::UString& s)
  :text(s), total_count(0), pa_count(0), ab_count(0), de_count(0), es_count(0)
  {
  }
  izenelib::util::UString text;
  uint64_t total_count;
  //for example prefix, abcde, suffix
  uint64_t pa_count;
  uint64_t ab_count;
  uint64_t de_count;
  uint64_t es_count;
};

void debug(const std::string& info)
{
  return;
  std::cout<<info<<std::endl;
}

int main(int ac, char** av)
{
  
  std::string input_file(av[1]);
  std::string output_file(av[2]);

  
  
  std::string line;
  izenelib::util::UString content;
  izenelib::util::UString flag_begin("%", izenelib::util::UString::UTF_8);
  izenelib::util::UString flag_end("%", izenelib::util::UString::UTF_8);
  //stat value
  uint64_t unigram_count = 0;
  uint64_t bigram_count = 0;
  izenelib::am::rde_hash<izenelib::util::UString, uint64_t> ngram_index;
  
  std::vector<TUnigramInfo> unigram_info_list;
  std::vector<TBigramInfo> bigram_info_list;
  
  uint32_t line_num = 0;
  
  izenelib::am::rde_hash<izenelib::util::UCS2Char, izenelib::util::UCS2Char> t2s_map;
  
  //t to s
  {
    std::ifstream t2s_ifs("cs_ct.utf8");
    while( getline( t2s_ifs, line) )
    {
      boost::algorithm::trim( line );
      izenelib::util::UString uline(line, izenelib::util::UString::UTF_8);
      t2s_map.insert( uline[2], uline[0] );
    }
    t2s_ifs.close();
  }
  std::ifstream ifs( input_file.c_str() );
  while ( getline ( ifs,line ) )
  {
    line_num++;
    if(line_num%10000 == 0 )
    {
      std::cout<<"Processed line "<<line_num<<std::endl;
    }
    boost::algorithm::trim( line );
    debug(line);
    if( line.length() > 2 )
    {
      std::string start = line.substr(0,2);
      if( start == "T:" )
      {
        continue;
      }
      else if( start == "C:" )
      {
        content = izenelib::util::UString(line.substr(2, line.length()-2), izenelib::util::UString::UTF_8);
      }
      else
      {
        content = izenelib::util::UString(line, izenelib::util::UString::UTF_8);
      }
    }
    for(uint32_t i=0;i<content.length();i++)
    {
      izenelib::util::UCS2Char* p_char = t2s_map.find(content[i]);
      if( p_char != NULL )
      {
        content[i] = *p_char;
      }
    }
//     {
//       std::string scontent;
//       content.convertString(scontent, izenelib::util::UString::UTF_8);
//       std::cout<<scontent<<std::endl;
//     }
    debug("a");
    izenelib::util::UString concept;
    izenelib::util::UString prefix_char;
    izenelib::util::UString last_char;
    
    std::vector<std::pair<uint32_t, uint32_t> > concept_pos_list;
    uint32_t pos_begin = 0;
    int status = 0;//0:other; 1: prefix; 2:a; 3:b; 4:c; d:5; e:6; s: 7
    for( uint32_t i=0; i<content.length(); i++)
    {
      izenelib::util::UString ch = content.substr(i, 1);
      izenelib::util::UString last = ch;

      if( ch.isChineseChar(0) )
      {
        //update unigram info
        {
          unigram_count++;
          uint64_t* p_index = ngram_index.find(ch);
          uint64_t index = 0;
          if( p_index == NULL )
          {
            index = unigram_info_list.size();
            ngram_index.insert(ch, index );
            unigram_info_list.push_back( TUnigramInfo(ch) );
          }
          else
          {
            index = *p_index;
          }
          unigram_info_list[index].total_count += 1;
        }
        if( last_char.length()>0 && last_char.isChineseChar(0) )
        {
          bigram_count++;
          izenelib::util::UString bigram = last_char;
          bigram += ch;
          
          uint64_t* p_index = ngram_index.find(bigram);
          uint64_t index = 0;
          if( p_index == NULL )
          {
            index = bigram_info_list.size();
            ngram_index.insert(bigram, index );
            bigram_info_list.push_back( TBigramInfo(bigram) );
          }
          else
          {
            index = *p_index;
          }
          bigram_info_list[index].total_count += 1;
          
        }
      }
      else if(ch == flag_begin || ch == flag_end)
      {
        last = last_char;
      }
      
      last_char = last;
      
      if( ch == flag_begin && status ==0 )
      {
        pos_begin = i;
        status = 1;
      }
      else if( ch == flag_end && status == 1 )
      {
        concept_pos_list.push_back(std::make_pair( pos_begin, i ) );
        status = 0;
      }
      
    }
    debug("b");
        
    //iterate all concepts
    for(uint32_t i=0;i<concept_pos_list.size();i++)
    {
      uint32_t begin = concept_pos_list[i].first;
      uint32_t end = concept_pos_list[i].second;
      izenelib::util::UString concept = content.substr(begin+1, end-begin-1);
      
//       std::string test_str;
//       concept.convertString(test_str, izenelib::util::UString::UTF_8);
//       std::cout<<begin<<","<<end<<","<<test_str<<std::endl;
      
      if( concept.length() == 0 ) continue;
      bool all_cn = true;
      for(uint32_t j=0;j<concept.length();j++)
      {
        if( !concept.isChineseChar(j) )
        {
          all_cn = false;
          break;
        }
      }
      if( !all_cn ) continue;
      //find prefix
      izenelib::util::UString prefix;
      if( begin>0 )
      {
        izenelib::util::UString ch = content.substr(begin-1, 1);
        if( ch.isChineseChar(0) )
        {
          prefix = ch;
        }
        else if( ch == flag_end && begin > 1)
        {
          izenelib::util::UString ch2 = content.substr(begin-2, 1);
          if( ch2.isChineseChar(0) )
          {
            prefix = ch2;
          }
        }
      }
      
      //find suffix
      izenelib::util::UString suffix;
      if( end< content.length()-1 )
      {
        izenelib::util::UString ch = content.substr(end+1, 1);
        if( ch.isChineseChar(0) )
        {
          suffix = ch;
        }
        else if( ch == flag_begin && end< content.length()-2)
        {
          izenelib::util::UString ch2 = content.substr(end+2, 1);
          if( ch2.isChineseChar(0) )
          {
            suffix = ch2;
          }
        }
      }
//       std::cout<<"sss"<<std::endl;
      izenelib::util::UString a = concept.substr(0,1);
      izenelib::util::UString e = concept.substr(concept.length()-1, 1);
      
      if( prefix.length()>0 )
      {
        uint64_t* p_index = ngram_index.find(prefix);
        unigram_info_list[*p_index].p_count += 1;
        izenelib::util::UString pa = prefix;
        pa += a;
        p_index = ngram_index.find(pa);
        bigram_info_list[*p_index].pa_count += 1;
      }
//       std::cout<<"sss2"<<std::endl;
      if( suffix.length()>0 )
      {
        uint64_t* p_index = ngram_index.find(suffix);
        unigram_info_list[*p_index].s_count += 1;
        izenelib::util::UString es = e;
        es += suffix;
        p_index = ngram_index.find(es);
        bigram_info_list[*p_index].es_count += 1;
      }
//       std::cout<<"sss3"<<std::endl;
      if( concept.length() == 1) continue;
      for(uint32_t i=0;i<concept.length();i++)
      {
        izenelib::util::UString u = concept.substr(i,1);
        uint64_t* p_index = 0;
        if( i==0 )
        {
          p_index = ngram_index.find(u);
          unigram_info_list[*p_index].a_count += 1;
        }
        if( i==1 )
        {
          p_index = ngram_index.find(u);
          unigram_info_list[*p_index].b_count += 1;
          izenelib::util::UString bigram = concept.substr(i-1, 2);
          p_index = ngram_index.find(bigram);
          bigram_info_list[*p_index].ab_count += 1;
        }
        if( i== concept.length()-2 )
        {
          p_index = ngram_index.find(u);
          unigram_info_list[*p_index].d_count += 1;
        }
        if( i== concept.length()-1 )
        {
          p_index = ngram_index.find(u);
          unigram_info_list[*p_index].e_count += 1;
          izenelib::util::UString bigram = concept.substr(i-1, 2);
          p_index = ngram_index.find(bigram);
          bigram_info_list[*p_index].de_count += 1;
        }
      }//end output for one concept
//       std::cout<<"sss4"<<std::endl;
      
    }//end for all concepts in one line.
    debug("c");
  }//end for while line

  ifs.close();
  std::ofstream ofs( output_file.c_str() );
  ofs<<"stat,"<<unigram_count<<","<<bigram_count<<std::endl;
  std::string str;
  for(uint32_t i=0;i<bigram_info_list.size();i++)
  {
    bigram_info_list[i].text.convertString(str, izenelib::util::UString::UTF_8);
    ofs<<"bigram,"<<str<<","<<bigram_info_list[i].total_count<<","<<bigram_info_list[i].pa_count<<","<<bigram_info_list[i].ab_count<<","<<bigram_info_list[i].de_count<<","<<bigram_info_list[i].es_count<<std::endl;
  }
  
  for(uint32_t i=0;i<unigram_info_list.size();i++)
  {
    unigram_info_list[i].text.convertString(str, izenelib::util::UString::UTF_8);
    ofs<<"unigram,"<<str<<","<<unigram_info_list[i].total_count<<","<<unigram_info_list[i].p_count<<","<<unigram_info_list[i].a_count<<","<<unigram_info_list[i].b_count<<","<<unigram_info_list[i].d_count<<","<<unigram_info_list[i].e_count<<","<<unigram_info_list[i].s_count<<std::endl;
  }
  ofs.close();
  return 0;

  
}
