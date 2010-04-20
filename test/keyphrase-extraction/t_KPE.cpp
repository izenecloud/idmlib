#include <idmlib/keyphrase-extraction/fwd.h>
#include "kpe_types.h"
using namespace idmlib::kpe;
// void test_func(const wiselib::UString& str, const std::vector<std::pair<uint32_t, uint32_t> >& id2countList, uint8_t score)
// {
//     
// }

int main()
{
//     typedef IDInputType<TestIDManager> InputType;
//     typedef OutputType<true, true> OutputType1;
//     typedef Algorithm1<InputType, OutputType1> KPE_TYPE;
    TestIDManager idManager("./");
//     InputType input(&idManager);
    KPE_ALL<TestIDManager>::function_type func;
//     OutputType1 output(func);
//     KPE_TYPE kpe(input, output, "./test");
    KPE_ALL<TestIDManager> kpe( &idManager, func, "./");
    std::vector<uint32_t> termList;
    std::vector<char> posList;
    std::vector<uint32_t> positionList;
    kpe.insert(termList,posList,positionList, 1);
    kpe.close();
}
