#include <idmlib/semantic_space/esa/DocumentRepresentor.h>

using namespace idmlib::ssp;

void DocumentRepresentor::gatherTFDF()
{
    term_tf_map_.clear();

    termid_t termid;
    for (TermIdList::iterator iter = pTermIdList_->begin(); iter != pTermIdList_->end(); iter++)
    {
        termid = iter->termid_;

        hashmap_iterator_t tf_ret = term_tf_map_.find(termid);
        if (tf_ret == term_tf_map_.end())
        {
            term_tf_map_.insert(hashmap_t::value_type(termid, 1));

            // df
            hashmap_iterator_t df_ret = term_df_map_.find(termid);
            if (df_ret == term_df_map_.end())
            {
                term_df_map_.insert(hashmap_t::value_type(termid, 1));
            }
            else
            {
                df_ret->second ++;
            }
        }
        else
        {
            tf_ret->second ++;
        }
    }

    // document representation vector
    SparseVectorType sv(curDocId_, term_tf_map_.size());

    size_t docLen = pTermIdList_->size();
    for(hashmap_iterator_t iter = term_tf_map_.begin(); iter != term_tf_map_.end(); iter++)
    {
        iter->second /= docLen;  // normalize tf
        sv.insertItem(iter->first, iter->second);
    }

#ifdef DOC_SIM_TEST
    sv.print();//
#endif
    docRepVecOFile_.put(sv);
}


void DocumentRepresentor::calcWeight()
{
    DLOG(INFO) << "post processing (start calcWeight)" <<endl;

    SparseVectorSetIFileType inf(docSetDir_+"/doc_rep_tf.tmp");
    inf.open();

    SparseVectorSetOFileType docRepFile(docSetDir_+"/doc_rep.vec");
    docRepFile.open();

    size_t total = 0;
    while(inf.next())
    {
        SparseVectorType sv = inf.get();

        float idf;
        int df = 0;
        SparseVectorType::list_iter_t iter;
        for (iter = sv.list.begin(); iter != sv.list.end(); )
        {
            df = term_df_map_[iter->itemid];
            if (df == 0)
                df = 1;

            idf = std::log((float)docCount_ / term_df_map_[iter->itemid]);
            iter->value *= idf;

            //filt
            if(iter->value > thresholdWegt_)
            {
                iter->value = (int)((iter->value+0.0005)*1000)/1000.0;
                iter++;
            }
            else
            {
                iter = sv.list.erase(iter);
                sv.len --;
            }
        }

        if (sv.len > 0)
            docRepFile.put(sv);

        ++total;
        if ((total % 5000) == 0)
            DLOG(INFO) << total*100/docCount_ <<"%"<< endl;
    }

    inf.close();
    inf.remove(); //remove it

    docRepFile.close();

    DLOG(INFO) << "post processing (end calcWeight)" <<endl;

    cout << "inerpretation vector count: " <<total << endl;
}
