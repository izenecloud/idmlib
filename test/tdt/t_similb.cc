#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <idmlib/tdt/similb.h>
#include <idmlib/tdt/ema.h>

BOOST_AUTO_TEST_SUITE( t_similb )

using namespace idmlib::tdt;

template <class T>
void PrintVec(const std::string& info, const std::vector<T>& vec)
{
    std::cout<<info;
    for(std::size_t i=0;i<vec.size();i++)
    {
        if(i!=0)
        {
            std::cout<<",";
        }
        std::cout<<vec[i];
    }
    std::cout<<std::endl;
}

template <class T>
void PrintVec2(const std::string& info, const std::vector<T>& vec)
{
    std::cout<<info<<std::endl;
    for(std::size_t i=0;i<vec.size();i++)
    {
        std::cout<<i<<"\t"<<vec[i]<<std::endl;
    }
}

BOOST_AUTO_TEST_CASE(shape_test )
{
    std::vector<double> s1(10), s2(10), s3(10);
    s1[0] = 3.5;
    s1[1] = 3;
    s1[2] = 4;
    for (int i = 3; i < 10; i++) 
        s1[i] = s1[i-1] + 1./14.;
    s2[0] = 2;
    s2[1] = 2;
    for (int i = 2; i < 10; i++)
        s2[i] = 3;

    s3[0] = 0.5;
    s3[1] = 1;
    s3[2] = 2;
    for (int i = 3; i < 10; i++)
        s3[i] = s3[i-1] - 1./14.;
    
    std::vector<double> psi1;
    SimilB::GetPsi(s1, s1, psi1);
    PrintVec("psi1 : ", psi1);
    std::cout << "The Similarity Value for s1 and s1: " << SimilB::Sim(s1, s1) << std::endl;
    std::cout << "The Similarity Value for s2 and s2: " << SimilB::Sim(s2, s2) << std::endl;
    std::cout << "The Similarity Value for s3 and s3: " << SimilB::Sim(s3, s3) << std::endl;
    std::cout << "The Similarity Value for s1 and s2: " << SimilB::Sim(s1, s2) << std::endl;
    std::cout << "The Similarity Value for s2 and s1: " << SimilB::Sim(s2, s1) << std::endl;
    std::cout << "The Similarity Value for s2 and s3: " << SimilB::Sim(s2, s3) << std::endl;
    std::cout << "The Similarity Value for s3 and s2: " << SimilB::Sim(s3, s2) << std::endl;
}

BOOST_AUTO_TEST_CASE(perturb_test )
{
    std::vector<double> s1(500), s2(500);
    for (int i = 0; i < 500; i++) {
        s1[i] = sin((double)i);
        s2[i] = sin((double)i);
        if (i == 200)
            s2[i] = 4;
    }
    std::cout << "perturb_test The Similarity Value for s1 and s2: " << SimilB::Sim(s1, s2) << std::endl; 
    int SPAN = 25;
    for (int i = 0; i < 475; i++)
    {
        std::cout << "perturb_test The Similarity Value for s1 and s2 on [" << i << ", " << i + SPAN << "] -> " << SimilB::Sim(s1, s2, i, i+SPAN) << std::endl;   
    }
//     std::vector<double> psi;
//     SimilB::GetPsi(s1, s2, psi);
//     PrintVec2("perturb_test psi : ", psi);
}

BOOST_AUTO_TEST_CASE(chirp_test )
{
  std::vector<double> s1(250), s2(250);

  int freq = 1;
  for (int i = 0; i < 250; i++, freq++) {
    s1[i] = sin((double)freq * (double)i);
    s2[i] = sin((double)(250 - freq) * (double)i);
  }
  std::vector<double> psi;
  SimilB::GetPsi(s1, s2, psi);
//   PrintVec2("chirp_test psi : ", psi);
//   std::cout << "s1: " << s1 << std::endl;
//   std::cout << "s2: " << s2 << std::endl;
  
  std::cout << "chirp_test The Similarity Value for s1 and s2: " << SimilB::Sim(s1, s2) << std::endl;  

  int SPAN = 25;
  for (int i = 0; i < 225; i++)
    std::cout << "chirp_test The Similarity Value for s1 and s2 on [" << i << ", " << i + SPAN << "] -> " << SimilB::Sim(s1, s2, i, i+SPAN) << std::endl;   
}

// BOOST_AUTO_TEST_CASE(chirp_test2 )
// {
//   std::vector<double> s1(250), s2(250);
// 
//   double step = 0.1/150;
//   for (int i = 0; i < 250; i++) {
//     s1[i] = 0.05 + step*i;
//   }
//   
//   for (int i = 0; i < 250; i++) {
//     s2[i] = 0.05 + step* (250-i);
//   }
// //   PrintVec("chirp_test2 s1 : ", s1);
// //   PrintVec("chirp_test2 s2 : ", s2);
// 
//   std::vector<double> psi1;
//   SimilB::GetPsi(s1, s1, psi1);
// //   std::cout<<psi1.size()<<std::endl;
//   PrintVec("chirp_test2 psi1 : ", psi1);
//   std::vector<double> psi2;
//   SimilB::GetPsi(s2, s2, psi2);
//   PrintVec("chirp_test2 psi2 : ", psi2);
//   
//   std::vector<double> psi;
//   SimilB::GetPsi(s1, s2, psi);
//   PrintVec("chirp_test2 psi : ", psi);
//   std::cout << "chirp_test2 The Similarity Value for s1 and s1: " << SimilB::Sim(s1, s1) << std::endl;  
//   
//   std::cout << "chirp_test2 The Similarity Value for s2 and s2: " << SimilB::Sim(s2, s2) << std::endl;  
//   
//   std::cout << "chirp_test2 The Similarity Value for s1 and s2: " << SimilB::Sim(s1, s2) << std::endl;  
// 
// }




BOOST_AUTO_TEST_SUITE_END()
