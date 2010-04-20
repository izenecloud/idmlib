/**
   @file  algo_test.h
   @author Kevin Hu
   @date 2009.12.30
*/

#ifndef ALGO_TEST_H
#define ALGO_TEST_H

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include  <iostream>
#include <idmlib/duplicate-detection/DupDetector.h>
#include <idmlib/duplicate-detection/rand_proj_gen.h>
#include <idmlib/duplicate-detection/rand_proj.h>
#include <string>
#include <am/cccr_hash/cccr_hash.h>
#include <util/hashFunction.h>

using   namespace  std;
namespace  fs  =  boost::filesystem;

class AlgoTest
{
  sf1v5::DupDetector dupd_;

  vector<string> pre_process_(const string& filenm)
  {
    FILE* f = fopen(filenm.c_str(), "r");
    if (f == NULL)
    {
      cout<<"Can't find file: "<<filenm<<"\n";
      return vector<string>();
    }

    fseek(f, 0, SEEK_END);
    uint32_t len = ftell(f);
    char* buf = new char[len];

    fseek(f, 0, SEEK_SET);
    fread(buf, len, 1, f);

    string content(buf, len);
    delete buf;

    vector<string> r;
    uint32_t start = 0;
    //cout<<content<<endl;
    
    for (uint32_t i=0; i<content.length(); ++i)
    {
      // std::cout<<content[i];
      if (content[i]>='a' && content[i]<='z' || content[i]>='A'&&content[i]<='Z')
        continue;

      //cout<<endl<<start<<","<<i<<endl;
      if (i-start>=2)
      {
        string sub = content.substr(start, i-start);
        r.push_back(sub);
        //std::cout<<sub<<endl;
      }

      start = i +1;
    }

    return r;
  }
  
 public:

 AlgoTest(const char* nm)
   :dupd_(1, nm)
  {
  }

  ~AlgoTest()
  {
  }

  void start(const char* path)
  {
    izenelib::am::cccr_hash<uint32_t, uint32_t> hash;
    izenelib::am::cccr_hash<uint32_t, string> nmhash;
    
    fs::path full_path(path, fs::native);
    if(!fs::exists(full_path))
    {
      cout<<"\nPATH is not right! ("<<path<<")\n";
      return;
    }

    dupd_.ready_for_insert();
    vector<uint32_t> docids;
    
    fs::directory_iterator item_begin(full_path);
    fs::directory_iterator item_end;
    for ( ;item_begin   !=  item_end; item_begin++ )
    {
      if (fs::is_directory( * item_begin))
        continue;

      string str = item_begin->path().file_string();
      if (str[str.length()-1] == 'n')
        continue;

      uint32_t docid1 = izenelib::util::HashFunction<std::string>::generateHash32(str);
      dupd_.insertDocument(docid1, pre_process_(str));
      nmhash.insert(docid1, str.substr(str.find_last_of("/")+1));
      
      str += "-syn";
      uint32_t docid2 = izenelib::util::HashFunction<std::string>::generateHash32(str);
      dupd_.insertDocument(docid2, pre_process_(str));
      nmhash.insert(docid2, str.substr(str.find_last_of("/")+1));

      hash.insert(docid1, docid2);
      docids.push_back(docid1);

      str = item_begin->path().file_string();
    }

    dupd_.runDuplicateDetectionAnalysis();

    uint32_t t = 0;
    for (uint32_t i=0; i<docids.size(); ++i)
    {
      vector<uint32_t> dids;
      dupd_.getDuplicatedDocIdList(docids[i], dids);

      if (dids.size()>0)
        ++t;
      else
        continue;
      
      cout<< "--------------\n";
      for (uint32_t j=0; j<dids.size(); ++j)
        cout<<*nmhash.find(dids[j])<<"\n";
      cout<<endl;
    }

    std::cout<<"recall rate: "<<(double)t/docids.size()<<endl;
  }
};
//

#endif//def ALGO_TEST_H
