#include <idmlib/keyphrase-extraction/kpe_scorer.h>
#include <idmlib/util/resource_util.h>
using namespace idmlib::kpe;

KPEScorer::KPEScorer(idmlib::util::IDMAnalyzer* analyzer)
:analyzer_(analyzer), swContainer_(NULL),
invalidChnBigram_(new izenelib::am::rde_hash<uint64_t, bool>()), 
nonAppearTerms_(new izenelib::am::rde_hash<uint32_t, bool>()), 
midAppearTerms_(new izenelib::am::rde_hash<uint32_t, bool>())
{
    
}
KPEScorer::~KPEScorer()
{
//             if( langScorer_ != NULL ) delete langScorer_;
    if( swContainer_ != NULL ) delete swContainer_;
    if( invalidChnBigram_ != NULL ) delete invalidChnBigram_;
    if( nonAppearTerms_ != NULL ) delete nonAppearTerms_;
    if( midAppearTerms_ != NULL ) delete midAppearTerms_;
}
bool KPEScorer::load(const std::string& resPath)
{
  std::cout<<"[KPEScorer] loading KPE resources..."<<std::endl;
  namespace bfs = boost::filesystem;
  swContainer_ = new idmlib::util::StopWordContainer();
  {
      std::istream* ifs = idmlib::util::getResourceStream(resPath+"/stop_words");
      if( ifs== NULL)
      {
        std::cerr<<"[KPEScorer] load stop_words failed."<<std::endl;
        return false;
      }
      std::string word;
      while ( getline ( *ifs,word ) )
      {
          boost::to_lower(word);
          if ( word.length() >0 )
          {
//                       std::cout<<"add stop "<<word<<std::endl;
              std::vector<uint32_t> termIdList;
              analyzer_->GetIdList(izenelib::util::UString(word,izenelib::util::UString::UTF_8), termIdList);

              if( termIdList.size() > 0 )
              {
                  swContainer_->insert(termIdList);
              }
              
          }
      }
      delete ifs;
  }
    
  std::istream* ifs = idmlib::util::getResourceStream(resPath+"/invalid_chn_bigram");
  if( ifs== NULL)
  {
    std::cerr<<"[KPEScorer] load invalid_chn_bigram failed."<<std::endl;
    return false;
  }
  std::string word;
  while ( getline ( *ifs,word ) )
  {
      if ( word.length() >0 )
      {
//                     std::cout<<"[KPE] "<<word<<std::endl;
          if( word[0] == '@' ) break;
          izenelib::util::UString ustr(word, izenelib::util::UString::UTF_8);
          if( ustr.length()!= 2 ) continue;
          if( !ustr.isChineseChar(0) ) continue;
          if( !ustr.isChineseChar(1) ) continue;
          izenelib::util::UString ustr1;
          izenelib::util::UString ustr2;
          ustr.substr(ustr1, 0,1);
          ustr.substr(ustr2, 1,1);
          uint32_t id1 = idmlib::util::IDMIdConverter::GetId(ustr1, idmlib::util::IDMTermTag::CHN);
          uint32_t id2 = idmlib::util::IDMIdConverter::GetId(ustr2, idmlib::util::IDMTermTag::CHN);
          uint64_t key = idmlib::util::make64UInt(id1, id2);
          invalidChnBigram_->insert(key, 0);
      }
  }
  delete ifs;
  
  arabicNumber_ = idmlib::util::IDMIdConverter::GetId( izenelib::util::UString("ARABICNUMBER",izenelib::util::UString::UTF_8) );
  singleEnglishChar_ = idmlib::util::IDMIdConverter::GetId( izenelib::util::UString("SINGLEENGLISHCHAR",izenelib::util::UString::UTF_8) );
  symbol_id_ = idmlib::util::IDMIdConverter::GetId( izenelib::util::UString("THISISSYMBOL",izenelib::util::UString::UTF_8) );
  if(!ub_info_.load(resPath+"/ub_info"))
  {
    std::cerr<<"[KPEScorer] ubinfo load failed."<<std::endl;
    return false;
  }
  std::cout<<"[KPEScorer] loading KPE resources finished."<<std::endl;
  return true;
}

int KPEScorer::prefixTest(const std::vector<uint32_t>& termIdList)
{
    uint32_t termCount = termIdList.size();
    if( termCount == 0 ) return KPStatus::NON_KP;
    uint32_t first_termid = termIdList[0];
    if( isNonAppearTerm(first_termid) )
    {
        return KPStatus::RETURN;
    }
    if(termCount == 1)
    {
        //check if single korean noun
        if( idmlib::util::IDMIdConverter::IsKP(first_termid) )
        {
            return KPStatus::KP;
        }
        else
        {
          return KPStatus::NON_KP;
        }
    }
    if( termCount == 2 )
    {
        uint64_t key = idmlib::util::IDMIdConverter::make64UInt(termIdList[0], termIdList[1]);
        if( invalidChnBigram_->find(key) != NULL )
        {
            return KPStatus::NON_KP;
        }
    }
    //is it should be startWithStopWord?
    if ( swContainer_->endWithStopWord( termIdList ) )
    {
        return KPStatus::RETURN;
    }
    if( isMidAppearTerm(termIdList.front()) || isMidAppearTerm(termIdList.back())  )
    {
        return KPStatus::NON_KP;
    }
    
    return ub_prefix_ab_verify_(termIdList[0], termIdList[1]);
    
}

double KPEScorer::ub_score_(uint32_t u, uint32_t d)
{
  if( u==0 ) return 0.0;
  double du = (double)u;
  double result = du*std::log(du)*0.7/d;
  if( result>1.0) result = 1.0;
  return result;
}

double KPEScorer::ub_p_score_(uint32_t p)
{
  double r = -1.0;
  if( p== idmlib::util::IDMIdConverter::GetSpaceId() )
  {
    return ub_space_score_();
  }
  UnigramInfo* u = ub_info_.GetUnigramInfo(p);
  if( u != NULL)
  {
    r = ub_score_((*u).p_count, (*u).total_count);
  }
  return r;
}

double KPEScorer::ub_a_score_(uint32_t a)
{
  double r = -1.0;
  UnigramInfo* u = ub_info_.GetUnigramInfo(a);
  if( u != NULL)
  {
    r = ub_score_((*u).a_count, (*u).total_count);
  }
  return r;
}

double KPEScorer::ub_b_score_(uint32_t b)
{
  double r = -1.0;
  UnigramInfo* u = ub_info_.GetUnigramInfo(b);
  if( u != NULL)
  {
    r = ub_score_((*u).b_count, (*u).total_count);
  }
  return r;
}

double KPEScorer::ub_d_score_(uint32_t d)
{
  double r = -1.0;
  UnigramInfo* u = ub_info_.GetUnigramInfo(d);
  if( u != NULL)
  {
    r = ub_score_((*u).d_count, (*u).total_count);
  }
  return r;
}

double KPEScorer::ub_e_score_(uint32_t e)
{
  double r = -1.0;
  UnigramInfo* u = ub_info_.GetUnigramInfo(e);
  if( u != NULL)
  {
    r = ub_score_((*u).e_count, (*u).total_count);
  }
  return r;
}

double KPEScorer::ub_s_score_(uint32_t s)
{
  double r = -1.0;
  if( s== idmlib::util::IDMIdConverter::GetSpaceId() )
  {
    return ub_space_score_();
  }
  UnigramInfo* u = ub_info_.GetUnigramInfo(s);
  if( u != NULL)
  {
    r = ub_score_((*u).s_count, (*u).total_count);
  }
  return r;
}

uint64_t KPEScorer::bigram_id_(uint32_t id1, uint32_t id2)
{
  return idmlib::util::IDMIdConverter::make64UInt(id1, id2);
}

double KPEScorer::ub_pa_score_(uint64_t pa)
{
  double r = -1.0;
  BigramInfo* b = ub_info_.GetBigramInfo(pa);
  if( b != NULL)
  {
    r = ub_score_((*b).pa_count, (*b).total_count);
  }
  return r;
}

double KPEScorer::ub_pa_score_(uint32_t p, uint32_t a)
{
  if( p== idmlib::util::IDMIdConverter::GetSpaceId() )
  {
    return ub_space_score_();
  }
  uint64_t pa = bigram_id_(p,a);
  return ub_pa_score_(pa);
}

double KPEScorer::ub_ab_score_(uint64_t ab)
{
  double r = -1.0;
  BigramInfo* b = ub_info_.GetBigramInfo(ab);
  if( b != NULL)
  {
    r = ub_score_((*b).ab_count, (*b).total_count);
  }
  return r;
}

double KPEScorer::ub_de_score_(uint64_t de)
{
  double r = -1.0;
  BigramInfo* b = ub_info_.GetBigramInfo(de);
  if( b != NULL)
  {
    r = ub_score_((*b).de_count, (*b).total_count);
  }
  return r;
}

double KPEScorer::ub_es_score_(uint64_t es)
{
  double r = -1.0;
  BigramInfo* b = ub_info_.GetBigramInfo(es);
  if( b != NULL)
  {
    r = ub_score_((*b).es_count, (*b).total_count);
  }
  return r;
}

double KPEScorer::ub_se_score_(uint32_t s, uint32_t e)
{
  if( s== idmlib::util::IDMIdConverter::GetSpaceId() )
  {
    return ub_space_score_();
  }
  uint64_t es = bigram_id_(e,s);
  return ub_es_score_(es);
}


int KPEScorer::ub_prefix_ab_verify_(uint32_t a, uint32_t b)
{
  return ub_ab_verify_(a, b);
}

int KPEScorer::ub_ab_verify_(uint32_t a, uint32_t b)
{
  uint64_t ab = idmlib::util::IDMIdConverter::make64UInt(a, b);
  double ab_score = ub_ab_score_(ab);
  double a_score = ub_a_score_(a);
  double b_score = ub_b_score_(b);
//   std::cout<<"{AB_SCORE} "<<ab_score<<","<<a_score<<","<<b_score<<std::endl;
  return ub_aborde_score_verify_(ab_score, a_score, b_score);
  
}

int KPEScorer::ub_de_verify_(uint32_t d, uint32_t e)
{
  uint64_t de = idmlib::util::IDMIdConverter::make64UInt(d, e);
  double de_score = ub_de_score_(de);
  double d_score = ub_d_score_(d);
  double e_score = ub_e_score_(e);
//   std::cout<<"{DE_SCORE} "<<de_score<<","<<d_score<<","<<e_score<<std::endl;
  return ub_aborde_score_verify_(de_score, e_score, d_score);
}

int KPEScorer::ub_aborde_score_verify_(double aborde, double aore, double bord)
{
  if( aborde>=0.0 )
  {
    if( aborde < 0.1 )
    {
      return KPStatus::RETURN;
    }
    else if(aborde>=0.4) return KPStatus::CANDIDATE;
  }
  if( aore>=0.0 && bord>=0.0 && aore*bord<0.01 )
  {
    return KPStatus::RETURN;
  }
  return KPStatus::CANDIDATE;
}

int KPEScorer::ub_abcde_verify_(uint32_t a, uint32_t b, uint32_t d, uint32_t e)
{
  uint64_t ab = idmlib::util::IDMIdConverter::make64UInt(a, b);
  double ab_score = ub_ab_score_(ab);
  if( ab_score < 0.0) ab_score = ub_noappearance_score_();
  uint64_t de = idmlib::util::IDMIdConverter::make64UInt(d, e);
  double de_score = ub_de_score_(de);
  if( de_score < 0.0) de_score = ub_noappearance_score_();
//   std::cout<<"{AB,DE} "<<ab_score<<","<<de_score<<std::endl;
  double abde_score = ab_score*de_score;
  if( abde_score < 0.03 ) return KPStatus::NON_KP;
  else if( abde_score >= 0.1 ) return KPStatus::CANDIDATE;
  
  double a_score = ub_a_score_(a);
  if( a_score < 0.0) a_score = ub_noappearance_score_();
  double b_score = ub_b_score_(b);
  if( b_score < 0.0) b_score = ub_noappearance_score_();
  double d_score = ub_d_score_(d);
  if( d_score < 0.0) d_score = ub_noappearance_score_();
  double e_score = ub_e_score_(e);
  if( e_score < 0.0) e_score = ub_noappearance_score_();
//   std::cout<<"{A,B,D,E} "<<a_score<<","<<b_score<<","<<d_score<<","<<e_score<<std::endl;
  abde_score = a_score*b_score*d_score*e_score;
  
  if( abde_score<0.003 ) return KPStatus::NON_KP;
  
//   int ab_status = ub_ab_verify_(a, b);
//   if( ab_status!= KPStatus::CANDIDATE ) return ab_status;
//   int de_status = ub_de_verify_(d, e);
//   if( de_status!= KPStatus::CANDIDATE ) return de_status;
  return KPStatus::CANDIDATE;
}

int KPEScorer::ub_context_verify_(const std::vector<id2count_t>& term_list, uint32_t id1, uint32_t id2, uint32_t f, boost::function<double (uint32_t) > u_scorer, boost::function<double (uint32_t, uint32_t) > b_scorer )
{
  std::vector<double> pors_score_list(term_list.size()+1);
  std::vector<double> b_score_list(term_list.size()+1);
  std::vector<double> prob_list(term_list.size()+1);
  uint32_t sum = 0;
  for( uint32_t i=0;i<term_list.size();i++)
  {
    pors_score_list[i] = u_scorer(term_list[i].first);
    b_score_list[i] = b_scorer(term_list[i].first, id1 );
    prob_list[i] = (double) term_list[i].second / f;
    sum += term_list[i].second;
  }
  pors_score_list[pors_score_list.size()-1] = u_scorer(idmlib::util::IDMIdConverter::GetSpaceId());
  b_score_list[b_score_list.size()-1] = b_scorer(idmlib::util::IDMIdConverter::GetSpaceId(), id1);
  prob_list[prob_list.size()-1] = ((double)f - sum)/f;
  std::vector<double> result_weight_list;
  ub_context_weight_(pors_score_list, b_score_list, result_weight_list);
  double bce_score = bce_(prob_list, result_weight_list);
  //judge by bce_score
//   std::cout<<"[BCE]"<<bce_score<<std::endl;
  if( bce_score>2.0 ) return KPStatus::CANDIDATE;
  else return KPStatus::NON_KP;
}

int KPEScorer::ub_pab_verify_(const std::vector<id2count_t>& p_term_list, uint32_t a, uint32_t b, uint32_t f)
{
  boost::function<double (uint32_t) > u_scorer = boost::bind( &KPEScorer::ub_p_score_, this, _1);
  boost::function<double (uint32_t, uint32_t) > b_scorer = boost::bind( &KPEScorer::ub_pa_score_, this, _1, _2);
  return ub_context_verify_(p_term_list, a, b, f, u_scorer, b_scorer);
}

int KPEScorer::ub_sed_verify_(const std::vector<id2count_t>& s_term_list, uint32_t e, uint32_t d, uint32_t f)
{
  boost::function<double (uint32_t) > u_scorer = boost::bind( &KPEScorer::ub_s_score_, this, _1);
  boost::function<double (uint32_t, uint32_t) > b_scorer = boost::bind( &KPEScorer::ub_se_score_, this, _1, _2);
  return ub_context_verify_(s_term_list, e, d, f, u_scorer, b_scorer);
}

void KPEScorer::ub_context_weight_(const std::vector<double>& pors_score_list, const std::vector<double>& paorse_score_list, std::vector<double>& result_weight_list)
{
  result_weight_list.resize( pors_score_list.size() );
  for( uint32_t i=0;i<pors_score_list.size(); i++)
  {
    double pors_score = pors_score_list[i];
    if( pors_score<0.0 ) pors_score = ub_noappearance_score_();
    double paorse_score = paorse_score_list[i];
    if( paorse_score<0.0 ) paorse_score = ub_noappearance_score_();
    result_weight_list[i] = (pors_score+paorse_score)*2;
  }
}

double KPEScorer::bce_(const std::vector<double>& prob_list, const std::vector<double>& weight_list)
{
  double result = 0.0;
  for (std::size_t i=0; i<prob_list.size();++i)
  {
//     std::cout<<"{bce_} "<<prob_list[i]<<","<<weight_list[i]<<std::endl;
    if( prob_list[i] > 0.0)
    {
      double inc = 0.0;
      if(i==prob_list.size()-1)//this is the space
      {
        inc = weight_list[i] * prob_list[i]*(-1)*(-1);
      }
      else
      {
        inc = weight_list[i] * prob_list[i]*log2_(prob_list[i])*(-1);
      }
      
      result += inc;
    }
  }
  //normalize
  result *= normalize_(prob_list.size() );
  return result;
}

double KPEScorer::ub_space_score_()
{
  return 0.7;
}

double KPEScorer::ub_noappearance_score_()
{
  return 0.5;
}

double KPEScorer::normalize_(uint32_t context_count)
{
  return 1.0/log_(10, context_count+1 );
}


void KPEScorer::insertNonAppearTerm( uint32_t termId )
{
    if( nonAppearTerms_ == NULL )
    {
        nonAppearTerms_ = new izenelib::am::rde_hash<uint32_t, bool>();
    }
    nonAppearTerms_->insert(termId, 0);
}

bool KPEScorer::isNonAppearTerm( uint32_t termId)
{
  if( termId == arabicNumber_ || termId == singleEnglishChar_ ) return true;
  if( nonAppearTerms_ == NULL ) return false;
  return (nonAppearTerms_->find(termId)!=NULL);
}

void KPEScorer::insertMidAppearTerm( uint32_t termId )
{
    if( midAppearTerms_ == NULL )
    {
        midAppearTerms_ = new izenelib::am::rde_hash<uint32_t, bool>();
    }
    midAppearTerms_->insert(termId, 0);
}

bool KPEScorer::isMidAppearTerm( uint32_t termId)
{
    if( midAppearTerms_ == NULL ) return false;
    return (midAppearTerms_->find(termId)!=NULL);
}

bool KPEScorer::isSplitTerm(const izenelib::util::UString& ustr, char tag, uint32_t termId, uint32_t& insertTermId)
{
  //TODO need to refactor
    uint32_t indicateId = termId;
    insertTermId = termId;
    bool result = false;
//             std::cout<<"aaa"<<std::endl;
    if( tag == idmlib::util::IDMTermTag::NUM)
    {
        insertTermId = arabicNumber_;
        return true;
    }
    else if(tag == idmlib::util::IDMTermTag::SYMBOL)
    {
      return true;
    }
    else if( tag == idmlib::util::IDMTermTag::ENG)
    {
        
        izenelib::util::UString lower = ustr;
        lower.toLowerString();
        indicateId = idmlib::util::IDMIdConverter::GetId(lower, tag);
        
        if( isNonAppearTerm(indicateId) )
        {
            insertTermId = indicateId;
            return true;
        }
        if( lower.length() == 1 )
        {
            insertNonAppearTerm(indicateId);
            insertTermId = indicateId;
            return true;
        }
    }
    else if( tag == idmlib::util::IDMTermTag::CHN)
    {
        if( isNonAppearTerm(indicateId) )
        {
            return true;
        }
    }
//             std::cout<<"bbb"<<std::endl;
    if( swContainer_->isStopWord( indicateId ) )
    {
        if( tag == idmlib::util::IDMTermTag::ENG)
        {
            insertTermId = indicateId;
        }
        return true;
    }
//             std::cout<<"ccc"<<std::endl;
    if ( tag == idmlib::util::IDMTermTag::KOR && ustr.isDigitChar(0) )
    {
        insertNonAppearTerm(termId);
        result = true;
    }
    else if( (tag == idmlib::util::IDMTermTag::KOR||tag == idmlib::util::IDMTermTag::NOUN) && ustr.length()==1 )
    {
        insertNonAppearTerm(termId);
        result = true;
    }
    else if( tag == idmlib::util::IDMTermTag::KOR )
    {
        insertMidAppearTerm(termId);
    }
//             std::cout<<"ddd"<<std::endl;
    
    return result;
}

//only care about SI more than 1 length
std::pair<bool, double> KPEScorer::test(const SI& item,const SCI& left_citem, const SCI& right_citem)
{
  std::pair<bool, double> result(false, 0.0);
  
  if(item.termid_list.size()<2) return result;
  uint32_t a = item.termid_list[0];
  uint32_t b = item.termid_list[1];
  uint32_t d = item.termid_list[item.termid_list.size()-2];
  uint32_t e = item.termid_list[item.termid_list.size()-1];
  int abcde_status = ub_abcde_verify_(a, b, d, e);
  if( abcde_status == KPStatus::KP )
  {
    result.first = true;
    result.second = 1.0;
    return result;
  }
  else if( abcde_status != KPStatus::CANDIDATE )
  {
    return result;//not KPE
  }
  
  int pab_status = ub_pab_verify_(left_citem.term_list, a, b, item.freq);
  if( pab_status == KPStatus::KP )
  {
    result.first = true;
    result.second = 1.0;
    return result;
  }
  else if( pab_status != KPStatus::CANDIDATE )
  {
    return result;//not KPE
  }
  
  int sed_status = ub_sed_verify_(right_citem.term_list, e, d, item.freq);
  if( sed_status == KPStatus::KP )
  {
    result.first = true;
    result.second = 1.0;
    return result;
  }
  else if( sed_status != KPStatus::CANDIDATE )
  {
    return result;//not KPE
  }
  //passed all filter, it is treated as KP.
  result.first = true;
  result.second = 1.0;
  return result;
}

void KPEScorer::flush()
{
    if( nonAppearTerms_ != NULL )
    {
        delete nonAppearTerms_;
        nonAppearTerms_ = NULL;
    }
    if( midAppearTerms_ != NULL )
    {
        delete midAppearTerms_;
        midAppearTerms_ = NULL;
    }
}
    
      
        
