Data mining libraries
=============================================
*A general data mining C++ library*

### Features
* _Keyphrase Extraction_. We've implemented two kinds of keyphrase extraction approaches. One refers to the translation model from thesis work of [Zhiyuan Liu](http://nlp.csai.tsinghua.edu.cn/~lzy/), the other comes from our innovatin which uses Wiki data as the semantic knowledge base.

* _Taxonomy Generation_. 

* _Duplicate Detection_.  Read the paper `Detecting Near-Duplicates for Web Crawling` firstly then we could understand the algorithm. We used the famous Charikar simhash fingerprints generation approach and set the dimensions(f) to 64.

* _Ctr Prediction_. We've implemented both [AdPredictor](http://machinelearning.wustl.edu/mlpapers/paper_files/icml2010_GraepelCBH10.pdf) and [FTRL](https://research.google.com/pubs/archive/41159.pdf).

* _Chinese Query Correction_. 


* _Collaborative Filtering_.  This is an item-based incremental collaborative filtering.


* _Others_.


### Dependencies
We've just switched to `C++ 11` for SF1R recently, and `GCC 4.8` is required to build SF1R correspondingly. We do not recommend to use Ubuntu for project building due to the nested references among lots of libraries. CentOS / Redhat / Gentoo / CoreOS are preferred platform. You also need `CMake` and `Boost 1.56` to build the repository . Here are the dependent repositories list:

* __[cmake](https://github.com/izenecloud/cmake)__: The cmake modules required to build all iZENECloud C++ projects.


* __[izenelib](https://github.com/izenecloud/izenelib)__: The general purpose C++ libraries.


* __[icma](https://github.com/izenecloud/icma)__: The Chinese morphological analyzer library.
  

* __[ijma](https://github.com/izenecloud/ijma)__: The Japanese morphological analyzer library.


* __[ilplib](https://github.com/izenecloud/ilplib)__: The language processing libraries.
  


### License
The project is published under the Apache License, Version 2.0:
http://www.apache.org/licenses/LICENSE-2.0
