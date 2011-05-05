#include <idmlib/itemset/fptree.h>
#include <algorithm>
#include <cstdlib>
#include <cassert>

//#define DEBUG 1

namespace idmlib{

int FPtree::min_sup = 5;

FPtree::FPtree():root_(NULL), item_num_(50) {}

FPtree::~FPtree()
{
    // delete fp-tree
    //del_fptree( root_ );
    //if fp-growth excuted,only need to delete root_
    delete root_;
}

void FPtree::set_item_num(int item_num)
{
    item_num_ = item_num;
}

//delete fp-tree by post-traverse
void FPtree::del_fptree( FPnode* par )
{
    if ( !par )
        return;
    FPnode* p = par->first_child;
    while ( p )
    {
        FPnode* q = p->sibling;
        del_fptree( p );
        p = q;
    }
#ifdef DEBUG
    cout<< par->item_id <<" ";
#endif
    delete par;
}

// the first record are items id
void FPtree::init_attribs_head()
{
    int i = 0;
    while ( i < item_num_ )
    {
        Item it( ++i, 0);
        freq_items_.push_back( it );
    }
}

void FPtree::count_attribs()
{
    int result;
    uint32_t set_id;
    std::vector<uint32_t> current_set;
    while ((result = data_->Next(&set_id, &current_set)) > 0) 
    {
        for(std::vector<uint32_t>::iterator it = current_set.begin();
              it != current_set.end(); ++it)
        {
            for(std::list<Item>::iterator itemIt = freq_items_.begin();
               itemIt != freq_items_.end(); ++itemIt)
            {
                if(itemIt->item_id == (int)*it)
                    itemIt->sup++;
            }
        }
    }
    data_->Seek(0);
}

// compute wheather a sup below min_support
bool sup_compare( const Item& it )
{
    return it.sup < FPtree::min_sup?true:false;
}

// result is stored in member var vector<Item>:freq_items_
void FPtree::sort_and_filter()
{
    freq_items_.remove_if( sup_compare );
    freq_items_.sort();
    freq_items_.reverse();
}

// first scan transactions database
// items are stored in member std::list freq_items_ in descending order by sup
void FPtree::find_freq_items()
{
    // 1.record item's name
    init_attribs_head();

    // 2.count every item's support
    count_attribs();

    // 3.sort items by support and filter items with too low support
    sort_and_filter();
}

// reorder item_ids corresponding to freq_items_
std::queue<int> FPtree::order_by( std::vector<uint32_t> item_ids,std::list<Item>& freq_items )
{
    std::queue<int> ordered_item_ids;
    for ( std::list<Item>::iterator pos = freq_items.begin();
            pos != freq_items_.end();
            pos++ )
    {
        std::vector<uint32_t>::iterator id = item_ids.begin();
        while ( id != item_ids.end() )
        {
            if ( (int)*id == pos->item_id )
            {
                ordered_item_ids.push( (int)*id );
                item_ids.erase( id );
                break;
            }
            else
            {
                id++;
            }
        }
    }
    return ordered_item_ids;
}

// insert items under parent node--!main!
void FPtree::insert_tree( std::queue<int>& item_ids,FPnode* parent )
{
    if ( item_ids.empty() )
    {
        return;
    }

    int myid = item_ids.front(); // the item to be insert

    //seach the first item whether is a child of parent
    FPnode* cur = parent->first_child;

    while ( cur != NULL && cur->item_id != myid )
    {
        cur = cur->sibling;
    }

    if ( cur == NULL )
    {
        // not a child of parent,insert it
        FPnode* mynode = new FPnode( myid );
        mynode->sup = 1;
        mynode->parent = parent;

        if ( parent->first_child == NULL )
        {
            parent->first_child = mynode;
        }
        else
        {
            // insert at the beginning of sibling std::list
            mynode->sibling = parent->first_child;
            parent->first_child = mynode;
        }
        // for insert_tree( item_ids,cur )
        cur = mynode;

        // update head table
        std::list<Head>::iterator pos = head_table_.begin();
#ifdef DEBUG
        cout << "insert item:" << myid <<" parent:"<<parent->item_id<< endl;
#endif
        for ( ;pos != head_table_.end(),pos->item_id != myid;pos++ );

        if ( pos != head_table_.end() )
        {
            // insert at the beginning of node_link at head table
            mynode->node_link = pos->node_link;
            pos->node_link = mynode;
        }
        else
        {
            cerr<<"head table is bad!"<<endl;
            exit(1);
        }

    }
    else
    {
        // is a child of parent,update it count
        cur->sup++;
#ifdef DEBUG
        cout<< "update count of :" << cur->item_id << "("
            << cur->sup << ")" << " parent:"<< parent->item_id
            <<endl;
#endif
    }

    // because std::queue.front() returns a reference of the object,
    // std::queue.pop recall the destructor of the object,
    // so it's best to pop at the end
    item_ids.pop();
    insert_tree( item_ids,cur );
}

// create fptree when second scan records database
void FPtree::create_fptree()
{
#ifdef DEBUG
    cout<<endl<<"Create fptree..."<<endl;
#endif
    root_ = new FPnode(0);
    std::string record;

    // initialize head table
    for ( std::list<Item>::iterator pos = freq_items_.begin();
            pos != freq_items_.end();
            pos++ )
    {
        // ascending order by sup
        Item it( *pos );
        head_table_.push_front( it );
    }

#ifdef DEBUG
    cout<<"head table:"<<endl;
    for ( std::list<Head>::iterator pos = head_table_.begin();
            pos != head_table_.end();
            pos++)
    {
        cout<<pos->item_id<<"  ";
    }
    cout<<endl;
#endif

    int result;
    uint32_t set_id;
    std::vector<uint32_t> current_set;
    while ((result = data_->Next(&set_id, &current_set)) > 0) 
    {
#ifdef DEBUG
        cout<<endl<<"treating record: "<<record<<endl;
#endif
        std::queue<int> item_ids = order_by( current_set,freq_items_ );

#ifdef DEBUG
        cout<<"refine frequent items to be inserted:";
        std::queue<int> tmp_ids = item_ids;
        while ( !tmp_ids.empty() )
        {
            cout<<tmp_ids.front()<<" ";
            tmp_ids.pop();
        }
        cout<<endl;
#endif

        // update the fp-tree
        insert_tree( item_ids,root_ );
    }
}

// traverse fp-tree upward from node link,and make a copy of the path
// already copied,update the count,p is bottom node.
// No root_ node,it is useless.
void FPtree::project_path( FPnode* p,std::list<Head>& cheads )
{
#ifdef DEBUG
    cout<<"project path for:"<<p->item_id<<endl;
#endif
    while ( p->parent != NULL && p->parent!=root_ )
    {
        if ( p->parent->shadow == NULL )
        {
            // project a shadow for parent node
            FPnode* sha = new FPnode();
            sha->item_id = p->parent->item_id;
#ifdef DEBUG
            cout<<" "<<sha->item_id;
#endif
            sha->sup = p->parent->sup;
            p->parent->shadow = sha;

            // shadow node also has its own parent.
            // but first_child and sibling info don't need any more.
            // projection is easier than construction fp-tree.
            if ( p->shadow !=NULL )
            {
                p->shadow->parent = sha;
#ifdef DEBUG
                cout<<"(has child:"<<p->shadow->item_id<<") ";
#endif
            }

            // update info in condition head table
            std::list<Head>::iterator pos = cheads.begin();
            while ( pos!=cheads.end() && pos->item_id != sha->item_id )
            {
                pos++;
            }

            if ( pos == cheads.end() )
            {
                // case 1:add new head
                Head head( sha->item_id, sha->sup, sha );
                cheads.push_back( head );
            }
            else
            {
                // case 2:add sha to node_link and update sup in head
                sha->node_link = pos->node_link;
                pos->node_link = sha;
                pos->sup += sha->sup;
            }
        }
        else
        {
            p->parent->shadow->sup += p->sup;

            // update count info in condition head table
            std::list<Head>::iterator pos = cheads.begin();
            while ( pos->item_id != p->parent->item_id )
            {
                pos++;
            }
            if ( pos == cheads.end() )
            {
                cerr<< "condition head table is bad!"<<endl;
            }
            else
            {
                // case 3:only update sup in head
                pos->sup += p->sup;
            }
        }
        p = p->parent;
    }
#ifdef DEBUG
    cout<<endl;
#endif
}

// detach projected fp-tree from original fp-tree
// p is the first node  in node link std::list
void FPtree::detach( FPnode* p )
{
#ifdef DEBUG
    if (p)
    {
        cout<< "detach projected fp-tree for item: " << p->item_id << endl;
    }
#endif
    while ( p !=NULL )
    {
        // upward
        FPnode* q = p->parent;
        while ( q != NULL )
        {
            q->shadow = NULL;
            q = q->parent;
        }

        p = p->node_link;
    }
}

// prune a condition fp-tree,prune nodes below min_sup
void FPtree::prune( std::list<Head>& cheads )
{
#ifdef DEBUG
    cout<< "prune projected fp-tree.Head table: ";
    for ( std::list<Head>::iterator pos = cheads.begin();
            pos != cheads.end();
            pos++)
    {
        cout<<pos->item_id<<"("<<pos->sup<<")"<<"  ";
    }
    cout<<endl;
#endif
    cheads.sort();

    for ( std::list<Head>::iterator pos = cheads.begin();
            pos != cheads.end();
        )
    {
        if ( pos->sup < min_sup )
        {
            FPnode* p = pos->node_link;
            // delete the nodes in node_link
            while ( p != NULL )
            {
                FPnode* q = p;
                p = p->node_link;
                // find q's children,update their parent to q's parent
                for ( std::list<Head>::iterator it = cheads.begin();
                        it != cheads.end();
                        ++it)
                {
                    if ( it->item_id == q->item_id )
                    {
                        continue;
                    }
                    FPnode* link = it->node_link;
                    while ( link )
                    {
                        if ( link->parent && link->parent->item_id == q->item_id )
                        {
                            link->parent = link->parent->parent;
                        }
                        link = link->node_link;
                    }
                }

                delete q;
            }
            pos = cheads.erase( pos );
        }
        else
        {
            pos++;
        }
    }
}

// create condition fptree for suffix recursively
// p is point of the first node in node link std::list,cheads is empty when initialized
void FPtree::create_condition_fptree( FPnode* p,std::list<Item>& suffix,std::list<Head>& cheads )
{
#ifdef DEBUG
    cout<<endl<<"Create condition fp-tree for item:"<< p->item_id << "    ";
    cout<<"suffix: ";
    for ( std::list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        cout<<pos->item_id<<" ";
    }
    cout<<endl;
#endif

    FPnode* q=p;
    while ( p != NULL )
    {
        project_path( p,cheads );
        p = p->node_link;
    }

    detach( q );

    prune( cheads );
}

bool FPtree::is_single_path( std::list<Head>& heads )
{
    for ( std::list<Head>::iterator pos = heads.begin();
            pos != heads.end();
            pos++)
    {
        assert( pos->node_link != NULL );
        if ( pos->node_link->node_link != NULL )
            return false;
    }
    return true;
}

// compute combination and output them to target file
void FPtree::generate_rules( std::list<Head>& heads,std::list<Item>& suffix )
{
#ifdef DEBUG
    cout<<"rule been generated: ";
    for ( std::list<Head>::reverse_iterator pos = heads.rbegin();
            pos != heads.rend();
            pos++)
    {
        cout<<pos->item_id<<"("<< pos->sup<<")"<<" ";
    }
    for ( std::list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        cout<<pos->item_id<<"("<< pos->sup<<")"<<" ";;
    }
    cout<<endl;
#endif

    for ( std::list<Head>::reverse_iterator pos = heads.rbegin();
            pos != heads.rend();
            pos++)
    {
        out_<<pos->item_id<<"  ";
    }
    for ( std::list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        out_<<pos->item_id<<" ";
    }
    out_<<endl;
}

// remove condition fp-tree
void FPtree::clean_condition_fptree( std::list<Head>& heads )
{
#ifdef DEBUG
    cout<<"clean condition fp-tree.Head table: ";
    for ( std::list<Head>::iterator pos = heads.begin();
            pos != heads.end();
            pos++)
    {
        cout<< pos->item_id<<"("<<pos->sup<<")"<<" ";
    }
    cout<<endl;
#endif
    for ( std::list<Head>::iterator pos = heads.begin();
            pos != heads.end();
            pos++)
    {
        FPnode* p = pos->node_link;
        while ( p )
        {
            FPnode* q = p;
            p = p->node_link;
            delete q;
        }
        pos->node_link = NULL;
    }
}

// fp growth algorithm
void FPtree::fp_growth( std::list<Head>& heads,std::list<Item>& suffix )
{
#ifdef DEBUG
    cout<<endl<<"fp-growth at suffix: ";
    for ( std::list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        cout<<pos->item_id<<" ";
    }
    cout<<endl;
    cout<<"Head table:";
    for ( std::list<Head>::iterator pos = heads.begin();
            pos != heads.end();
            pos++)
    {
        cout<< pos->item_id<<"("<<pos->sup<<")"<<" ";
    }
    cout<<endl;
#endif

    if ( is_single_path( heads ) )
    {
        generate_rules( heads,suffix);
    }
    else
    {
        // 1.treat nodes in head table one by one
        for ( std::list<Head>::iterator pos = heads.begin();
                pos != heads.end();
                pos++)
        {
            std::list<Item> con_suffix( suffix );
            Item it( pos->item_id,pos->sup );
            con_suffix.push_front( it );

            std::list<Head> cheads;  // output parament,condition head table
            create_condition_fptree( pos->node_link,con_suffix,cheads );
            if ( !cheads.empty() )
            {
                fp_growth( cheads,con_suffix );
            }
        }
    }
    clean_condition_fptree( heads );
}

void FPtree::run(DataSourceIterator* data, const std::string& output_path)
{
    // 1. open records file
    data_ = data;
    // 2. find frequent items in records database
    find_freq_items();
#ifdef DEBUG
    cout<<freq_items_.size()<<" freqent items:"<<endl;
    for ( std::list<Item>::iterator pos = freq_items_.begin();
            pos != freq_items_.end();
            pos++)
    {
        cout<<pos->item_id<<":"<<pos->sup<<"  ";
    }
    cout<<endl;
#endif

    // 3.create fp-tree
    create_fptree();
    ///FP-GROWTH
    std::list<Item> suffix;

    // 4.use fp-growth to compute association rules and output to target file

    out_.open( output_path.c_str() );
    fp_growth( head_table_,suffix );
}

}
