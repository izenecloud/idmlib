#ifndef IntKey_h
#define IntKey_h

#include <wiselib/basics.h>
#include <fstream>
#include <iostream>

// generic typekey for integral types.
namespace wiselib {
    template <class KeyType>
    class TypeKey {
    public:
        inline TypeKey() : key(0) {}
        inline TypeKey(const KeyType& k) : key(k) {}
        KeyType key;

        inline TypeKey(const TypeKey& s) : key(s.key) {}
        inline ~TypeKey() {}
        TypeKey& operator=(const TypeKey& s) {
            if (this != &s)
                key = s.key;
            return *this;
        }
        // AccessMethods and HashFunciton interfaces
        inline const char* c_str() const { return (const char*)&key; }
        inline size_t size() const { return sizeof(KeyType); }
        inline const TypeKey& get_key() const { return *this; }
        inline TypeKey& get_key() { return *this; }
        // AccessMethods and DyanmicArray interfaces
        inline int compare(const TypeKey& s) const { if (key > s.key) return 1; else if (key < s.key) return -1; else return 0; }
        friend int operator-(const TypeKey& s1, const TypeKey& s2) { return s1.compare(s2); }
        inline bool operator==(const TypeKey& source) const { return (key == source.key); }
        inline bool operator!=(const TypeKey& source) const { return (key != source.key); }
        inline bool operator<(const TypeKey& s) const { return (key < s.key); }
        inline bool operator>(const TypeKey& s) const { return (key > s.key); }
        inline bool operator<=(const TypeKey& s) const { return (key <= s.key); }
        inline bool operator>=(const TypeKey& s) const { return (key >= s.key); }

        inline void display(std::ostream& stream) const { stream << key; }
        template<class Archive> void serialize(Archive & ar,
            				const unsigned int version) {
        	ar & key;
          }
    };
        DEF_TEMPLATE_DISPLAY1(TypeKey);
};
#endif
