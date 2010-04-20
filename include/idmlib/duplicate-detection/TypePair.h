#ifndef StringInt_h
#define StringInt_h 1

#include <wiselib/basics.h>
#include <string>
#include <iostream>
using  namespace std;

namespace wiselib {
  template <class Type1, class Type2>
  class TypePair {
  public:
    Type1 first;
    Type2 second;

    inline TypePair() {}
    inline TypePair(const Type1& f, const Type2& s) : first(f), second(s) {}
    inline TypePair(const TypePair& s) : first(s.first), second(s.second) {}
    inline ~TypePair() {}
    TypePair& operator=(const TypePair& s) {
      if (this != &s) {
	first = s.first;
	second = s.second;
      }
      return *this;
    }

    inline const Type1& get_key() const { return first; }
    inline bool operator==(const TypePair& source) const { return (first == source.first); }
    inline bool operator!=(const TypePair& source) const { return (first != source.first); }
    inline bool operator<(const TypePair& s) const { return (second < s.second); }
    inline bool operator>(const TypePair& s) const { return (second > s.second); }
    inline bool operator<=(const TypePair& s) const { return (second <= s.second); }
    inline bool operator>=(const TypePair& s) const { return (second >= s.second); }
    inline void display(ostream& stream) const { stream << "(" << first << ", " << second << ")"; }
  };
  DEF_TEMPLATE_DISPLAY2(TypePair);
}
#endif




