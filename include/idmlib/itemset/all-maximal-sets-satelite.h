// Copyright 2010 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// ---
// This class implements an algorithm that computes all maximal sets
// from a given input list of sets based on the method used by the
// SATeLite satisfiability simplifier.
//
// The input list must have the following properties for the algorithm
// to behave correctly and/or efficiently:
//
// 1) Items within a set must always appear from least to most
// frequent in a consistent order.  That is if item $x$ appears less
// frequently than item $y$ within the dataset, then $x$ should always
// appear before $y$ within any set containing both items. Furthermore
// if two items $x$ and $y$ have the same frequency, then one must be
// chosen to consistently appear before the other should they both
// appear in a given set.
//
// 2) A set must not contain duplicate items.
// ---
// Author: Roberto Bayardo
//
#ifndef _ALL_MAXIMAL_SETS_SATELITE_H_
#define _ALL_MAXIMAL_SETS_SATELITE_H_

#include <vector>

namespace idmlib
{

class DataSourceIterator;
class SetProperties;

// A list of itemsets used to store candidates within the map.
typedef std::vector<SetProperties*> OccursList;

class AllMaximalSetsSateLite
{
public:
    AllMaximalSetsSateLite()
    {
    }

    // Finds all maximal sets in the "data" stream. Does not assume
    // ownership of the data stream. Returns false if the computation
    // could not complete successfully because of a data stream error. A
    // call to data->GetErrorMessage() will return a human-readable
    // description of the problem.
    //
    // The caller must provide an upperbound on max_item_id which will
    // be used to preallocate buffers.
    //
    // The caller must also specify a bound on the number of 4-byte item
    // ids that will be stored in main memory during algorithm
    // execution. Should the dataset contain more items than the limit,
    // the algorithm will return false and fail to produce correct
    // results.
    //
    // This method may output status & progress messages to stderr.
    bool FindAllMaximalSets(
        DataSourceIterator* data,
        uint32_t max_item_id,
        uint32_t max_items_in_ram,
        std::ostream& os);

    // Returns the number of maximal sets found by the last call to
    // FindAllMaximalSets.
    long MaximalSetsCount() const
    {
        return maximal_sets_count_;
    }

    // Returns the number of itemsets encountered in the input stream
    // during the last call to FindAllMaximalSets.
    long long InputSetsCount() const
    {
        return input_sets_count_;
    }

    // Returns the number of explicit subsumption checks performed by
    // the last call to FindAllMaximalSets.
    long long SubsumptionChecksCount() const
    {
        return subsumption_checks_count_;
    }

private:
    // First method called by FindAllMaximalSets for rudimentary variable
    // initialization.
    void Init();

    // Prepare datastructures for scanning the data beginning at the
    // provided offset. Returns false if IO error encountered.
    bool PrepareForDataScan(
        DataSourceIterator* data, uint32_t max_item_i, off_t seek_offset);

    // Once the occurs_ lists have been populated, this method can be
    // called to determine whether a given candidate is properly
    // subsumed by some other set.
    bool IsSubsumed(const SetProperties& candidate);

    // Invoked for each maximal set found.
    void FoundMaximalSet(const SetProperties& maximal_set, std::ostream& os);

    // Stats variables.
    long maximal_sets_count_;
    long input_sets_count_;
    long long subsumption_checks_count_;

    // Stores a pointer to each indexed itemset.
    OccursList all_sets_;

    // Maps each item to the list of itemsets that contain the item.
    std::vector<OccursList> occurs_;
};

}  // namespace idmlib

#endif  // _ALL_MAXIMAL_SETS_SATELITE_H_
