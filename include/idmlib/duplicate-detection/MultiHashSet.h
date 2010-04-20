/**
   @file MultiHashSet.h
   @author Yeogirl Yun
   @date 2009.11.24
 */
#ifndef MultiHashSet_h
#define MultiHashSet_h 1

#include "PrimeHashTable.h"
//#include <wiselib/basics.h>
#include <wiselib/DArray.h>
#include <wiselib/hash/ExtendibleHash.h>


/*********************************************************************
Description: This has a bug.
@see t_MultiHashSet.cc
Comments   :
 *********************************************************************/
namespace wiselib {
    template <class SubKeyType, class KeyType, class DataType>
        class MultiHashSetEntry {
        public:
            SubKeyType subkey;
            ExtendibleHash<KeyType, DataType> hashset;
            DynamicArray<DataType*> bag;

            inline MultiHashSetEntry() {}
            inline MultiHashSetEntry(const DataType& data) : subkey(data.get_sub_key()) {
                ASSERT(hashset.insert(data));  // first time  for this data
                bag.push_back(hashset.find(data.get_key()));
            }
            inline ~MultiHashSetEntry() {}

            inline const SubKeyType& get_key() const { return subkey; }
            inline bool operator==(const MultiHashSetEntry& src) const { return subkey == src.subkey; }
            /*
            void display(ostream& stream) const {

                stream << "{" << subkey;
                for (int i = 0; i < bag.size(); i++)
                    stream << *bag[i] << " ";
                stream << "}";
            }
            */
        };
    DEF_TEMPLATE_DISPLAY3(MultiHashSetEntry);

    template <class SubKeyType, class KeyType, class DataType>
        class MultiHashSet {
        private:
            PrimeHashTable<SubKeyType, MultiHashSetEntry<SubKeyType, KeyType, DataType> > table;
            PixHT<SubKeyType, MultiHashSetEntry<SubKeyType, KeyType, DataType> > pix;

        public:
            inline MultiHashSet(int estimatedNumUniqueKeys) : table(estimatedNumUniqueKeys) {}
            inline ~MultiHashSet() {}

            // pix iterations.
            inline void set_pix() { table.set_pix(pix); }
            bool is_pix() { return (bool)pix; }
            void next_pix() { pix++; }

            void release() { table.release(); }
            void remove_all(const KeyType& toRemove) {
                MultiHashSetEntry<SubKeyType, KeyType, DataType>* me = table.find(toRemove);
                if (me != NULL)
                    table.del(toRemove);
            }

            int num_elements() {
                int count = 0;

                PixHT<SubKeyType, MultiHashSetEntry<SubKeyType, KeyType, DataType> > p;
                for (table.set_pix(p); p; p++)
                    count += table[p]->hashset.num_items();

                return count;
            }

            int num_distinct_elements() const { return table.num_items();}
            int count_occurrences(const KeyType& key) const {
                const MultiHashSetEntry<SubKeyType, KeyType, DataType>* me = table.find(key);
                if (me)
                    return me->hashset.num_items();
                else
                    return 0;
            }

            // add method
            void add(const DataType& data) {
                MultiHashSetEntry<SubKeyType, KeyType, DataType>* me = table.find(data.get_sub_key());
                if (me) {
                    if (me->hashset.insert(data))
                        me->bag.push_back(me->hashset.find(data.get_key()));
                }
                else
                    table.insert(MultiHashSetEntry<SubKeyType, KeyType, DataType>(data));
            }

            // access methods either by KeyType or Pix.
            const DynamicArray<DataType*>* get_bag(const SubKeyType& key) const {
                MultiHashSetEntry<SubKeyType, KeyType, DataType>* mb = table.find(key);
                if (mb)
                    return &mb->bag;
                else
                    return NULL;
            }
            DynamicArray<DataType*>* get_bag(const SubKeyType& key) {
                MultiHashSetEntry<SubKeyType, KeyType, DataType>* mb = table.find(key);
                if (mb)
                    return &mb->bag;
                else
                    return NULL;
            }

            inline const DynamicArray<DataType*>& get_pix_bag() const {
                return table[pix]->bag;
            }
            inline DynamicArray<DataType*>& get_pix_bag() {
                return table[pix]->bag;
            }
            //closing display
            /*inline void display(ostream& stream) const {
                stream << "[MultiHashSet with " << table.num_items() << " items] ";
            }
            */
        };
    DEF_TEMPLATE_DISPLAY3(MultiHashSet);
}
#endif
