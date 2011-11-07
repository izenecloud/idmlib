#include <idmlib/util/idm_analyzer.h>

using namespace idmlib::util;

IDMAnalyzer::IDMAnalyzer()
:la_(new la::LA() ), simpler_set_(false)
{
    InitWithConfig_(IDMAnalyzerConfig::GetCommonConfig("","",""));
}

IDMAnalyzer::IDMAnalyzer(const IDMAnalyzerConfig& config)
:la_(new la::LA() ), simpler_set_(false)
{
    InitWithConfig_(config);
}

void IDMAnalyzer::SetLIDPath(const std::string& path)
{
    std::cout<<"IDMAnalyzer::SetLIDPath "<<path<<std::endl;
    ilplib::langid::Factory* langIdFactory = ilplib::langid::Factory::instance();
    ilplib::langid::Analyzer* langIdAnalyzer = langIdFactory->createAnalyzer();
    ilplib::langid::Knowledge* langIdKnowledge = langIdFactory->createKnowledge();
    string encodingPath = path + "/model/encoding.bin";
    if(!langIdKnowledge->loadEncodingModel(encodingPath.c_str()))
    {
        std::cout<<"langIdKnowledge->loadEncodingModel failed"<<std::endl;
    }
    // load language model for language identification or sentence tokenization
    string langPath = path + "/model/language.bin";
    if(!langIdKnowledge->loadLanguageModel(langPath.c_str()))
    {
        std::cout<<"langIdKnowledge->loadLanguageModel failed"<<std::endl;
    }
    // set knowledge
    langIdAnalyzer->setKnowledge(langIdKnowledge);
    la::MultiLanguageAnalyzer::langIdAnalyzer_ = langIdAnalyzer;
}

void IDMAnalyzer::InitWithConfig_(const IDMAnalyzerConfig& config)
{
    gran_ = la::SENTENCE_LEVEL;
    config_ = config;
    symbol_map_.insert("SC", 1);
    symbol_map_.insert("PUNCT-L", 1);
    symbol_map_.insert("PUNCT-R", 1);
    symbol_map_.insert("EOS", 1);
    symbol_map_.insert("S-S", 1);
    symbol_map_.insert("S-G", 1);
    symbol_map_.insert("PUNCT-C", 1);
    boost::shared_ptr<la::MultiLanguageAnalyzer> ml_analyzer(new la::MultiLanguageAnalyzer() );
    ml_analyzer->setExtractSpecialChar(true, false);

    boost::shared_ptr<la::Analyzer> ema_analyzer;
    boost::shared_ptr<la::Analyzer> kma_analyzer;
    boost::shared_ptr<la::Analyzer> cma_analyzer;
    boost::shared_ptr<la::Analyzer> jma_analyzer;

    if(config.ema_config.enable)
    {
        ema_analyzer.reset(new la::EnglishAnalyzer() );
        static_cast<la::EnglishAnalyzer*>(ema_analyzer.get())->setCaseSensitive(config.ema_config.case_sensitive, false);
    }

    if(config.kma_config.path!="")
    {
        kma_analyzer.reset(new la::KoreanAnalyzer( config.kma_config.path ) );
        la::KoreanAnalyzer* p_korean_analyzer = static_cast<la::KoreanAnalyzer*>(kma_analyzer.get());
        p_korean_analyzer->setLabelMode();
        //     p_korean_analyzer->setNBest(1);
        p_korean_analyzer->setExtractEngStem( false );
        p_korean_analyzer->setExtractSynonym(false);
        p_korean_analyzer->setCaseSensitive(config.ema_config.case_sensitive, false);
        ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::KOREAN, kma_analyzer );
    }

    if(config.cma_config.use_char)
    {
        cma_analyzer.reset(new la::CharAnalyzer() );
        la::CharAnalyzer* p_char_analyzer = static_cast<la::CharAnalyzer*>(cma_analyzer.get());
        p_char_analyzer->setSeparateAll(false);
    }
    else
    {
        if(config.cma_config.path!="")
        {
            bool load_model = false;
            if (config.cma_config.type == la::ChineseAnalyzer::maximum_entropy) {
                load_model = true;
            }
            cma_analyzer.reset( new la::ChineseAnalyzer(config.cma_config.path, load_model) );

            la::ChineseAnalyzer* pch = dynamic_cast<la::ChineseAnalyzer*>(cma_analyzer.get());
            pch->setCaseSensitive(config.ema_config.case_sensitive, false);
            pch->setAnalysisType( config.cma_config.type);
            pch->setLabelMode();
            pch->setRemoveStopwords(config.cma_config.remove_stopwords);
        }
    }

    if(config.jma_config.path!="")
    {
        jma_analyzer.reset( new la::JapaneseAnalyzer(config.jma_config.path) );
//         if( !config.jma_config.noun_only )
//         {
//             jma_analyzer.reset( new la::JapaneseAnalyzer(config.jma_config.path) );
//         }
//         else
//         {
//             std::vector<std::string> keywords;
//             keywords.push_back("PL-N");
//             keywords.push_back("N-VS");
//             keywords.push_back("N-NE");
//             keywords.push_back("NC-G");
//             keywords.push_back("N-R");
//             keywords.push_back("N-AJV");
//             keywords.push_back("NP-G");
//             keywords.push_back("NP-H");
//             keywords.push_back("NP-S");
//             keywords.push_back("NP-GN");
//             keywords.push_back("NP-O");
//             keywords.push_back("NP-P");
//             keywords.push_back("NP-C");
//             keywords.push_back("NN");
//             keywords.push_back("NJ");
//             keywords.push_back("NS-SN");
//             keywords.push_back("NS-G");
//             keywords.push_back("NS-AJV");
//             keywords.push_back("NU");
//             keywords.push_back("NS-AUV");
//             keywords.push_back("NS-H");
//             keywords.push_back("NS-P");
//             keywords.push_back("NS-S");
//             keywords.push_back("NS-D");
//             keywords.push_back("NR-G");
//             keywords.push_back("NR-A");
//             keywords.push_back("N-VD");
//             keywords.push_back("N-AUV");
//             keywords.push_back("ND-G");
//             keywords.push_back("ND-AUV");
//             keywords.push_back("ND-D");
//             keywords.push_back("N-D");
//             keywords.push_back("N-USER");
//             jma_analyzer.reset( new la::JapaneseAnalyzer(config.jma_config.path, keywords) );
//         }
        la::JapaneseAnalyzer* pja = dynamic_cast<la::JapaneseAnalyzer*>(jma_analyzer.get());
        pja->setCaseSensitive(config.ema_config.case_sensitive, false);
        pja->setLabelMode();
    }

    if(ema_analyzer)
    {
        ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::ENGLISH, ema_analyzer );
    }
    if(kma_analyzer)
    {
        ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::KOREAN, kma_analyzer );
    }
    if(cma_analyzer)
    {
        ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::CHINESE, cma_analyzer );
    }
    if(jma_analyzer)
    {
        ml_analyzer->setAnalyzer( la::MultiLanguageAnalyzer::JAPANESE, jma_analyzer );
    }

    if(config.default_language == IDMAnalyzerConfig::ENGLISH)
    {
        ml_analyzer->setDefaultAnalyzer( ema_analyzer );
    }
    else if(config.default_language == IDMAnalyzerConfig::KOREAN)
    {
        ml_analyzer->setDefaultAnalyzer( kma_analyzer );
    }
    else if(config.default_language == IDMAnalyzerConfig::CHINESE)
    {
        ml_analyzer->setDefaultAnalyzer( cma_analyzer );
    }
    else if(config.default_language == IDMAnalyzerConfig::JAPANESE)
    {
        ml_analyzer->setDefaultAnalyzer( jma_analyzer );
    }

    la_->setAnalyzer( ml_analyzer );

    stemmer_ = new la::stem::Stemmer();
    stemmer_->init(la::stem::STEM_LANG_ENGLISH);
}



IDMAnalyzer::~IDMAnalyzer()
{
    delete la_;
    delete stemmer_;
}

// void IDMAnalyzer::ExtractSpecialChar(bool extractSpecialChar, bool convertToPlaceHolder)
// {
//     la_->getAnalyzer()->setExtractSpecialChar(extractSpecialChar, convertToPlaceHolder);
// }
//
// void IDMAnalyzer::ExtractSymbols()
// {
//     ExtractSpecialChar(true, false);
//     symbol_ = true;
// }

bool IDMAnalyzer::LoadSimplerFile(const std::string& file)
{
    std::istream* simpler_ifs = idmlib::util::getResourceStream(file);
    if( simpler_ifs==NULL)
    {
        std::cerr<<"Load simpler file failed, check the resource file."<<std::endl;
        return false;
    }
    std::string line;
    while( getline( *simpler_ifs, line) )
    {
        boost::algorithm::trim( line );
        izenelib::util::UString uline(line, izenelib::util::UString::UTF_8);
        simpler_map_.insert( uline[2], uline[0] );
    }
    delete simpler_ifs;
    simpler_set_ = true;
    return true;
}

bool IDMAnalyzer::LoadT2SMapFile(const std::string& file)
{
    return LoadSimplerFile(file);
}


void IDMAnalyzer::GetTermList(const izenelib::util::UString& utext, la::TermList& term_list, bool convert)
{
    izenelib::util::UString text(utext);
    if (convert) {
        la::convertFull2HalfWidth(text);
    }
    {
        boost::mutex::scoped_lock lock(la_mtx_);
        la_->process( text, term_list, gran_);
    }
    //do t_s translation
    if(simpler_set_)
    {
        la::TermList::iterator it = term_list.begin();
        while( it!= term_list.end() )
        {
            simple_( it->text_ );
            ++it;
        }
    }

//     la::TermList::iterator it = term_list.begin();
//     while( it!= term_list.end() )
//     {
//         std::cout<<"("<<it->textString()<<","<<it->wordOffset_<<","<<it->pos_<<"),";
//         ++it;
//     }
//     std::cout<<std::endl;

}

void IDMAnalyzer::GetTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    term_list.reserve( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
//     uint32_t i=0;
    while( it!= la_term_list.end() )
    {
        if(!TermIgnore_(*it))
        {
            idmlib::util::IDMTerm term;
            term.text = it->text_;
            term.tag = IDMTermTag::GetTermTag( it->pos_ );
            term.id = IDMIdConverter::GetId( it->text_, term.tag);
            term.position = it->wordOffset_;
            term_list.push_back(term);
        }
        it++;
    }
}

void IDMAnalyzer::GetStemTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    term_list.resize( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
    uint32_t i=0;
    while( it!= la_term_list.end() )
    {
        term_list[i].tag = IDMTermTag::GetTermTag( it->pos_ );
        if( term_list[i].tag == IDMTermTag::ENG )
        {
            //do stemming
            it->text_.toLowerString();
            std::string str;
            std::string str_stem;
            it->text_.convertString(str, izenelib::util::UString::UTF_8);
            stemmer_->stem( str, str_stem );
            term_list[i].text = izenelib::util::UString(str_stem, izenelib::util::UString::UTF_8);

        }
        else
        {
            term_list[i].text = it->text_;
        }
        term_list[i].id = IDMIdConverter::GetId( term_list[i].text, term_list[i].tag );
        term_list[i].position = it->wordOffset_;
        i++;
        it++;
    }
}

void IDMAnalyzer::GetTermList(const izenelib::util::UString& text, std::vector<izenelib::util::UString>& str_list, std::vector<uint32_t>& id_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    str_list.resize( la_term_list.size() );
    id_list.resize( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
    uint32_t i=0;
    while( it!= la_term_list.end() )
    {
        str_list[i] = it->text_;
        id_list[i] = IDMIdConverter::GetId( it->text_, it->pos_ );
        i++;
        it++;
    }
}

void IDMAnalyzer::GetStringList(const izenelib::util::UString& text, std::vector<izenelib::util::UString>& str_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    str_list.resize( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
    uint32_t i=0;
    while( it!= la_term_list.end() )
    {
        str_list[i] = it->text_;
        i++;
        it++;
    }
}

void IDMAnalyzer::GetFilteredStringList(const izenelib::util::UString& text, std::vector<izenelib::util::UString>& str_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    str_list.resize(0);
    la::TermList::iterator it = la_term_list.begin();
    while( it!= la_term_list.end() )
    {
        bool valid = true;
        if( it->text_.length()==1 )
        {
            valid = false;
//             if( !it->text_.isKoreanChar(0) && !it->text_.isChineseChar(0) )
//             {
//                 valid = false;
//             }
        }
        if( valid)
        {
            it->text_.toLowerString();
            str_list.push_back(it->text_);
        }
        it++;
    }
}

void IDMAnalyzer::GetIdList(const izenelib::util::UString& text, std::vector<uint32_t>& id_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    id_list.resize( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
    uint32_t i=0;
    while( it!= la_term_list.end() )
    {
        id_list[i] = IDMIdConverter::GetId( it->text_, it->pos_ );
        i++;
        it++;
    }
}

void IDMAnalyzer::GetIdListForMatch(const izenelib::util::UString& text, std::vector<uint32_t>& id_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    id_list.resize(0);
    la::TermList::iterator it = la_term_list.begin();
    izenelib::util::UString lastText;
    bool bLastChinese = false;
    while (it != la_term_list.end())
    {
        char tag = IDMTermTag::GetTermTag(it->pos_);
        if( tag == IDMTermTag::ENG )
        {
    //                             it->text_.toLowerString();
        }

        if( tag != IDMTermTag::CHN )//is not chinese
        {
            uint32_t term_id = IDMIdConverter::GetId(it->text_, tag );
            id_list.push_back(term_id);
            bLastChinese = false;
        }
        else
        {
            if(bLastChinese)
            {
                izenelib::util::UString chnBigram(lastText);
                chnBigram.append(it->text_);
                uint32_t bigram_id = IDMIdConverter::GetId(chnBigram, IDMTermTag::CHN );
                id_list.push_back(bigram_id);
            }
            bLastChinese = true;
            lastText = it->text_;
        }
        it++;
    }

}

void IDMAnalyzer::CompoundInSamePosition_(const std::vector<idmlib::util::IDMTerm>& terms_in_same_position, idmlib::util::IDMTerm& compound_kor)
{
    if(terms_in_same_position.size()==1)//
    {
        compound_kor = terms_in_same_position[0];

        if(compound_kor.text.isKoreanChar(0) ) //is korean
        {
            if( compound_kor.tag== idmlib::util::IDMTermTag::NOUN && compound_kor.text.length()>=4)
            {
                compound_kor.tag = idmlib::util::IDMTermTag::COMP_NOUN;
            }
            else if( compound_kor.tag!=idmlib::util::IDMTermTag::NOUN
                && compound_kor.tag!=idmlib::util::IDMTermTag::KOR)
            {
                compound_kor.tag = idmlib::util::IDMTermTag::KOR;
            }
        }
//         else if(compound_kor.tag== idmlib::util::IDMTermTag::NOUN && compound_kor.text.length()>=6)
//         {
//             compound_kor.tag = idmlib::util::IDMTermTag::COMP_NOUN;
//         }
        compound_kor.id = IDMIdConverter::GetId( compound_kor.text, compound_kor.tag );
    }
    else
    {
        if(terms_in_same_position[0].tag == idmlib::util::IDMTermTag::KOR)
        {
            for(uint32_t p=1;p<terms_in_same_position.size();p++)
            {
                if( terms_in_same_position[p].text.isKoreanChar(0) && terms_in_same_position[p].tag == idmlib::util::IDMTermTag::NOUN )
                {
                    compound_kor.text.append(terms_in_same_position[p].text);
                }
                else
                {
                    break;
                }
            }

            bool b_noun = false;
            if( compound_kor.text.length()>0 )
            {
                if( compound_kor.text.isKoreanChar(0) && compound_kor.text.length() >= terms_in_same_position[0].text.length()-1 && compound_kor.text.length() >= 3 )
                {
                    b_noun = true;
                }
            }
            if( b_noun)
            {
                compound_kor.tag = idmlib::util::IDMTermTag::NOUN;
                if(compound_kor.text.length()>=4)
                {
                    compound_kor.tag = idmlib::util::IDMTermTag::COMP_NOUN;
                }

                compound_kor.id = IDMIdConverter::GetId( compound_kor.text, compound_kor.tag );
                compound_kor.position = terms_in_same_position[0].position;
            }
            else
            {
                compound_kor = terms_in_same_position[0];
                compound_kor.id = IDMIdConverter::GetId( compound_kor.text, compound_kor.tag );
            }
        }
    }
}

bool IDMAnalyzer::TermIgnore_(la::Term& term)
{
    if( symbol_map_.find(term.pos_) != NULL)
    {
        if( !config_.symbol ) return true;
        else
        {
            term.pos_ = "SC";
            return false;
        }
    }
    if( term.pos_=="PL-N" )
    {
        term.pos_ = "L";
        return false;
    }
    else if( term.pos_=="S-A" )
    {
        term.pos_ = "F";
        return false;
    }
//     else if (term.pos_ == "NN" ) //numberic in JA
//     {
//         term.pos_ = "S";
//         return false;
//     }
    if( config_.jma_config.path== "" || !config_.jma_config.noun_only ) return false;
    if( term.text_.isJapaneseChar(0) && term.pos_[0]!='N' ) return true;
    else if( term.text_.isChineseChar(0) )
    {
        if( term.pos_[0]=='C' ) return false;
        else
        {
            if(config_.jma_config.noun_only && term.pos_[0]!='N' ) return true;
            return false;
        }
    }

    return false;
}

bool IDMAnalyzer::TermIgnoreInTg_(const idmlib::util::IDMTerm& term)
{
    bool ignore = false;
    char tag = term.tag;
    if(tag == idmlib::util::IDMTermTag::SYMBOL ) return false;
    if(term.text.isJapaneseChar(0) && tag!=idmlib::util::IDMTermTag::COMP_NOUN) ignore = true;
    if(term.text.isChineseChar(0) && tag!=idmlib::util::IDMTermTag::COMP_NOUN && tag!=idmlib::util::IDMTermTag::CHN ) ignore = true;

    return ignore;
}

void IDMAnalyzer::CompoundInContinous_(std::vector<idmlib::util::IDMTerm>& terms_in_continous)
{
    if(config_.jma_config.path=="") return;
//     std::cout<<"[in-continous] : ";
//     for(uint32_t i=0;i<terms_in_continous.size();i++)
//     {
//         std::cout<<terms_in_continous[i].TextString()<<",";
//     }
//     std::cout<<std::endl;
    std::vector<idmlib::util::IDMTerm> result;
    idmlib::util::IDMTerm compound;
    char first_tag, last_tag;
    for(uint32_t i=0;i<terms_in_continous.size();i++)
    {
        if( !terms_in_continous[i].text.isKoreanChar(0) && (terms_in_continous[i].tag== idmlib::util::IDMTermTag::NOUN || terms_in_continous[i].tag== idmlib::util::IDMTermTag::LINK) )
        {
            if(compound.text.length()==0)
            {
                compound = terms_in_continous[i];
                if( terms_in_continous[i].tag == idmlib::util::IDMTermTag::NOUN && terms_in_continous[i].text.length()>=6)
                {
                    compound.tag = idmlib::util::IDMTermTag::COMP_NOUN;
                }
                first_tag = compound.tag;
            }
            else
            {
                compound.text.append(terms_in_continous[i].text);
                last_tag = terms_in_continous[i].tag;
                compound.tag = idmlib::util::IDMTermTag::COMP_NOUN;
            }
        }
        else
        {
            if(compound.text.length()>0)
            {
                if(!TermIgnoreInTg_(compound) && first_tag!=idmlib::util::IDMTermTag::LINK && last_tag!=idmlib::util::IDMTermTag::LINK)
                {
                    result.push_back(compound);
                }
                compound.text.clear();
            }
            //ignore non compound japanese word
            if(!TermIgnoreInTg_(terms_in_continous[i]))
            {
                result.push_back(terms_in_continous[i]);
            }
        }
    }
    if(compound.text.length()>0)
    {
        if(!TermIgnoreInTg_(compound) && first_tag!=idmlib::util::IDMTermTag::LINK && last_tag!=idmlib::util::IDMTermTag::LINK)
        {
            result.push_back(compound);
        }
    }
    terms_in_continous.resize(0);
//     std::cout<<"[result] : "<<result.size()<<std::endl;
    terms_in_continous.assign(result.begin(), result.end());
}

void IDMAnalyzer::JKCompound_(const std::vector<idmlib::util::IDMTerm>& raw_term_list, std::vector<idmlib::util::IDMTerm>& term_list)
{
    if(raw_term_list.empty()) return;
    std::vector<idmlib::util::IDMTerm> terms_in_same_position(1, raw_term_list[0]);
    std::vector<idmlib::util::IDMTerm> terms_in_continous;
    uint32_t last_position = raw_term_list[0].position;
    for(uint32_t i=1; i<raw_term_list.size(); i++)
    {

        if( raw_term_list[i].position > last_position )
        {
            if(!terms_in_same_position.empty())
            {
                idmlib::util::IDMTerm compound_kor;//from terms_in_same_position

                CompoundInSamePosition_( terms_in_same_position, compound_kor);
                terms_in_continous.push_back(compound_kor);
                terms_in_same_position.resize(0);
            }

            if(raw_term_list[i].position> last_position+1 )
            {
                if(!terms_in_continous.empty())
                {
                    CompoundInContinous_( terms_in_continous);
                    term_list.insert(term_list.end(), terms_in_continous.begin(), terms_in_continous.end());
                    terms_in_continous.resize(0);
                }
            }
        }
        terms_in_same_position.push_back(raw_term_list[i]);
        last_position = raw_term_list[i].position;
    }
    if(!terms_in_same_position.empty())
    {
        idmlib::util::IDMTerm compound_kor;//from terms_in_same_position

        CompoundInSamePosition_( terms_in_same_position, compound_kor);
        terms_in_continous.push_back(compound_kor);
        terms_in_same_position.resize(0);
    }
    if(!terms_in_continous.empty())
    {
        CompoundInContinous_( terms_in_continous);
        term_list.insert(term_list.end(), terms_in_continous.begin(), terms_in_continous.end());
        terms_in_continous.resize(0);
    }
    for(uint32_t i=0;i<term_list.size();i++)
    {
        idmlib::util::IDMIdConverter::GenId(term_list[i]);
    }
}

void IDMAnalyzer::GetTgTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list)
{

//     std::string str;
//     text.convertString( str, izenelib::util::UString::UTF_8);
//     std::cout<<"#"<<str<<"#"<<std::endl;

    std::vector<idmlib::util::IDMTerm> raw_term_list;
    GetTermList(text, raw_term_list);

//     for(uint32_t i=0;i<raw_term_list.size();i++)
//     {
//         std::cout<<"{"<<raw_term_list[i].TextString()<<","<<raw_term_list[i].position<<","<<raw_term_list[i].tag<<"},";
//     }
//     std::cout<<std::endl;

    JKCompound_(raw_term_list, term_list);

//     for(uint32_t i=0;i<term_list.size();i++)
//     {
//         std::cout<<"["<<term_list[i].TextString()<<","<<term_list[i].id<<","<<term_list[i].position<<","<<term_list[i].tag<<"],";
//     }
//     std::cout<<std::endl;
}

void IDMAnalyzer::simple_(izenelib::util::UString& content)
{
    for(uint32_t i=0;i<content.length();i++)
    {
        izenelib::util::UCS2Char* p_char = simpler_map_.find(content[i]);
        if( p_char != NULL )
        {
            content[i] = *p_char;
        }
    }
}
