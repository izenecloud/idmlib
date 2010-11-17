#include <idmlib/util/directory_switcher.h>
#include <boost/filesystem.hpp>
#include <fstream>
using namespace idmlib::util;

DirectorySwitcher::DirectorySwitcher(const std::string& parent):parent_(parent),choice_(0)
{
}

DirectorySwitcher::~DirectorySwitcher()
{
}

bool DirectorySwitcher::Open()
{
  dirs_[0] = parent_+"/MAIN1";
  dirs_[1] = parent_+"/MAIN2";
  std::string done[2];
  done[0] = dirs_[0]+"/DONE";
  done[1] = dirs_[1]+"/DONE";
  uint64_t time[2];
  time[0] = 0;
  time[1] = 0;
  for(uint8_t i=0;i<2;i++)
  {
    bool exist = boost::filesystem::exists(done[i]);
    if(exist)
    {
      time[i] = boost::filesystem::last_write_time(done[i]);
    }
  }
  uint64_t max_time = 0;
  for(uint8_t i=0;i<2;i++)
  {
    if( time[i] > max_time )
    {
      max_time = time[i];
      choice_ = i;
    }
  }
  if( max_time ==0 ) choice_ = 0;
  return true;
}

bool DirectorySwitcher::GetCurrent(std::string& dir)
{
  dir = dirs_[choice_];
  return true;
}

bool DirectorySwitcher::GetNext(std::string& dir)
{
  uint8_t next = choice_==0?1:0;
  dir = dirs_[next];
  return true;
}

bool DirectorySwitcher::GetNextWithDelete(std::string& dir)
{
  bool b_next = GetNext(dir);
  if(!b_next) return false;
  try
  {
    boost::filesystem::remove_all(dir);
  }
  catch(std::exception& ex)
  {
    return false;
  }
  return true;
}

bool DirectorySwitcher::SetNextValid()
{
  std::string next;
  if(!GetNext(next)) return false;
  std::string done = next+"/DONE";
  std::ofstream ofs(done.c_str() );
  ofs<<"a";
  ofs.close();
  if( ofs.fail() ) return false;
  uint8_t next_choice = choice_==0?1:0;
  choice_ = next_choice;
  return true;
}

