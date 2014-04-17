#include <idmlib/query-correction/cn_corpus_trainer.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include "../TestResources.h"
using namespace idmlib::qc;

int main(int ac, char** av)
{
    idmlib::util::IDMAnalyzer* analyzer = new idmlib::util::IDMAnalyzer(idmlib::util::IDMAnalyzerConfig::GetCommonConfig(WISEKMA_KNOWLEDGE,"",IZENEJMA_KNOWLEDGE));
    CnCorpusTrainer trainer(analyzer);
    trainer.Train(av[1], av[2] );
    delete analyzer;
}
