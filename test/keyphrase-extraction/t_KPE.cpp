#include <idmlib/keyphrase-extraction/fwd.h>
#include "kpe_types.h"
using namespace idmlib::kpe;
void test_func(const wiselib::UString& ustr, const std::vector<std::pair<uint32_t, uint32_t> >& id2countList, uint8_t score)
{
    std::string str;
    ustr.convertString(str, wiselib::UString::UTF_8);
    std::cout<<"Find KP: "<<str<<std::endl;
}

int main()
{
    TestIDManager idManager("./id");
    KPE_ALL<TestIDManager>::function_type func = &test_func;
    KPE_ALL<TestIDManager> kpe( &idManager, func, "./tmp");
    kpe.load("/home/jarvis/projects/idmlib/resource/kpe");
    wiselib::UString article("中国就是中国而且中国很强大中国", wiselib::UString::UTF_8);
    kpe.insert(article, 1);
    kpe.close();
}
