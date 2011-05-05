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
// ---
// An algorithm for finding all maximal sets based on the approach
// used by SateLite (minus the bloom filters).
// ---
// Author: Roberto Bayardo

#include <idmlib/itemset/data-source-iterator.h>
#include <idmlib/itemset/all-maximal-sets-satelite.h>
#include <idmlib/itemset/set-properties.h>

#include <assert.h>
#include <algorithm>
#include <iostream>
#include <vector>

namespace idmlib
{

namespace
{

// Returns true if set2 properly subsumes set1.
inline bool IsSubsumedBy(const SetProperties& set1, const SetProperties& set2)
{
    if (set1.size >= set2.size)
        return false;
    const uint32_t* it1 = set1.begin();
    const uint32_t* it2 = set2.begin();
    const uint32_t* it1_end = set1.end();
    const uint32_t* it2_end = set2.end();

    while (it1 != it1_end)
    {
        it2 = std::lower_bound(it2, it2_end, *it1);
        if (it2 == it2_end || *it2 > *it1)
            return false;
        ++it1;
        ++it2;
    }
    return true;
}

// Used to free up all itemset resources when it goes out of scope.
class CleanerUpper
{
public:
    CleanerUpper(OccursList* all_sets) :
            all_sets_(all_sets) {}
    ~CleanerUpper()
    {
        for (unsigned int i = 0; i < all_sets_->size(); ++i)
        {
            SetProperties::Delete((*all_sets_)[i]);
        }
        all_sets_->clear();
    }
private:
    OccursList* all_sets_;
};

}  // namespace

bool AllMaximalSetsSateLite::FindAllMaximalSets(
    DataSourceIterator* data,
    uint32_t max_item_id,
    uint32_t max_items_in_ram,
    std::ostream& os)
{
    Init();

    // Vars set by the data source iterator.
    int result;
    uint32_t set_id;
    std::vector<uint32_t> current_set;

    if (!PrepareForDataScan(data, max_item_id, 0))
        return false;  // IO error
    uint32_t items_in_ram = 0;
    CleanerUpper cleanup(&all_sets_);

    // This loop scans the input data from beginning to end and indexes
    // each candidate on the occurrs_ lists.
    while ((result = data->Next(&set_id, &current_set)) > 0)
    {
        SetProperties* index_me = SetProperties::Create(set_id, current_set);
        items_in_ram += current_set.size();
        all_sets_.push_back(index_me);
        if (items_in_ram >= max_items_in_ram)
        {
            std::cerr << "; ERROR: max_items_in_ram exceeded." << std::endl;
            return false;
        }
        ++input_sets_count_;
        for (unsigned int i = 0; i < index_me->size; ++i)
        {
            occurs_[index_me->item[i]].push_back(index_me);
        }
    }
    if (result != 0)
        return false;  // IO error

    std::cerr << "; Starting subsumption checking scan." << std::endl;
    for (unsigned int i = 0; i < all_sets_.size(); ++i)
    {
        if (!IsSubsumed(*all_sets_[i]))
            FoundMaximalSet(*(all_sets_[i]),os);
    }
    return true;
}

void AllMaximalSetsSateLite::Init()
{
    maximal_sets_count_ = input_sets_count_ = subsumption_checks_count_ = 0;
}

bool AllMaximalSetsSateLite::PrepareForDataScan(
    DataSourceIterator* data, uint32_t max_item_id, off_t resume_offset)
{
    occurs_.clear();
    occurs_.resize(max_item_id);
    std::cerr << "; Starting new dataset scan at offset: "
              << resume_offset << std::endl;
    return data->Seek(resume_offset);
}

bool AllMaximalSetsSateLite::IsSubsumed(const SetProperties& candidate)
{
    const OccursList& occurs = occurs_[candidate[0]];
    for (unsigned int j = 0; j < occurs.size(); ++j)
    {
        SetProperties* check_me = occurs[j];
        ++subsumption_checks_count_;
        if (IsSubsumedBy(candidate, *check_me))
            return true;
    }
    return false;
}

void AllMaximalSetsSateLite::FoundMaximalSet(
    const SetProperties& maximal_set,
    std::ostream& os
    )
{
    ++maximal_sets_count_;
    for (unsigned int i = 0; i < maximal_set.size; ++i)
    {
        os << ' ' << maximal_set.item[i];
    }
    os<<std::endl;
}

}  // namespace idmlib
