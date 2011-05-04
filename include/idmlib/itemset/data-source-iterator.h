#ifndef _DATA_SOURCE_ITERATOR_H_
#define _DATA_SOURCE_ITERATOR_H_

#include <stdio.h>
#include <string>
#include <vector>
#include <list>
#include <idmlib/idm_types.h>

namespace idmlib
{

typedef std::vector<uint32_t> ItemSet;
//
//The dataset format expected by the algorithm is "apriori binary."  In
//an apriori binary encoded dataset, each vector has the following
//format where each component is encoded as a raw 4-byte integer:
//<record id> <number of features> <fid 1> <fid 2> ... <fid n>
//
class DataSourceIterator
{
public:
    // Factory method for obtaining an iterator. The filepath is the
    // pathname to the file containing the data. Returns NULL on error
    // and reports the error details to stderr.
    static DataSourceIterator* Get(const char* filepath);
    ~DataSourceIterator();

    // Returns a human-readable string describing any error condition
    // encountered during a call to Next()/NextText().
    std::string GetErrorMessage()
    {
        return error_;
    }

    // Reads the next input itemset from an "apriori binary" formatted
    // input file.  Returns -1 on error, 0 on EOF, and 1 on
    // success. Each itemset consists of a 4 byte integer ID, a 4 byte
    // integer length, and then 4 byte integer IDs for each item. Checks
    // for many dataset format errors, but not all of them. For example
    // it does not check that the items are duplicate free and are
    // consistently ordered according to frequency.
    int Next(uint32_t* vector_id, ItemSet* input_vector);

    // Like Next, but used when testing with text format files.  Text
    // format assumes whitespace separators between vector and item
    // IDs. Instead of encoding vector lengths, use item id "0" to
    // terminate vectors.  E.g.:
    //
    // 1 1 2 3 0
    // 2 1 2 3 4 0
    // 3 2 3 0
    // ...
    //
    // The first value for a vector is its ID. The remaining values are
    // the IDs of its elements. End of line chars are encouraged, but
    // are not required to separate the vectors.
    int NextText(uint32_t* vector_id_, ItemSet* input_vector);

    bool Seek(off_t seek_offset);
    off_t Tell();

private:
    DataSourceIterator(FILE* data);

    FILE* data_;
    uint32_t last_vector_size_;
    int lines_processed_;
    std::string error_;
};

}

#endif  // _DATA_SOURCE_ITERATOR_H_
