#include <idmlib/similarity/term_similarity.h>
#include <idmlib/semantic_space/cosine_similarity.h>
using namespace idmlib::ssp;
using namespace idmlib::sim;

TermSimilarity::TermSimilarity(const std::string& dir, FunctionType callback)
:dir_(dir), callback_(callback), ri_(new RIType(dir))
{
}

TermSimilarity::~TermSimilarity()
{
  if(ri_!=NULL) delete ri_;
}

bool TermSimilarity::Open()
{
  return ri_->Open();
}

bool TermSimilarity::Flush()
{
  return ri_->Flush();
}

bool TermSimilarity::Append(IdType id, const std::vector<std::pair<ContextIdType, FreqType> >& doc_item)
{
  return ri_->Append(id, doc_item);
}

bool TermSimilarity::Compute()
{
  IdType min = 1;
  IdType max = ri_->VectorCount();
  VectorType vec;
  VectorType vec2;
  for(IdType i = min;i<=max;i++)
  {
    if(!ri_->GetVector(i, vec))
    {
      continue;
    }
    for(IdType j=i+1;j<=max;j++)
    {
      if(!ri_->GetVector(j, vec2))
      {
        continue;
      }
      double sim = idmlib::ssp::CosineSimilarity<VectorType>::Sim(vec, vec2);
      callback_(i,j,sim);
    }
  }
  return true;
}
