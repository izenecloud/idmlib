
///
/// @file StringDistance.hpp
/// @brief String distance computation util.
/// @author Jinglei <guojia@gmail.com>
/// @date Created 2010-01-02
/// @date Updated 2010-04-30 Migrate to iDMlib.
///  --- Log

#ifndef STRINGDISTANCE_H_
#define STRINGDISTANCE_H_

#include "../idm_types.h"
#include <list>

#include <util/ustring/algo.hpp>
#include <vector>
#include <algorithm>

NS_IDMLIB_UTIL_BEGIN

    
class StringDistanceCommon
{
    
public:
    /**
     * @ brief Compute UString-based edit distance.
     */
    static float getDistance(
            const izenelib::util::UString& s1,
            const izenelib::util::UString& s2)
    {
        izenelib::util::UString ls1(s1);
        izenelib::util::UString ls2(s2);
        ls1.toLowerString();
        ls2.toLowerString();
        const unsigned int HEIGHT = ls1.length() + 1;
        const unsigned int WIDTH = ls2.length() + 1;
        unsigned int eArray[HEIGHT][WIDTH];
        unsigned int i;
        unsigned int j;

        for (i = 0; i < HEIGHT; i++)
            eArray[i][0] = i;

        for (j = 0; j < WIDTH; j++)
            eArray[0][j] = j;

        for (i = 1; i < HEIGHT; i++) {
            for (j = 1; j < WIDTH; j++) {
                eArray[i][j] = min(
                        eArray[i - 1][j - 1] +
                            (ls1[i-1] == ls2[j-1] ? 0 : 1),
                        min(eArray[i - 1][j] + 1, eArray[i][j - 1] + 1));
            }
        }

       return eArray[HEIGHT - 1][WIDTH - 1];

    }
    
    
};


template< class T >
class NameGetter
{
public:
    static izenelib::util::UString getTerm(const T& data, uint32_t i) 
    {
        return data.getTerm(i);
    }
    
    static std::vector<izenelib::util::UString> getTermList(const T& data) 
    {
        return data.getTermList();
    }
    
    static izenelib::util::UString getName(const T& data) 
    {
        return data.getName();
    }
};

template<>
class NameGetter<std::vector<izenelib::util::UString> >
{
public:
    static izenelib::util::UString& getTerm(std::vector<izenelib::util::UString>& data, uint32_t i)
    {
        return data[i];
    }
    
    static std::vector<izenelib::util::UString>& getTermList(std::vector<izenelib::util::UString>& data)
    {
        return data;
    }
    
    static izenelib::util::UString& getName(std::vector<izenelib::util::UString>& data)
    {
        return data[0];
    }
};


template <class T, template <class Z> class Container >
class StringDistance
{
typedef std::vector<T> T_LIST;
public:
    /**
     * @ brief check a label's uniqueness against a lexicon.
     */
    static inline bool isDuplicate(
            const T& str,
            const Container< T >& strList)
    {
        
        if(str.size()==0) return true;
        izenelib::util::UString firstTerm = NameGetter<T>::getTerm(str, 0);
        if(firstTerm.length()==0) return true;
        if(firstTerm.isChineseChar(0))
            return isChineseDuplicate_(str,strList);

        typename Container< T >::const_iterator it=strList.begin();
        typename Container< T >::const_iterator itEnd= strList.end();
        for(;it!=itEnd;it++)
        {
            if(it->size()!=str.size() ||
                !isFirstCharacterEqual_( NameGetter<T>::getTermList(*it)
                , NameGetter<T>::getTermList(str) ) )
            {
                continue;
            }
            float dis=StringDistanceCommon::getDistance(NameGetter<T>::getName(*it)
                , NameGetter<T>::getName(str));
            if(dis<4)
            {
                return true;
            }

        }
        return false;
    }

    /**
     * @ brief check a label's uniqueness against a lexicon and a given query.
     */
    static inline bool isDuplicate(
            const T& str,
            const std::vector<izenelib::util::UString>& normalizedQueryTermList,
            const izenelib::util::UString& queryStr)
    {
        uint32_t size = str.size();
        if(size==0)
            return true;
        izenelib::util::UString firstTerm = NameGetter<T>::getTerm(str, 0);
        if(firstTerm.length()==0) return true;
        izenelib::util::UString name = NameGetter<T>::getName(str);
        if(firstTerm.isChineseChar(0))
        {
            
            if(name==queryStr)
            {
                std::cout<<"The query and the label is equal!"<<std::endl;
                return true;
            }
            else if(size<queryStr.length())
            {
                if(queryStr.find(name)!=izenelib::util::UString::NOT_FOUND)
                    return true;
            }
            else if(size-queryStr.length()==1)
            {
                if(StringDistanceCommon::getDistance(name, queryStr)==1)
                    return true;
            }
            else
            {
                return (isChineseDuplicate_(NameGetter<T>::getTermList(str),normalizedQueryTermList));
            }

        }

        if(size!=normalizedQueryTermList.size()
                ||!isFirstCharacterEqual_(NameGetter<T>::getTermList(str), normalizedQueryTermList))
            return false;
        float dis=StringDistanceCommon::getDistance(name,queryStr);
        if(dis<4)
        {
            return true;
        }
        return false;

    }
   
private:
    /**
     * @ brief Do pre-filtering for English label de-duplication.
     */
    static bool isFirstCharacterEqual_(
            const std::vector<izenelib::util::UString>& label1,
            const std::vector<izenelib::util::UString>& label2
            )
    {
        assert(label1.size()==label2.size());

        for(unsigned int i=0;i<label1.size();i++)
        {

            if(!label1[i].length()||!label2[i].length())
                return false;
            if(!is_alphabet<uint16_t>::equal_ignore_case(label1[i].at(0),label2[i].at(0)))
                return false;
        }
        return true;

    }
    
    /**
     * @ brief check a label's uniqueness against a lexicon for Chinese.
     */
    static bool isChineseDuplicate_(
            const T& str,
            const Container< T >& lexicon
            )
    {
         typename Container< T >::const_iterator it=lexicon.begin();
         typename Container< T >::const_iterator itEnd= lexicon.end();
         for(;it!=itEnd;it++)
         {
             bool ret=isChineseDuplicate_(str, *it);
             if(ret)
             {
                 return true;
             }
         }
         return false;
    }

    /**
     * @ brief check whether two Chinese labels are duplicates of each other.
     */
    static bool isChineseDuplicate_(
            const T& str1,
            const T& str2)
    {

        if(str1.size()<str2.size())
        {
            if(StringDistanceCommon::getDistance(NameGetter<T>::getName(str1), NameGetter<T>::getName(str2))<2)
                return true;
        }
        else if(NameGetter<T>::getTermList(str1).size()<NameGetter<T>::getTermList(str2).size())
        {
            if(NameGetter<T>::getName(str2).find(NameGetter<T>::getName(str1))!=izenelib::util::UString::NOT_FOUND)
                return true;
        }
        return isChineseDuplicate_(NameGetter<T>::getTermList(str1), NameGetter<T>::getTermList(str2));

    }

    /**
     * @ brief check whether two Chinese labels are duplicates of each other.
     */
    static bool isChineseDuplicate_(
            const std::vector<izenelib::util::UString>& label1,
            const std::vector<izenelib::util::UString>& label2)
    {
        if(label1.size()==0||label2.size()==0)
        {
            return false;
        }
        if(label1.size()>2&&label2.size()>2
                &&label1[0]==label2[0]
                &&label1[label1.size()-1]==label2[label2.size()-1]
           )
        {
            return true;
        }
        if(label1.size()<=label2.size())
        {
            std::vector<izenelib::util::UString> tempLabel1(label1.begin(), label1.end());
            std::sort(tempLabel1.begin(), tempLabel1.end());
            std::vector<izenelib::util::UString> tempLabel2(label2.begin(), label2.end());
            std::sort(tempLabel2.begin(), tempLabel2.end());
            izenelib::util::UString tempStr1, tempStr2;
            for(uint32_t i=0;i<tempLabel1.size();i++)
            {
                tempStr1+=tempLabel1[i];
                tempStr2+=tempLabel2[i];
            }
            if(StringDistanceCommon::getDistance(tempStr1, tempStr2)==0||(tempStr1.length()>2&&StringDistanceCommon::getDistance(tempStr1, tempStr2)<2))
            {
                return true;
            }
        }
        return false;

    }

};


NS_IDMLIB_UTIL_END

#endif /* LABELDISTANCE_H_ */
