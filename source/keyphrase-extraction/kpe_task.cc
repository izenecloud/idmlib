#include <idmlib/keyphrase-extraction/kpe_task.h>
#include <idmlib/translation/ibm_model1.h>
#include <util/functional.h>
#include <boost/unordered_map.hpp>
using namespace idmlib::kpe;

KpeTask::KpeTask(const std::string& dir, idmlib::util::IDMAnalyzer* analyzer, idmlib::util::IDMAnalyzer* cma_analyzer, idmlib::util::IDMIdManager* id_manager , KpeKnowledge* knowledge)
: dir_(dir), analyzer_(analyzer), cma_analyzer_(cma_analyzer), id_manager_(id_manager), knowledge_(knowledge)
, max_phrase_len_(7)
, title_writer_(NULL), ngram_writer_(NULL), kp_writer_(NULL)
{
    Init_();
}

KpeTask::~KpeTask()
{
    Release_();
}

void KpeTask::SetCallback(const DocKpCallback& dk_callback)
{
    dk_callback_ = dk_callback;
}


void KpeTask::SetNoFreqLimit()
{
    no_freq_limit_ = true;
}

void KpeTask::AddManMade(const StringType& ustr)
{
    std::vector<idmlib::util::IDMTerm> term_list;
    analyzer_->GetTgTermList(ustr, term_list );
    std::vector<uint32_t> termid_list(term_list.size());
    for(uint32_t i=0;i<term_list.size();i++)
    {
        termid_list[i] = term_list[i].id;
    }
    manmade_.insert(termid_list,0);
}

void KpeTask::SetTracing(const StringType& ustr)
{
    std::vector<idmlib::util::IDMTerm> term_list;
    analyzer_->GetTgTermList(ustr, term_list );
    tracing_.resize(term_list.size());
    std::cout<<"[tracing] ";
    for(uint32_t i=0;i<term_list.size();i++)
    {
        tracing_[i] = term_list[i].id;
        std::cout<<tracing_[i]<<",";
    }
    std::cout<<std::endl;
}

void KpeTask::SetMaxPhraseLen(uint8_t max_len)
{
    max_phrase_len_ = max_len;
}


void KpeTask::Insert( uint32_t docid, const StringType& title, const StringType& content)
{
    if(title.length()==0 || content.length()==0 ) return;
    std::string title_str;
    std::string content_str;
    title.convertString(title_str, StringType::UTF_8);
    content.convertString(content_str, StringType::UTF_8);
    title_writer_->Append(docid, title_str);
    title_writer_->Append(docid, content_str);
    {
        std::vector<idmlib::util::IDMTerm> term_list;
        analyzer_->GetTgTermList(title, term_list );
        if( !term_list.empty() )
        {
            uint32_t begin = 0;
            while( AddTerms_(docid, term_list, begin) ) {}
        }
    }
    {
        std::vector<idmlib::util::IDMTerm> term_list;
        analyzer_->GetTgTermList(content, term_list );
        if( !term_list.empty() )
        {
            uint32_t begin = 0;
            while( AddTerms_(docid, term_list, begin) ) {}
        }
    }
    ++doc_count_;
    last_doc_id_ = docid;
}

bool KpeTask::Close()
{
    return Compute_();
}



uint32_t KpeTask::GetDocCount() const
{
    return doc_count_;
}


bool KpeTask::Compute_()
{
    uint64_t ngram_count = ngram_writer_->Count();
    if(ngram_count==0) return true;
    ngram_writer_->Close();
    std::string ngram_path = ngram_writer_->GetPath();
    delete ngram_writer_;
    ngram_writer_ = NULL;


    //set thresholds
    uint32_t docCount = GetDocCount();
    std::cout<<"[KPE] running for "<<docCount<<" docs, ngram count: "<<ngram_count<<std::endl;
    min_freq_threshold_ = 1;
    min_df_threshold_ = 1;
    if(!no_freq_limit_)
    {
        if( docCount >=10 )
        {
            min_freq_threshold_ = (uint32_t)std::floor( std::log( (double)docCount )/2 );
            if( min_freq_threshold_ < 3 ) min_freq_threshold_ = 3;

            min_df_threshold_ = (uint32_t)std::floor( std::log( (double)docCount )/4 );
            if( min_df_threshold_ < 2 ) min_df_threshold_ = 2;
        }
    }


//       if(!tracing_.empty())
//       {
//         typename TermListSSFType::ReaderType fireader(inputItemPath);
//         fireader.open();
//         std::vector<uint32_t> keyList;
//         while( fireader.nextKeyList(keyList) )
//         {
//           if(VecStarts_(keyList, tracing_))
//           {
//             std::cout<<"[tracing] [before-suffix] ";
//             for(uint32_t dd=0;dd<keyList.size();dd++)
//             {
//               std::cout<<keyList[dd]<<",";
//             }
//             std::cout<<std::endl;
//           }
//         }
//         fireader.close();
//       }
    izenelib::am::ssf::Sorter<uint32_t, uint32_t, true>::Sort(ngram_path);

//       if(!tracing_.empty())
//       {
//         typename TermListSSFType::ReaderType fireader(inputItemPath);
//         fireader.open();
//         std::vector<uint32_t> keyList;
//         while( fireader.nextKeyList(keyList) )
//         {
//           if(VecStarts_(keyList, tracing_))
//           {
//             std::cout<<"[tracing] [after-suffix] ";
//             for(uint32_t dd=0;dd<keyList.size();dd++)
//             {
//               std::cout<<keyList[dd]<<",";
//             }
//             std::cout<<std::endl;
//           }
//         }
//         fireader.close();
//       }
    MEMLOG("[KPE3] finished");

    if(!ProcessNgram_(ngram_path)) return false;
    if(!ProcessFoundKp2_()) return false;
    return true;


}

bool KpeTask::ProcessFoundKp_()
{
//     std::vector<StringType> kp_text_list;
//     std::vector<uint32_t> kp_freq_list;
//     std::string title_file = title_writer_->GetPath();
//     title_writer_->Close();
//     delete title_writer_;
//     title_writer_ = NULL;
//     std::string kp_file = kp_writer_->GetPath();
//     kp_writer_->Close();
//     delete kp_writer_;
//     kp_writer_ = NULL;
//     izenelib::am::ssf::Writer<> dk_tmp_writer(dir_+"/doc_kp_distribute");
//     dk_tmp_writer.Open();
//     izenelib::am::ssf::Reader<> reader(kp_file);
//     reader.Open();
//     hash_t hash_id;
//     KPItem item;
//     while( reader.Next(hash_id, item) )
//     {
//         kp_text_list.push_back(item.text);
//         uint32_t kpid = kp_text_list.size();
//
//         double idf = std::log( GetDocCount()/(double)item.docitem_list.size() );
//         for(uint32_t i=0;i<item.docitem_list.size();i++)
//         {
//             uint32_t docid = item.docitem_list[i].first;
//             uint32_t tf = item.docitem_list[i].second;
//             double tfidf = tf*idf;
//             dk_tmp_writer.Append(docid, std::make_pair(kpid, tfidf) );
//         }
//     }
//     reader.Close();
//     dk_tmp_writer.Close();
//     kp_text_list.push_back(StringType("", StringType::UTF_8));
//     kp_freq_list.resize(kp_text_list.size(), 0);
//     izenelib::am::ssf::Sorter<uint32_t, uint32_t>::Sort(dk_tmp_writer.GetPath());
//
//     boost::unordered_map<std::string, uint32_t> term_id_map;
//     boost::unordered_map<std::string, uint32_t>::iterator term_id_map_it;
//     boost::unordered_map<uint32_t, uint32_t> df_map;
//     boost::unordered_map<uint32_t, uint32_t>::iterator df_map_it;
//     {
//         izenelib::am::ssf::Joiner<uint32_t, uint32_t, std::string> joiner(title_file);
//         joiner.Open();
//         std::vector<std::string> title_list;
//         uint32_t docid = 0;
//         while(joiner.Next(docid, title_list))
//         {
//             if(title_list.size()!=2)
//             {
//                 std::cout<<"title list size error "<<title_list.size()<<std::endl;
//                 continue;
//             }
//             std::string title_str = title_list[0];
//             std::string content_str = title_list[1];
//             StringType title(title_str, StringType::UTF_8);
//             StringType content(content_str, StringType::UTF_8);
//
//
// //             std::cout<<"[docid] "<<docid<<std::endl;
// //             std::cout<<"[title] "<<title_str<<std::endl;
// //             std::cout<<"[content] "<<content_str<<std::endl;
//             std::vector<StringType> title_str_list;
//             cma_analyzer_->GetFilteredStringList(title, title_str_list);
//             std::vector<StringType> content_str_list;
//             cma_analyzer_->GetFilteredStringList(content, content_str_list);
//
//             for(uint32_t i=0;i<title_str_list.size();i++)
//             {
//                 const StringType& term = title_str_list[i];
//                 uint32_t term_id = 0;
//                 std::string str_term;
//                 term.convertString(str_term, StringType::UTF_8);
// //                 std::cout<<"[title-term] "<<str_term<<std::endl;
//                 if(str_term.empty()) continue;
//                 term_id_map_it = term_id_map.find(str_term);
//                 if(term_id_map_it==term_id_map.end())
//                 {
//                     term_id = kp_text_list.size()+1;
//                     term_id_map.insert(std::make_pair(str_term, term_id));
//                     kp_text_list.push_back(term);
//                     kp_freq_list.push_back(1);
//                 }
//                 else
//                 {
//                     term_id = term_id_map_it->second;
//                     kp_freq_list[term_id-1] += 1;
//                 }
//             }
//             boost::unordered_map<uint32_t, bool> df_exist;
//             boost::unordered_map<uint32_t, bool>::iterator df_exist_it;
//
//             std::vector<uint32_t> content_id_list;
//             for(uint32_t i=0;i<content_str_list.size();i++)
//             {
//                 const StringType& term = content_str_list[i];
//                 uint32_t term_id = 0;
//                 std::string str_term;
//                 term.convertString(str_term, StringType::UTF_8);
// //                 std::cout<<"[content-term] "<<str_term<<std::endl;
//                 if(str_term.empty()) continue;
//                 term_id_map_it = term_id_map.find(str_term);
//                 if(term_id_map_it==term_id_map.end())
//                 {
//                     term_id = kp_text_list.size()+1;
//                     term_id_map.insert(std::make_pair(str_term, term_id));
//                     kp_text_list.push_back(term);
//                     kp_freq_list.push_back(1);
//                 }
//                 else
//                 {
//                     term_id = term_id_map_it->second;
//                     kp_freq_list[term_id-1] += 1;
//                 }
// //                 std::cout<<"[content-id] "<<term_id<<std::endl;
//                 df_exist_it = df_exist.find(term_id);
//                 if(df_exist_it==df_exist.end())
//                 {
//                     df_exist.insert(std::make_pair(term_id, true));
//                     content_id_list.push_back(term_id);
//                     df_map_it = df_map.find(term_id);
//                     if(df_map_it == df_map.end())
//                     {
//                         df_map.insert(std::make_pair(term_id, 1));
//                     }
//                     else
//                     {
//                         df_map_it->second += 1;
//                     }
//                 }
//             }
//         }
//
//     }
//
//
//
//     izenelib::am::ssf::Reader<> reader1(title_file);
//     reader1.Open();
//     izenelib::am::ssf::Reader<> reader2(dk_tmp_writer.GetPath());
//     reader2.Open();
//     std::ofstream other_ofs("./other");
//     izenelib::am::ssf::Merger<uint32_t, uint32_t, std::string, std::pair<uint32_t, double> > merger(&reader1, &reader2);
//     uint32_t docid = 0;
//     std::vector<std::string> title_list;
//     std::vector<std::pair<uint32_t, double> > kp_list;
//
//     typedef izenelib::util::second_greater<std::pair<uint32_t, double> > greater_than;
//
//     while( merger.Next( docid, title_list, kp_list) )
//     {
//         if(title_list.size()!=2)
//         {
//             std::cout<<"title list size error "<<title_list.size()<<std::endl;
//             continue;
//         }
//         std::string title_str = title_list[0];
//         std::string content_str = title_list[1];
//         StringType title(title_str, StringType::UTF_8);
//         StringType content(content_str, StringType::UTF_8);
//
//         std::vector<StringType> title_str_list;
//         cma_analyzer_->GetFilteredStringList(title, title_str_list);
//         std::vector<StringType> content_str_list;
//         cma_analyzer_->GetFilteredStringList(content, content_str_list);
//
// //         {
// //             std::cout<<"[docid "<<docid<<"] ";
// //             for(uint32_t i=0;i<str_list.size();i++)
// //             {
// //                 std::string str;
// //                 str_list[i].convertString(str, izenelib::util::UString::UTF_8);
// //                 std::cout<<str<<",";
// //             }
// //             std::cout<<std::endl;
// //         }
//
//
//         std::sort(kp_list.begin(), kp_list.end(), greater_than());
//         uint32_t count = title_str_list.size()>kp_list.size()?kp_list.size():title_str_list.size();
//         if(count==0) continue;
//         std::vector<uint32_t> doc_kp_list;
//         for(uint32_t i=0;i<count;i++)
//         {
//             const StringType& title_term = title_str_list[i];
//             std::string str_term;
//             title_term.convertString(str_term, StringType::UTF_8);
//             if(str_term.empty()) continue;
//             term_id_map_it = term_id_map.find(str_term);
//             uint32_t term_id = term_id_map_it->second;
//             doc_kp_list.push_back(term_id);
//         }
//         doc_kp_list.push_back(0);
//         for(uint32_t i=0;i<count;i++)
//         {
//             uint32_t kpid = kp_list[i].first;
//             kp_freq_list[kpid-1] += 1;
//             doc_kp_list.push_back(kpid);
//         }
//         dk_callback_(docid, doc_kp_list);
//
//         izenelib::am::rde_hash<uint32_t, uint32_t> in_doc_count;
//         std::vector<uint32_t> content_id_list;
//         std::vector<double> content_id_score_list;
//         for(uint32_t i=0;i<content_str_list.size();i++)
//         {
//             const StringType& term = content_str_list[i];
//             std::string str_term;
//             term.convertString(str_term, StringType::UTF_8);
//             if(str_term.empty()) continue;
//             term_id_map_it = term_id_map.find(str_term);
//             uint32_t term_id = term_id_map_it->second;
//             uint32_t* p_count = in_doc_count.find(term_id);
//             if(p_count==NULL)
//             {
//                 in_doc_count.insert(term_id, 1);
//                 content_id_list.push_back(term_id);
//             }
//             else
//             {
//                 *p_count = (*p_count)+1;
//             }
//         }
//
//         //output to other for further process after giza
//         other_ofs<<docid<<std::endl;
//         for(uint32_t i=0;i<kp_list.size();i++)
//         {
//             if(i>0) other_ofs<<" ";
//             other_ofs<<kp_list[i].first;
//         }
//         other_ofs<<std::endl;
//         for(uint32_t i=0;i<content_id_list.size();i++)
//         {
//             uint32_t tf = *(in_doc_count.find(content_id_list[i]));
//             df_map_it = df_map.find(content_id_list[i]);
//             uint32_t df = df_map_it->second;
//             double tdidf = (double)tf * std::log( (double)GetDocCount()/df);
//             if(i>0) other_ofs<<" ";
//             other_ofs<<content_id_list[i]<<" "<<tdidf;
//         }
//         other_ofs<<std::endl;
//     }
//     reader1.Close();
//     reader2.Close();
//
//     for(uint32_t i=0;i<kp_text_list.size();i++)
//     {
//         kp_callback_(i+1, kp_text_list[i], kp_freq_list[i]);
//     }
    return true;
}

bool KpeTask::ProcessFoundKp2_()
{
    std::vector<StringType> kp_text_list;
    std::vector<uint32_t> kp_freq_list;
    std::string title_file = title_writer_->GetPath();
    title_writer_->Close();
    delete title_writer_;
    title_writer_ = NULL;
    std::string kp_file = kp_writer_->GetPath();
    kp_writer_->Close();
    delete kp_writer_;
    kp_writer_ = NULL;
    izenelib::am::ssf::Writer<> dk_tmp_writer(dir_+"/doc_kp_distribute");
    dk_tmp_writer.Open();
    izenelib::am::ssf::Reader<> reader(kp_file);
    reader.Open();
    hash_t hash_id;
    KPItem item;
    while( reader.Next(hash_id, item) )
    {
        kp_text_list.push_back(item.text);
        uint32_t kpid = kp_text_list.size();

        double idf = std::log( GetDocCount()/(double)item.docitem_list.size() );
        for(uint32_t i=0;i<item.docitem_list.size();i++)
        {
            uint32_t docid = item.docitem_list[i].first;
            uint32_t tf = item.docitem_list[i].second;
            double tfidf = tf*idf;
            dk_tmp_writer.Append(docid, std::make_pair(kpid, tfidf) );
        }
    }
    reader.Close();
    dk_tmp_writer.Close();
//     kp_text_list.push_back(StringType("", StringType::UTF_8));
//     kp_freq_list.resize(kp_text_list.size(), 0);
    izenelib::am::ssf::Sorter<uint32_t, uint32_t>::Sort(dk_tmp_writer.GetPath());

    std::vector<idmlib::tl::IbmModel1::SentencePair> model_corpus;
    uint32_t kp_num = kp_text_list.size();
    boost::unordered_map<uint32_t, uint32_t> df_map;

    izenelib::am::ssf::Writer<> doc_term_writer(dir_+"/doc_term_writer");
    doc_term_writer.Open();

    {
        izenelib::am::ssf::Reader<> reader1(title_file);
        reader1.Open();
        izenelib::am::ssf::Reader<> reader2(dk_tmp_writer.GetPath());
        reader2.Open();

        izenelib::am::ssf::Merger<uint32_t, uint32_t, std::string, std::pair<uint32_t, double> > merger(&reader1, &reader2);
        uint32_t docid = 0;
        std::vector<std::string> title_list;
        std::vector<std::pair<uint32_t, double> > kp_list;

        typedef izenelib::util::second_greater<std::pair<uint32_t, double> > greater_than;

        while( merger.Next( docid, title_list, kp_list) )
        {
            if(docid%100==0)
            {
                std::cout<<"Generating sentence pair on docid "<<docid<<std::endl;
            }

            if(title_list.size()!=2)
            {
                std::cout<<"title list size error "<<title_list.size()<<std::endl;
                continue;
            }
            std::string title_str = title_list[0];
            std::string content_str = title_list[1];
            StringType title(title_str, StringType::UTF_8);
            StringType content(content_str, StringType::UTF_8);

            std::vector<StringType> title_str_list;
            cma_analyzer_->GetFilteredStringList(title, title_str_list);
            std::vector<StringType> content_str_list;
            cma_analyzer_->GetFilteredStringList(content, content_str_list);

            //use content and title as refer source
            uint32_t title_weight = 1;
            uint32_t content_weight = 0;
            std::vector<uint32_t> weights(2);
            weights[0] = title_weight;
            weights[1] = content_weight;
            std::vector<std::vector<StringType>* > sources(2);
            sources[0] = &title_str_list;
            sources[1] = &content_str_list;
            boost::unordered_map<uint32_t, uint32_t> in_doc;

            for(uint32_t i=0;i<sources.size();i++)
            {
                std::vector<StringType>& source_str_list = *(sources[i]);
                for(uint32_t j=0;j<source_str_list.size();j++)
                {
                    StringType& term = source_str_list[j];
                    if(term.length()==0) continue;
                    uint32_t term_id = izenelib::util::HashFunction<StringType>::generateHash32(term);

    //                 std::cout<<"[content-id] "<<term_id<<std::endl;
                    boost::unordered_map<uint32_t, uint32_t>::iterator in_doc_it = in_doc.find(term_id);
                    if(in_doc_it == in_doc.end())
                    {
                        in_doc_it = in_doc.insert(std::make_pair(term_id, 0)).first;
                        boost::unordered_map<uint32_t, uint32_t>::iterator df_map_it = df_map.find(term_id);
                        if(df_map_it == df_map.end())
                        {
                            df_map.insert(std::make_pair(term_id, 1));
                        }
                        else
                        {
                            df_map_it->second += 1;
                        }
                    }
                    in_doc_it->second += weights[i];
                }
            }


            std::vector<std::pair<uint32_t, uint32_t> > doc_id_list;
            boost::unordered_map<uint32_t, uint32_t>::iterator in_doc_it = in_doc.begin();
            while( in_doc_it!=in_doc.end())
            {
                doc_id_list.push_back(*in_doc_it);
                ++in_doc_it;
            }
            doc_term_writer.Append(docid, doc_id_list);

    //         {
    //             std::cout<<"[docid "<<docid<<"] ";
    //             for(uint32_t i=0;i<str_list.size();i++)
    //             {
    //                 std::string str;
    //                 str_list[i].convertString(str, izenelib::util::UString::UTF_8);
    //                 std::cout<<str<<",";
    //             }
    //             std::cout<<std::endl;
    //         }


            std::sort(kp_list.begin(), kp_list.end(), greater_than());
            uint32_t count = title_str_list.size()>kp_list.size()?kp_list.size():title_str_list.size();
            if(count==0) continue;
            std::vector<uint32_t> doc_kp_list;
            //kp as source and title terms as target so we can get P(kp|title terms)
            idmlib::tl::IbmModel1::SentencePair sent_pair;
            for(uint32_t i=0;i<count;i++)
            {

                sent_pair.first.push_back(kp_list[i].first);
                const StringType& title_term = title_str_list[i];
                uint32_t title_term_id = izenelib::util::HashFunction<StringType>::generateHash32(title_term);
                sent_pair.second.push_back(title_term_id);
            }

            model_corpus.push_back(sent_pair);

        }
        reader1.Close();
        reader2.Close();
    }
    doc_term_writer.Close();
    idmlib::tl::IbmModel1 ibm_model1(20);
    boost::unordered_map<std::pair<uint32_t, uint32_t>, double>* prob_map = ibm_model1.Train(kp_num, model_corpus);

    {
        izenelib::am::ssf::Reader<> reader1(doc_term_writer.GetPath());
        reader1.Open();
        izenelib::am::ssf::Reader<> reader2(dk_tmp_writer.GetPath());
        reader2.Open();

        izenelib::am::ssf::Merger<uint32_t, uint32_t, std::vector<std::pair<uint32_t, uint32_t> >, std::pair<uint32_t, double> > merger(&reader1, &reader2);
        uint32_t docid = 0;
        std::vector<std::vector<std::pair<uint32_t, uint32_t> > > all_term_id_list;
        std::vector<std::pair<uint32_t, double> > kp_list;

        double min_score = 4.0;
        uint32_t max_kp_num = 15;
        while( merger.Next( docid, all_term_id_list, kp_list) )
        {
            if(all_term_id_list.size()!=1)
            {
                std::cout<<"all_term_id_list size error "<<all_term_id_list.size()<<std::endl;
                continue;
            }
            if(docid%100==0)
            {
                std::cout<<"output docid "<<docid<<std::endl;
            }
            const std::vector<std::pair<uint32_t, uint32_t> >& term_id_list = all_term_id_list[0];
            std::vector<std::pair<DocKpItem, double> > kp_result_list;
            for(uint32_t i=0;i<kp_list.size();i++)
            {
                uint32_t kpid = kp_list[i].first;
                double score = 0.0;
                for(uint32_t j=0;j<term_id_list.size();j++)
                {
                    uint32_t term_id = term_id_list[j].first;
                    uint32_t tf = term_id_list[j].second;
                    std::pair<uint32_t, uint32_t> key(kpid, term_id);
                    boost::unordered_map<std::pair<uint32_t, uint32_t>, double>::iterator prob_map_it = prob_map->find(key);
                    if(prob_map_it==prob_map->end()) continue;

                    uint32_t df = df_map.find(term_id)->second;
                    double tfidf = std::log( (double)GetDocCount()/df) * tf;
                    score += tfidf*prob_map_it->second;
                }
                StringType& text = kp_text_list[kpid-1];
                uint32_t kp_length = text.length();
                double len_weight = (double)kp_length*kp_length;
                score *= len_weight;
                DocKpItem kp_item;
                kp_item.id = kpid;
                kp_item.text = text;
                kp_item.score = score;
                kp_result_list.push_back(std::make_pair(kp_item, kp_item.score));
            }
            typedef izenelib::util::second_greater<std::pair<DocKpItem, double> > greater_than;
            std::sort(kp_result_list.begin(), kp_result_list.end(), greater_than());

            std::vector<std::pair<DocKpItem, double> > post_result_list;
            knowledge_->PostProcess_(kp_result_list, post_result_list);
            uint32_t kp_result_num = std::min((uint32_t)post_result_list.size(), max_kp_num);
            std::vector<DocKpItem> final_result;
            for(uint32_t i=0;i<kp_result_num;i++)
            {
                double score = post_result_list[i].second;
                if(score<min_score) break;
                final_result.push_back(post_result_list[i].first);
            }
            dk_callback_(docid, final_result);


        }
        reader1.Close();
        reader2.Close();
    }
    delete prob_map;
    return true;
}

bool KpeTask::ProcessNgram_(const std::string& input_ngram_file)
{

    izenelib::am::ssf::Reader<> reader(input_ngram_file);
    if(!reader.Open()) return false;
    uint64_t p=0;
    std::vector<uint32_t> key_list;
    izenelib::am::ssf::Writer<> candidate_writer(dir_+"/candidate_writer");
    izenelib::am::ssf::Writer<> h2h_writer(dir_+"/h2h_writer");
    izenelib::am::ssf::Writer<> h2c_writer(dir_+"/h2c_writer");
    candidate_writer.Open();
    h2h_writer.Open();
    h2c_writer.Open();
    LOG_BEGIN2("Suffix", &reader);
    NgramInCollection last_ngram;
    std::vector<NgramInCollection> for_process;
    while( reader.Next(key_list) )
    {
        if(!tracing_.empty())
        {
            if(VecStarts_(key_list, tracing_))
            {
                std::cout<<"[tracing] [suffix] ";
                for(uint32_t dd=0;dd<key_list.size();dd++)
                {
                    std::cout<<key_list[dd]<<",";
                }
                std::cout<<std::endl;
            }
        }
        Ngram ngram = Ngram::ParseUint32List(key_list);
        if(last_ngram.IsEmpty())
        {
            last_ngram += ngram;
        }
        else
        {
            uint32_t inc = last_ngram.GetInc(ngram);
            if(inc==ngram.term_list.size()) //the same
            {
                last_ngram += ngram;
            }
            else
            {
                last_ngram.Flush();
                for_process.push_back(last_ngram);
                if(inc==0)
                {
                    GetCandidate_(for_process, &candidate_writer, &h2h_writer, &h2c_writer);
                    for_process.resize(0);
                }
                last_ngram.Clear();
                last_ngram.inc = inc;
                last_ngram += ngram;

            }
        }


        p++;
        LOG_PRINT2("Suffix", 1000000);
    }
    GetCandidate_(for_process, &candidate_writer, &h2h_writer, &h2c_writer);
    LOG_END();
    reader.Close();
    candidate_writer.Close();
    h2h_writer.Close();
    h2c_writer.Close();
    std::cout<<"Sorting h2c_writer with "<<h2c_writer.Count()<<std::endl;
    izenelib::am::ssf::Sorter<uint32_t, hash_t>::Sort(h2c_writer.GetPath());
    std::cout<<"Sorting candidate_writer with "<<candidate_writer.Count()<<std::endl;
    izenelib::am::ssf::Sorter<uint32_t, hash_t>::Sort(candidate_writer.GetPath());
    std::cout<<"Sorting h2h_writer with "<<h2h_writer.Count()<<std::endl;
    izenelib::am::ssf::Sorter<uint32_t, hash_t>::Sort(h2h_writer.GetPath());
    MEMLOG("[KPE4] finished.");

    izenelib::am::ssf::Writer<> hh_writer( dir_+"/hh_writer" );
    hh_writer.Open();
    {
        izenelib::am::ssf::Reader<> reader1(h2c_writer.GetPath());
        reader1.Open();
        izenelib::am::ssf::Reader<> reader2(h2h_writer.GetPath());
        reader2.Open();

        izenelib::am::ssf::Merger<uint32_t, hash_t, uint32_t, Hash2HashItem> merger(&reader1, &reader2);


        hash_t key = 0;
        std::vector<uint32_t> valueList1;
        std::vector<Hash2HashItem> valueList2;

        while( merger.Next( key, valueList1, valueList2) )
        {
    //             std::cout<<"[merger] "<<key<<","<<valueList1.size()<<","<<valueList2.size()<<std::endl;

            if( valueList1.size() > 1 )
            {
                //impossible, just a reminder.
                std::cout<<"[warning] valuelist1 size "<<valueList1.size()<<std::endl;
                continue;
            }
            uint32_t count = 0;
            if( valueList1.size() == 0 )
            {
                //for example ABCD, ABC never appears independnently, only with ABCD, ABCE, etc.. then ABC will not in h2c files.
                //so count(ABC)=0
            }
            else
            {
                count = valueList1[0];
            }

            for(uint32_t i=0;i<valueList2.size();i++)
            {
                hh_writer.Append( valueList2[i].first, Hash2CountItem(count, valueList2[i].second) );

            }
        }
        reader1.Close();
        reader2.Close();
        hh_writer.Close();
    }
    {
        izenelib::am::ssf::Sorter<uint32_t, uint64_t>::Sort(hh_writer.GetPath());
    }

    izenelib::am::ssf::Writer<> hclt_writer( dir_+"/HCLTWRITER" );
    hclt_writer.Open();
    {
        izenelib::am::ssf::Reader<> hh_reader(hh_writer.GetPath());
        hh_reader.Open();
        Hash2CountItem hash2CountValue;
        LOG_BEGIN2("HashCount", &hh_reader);
        HashCountListItem output(0, 0, 0, 0);
        hash_t key;
        hash_t saveKey;
        while( hh_reader.Next(key, hash2CountValue) )
        {
            if(p==0)
            {
                saveKey = key;
            }
            else
            {
                if(saveKey != key )
                {
                    hclt_writer.Append(saveKey, output);
                    output = boost::make_tuple(0,0,0,0);
                    saveKey = key;
                }
            }
            if( hash2CountValue.second == 1 )
            {
                boost::get<0>(output) = hash2CountValue.first;
            }
            else if( hash2CountValue.second == 2 )
            {
                boost::get<1>(output) = hash2CountValue.first;
            }
            else if( hash2CountValue.second == 3 )
            {
                boost::get<2>(output) = hash2CountValue.first;
            }
            else if( hash2CountValue.second == 4 )
            {
                boost::get<3>(output) = hash2CountValue.first;
            }
            else
            {
                //impossible, just a reminder.
            }
            p++;
            LOG_PRINT2("HashCount", 100000);
        }
        hclt_writer.Append(saveKey, output);
        LOG_END2();
        hh_reader.Close();
    }
    hclt_writer.Close();

    {
        double min_logl = 10.0;
        double min_mi = 5.0;
        izenelib::am::ssf::Reader<> reader1(candidate_writer.GetPath());
        reader1.Open();
        izenelib::am::ssf::Reader<> reader2(hclt_writer.GetPath());
        reader2.Open();

        izenelib::am::ssf::Merger<uint32_t, hash_t, CandidateItem, HashCountListItem > merger(&reader1, &reader2);

        hash_t key = 0;
        std::vector<CandidateItem> valueList1;
        std::vector<HashCountListItem> valueList2;
        uint32_t n = all_term_count_;
        while( merger.Next(key, valueList1, valueList2) )
        {
            if( valueList1.size()==0 )
            {
                //impossible, just a reminder.
                continue;
            }
            if( valueList1.size()>1 )
            {
                //we may face to a hash conflict, ignore them
                continue;
            }
            const CandidateItem& hlItem = valueList1[0];
            const std::vector<uint32_t>& termIdList = hlItem.termid_list;
            if( termIdList.size()==1 )
            {
//                 FindKP_( termIdList, docItem, GetScore(f, termIdList.size(),0.0) , leftTermList,rightTermList);
                continue;
            }
            if( valueList2.size()!=1 )
            {
                //impossible, just a reminder.
                continue;
            }

            const HashCountListItem& hclItem = valueList2[0];

            const std::vector<id2count_t>& docItem = hlItem.docitem_list;
            uint32_t f = hlItem.freq;
            const std::vector<id2count_t>& leftTermList = hlItem.lc_list;
            const std::vector<id2count_t>& rightTermList = hlItem.rc_list;

            if(termIdList.size()==2 )
            {
                uint32_t f1 = boost::get<0>(hclItem);
                uint32_t f2 = boost::get<1>(hclItem);
                if( f1==0 || f2==0 )
                {
                    //impossible, just a reminder.
                    continue;
                }
                double logL = KpeStatistic::LogL(f,f1,f2,n);
                double mi = KpeStatistic::MI(f,f1,f2,n);
//                     std::cout<<"LogL: "<<logL<<" , MI: "<<mi<<std::endl;
                if( logL>=min_logl && mi>=min_mi )
                {
                    FindKP_( termIdList, docItem, GetScore(f, termIdList.size(),logL) , leftTermList,rightTermList);
                }
            }
            else if(termIdList.size()>2)
            {
                uint32_t f1 = boost::get<0>(hclItem);
                uint32_t f2 = boost::get<1>(hclItem);
                uint32_t f3 = boost::get<2>(hclItem);
                uint32_t f4 = boost::get<3>(hclItem);
                if( f1==0 || f2==0 || f3==0 || f4==0 )
                {
                    //impossible, just a reminder.
                    continue;
                }
                double logL = KpeStatistic::LogL(f,f1,f2,n);
                if( logL>=min_logl && KpeStatistic::LogL(f,f1,f4,n)>=min_logl
                    && KpeStatistic::LogL(f,f3,f2,n)>=min_logl )
                {
                    FindKP_( termIdList, docItem, GetScore(f, termIdList.size(),logL) , leftTermList,rightTermList);
                }
            }
        }
        reader1.Close();
        reader2.Close();
    }
    return true;
}



void KpeTask::Init_()
{
    test_num_ = 0;
    no_freq_limit_ = false;
    last_doc_id_ = 0;
    doc_count_ = 0;
    all_term_count_ = 0;
    boost::filesystem::create_directories(dir_);

    if( title_writer_ == NULL )
    {
        title_writer_ = new izenelib::am::ssf::Writer<>(dir_+"/title");
        title_writer_->Open();
    }
    if( ngram_writer_ == NULL )
    {
        ngram_writer_ = new izenelib::am::ssf::Writer<>(dir_+"/ngram");
        ngram_writer_->Open();
    }
    if( kp_writer_ == NULL)
    {
        kp_writer_ = new izenelib::am::ssf::Writer<>(dir_+"/all_kp");
        kp_writer_->Open();
    }

}


void KpeTask::Release_()
{
    if( title_writer_!= NULL)
    {
        delete title_writer_;
        title_writer_ = NULL;
    }
    if( ngram_writer_!= NULL)
    {
        delete ngram_writer_;
        ngram_writer_ = NULL;
    }
    if( kp_writer_!= NULL)
    {
        delete kp_writer_;
        kp_writer_ = NULL;
    }
    boost::filesystem::remove_all(dir_);
}



bool KpeTask::AddTerms_(uint32_t docId, const std::vector<idmlib::util::IDMTerm>& termList, uint32_t& iBegin)
{
//         std::cout<<docId<<"#### "<<termList.size()<<std::endl;
    if(iBegin >= termList.size()) return false;
    std::vector<std::pair<bool,TermInNgram> > splitVec;
    uint32_t i=iBegin;
    uint32_t _begin = iBegin;

    for ( ;i<termList.size();i++ )
    {
        bool bSplit = knowledge_->IsSplitTerm(termList[i]);
        TermInNgram new_ngram;
        new_ngram.id = termList[i].id;
        new_ngram.tag = termList[i].tag;
        splitVec.push_back(std::make_pair(bSplit, new_ngram));
        if( i == iBegin ) //first term
        {
            continue;
        }
        else
        {
            if(bSplit)
            {
                break;
            }
            if( termList[i].position != termList[i-1].position+1 )
            {
                splitVec.erase( splitVec.end()-1 );
                break;
            }
        }

    }
    iBegin = i;
    if( splitVec.size() == 0 ) return false;
    bool bFirstTerm = true;
    bool bLastTerm = true;
    if( splitVec.size() == 1 )
    {
        if( splitVec[0].first == true )
        {
            return true;
        }
    }
    else
    {
        bFirstTerm = !splitVec.front().first;
        bLastTerm = !splitVec.back().first;

    }
    std::vector<TermInNgram> terms( splitVec.size() );
    for(uint32_t p=_begin;p<_begin+splitVec.size();p++)
    {
        uint32_t _index = p-_begin;
        terms[_index] = splitVec[_index].second;
        id_manager_->Put(terms[_index].id, termList[p].text);
//             if( !splitVec[_index].first )
//             {
//
//             }
    }
    AppendNgram_(docId, terms, bFirstTerm, bLastTerm);
    return true;


}


void KpeTask::AppendNgram_(uint32_t docid, const std::vector<TermInNgram>& termList, bool bFirstTerm, bool bLastTerm )
{
//         std::cout<<docId<<"### "<<termList.size()<<" "<<(int)bFirstTerm<<" "<<(int)bLastTerm<<std::endl;
    if( termList.size() == 0 ) return;
    if( termList.size() == 1 )
    {
        if( bFirstTerm && bLastTerm )
        {
            Ngram ngram(termList, docid);
            WriteNgram_(ngram);
//             if( scorer_->prefixTest(termList) != KPStatus::RETURN)
//             {
//
//             }
//             appendHashItem_(hash_(termList));
            all_term_count_ += 1;
        }
        return;
    }
    if( termList.size() == 2 && !bFirstTerm && !bLastTerm )
    {
        return;
    }
    uint32_t start = 0;
    uint32_t end = termList.size();
    uint32_t increase = termList.size();
    if(!bFirstTerm)
    {
        increase--;
        start = 1;
    }
    if(!bLastTerm)
    {
        increase--;
        end -= 1;
    }

    for(uint32_t i=start; i<end; i++)
    {
        uint32_t len = std::min(end-i,(uint32_t)(max_phrase_len_+1));
        std::vector<TermInNgram> new_term_list(termList.begin()+i, termList.begin()+i+len );
//         bool valid_frag =true;
        Ngram ngram(new_term_list, docid);
        if( i!= 0 )
        {
            ngram.left_term = termList[i-1];
        }
        WriteNgram_(ngram);

//         if(!tracing_.empty())
//         {
//             if(VecStarts_(frag, tracing_))
//             {
//             std::cout<<"[tracing] ";
//             for(uint32_t dd=0;dd<frag.size();dd++)
//             {
//                 std::cout<<frag[dd]<<",";
//             }
//             std::cout<<std::endl;
//             }
//         }

        //append hash item
//         for(uint32_t j= i+1; j<= end; j++)
//         {
//             if( j-i >= max_phrase_len_ ) continue;
//             std::vector<uint32_t> ifrag( termList.begin()+i, termList.begin()+j );
//             appendHashItem_(hash_(ifrag));
//         }

    }
    all_term_count_ += increase;
}

void KpeTask::WriteNgram_(const Ngram& ngram)
{
    std::vector<uint32_t> list;
    ngram.ToUint32List(list);
    ngram_writer_->Append(list);
}

// void KpeTask::AppendHashItem_(hash_t hash_value)
// {
//     if( cache_size_ > 0 )
//     {
//     if( cache_vec_.size() >= cache_size_ )
//     {
// //           std::cout<<"[FULL]"<<std::endl;
//         //output
//         for(uint32_t i=0;i<cache_vec_.size();i++)
//         {
//         pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
//         }
//         //clean
//         cache_vec_.resize(0);
//         cache_map_.clear();
//     }
//     uint32_t* index=  cache_map_.find(hash_value);
//     if( index == NULL)
//     {
//         cache_vec_.push_back( std::make_pair(hash_value, 1) );
//         cache_map_.insert( hash_value, (uint32_t)(cache_vec_.size())-1 );
//     }
//     else
//     {
//         cache_vec_[*index].second += 1;
//     }
//     }
//     else
//     {
//     pHashWriter_->append(hash_value);
//     }
//
// }
//
// void KpeTask::ReleaseCachedHashItem_()
// {
//     if( cache_size_ > 0 )
//     {
//         for(uint32_t i=0;i<cache_vec_.size();i++)
//         {
//             pHashWriter_->append(cache_vec_[i].first, cache_vec_[i].second);
//         }
//         //clean
//         cache_vec_.resize(0);
//         cache_map_.clear();
//     }
// }



bool KpeTask::AstartWithB(const std::vector<uint32_t>& termIdList1, const std::vector<uint32_t>& termIdList2)
{
    if(termIdList1.size()<termIdList2.size()) return false;
    for(uint32_t i=0;i<termIdList2.size();i++)
    {
        if( termIdList1[i] != termIdList2[i] ) return false;
    }
    return true;
}

bool KpeTask::MakeKPStr_(const std::vector<uint32_t>& termIdList,
    std::vector<izenelib::util::UString>& strList,
    izenelib::util::UString& result)
{
    bool b = MakeKPStr_(termIdList, strList);
    if(!b) return false;
    b = MakeKPStr_(strList, result);
    if(!b) return false;
    return true;
}

bool KpeTask::MakeKPStr_(const std::vector<uint32_t>& termIdList, std::vector<izenelib::util::UString>& strList)
{
    if ( termIdList.empty() ) return false;//if empty
    strList.resize(termIdList.size());
    izenelib::util::UString ustr;
    bool bb = true;
    bb = id_manager_->GetStringById(termIdList[0], strList[0]);
    if(!bb)
    {
        std::cout<<"!!!!can not find term id "<<termIdList[0]<<std::endl;
        return false;
    }
    for(std::size_t i=1;i<termIdList.size();i++)
    {
        bb = id_manager_->GetStringById(termIdList[i], strList[i]);
        if(!bb)
        {
            std::cout<<"!!!!can not find term id "<<termIdList[i]<<std::endl;
            return false;
        }
    }
    return true;
}

bool KpeTask::MakeKPStr_(const std::vector<izenelib::util::UString>& strList, izenelib::util::UString& result)
{
    if(strList.size()>0)
    {
        if( strList[0].length() == 0 ) return false;
        result.append(strList[0]);
        for(std::size_t i=1;i<strList.size();i++)
        {
            if( strList[i].length() == 0 ) return false;
            if( result.isAlphaChar( result.length()-1 )
                && strList[i].isAlphaChar(0) )
            {
                result.push_back(' ');
            }
            else
            {
            }
            result.append(strList[i]);
        }
    }
    return true;
}

uint8_t KpeTask::GetScore(uint32_t f, uint32_t termCount, double logL)
{
    double score=sqrt(f> 200 ? 100 : f/2)*(termCount > 4 ? 4: termCount);
    if(logL>=10.0)
    {
        score *= sqrt(logL> 100 ? 100 : logL);
    }
    else
    {
        score *= 5;
    }
    return (uint8_t)(ceil(score)+1);
}

void KpeTask::FindKP_(const std::vector<uint32_t>& terms, const std::vector<id2count_t>& docItem
            , uint8_t score ,const std::vector<id2count_t>& leftTermList
            ,const std::vector<id2count_t>& rightTermList)
{
    std::vector<izenelib::util::UString> strList;
    izenelib::util::UString kp_text;
    bool validKP = MakeKPStr_(terms, strList, kp_text);
    if(validKP)
    {
        hash_t hash_id = Hash_(terms);
        kp_writer_->Append(hash_id, KPItem( kp_text, docItem, score, leftTermList, rightTermList));
//         std::string str;
//         kp_text.convertString(str, izenelib::util::UString::UTF_8);
//         std::cout<<"[Find KP] : "<<str<<" , "<<docItem.size()<<std::endl;
    }
}

void KpeTask::OutputKP_(const KPItem& kpItem)
{
//     output_.output(kpItem.text, kpItem.docitem_list, kpItem.docitem_list.size(), kpItem.score , kpItem.lc_list, kpItem.rc_list);
}



//with complete termidlist version.
void KpeTask::GetCandidate_(const std::vector<NgramInCollection>& data, izenelib::am::ssf::Writer<>* candidate_writer, izenelib::am::ssf::Writer<>* h2h_writer, izenelib::am::ssf::Writer<>* h2c_writer)
{
    if( data.size()==0 ) return;

    //sorting with postorder
    std::vector<uint32_t> post_order(data.size());
    {
        std::stack<uint32_t> index_stack;
        std::stack<uint32_t> depth_stack;
        uint32_t i=0;
        for( uint32_t m=0;m<data.size();m++)
        {
            uint32_t depth = data[m].inc;
            if(m>0)
            {
                while( !depth_stack.empty() && depth <= depth_stack.top() )
                {
                    post_order[i] = index_stack.top();
                    ++i;

                    index_stack.pop();
                    depth_stack.pop();
                }
            }
            index_stack.push(m);
            depth_stack.push(depth);
        }
        while( !index_stack.empty() )
        {
            post_order[i] = index_stack.top();
            ++i;

            index_stack.pop();
            depth_stack.pop();
        }
    }

    std::stack<uint32_t> depth_stack;
    std::stack<uint32_t> freq_stack;
    std::stack<std::vector<id2count_t> > doc_item_stack;
    std::stack<std::vector<std::pair<TermInNgram, uint32_t> > > prefix_term_stack;
    std::stack<std::pair<TermInNgram, uint32_t> > suffix_term_stack;
    if(!tracing_.empty())
    {
        test_num_++;
    }
    for( uint32_t m=0;m<data.size();m++)
    {
        uint32_t depth = data[post_order[m]].inc;
        std::vector<TermInNgram> termList = data[post_order[m]].term_list;
        std::vector<id2count_t> docItemList = data[post_order[m]].docitem_list;
        uint32_t freq = data[post_order[m]].freq;
        std::vector<std::pair<TermInNgram, uint32_t> > prefixTermList = data[post_order[m]].lc_list;
        std::vector<std::pair<TermInNgram, uint32_t> > suffixTermList;
//           if(!tracing_.empty())
//           {
//             if(vec_starts_(termIdList, tracing_))
//             {
//               std::cout<<"[tracing] {data} "<<test_num_<<","<<m<<","<<post_order[m]<<","<<depth;
//               for(uint32_t dd=0;dd<docItemList.size();dd++)
//               {
//                 std::cout<<"("<<docItemList[dd].first<<"|"<<docItemList[dd].second<<"),";
//               }
//               std::cout<<std::endl;
//             }
//           }
        while( !depth_stack.empty() && depth < depth_stack.top() )
        {
            freq+=freq_stack.top();
            docItemList.insert( docItemList.end(), doc_item_stack.top().begin(), doc_item_stack.top().end() );
            prefixTermList.insert( prefixTermList.end(), prefix_term_stack.top().begin(), prefix_term_stack.top().end() );
            //suffix howto?
            //suffixTermList, suffix_term_stack
    //             suffixTermList.insert( suffixTermList.end(), suffix_term_stack.top().begin(), suffix_term_stack.top().end() );
            suffixTermList.push_back(suffix_term_stack.top() );
            //pop stack
            depth_stack.pop();
            freq_stack.pop();
            doc_item_stack.pop();
            prefix_term_stack.pop();
            suffix_term_stack.pop();
        }
        depth_stack.push( depth);
        freq_stack.push(freq);
        doc_item_stack.push( docItemList);
        prefix_term_stack.push(prefixTermList);
        //get the suffix term
        TermInNgram suffix_term = termList[depth];
        suffix_term_stack.push(std::make_pair(suffix_term, freq) );

        if( termList.size() > max_phrase_len_ ) continue;

        std::vector<uint32_t> termIdList(termList.size());
        for(uint32_t i=0;i<termList.size();i++)
        {
            termIdList[i] = termList[i].id;
        }

        idmlib::util::accumulateList(docItemList);
        idmlib::util::accumulateList(prefixTermList);
        idmlib::util::accumulateList(suffixTermList);
        std::vector<std::pair<uint32_t, uint32_t> > left_termid_list(prefixTermList.size());
        for(uint32_t i=0;i<prefixTermList.size();i++)
        {
            left_termid_list[i].first = prefixTermList[i].first.id;
            left_termid_list[i].second = prefixTermList[i].second;
        }

        std::vector<std::pair<uint32_t, uint32_t> > right_termid_list(suffixTermList.size());
        for(uint32_t i=0;i<suffixTermList.size();i++)
        {
            right_termid_list[i].first = suffixTermList[i].first.id;
            right_termid_list[i].second = suffixTermList[i].second;
        }
        hash_t hash_id = Hash_(termIdList);
//         hash_t trace_key = 626763502908754ul;
//         trace_key = 18438664875524534895ul;
//         if(hash_id == trace_key )
//         {
//             std::cout<<"!!!XXXXXXX"<<std::endl;
//         }
//         std::cout<<"!!!FFFFF "<<termIdList.size()<<std::endl;
//         for(uint32_t t=0;t<termIdList.size();t++)
//         {
//             std::cout<<termIdList[t]<<",";
//         }
//         std::cout<<std::endl;

        if( termIdList.size()< max_phrase_len_)//
        {
            h2c_writer->Append(hash_id, freq);
        }


//         SI lri(termList, freq);
//         SCI leftLC( prefixTermList);
//         SCI rightLC( suffixTermList);
        if(!tracing_.empty())
        {
            if(VecEqual_(termIdList, tracing_))
            {
                std::cout<<"[tracing] freq: "<<freq<<std::endl;
                for(uint32_t dd=0;dd<docItemList.size();dd++)
                {
                    std::cout<<"[tracing] doc item0: "<<docItemList[dd].first<<","<<docItemList[dd].second<<"]"<<std::endl;
                }
            }
        }
        if(manmade_.find(termIdList)!=NULL)
        {
            FindKP_( termIdList, docItemList, freq, left_termid_list, right_termid_list );
            continue;
        }
        int status = KPStatus::CANDIDATE;
        if( freq<min_freq_threshold_ ) status = KPStatus::NON_KP;
        else if( docItemList.size() < min_df_threshold_ ) status = KPStatus::NON_KP;
//         else
//         {
//             status = scorer_->PrefixTest(termIdList);
//         }

        if( status == KPStatus::NON_KP || status == KPStatus::RETURN )
        {
            continue;
        }
        if( status == KPStatus::KP )
        {
            FindKP_( termIdList, docItemList, freq, left_termid_list, right_termid_list );
            continue;
        }

//         {
//             std::vector<izenelib::util::UString> strList;
//             izenelib::util::UString kp_text;
//             bool validKP = MakeKPStr_(termIdList, strList, kp_text);
//             if(validKP)
//             {
//                 std::string str;
//                 kp_text.convertString(str, izenelib::util::UString::UTF_8);
//                 double chi = KpeStatistic::ChiTest(freq, docItemList, GetDocCount());
//                 std::cout<<"[Candidate KP] : "<<str<<" , "<<docItemList.size()<<" , "<<chi<<std::endl;
//             }
//         }

        double test_result = knowledge_->Test(termList, freq, docItemList, GetDocCount(), prefixTermList, suffixTermList);
        if( !test_result ) continue;



        CandidateItem htList(termIdList, docItemList, freq, left_termid_list,right_termid_list );

        candidate_writer->Append(hash_id , htList );

        if( termIdList.size() >= 2 )
        {
            {
                std::vector<uint32_t> vec(termIdList.begin(), termIdList.end()-1);
                Hash2HashItem item(hash_id, 1);
                hash_t key = Hash_(vec);
                h2h_writer->Append( key, item);
//                 if(key == trace_key )
//                 {
//                     std::cout<<"!!!YYYYYY1 "<<termIdList.size()<<std::endl;
//                     for(uint32_t t=0;t<vec.size();t++)
//                     {
//                         std::cout<<vec[t]<<",";
//                     }
//                     std::cout<<std::endl;
//                 }
            }
            {
                std::vector<uint32_t> vec(termIdList.begin()+1, termIdList.end());
                Hash2HashItem item(hash_id, 2);
                hash_t key = Hash_(vec);
                h2h_writer->Append( key, item);
//                 if(key == trace_key )
//                 {
//                     std::cout<<"!!!YYYYYY2 "<<termIdList.size()<<std::endl;
//                 }
            }
        }
        if( termIdList.size() >= 3 )
        {
            {
                std::vector<uint32_t> vec(termIdList.begin(), termIdList.begin()+1);
                Hash2HashItem item(hash_id, 3);
                hash_t key = Hash_(vec);
                h2h_writer->Append( key, item);
//                 if(key == trace_key )
//                 {
//                     std::cout<<"!!!YYYYYY3 "<<termIdList.size()<<std::endl;
//                 }
            }
            {
                std::vector<uint32_t> vec(termIdList.end()-1, termIdList.end());
                Hash2HashItem item(hash_id, 4);
                hash_t key = Hash_(vec);
                h2h_writer->Append( key, item);
//                 if(key == trace_key )
//                 {
//                     std::cout<<"!!!YYYYYY4 "<<termIdList.size()<<std::endl;
//                 }
            }
        }

    }

}
