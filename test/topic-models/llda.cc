#include <idmlib/topic-models/llda/LabeledLDA.h>

/****************************** MAIN ******************************/

using namespace idmlib::topicmodels;

int main(int ARGC, char *ARGV[])
{
    int nIter;
    int dumpInterval;

    if (ARGC != 5)
    {
        fprintf(stderr, "usage %s <entities> <labels> <nIter> <dumpInterval>\n", ARGV[0]);
        exit(1);
    }

    std::string trainingfile(ARGV[1]);
    std::string labelfile(ARGV[2]);
    nIter = atoi(ARGV[3]);
    dumpInterval = atoi(ARGV[4]);

    LabeledLDA lda(trainingfile,labelfile,nIter);

    fprintf(stderr, "-- This program was automatically generated using HBC (v 0.7 beta) from LDA.hier\n--     see http://hal3.name/HBC for more information\n");
    fflush(stderr);

    lda.estimate();

    return 0;
}

