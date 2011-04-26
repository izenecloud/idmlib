#include <idmlib/fpm/fptree.h>
#include <algorithm>
#include <cstdlib>
#include <cassert>

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
void FPtree::get_attribs_head()
{
    string record;
    getline( infile_,record );
    int i = 0;
    // init freq items,id begin from 1 to item_num
    while ( i < item_num_ )
    {
        Item it( ++i,0 );
        freq_items_.push_back( it );
    }
}

void FPtree::count_attribs()
{
    string record;

    while ( getline( infile_,record ) )
    {
        string::size_type idx = 0;
        int count = 0;
        list<Item>::iterator pos = freq_items_.begin();

        while ( count < item_num_ && pos != freq_items_.end() )
        {
            switch ( record[idx] )
            {
            case ',':
                count++;
                pos++;
                break;
            case '1':
                pos->sup++; // update support
                break;
            default:
                break;        // do nothing
            }
            idx++;
        }

        // validate format of this record
        if ( count != item_num_ )
        {
            cerr<<"Wrong record:"<<record<<endl;
            cerr<<"Actual count:"<<count<<endl;
            exit(1);
        }
    }
    // reset file stream
    infile_.clear();
    infile_.seekg( 0,ios::beg );
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
// items are stored in member list freq_items_ in descending order by sup
void FPtree::find_freq_items()
{
    // 1.record item's name
    get_attribs_head();

    // 2.count every item's support
    count_attribs();

    // 3.sort items by support and filter items with too low support
    sort_and_filter();
}

// reorder item_ids corresponding to freq_items_
queue<int> FPtree::order_by( list<int> item_ids,list<Item>& freq_items_ )
{
    queue<int> ordered_item_ids;
    for ( list<Item>::iterator pos = freq_items_.begin();
            pos != freq_items_.end();
            pos++ )
    {
        list<int>::iterator id = item_ids.begin();
        while ( id != item_ids.end() )
        {
            if ( *id == pos->item_id )
            {
                ordered_item_ids.push( *id );
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

// refine frequent items in a record and order them by frequent items
queue<int> FPtree::refine_items( string record )
{
    list<int> item_ids;
    string::size_type idx = 0;
    int count = 1;

    while ( idx < record.length() )
    {
        switch ( record[idx] )
        {
        case ',':
            count++;
            break;
        case '1':
            item_ids.push_back( count );
            break;
        default:
            break;        // do nothing
        }
        idx++;
    }

    return order_by( item_ids,freq_items_ );
}

// insert items under parent node--!main!
void FPtree::insert_tree( queue<int>& item_ids,FPnode* parent )
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
            // insert at the beginning of sibling list
            mynode->sibling = parent->first_child;
            parent->first_child = mynode;
        }
        // for insert_tree( item_ids,cur )
        cur = mynode;

        // update head table
        list<Head>::iterator pos = head_table_.begin();
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

    // because queue.front() returns a reference of the object,
    // queue.pop recall the destructor of the object,
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
    string record;

    // initialize head table
    for ( list<Item>::iterator pos = freq_items_.begin();
            pos != freq_items_.end();
            pos++ )
    {
        // ascending order by sup
        Item it( *pos );
        head_table_.push_front( it );
    }

#ifdef DEBUG
    cout<<"head table:"<<endl;
    for ( list<Head>::iterator pos = head_table_.begin();
            pos != head_table_.end();
            pos++)
    {
        cout<<pos->item_id<<"  ";
    }
    cout<<endl;
#endif

    // discard the first line
    getline( infile_,record );
    while ( getline( infile_,record ) )
    {
#ifdef DEBUG
        cout<<endl<<"treating record: "<<record<<endl;
#endif
        queue<int> item_ids = refine_items( record );

#ifdef DEBUG
        cout<<"refine frequent items to be inserted:";
        queue<int> tmp_ids = item_ids;
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
void FPtree::project_path( FPnode* p,list<Head>& cheads )
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
            list<Head>::iterator pos = cheads.begin();
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
            list<Head>::iterator pos = cheads.begin();
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
// p is the first node  in node link list
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
void FPtree::prune( list<Head>& cheads )
{
#ifdef DEBUG
    cout<< "prune projected fp-tree.Head table: ";
    for ( list<Head>::iterator pos = cheads.begin();
            pos != cheads.end();
            pos++)
    {
        cout<<pos->item_id<<"("<<pos->sup<<")"<<"  ";
    }
    cout<<endl;
#endif
    cheads.sort();

    for ( list<Head>::iterator pos = cheads.begin();
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
                for ( list<Head>::iterator it = cheads.begin();
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
// p is point of the first node in node link list,cheads³õÊ¼Îª¿Õ
void FPtree::create_condition_fptree( FPnode* p,list<Item>& suffix,list<Head>& cheads )
{
#ifdef DEBUG
    cout<<endl<<"Create condition fp-tree for item:"<< p->item_id << "    ";
    cout<<"suffix: ";
    for ( list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        cout<<pos->item_id<<" ";
    }
    cout<<endl;
#endif

    FPnode*	q=p;
    while ( p != NULL )
    {
        project_path( p,cheads );
        p = p->node_link;
    }

    detach( q );

    prune( cheads );
}

bool FPtree::is_single_path( list<Head>& heads )
{
    for ( list<Head>::iterator pos = heads.begin();
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
void FPtree::generate_rules( list<Head>& heads,list<Item>& suffix )
{
#ifdef DEBUG
    cout<<"rule been generated: ";
    for ( list<Head>::reverse_iterator pos = heads.rbegin();
            pos != heads.rend();
            pos++)
    {
        cout<<pos->item_id<<"("<< pos->sup<<")"<<" ";
    }
    for ( list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        cout<<pos->item_id<<"("<< pos->sup<<")"<<" ";;
    }
    cout<<endl;
#endif

    for ( list<Head>::reverse_iterator pos = heads.rbegin();
            pos != heads.rend();
            pos++)
    {
        out_<<pos->item_id<<"  ";
    }
    for ( list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        out_<<pos->item_id<<" ";
    }
    out_<<endl;
}

// remove condition fp-tree
void FPtree::clean_condition_fptree( list<Head>& heads )
{
#ifdef DEBUG
    cout<<"clean condition fp-tree.Head table: ";
    for ( list<Head>::iterator pos = heads.begin();
            pos != heads.end();
            pos++)
    {
        cout<< pos->item_id<<"("<<pos->sup<<")"<<" ";
    }
    cout<<endl;
#endif
    for ( list<Head>::iterator pos = heads.begin();
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
void FPtree::fp_growth( list<Head>& heads,list<Item>& suffix )
{
#ifdef DEBUG
    cout<<endl<<"fp-growth at suffix: ";
    for ( list<Item>::iterator pos = suffix.begin();
            pos != suffix.end();
            pos++)
    {
        cout<<pos->item_id<<" ";
    }
    cout<<endl;
    cout<<"Head table:";
    for ( list<Head>::iterator pos = heads.begin();
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
        for ( list<Head>::iterator pos = heads.begin();
                pos != heads.end();
                pos++)
        {
            list<Item> con_suffix( suffix );
            Item it( pos->item_id,pos->sup );
            con_suffix.push_front( it );

            list<Head> cheads;  // output parament,condition head table
            create_condition_fptree( pos->node_link,con_suffix,cheads );
            if ( !cheads.empty() )
            {
                fp_growth( cheads,con_suffix );
            }
        }
    }
    clean_condition_fptree( heads );
}

void FPtree::run(const string& input_path, const string& output_path)
{
    // 1. open records file
    infile_.open( input_path.c_str() );
    if ( !infile_ ) return;

    // 2. find frequent items in records database
    find_freq_items();
#ifdef DEBUG
    cout<<freq_items_.size()<<" freqent items:"<<endl;
    for ( list<Item>::iterator pos = freq_items_.begin();
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
    list<Item> suffix;

    // 4.use fp-growth to compute association rules and output to target file

    out_.open( output_path.c_str() );
    fp_growth( head_table_,suffix );
}
