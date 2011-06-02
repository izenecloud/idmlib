#ifndef IDMLIB_UTIL_CONTAINERSWITCHER_H_
#define IDMLIB_UTIL_CONTAINERSWITCHER_H_


#include <string>
#include <iostream>
#include <idmlib/idm_types.h>
#include <am/sequence_file/ssfr.h>
NS_IDMLIB_UTIL_BEGIN

template <class C>
class ContainerSwitch
{
public:
ContainerSwitch(const std::string& dir)
:dir_(dir), intermediate_(false)
{
    path_[0] = dir_+"/path1";
    path_[1] = dir_+"/path2";
    container_[0] = NULL;
    container_[1] = NULL;
    current_file_ = dir_+"/current";
}

~ContainerSwitch()
{
    if(container_[0]!= NULL)
    {
        delete container_[0];
    }
    if(container_[1]!= NULL)
    {
        delete container_[1];
    }
}

bool Open()
{
    try
    {
        boost::filesystem::create_directories(dir_);
        LoadCurrent_();
        uint8_t number = current_;
        if(boost::filesystem::exists(path_[number]))
        {
            if(!InitContainer_(number)) return false;
        }
        else
        {
            number = OtherNumber_(number);
            if(boost::filesystem::exists(path_[number]))
            {
                if(!InitContainer_(number)) return false;
            }
            else
            {
                number = 0;
                if(!InitContainer_(number)) return false;
            }
        }
    }
    catch(std::exception& ex)
    {
        std::cerr<<ex.what()<<std::endl;
        return false;
    }
    return true;
}

C* Current()
{
    return container_[current_];
}

C* Next()
{
    if(intermediate_)
    {
        EnsureSwitch();
    }
    uint8_t number = OtherNumber_(current_);
    if(container_[number]!=NULL)
    {
        delete container_[number];
        container_[number] = NULL;
    }
    std::string path = path_[number];
    try
    {
        boost::filesystem::remove_all(path);
    }
    catch(std::exception& ex)
    {
        std::cerr<<ex.what()<<std::endl;
        return NULL;
    }
    
    C* container = new C(path);
    if(!container->Open())
    {
        std::cerr<<"container open on "<<path<<" failed"<<std::endl;
        delete container;
        return NULL;
    }
    container_[number] = container;
    intermediate_ = true;
    return container;
}

bool EnsureSwitch()
{
    uint8_t new_number = OtherNumber_(current_);
    current_ = new_number;
    SaveCurrent_();
    intermediate_ = false;
    return true;
}

    
private:
  
  void LoadCurrent_()
  {
      current_ = 0;
      izenelib::am::ssf::Util<>::Load(current_file_, current_);
  }
  
  void SaveCurrent_()
  {
      izenelib::am::ssf::Util<>::Save(current_file_, current_);
  }
  
  bool InitContainer_(uint8_t number)
  {
    C* container = new C(path_[number]);
    if(!container->Open()) return false;
    uint8_t other = OtherNumber_(number);
    try
    {
        boost::filesystem::remove_all(path_[other]);
    }
    catch(std::exception& ex)
    {
        std::cerr<<ex.what()<<std::endl;
        return false;
    }
    current_ = number;
    container_[number] = container;
    return true;
  }
  
  inline uint8_t OtherNumber_(uint8_t number)
  {
      if(number==0) return 1;
      return 0;
  }
  
private:
  std::string dir_;
  std::string path_[2];
  uint8_t current_;
  C* container_[2];
  std::string current_file_;
  bool intermediate_;
};

   
NS_IDMLIB_UTIL_END



#endif /* SF1V5_FILEOBJECT_H_ */
