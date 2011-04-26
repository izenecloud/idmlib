/* FP tree is used to mine frequent itemsets
 * pre step:foucused items
 * input:<Tid,value>,value correspond to a focusd items (after combine)
 * output:frequent patterns list
 * next step:to construct focused association rules with hash tree
 * @author:l0he1g & gmail.com
 */
#ifndef FPTREE_H
#define FPTREE_H
#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <vector>
#include <queue>
using namespace std;

// item readed from transaction file after preprocessed
struct Item
{
    int item_id;
    int sup;
    Item( int id,int i ):item_id(id),sup(i){}
    // for list.sort()
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
    
    list<Head> head_table_;    // ascending order by sup
    list<Item> freq_items_;      // descending order by sup

    int item_num_;

    ifstream infile_;
    ofstream out_;
    
    void find_freq_items();     // impact on freq_items_
    void get_attribs_head();
    void count_attribs();
    void sort_and_filter();

    void create_fptree();
    queue<int> refine_items( string record );
    queue<int> order_by( list<int> item_ids,list<Item>& freq_items );
    void insert_tree( queue<int>& item_ids,FPnode* parent );
    
    void create_condition_fptree( FPnode* p,list<Item>& suffix,list<Head>& cheads );
    void project_path( FPnode* p,list<Head>& head_table );
    void prune( list<Head>& cheads );
    void detach( FPnode* p );

    bool is_single_path( list<Head>& heads );
    void generate_rules( list<Head>& heads,list<Item>& suffix );
    void clean_condition_fptree( list<Head>& heads );
    
    void del_fptree( FPnode* par );
public:
    static int min_sup;                // min support value as frequent item
public:
    FPtree();    
    ~FPtree();

    void set_item_num(int item_num);
    void fp_growth( list<Head>& heads,list<Item>& suffix );
    void run(const string& input_path, const string& output_path);
};

#endif
