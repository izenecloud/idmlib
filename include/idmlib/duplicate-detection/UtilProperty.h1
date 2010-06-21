/**
 * @file UtilProperty.h
 * @author Jinglei
 * @date   2009.11.24
 * @brief  Used to read the parameters from property configuring file.
*/

#ifndef UTILPROPERTY_H_
#define UTILPROPERTY_H_

#include <map>
#include <string>
#include <iostream>

class UtilProperty
{
    enum { VERBOSE = 0 };

public:

    typedef std::map<std::string, std::string> PropertyMapT;
    typedef PropertyMapT::value_type           value_type;
    typedef PropertyMapT::iterator             iterator;

    static void read(const char *filename, PropertyMapT &map);
    static void read(std::istream &is, PropertyMapT &map);
    static void write(const char *filename, PropertyMapT &map, const char *header = NULL);
    static void write(std::ostream &os, PropertyMapT &map, const char *header = NULL);
    static void print(std::ostream &os, PropertyMapT &map);

private:

    static inline char m_hex(int nibble)
    {
        static const char *digits = "0123456789ABCDEF";
        return digits[nibble & 0xf];
    }
};


#endif /* UTILPROPERTY_H_ */
