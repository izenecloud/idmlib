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
// Author: Roberto Bayardo

#include <string.h>
#include <iostream>
#include <idmlib/itemset/set-properties.h>

namespace idmlib
{

SetProperties::SetProperties(int id, const ItemSet& items)
        : set_id(id),
        size(items.size())
{
    memcpy(item, &(items[0]), (sizeof(uint32_t) * items.size()));
}

/*static*/
SetProperties* SetProperties::Create(
    uint32_t set_id, const ItemSet& items)
{
    size_t object_size =
        sizeof(SetProperties) + (sizeof(uint32_t) * items.size());
    return new (::operator new (object_size)) SetProperties(set_id, items);
}

/*static*/
void SetProperties::Delete(SetProperties* delete_me)
{
    ::operator delete(delete_me);
}

std::ostream& operator<<(std::ostream& os, const SetProperties& output_me)
{
    os << output_me.set_id << ": ";
    for (uint32_t i = 0; i < output_me.size; ++i)
    {
        if (i != 0)
            os << ' ';
        os << output_me.item[i];
    }
    return os;
}

}  // namespace idmlib
