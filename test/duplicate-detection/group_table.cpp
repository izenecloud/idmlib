#include <idmlib/duplicate-detection/group_table.h>

using namespace idmlib::dd;

int main()
{
    GroupTable<std::string, uint32_t> table("./group_table");
    table.Load();
    const std::vector<std::vector<std::string> >& info = table.GetGroupInfo();
    for(uint32_t i=0;i<info.size();i++)
    {
        for(uint32_t j=0;j<info[i].size();j++)
        {
            std::cout<<info[i][j]<<std::endl;
        }
        std::cout<<std::endl;
    }
}