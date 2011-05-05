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
// SetProperties: the class used to represent an itemset.
// ---
// Author: Roberto Bayardo
//

#ifndef _SET_PROPERTIES_H_
#define _SET_PROPERTIES_H_

#include "data-source-iterator.h"

namespace idmlib{

class SetProperties
{
public:
    const uint32_t* begin() const
    {
        return item;
    }
    const uint32_t* end() const
    {
        return item + size;
    }
    uint32_t operator[](uint32_t idx) const
    {
        return item[idx];
    }

    uint32_t set_id;
    uint32_t size;
    uint32_t item[0];

    // Factory method for constructing SetProperties objects. Objects
    // allocated with this factory must be released by
    // Delete().
    static SetProperties* Create(
        uint32_t set_id, const ItemSet& items);

    // Releases memory used by the SetProperties object, which must have
    // been constructed with the Create factory method.
    static void Delete(SetProperties* s);

private:
    // Do not use this constructor directly. Use Create().
    SetProperties(int set_id, const ItemSet& items);
};

std::ostream& operator<<(std::ostream&, const SetProperties& output_me);

struct SetPropertiesCompareFunctor
{
    bool operator()(SetProperties* s1, SetProperties* s2) const
    {
        uint32_t* s1_it = s1->item;
        uint32_t* s2_it = s2->item;
        const uint32_t* s1_end = s1->item + s1->size;
        const uint32_t* s2_end = s2->item + s2->size;
        while (s1_it != s1_end && s2_it != s2_end)
        {
            if (*s1_it < *s2_it)
                return true;
            if (*s1_it > *s2_it)
                return false;
            ++s1_it;
            ++s2_it;
        }
        return s2_it == s2_end ? false : true;
    }
};


// For sorting first by cardinality and then by lexicographic order.
struct SetPropertiesCardinalityCompareFunctor
            : public SetPropertiesCompareFunctor
{
    bool operator()(SetProperties* s1, SetProperties* s2) const
    {
        if (s1->size != s2->size)
            return s1->size < s2->size;
        return SetPropertiesCompareFunctor::operator()(s1, s2);
    }
};

}  // namespace idmlib

#endif  // _SET_PROPERTIES_H_
