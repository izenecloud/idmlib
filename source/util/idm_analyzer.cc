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

void IDMAnalyzer::InitWithConfig_(const IDMAnalyzerConfig& config)
{
    boost::shared_ptr<la::MultiLanguageAnalyzer> ml_analyzer(new la::MultiLanguageAnalyzer() );
    ml_analyzer->setExtractSpecialChar(false, false);
    
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
            if ( pch != NULL ) {
                pch->setExtractSpecialChar(false, false);
                pch->setAnalysisType( config.cma_config.type);
                pch->setLabelMode();
                pch->setRemoveStopwords(config.cma_config.remove_stopwords);
            }
        }
    }
    
    if(config.jma_config.path!="")
    {
        jma_analyzer.reset( new la::JapaneseAnalyzer(config.jma_config.path) );
        la::JapaneseAnalyzer* pja = dynamic_cast<la::JapaneseAnalyzer*>(jma_analyzer.get());
        pja->setExtractSpecialChar(false, false);
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

void IDMAnalyzer::ExtractSpecialChar(bool extractSpecialChar, bool convertToPlaceHolder)
{
    la_->getAnalyzer()->setExtractSpecialChar(extractSpecialChar, convertToPlaceHolder);
}

void IDMAnalyzer::ExtractSymbols()
{
    ExtractSpecialChar(true, false);
}

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
        la_->process( text, term_list );
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
}

void IDMAnalyzer::GetTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list)
{
    la::TermList la_term_list;
    GetTermList(text, la_term_list);
    term_list.resize( la_term_list.size() );
    la::TermList::iterator it = la_term_list.begin();
    uint32_t i=0;
    while( it!= la_term_list.end() )
    {
        term_list[i].text = it->text_;
        term_list[i].tag = IDMTermTag::GetTermTag( it->pos_ );
        term_list[i].id = IDMIdConverter::GetId( it->text_, term_list[i].tag );
        term_list[i].position = it->wordOffset_;
        i++;
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

void IDMAnalyzer::GetTgTermList(const izenelib::util::UString& text, std::vector<idmlib::util::IDMTerm>& term_list)
{
    typedef idmlib::util::IDMTerm TgTerm;
    std::vector<idmlib::util::IDMTerm> raw_term_list;
    GetTermList(text, raw_term_list);
    uint32_t size = raw_term_list.size();
    uint32_t p=0;
    std::vector<idmlib::util::IDMTerm> tmp_term_list;
    uint32_t last_position = 0;

    while (true)
    {
        bool bCheckTmpTermList = false;

        uint32_t position = 0;
        if(p == size) bCheckTmpTermList = true;
        else
        {
            position = raw_term_list[p].position;
            if(last_position != position) bCheckTmpTermList = true;
        }
        if( bCheckTmpTermList )
        {
            if( tmp_term_list.size() > 0 )
            {
                //TODO
                if(tmp_term_list.size()==1)//
                {
                    uint32_t termId = 0;
                    if(tmp_term_list[0].text.isKoreanChar(0) && tmp_term_list[0].tag== idmlib::util::IDMTermTag::NOUN && tmp_term_list[0].text.length()>=4)
                    {
                        tmp_term_list[0].tag = idmlib::util::IDMTermTag::KOR_COMP_NOUN;
                    }
                    else
                    {
                        if( tmp_term_list[0].text.isKoreanChar(0) && tmp_term_list[0].tag!=idmlib::util::IDMTermTag::NOUN
                            && tmp_term_list[0].tag!=idmlib::util::IDMTermTag::KOR)
                        {
                            tmp_term_list[0].tag = idmlib::util::IDMTermTag::KOR;
                        }
                    }
                    
                    termId = IDMIdConverter::GetId( tmp_term_list[0].text, tmp_term_list[0].tag );
                    TgTerm term(tmp_term_list[0].text, termId, tmp_term_list[0].tag, tmp_term_list[0].position);
                    term_list.push_back(term);

                }
                else
                {
                    if(tmp_term_list[0].tag == idmlib::util::IDMTermTag::KOR)
                    {
                        izenelib::util::UString combination;
                        for(uint32_t p=1;p<tmp_term_list.size();p++)
                        {
                            if( tmp_term_list[p].text.isKoreanChar(0) && tmp_term_list[p].tag == idmlib::util::IDMTermTag::NOUN )
                            {
                                combination.append(tmp_term_list[p].text);
                            }
                            else
                            {
                                break;
                            }
                        }
                        if( combination.isKoreanChar(0) && combination.length() >= tmp_term_list[0].text.length()-1 && combination.length() >= 3 )
                        {
                            char tag = idmlib::util::IDMTermTag::NOUN;
                            if(combination.length()>=4)
                            {
                                tag = idmlib::util::IDMTermTag::KOR_COMP_NOUN;
                            }
                            
                            uint32_t termId = IDMIdConverter::GetId( combination, tag );
                            TgTerm term(combination, termId, tag, tmp_term_list[0].position);
                            term_list.push_back(term);
                        }
                        else
                        {
                            uint32_t termId = IDMIdConverter::GetId( tmp_term_list[0].text, tmp_term_list[0].tag );
                            TgTerm term(tmp_term_list[0].text, termId, tmp_term_list[0].tag, tmp_term_list[0].position);
                            term_list.push_back(term);

                        }
                    }
                }
                tmp_term_list.clear();
            }
        }
        if(p == size) break;

        if (raw_term_list[p].text.length() == 0)
        {
            p++;
            last_position = position;
            continue;
        }
        char tag = raw_term_list[p].tag;
        if (tag == idmlib::util::IDMTermTag::ENG || tag == idmlib::util::IDMTermTag::CHN)//english or chinese
        {
            uint32_t termId = IDMIdConverter::GetId( raw_term_list[p].text, tag );
            TgTerm term(raw_term_list[p].text, termId, tag, position);
            term_list.push_back(term);
        }
        else
        {
            TgTerm term(raw_term_list[p].text, 0, tag, position);
            tmp_term_list.push_back(term);
        }
        last_position = position;
        ++p;
    }
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

