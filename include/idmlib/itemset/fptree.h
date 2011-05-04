/* FP tree is used to mine frequent itemsets
 * pre step:foucused items
 * input:<Tid,value>,value correspond to a focusd items (after combine)
 * output:frequent patterns std::list
 * next step:to construct focused association rules with hash tree
 * @author:l0he1g & gmail.com
 */
#ifndef IDMLIB_ITEMSET_FPTREE_H
#define IDMLIB_ITEMSET_FPTREE_H

#include "data-source-iterator.h"

#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <queue>
using namespace std;

namespace idmlib{

// item readed from transaction file after preprocessed
struct Item
{
    int item_id;
    int sup;
    Item( int id,int i ):item_id(id),sup(i){}
    // for std::list.sort()
    bool operator <(const Item& it)
    {
        return this->sup<it.sup?true:false;
    }
};

// FP tree node
struct FPnode
{
    int item_id;
    int sup;
    FPnode* parent;
    FPnode* first_child;
    FPnode* sibling;
    FPnode* node_link;
    FPnode* shadow;            // for create condition fp-tree

    FPnode():sup(0),parent(NULL),first_child(NULL),sibling(NULL),node_link(NULL),shadow(NULL){}
    FPnode(int id):item_id(id),sup(0),parent(NULL),
                      first_child(NULL),sibling(NULL),node_link(NULL),shadow(NULL){}
};

// head table node
// descending order by item's support 
struct Head
{
    int item_id;
    int sup;
    FPnode* node_link;
    Head():sup(0),node_link(NULL){}
    Head( Item it ):item_id(it.item_id),sup(it.sup),node_link(NULL){}
    Head( int id,int s,FPnode* link):item_id(id),sup(s),node_link(link){}
    bool operator <(const Head& h)
    {
        return this->sup<h.sup?true:false;
    }
};

class FPtree
{
private:
    FPnode* root_;
    
    std::list<Head> head_table_;    // ascending order by sup
    std::list<Item> freq_items_;      // descending order by sup

    int item_num_;

    DataSourceIterator* data_;

    ofstream out_;
    
    void find_freq_items();     // impact on freq_items_

    void init_attribs_head();  // init head table according to item_num_

    void count_attribs();

    void sort_and_filter();

    void create_fptree();

    std::queue<int> order_by( std::vector<uint32_t> item_ids, std::list<Item>& freq_items );

    void insert_tree( std::queue<int>& item_ids,FPnode* parent );
    
    void create_condition_fptree( FPnode* p,std::list<Item>& suffix,std::list<Head>& cheads );

    void project_path( FPnode* p,std::list<Head>& head_table );

    void prune( std::list<Head>& cheads );

    void detach( FPnode* p );

    bool is_single_path( std::list<Head>& heads );

    void generate_rules( std::list<Head>& heads,std::list<Item>& suffix );

    void clean_condition_fptree( std::list<Head>& heads );
    
    void del_fptree( FPnode* par );
public:
    static int min_sup; // min support value as frequent item
public:
    FPtree();    

    ~FPtree();

    void set_item_num(int item_num);

    void fp_growth( std::list<Head>& heads,std::list<Item>& suffix );

    void run(const std::string& input_path, const std::string& output_path);
};

}
#endif
