
#include "node.h"
extern sns result;
extern sns leaf;
extern set<tree*> forest;
extern int rcnt;
extern node* start;
extern node* previous;
extern set<ptree> restore_tree;


tree::tree(super_node* _root, int _wb)
{
    root = _root;
    wb  = _wb;
    done = 0;
    early = NULL;
    root->t = this; 
}

tree* build_tree(super_node* sn)
{
    tree* t_new = new tree(sn, 1);
    forest.insert(t_new);
    printf("tree: tree %X with root %s\n", t_new, sn->id.c_str());
    if(sn->pres.size() == 2)
    {
        super_node* left = *(sn->pres.begin());
        super_node* right = *(++sn->pres.begin());
        if
        (
            left->sucs.size() == 1 && 
            right->sucs.size() == 1 &&
            !left->t  && 
            !right->t
        )
        {
            t_new->extend(left);
            t_new->extend(right);
        }
        else
        {
            tree* t_left = left->t? left->t : build_tree(left);
            tree* t_right = right->t? right->t : build_tree(right);
            t_left->connect(t_new);
            t_right->connect(t_new);
            left->cut(sn);
            right->cut(sn);
        }
    }
    else if(sn->pres.size() == 1)
    {
        super_node* left = *(sn->pres.begin());
        tree* t_left = left->t? left->t : build_tree(left);
        t_left->connect(t_new);
        left->cut(sn);
    }
    
    return t_new;
}

void tree::extend(super_node* sn)
{
    sn->t = this;
    printf("tree: on %s\n", sn->id.c_str());
    if(sn->pres.size() == 2)
    {
        super_node* left = *(sn->pres.begin());
        super_node* right = *(++sn->pres.begin());
        if
        (
            left->sucs.size() == 1 && 
            right->sucs.size() == 1 &&
            !left->t && 
            !right->t
        )
        {
            this->extend(left);
            this->extend(right);
        }
        else
        {
            tree* t_left = left->t? left->t : build_tree(left);
            tree* t_right = right->t? right->t : build_tree(right);
            t_left->connect(this);
            t_right->connect(this);
            left->cut(sn);
            right->cut(sn);
        }
    }
    else if(sn->pres.size() == 1)
    {
        super_node* left = *(sn->pres.begin());
        tree* t_left = left->t? left->t : build_tree(left);
        t_left->connect(this);
        left->cut(sn);
    }
 
}

void tree::connect(tree* target)
{
    this->sucs.insert(target);
    target->pres.insert(this);
}

vector<tree*> tree::initialize()
{
    vector<tree*> t_ready;
    node* result = root->cas[0];

    // final result

    set<tree*>::iterator it, it2;
    for(it = sucs.begin(); it != sucs.end(); ++it)
    {
        if( (*it)->pres.size() == 1)
        {
            t_ready.push_back(*it);
        }
    }

    // some tree need this tree wb
    if(sucs.size() != t_ready.size())
    {
        result->rd = allocate();
        result->update_reg();
    }

    if(t_ready.size() != 0)
    {
        result->ops = result->ops == POP?
            WRITE : PUSH;
        // fix rs for each ready tree 
        for(int i = 0; i < t_ready.size(); i++)
        {
            if(t_ready[i]->done == 0)
                result->push(t_ready[i]);
        }
        // the last ready suc have to pop the stack
        t_ready.back()->root->cas.back()->ops = t_ready.back()->root->cas[0]->ops == PUSH ?
            WRITE : POP;
    }

    /*
    // cut the dependency of all sucs
    for(it = sucs.begin(); it != sucs.end(); ++it)
    {
        it2 = (*it)->pres.find(this);
        //(*it)->pres.erase(it2);

        // backup, so we can restore the scheduling
        ptree backup;
        backup.first = *it;
        backup.second = *it2;
        restore_tree.insert(backup);

    }
    */

    return t_ready;
}

void tree::early_schedule(vector<node*>& ret)
{
    if(!early || early == root)
        return;
    assert(early->sucs.size() == 1);
    super_node* par = *(early->sucs.begin());
    super_node* left = *(par->pres.begin());
    super_node* right = *(++par->pres.begin());
    super_node* sib;

    // find sib
    if( left == early )
        sib = right;
    else if( right == early )
        sib = left;
    else
    {
        printf("fatal: handle early schedule not match\n");
        exit(1);
    }

    early->cas[0]->ops = early->cas[0]->ops == POP?
        WRITE: PUSH;
    sib->cas[0]->ops = sib->cas[0]->ops == PUSH?
        WRITE: POP;

    if(par->cas.back()->pres.first == early->cas[0])
    {
        par->cas.back()->rs.first = STACK;
        par->cas.back()->rs.second = sib->cas[0]->op;
    }

    else if(par->cas.back()->pres.first == sib->cas[0])
    {
        par->cas.back()->rs.second = STACK;
        par->cas.back()->rs.first = sib->cas[0]->op;
    }
    else
    {
        printf("fatal: handle early schedule not match\n");
        exit(1);
    }
    early->schedule(ret);
}

vector<node*> tree::dispatch()
{
    vector<node*> ret;
    // main part of scheduling
    analyze_stack(root);
    vector<tree*> t_ready = initialize();
    early_schedule(ret);
    root->rec_schedule(ret);
    done = 1;
    for(int i = 0; i < t_ready.size(); i++)
    {
        vector<node*> _ret = t_ready[i]->dispatch();
        ret.insert(ret.end(), _ret.begin(), _ret.end());
    }
    if(sucs.size() == 0)
        printf("RESULT\n");
    return ret;
}


node::node(int _id, int _op)
{
    id = _id; 
    done = 0;
    assert(_op >= ADD && _op <= SHI);
    op = _op;
    ops = NOP;
    wrap = NULL;
    next = NULL;
    pres.first = pres.second = NULL;
    rd = rs.first = rs.second = -1;
}

void node::connect(node* dst)
{
    sucs.insert(dst);

    //===================================
    // To avoid reversing to operands, 
    // we must know which pre comes first
    //===================================
    if(!dst->pres.first) // not used yet
        dst->pres.first = this;
    else if(!dst->pres.second)
    {
        dst->pres.second = this;
        assert(dst->pres.first != dst->pres.second);
    }
    else
    {
        printf("fatal: more than 2 pres\n");
        exit(1);
    }
}
void super_node::connect(super_node* dst)
{
    sucs.insert(dst);
    dst->pres.insert(this);
}

super_node::super_node()
{
    t = NULL;
    ss = -1;
    done = 0;
    //ops = NOP;
    assert(cas.size() == 0);
}

void super_node::add_node(node* n)
{
    cas.push_back(n);
    n->wrap = this;
    id += to_string(n->id);
    id += " ";
    printf("%d ", n->id);
}

super_node::~super_node()
{

}

int super_node::get_level()
{
    int ret = 0; 
    super_node* sn = this;
    while(1)
    {
        assert(sn->sucs.size() <= 1);
        if(sn->sucs.size() == 0)   
            break;
        sn = *sn->sucs.begin();
        ret ++;
    }
    return ret;
}
    

int tree::analyze_stack(super_node* target)
{
    sns::iterator it;
    if(target->ss != -1)  // already found its stack size
        return target->ss;
    int psize = target->pres.size();
    if(psize == 0) // end of super node
    {
        target->ss = 0;
    }
    else if(psize == 2)
    {
        // consider left or right is share node
        // also consider both of them are share nodes
        // we have to access 2 queue simultaneously
        // maybe always schedule share node first is better
        it = target->pres.begin();
        int left = analyze_stack(*it);
        int right = analyze_stack(*(++it));
        target->ss = (left > right)? right + 1 : left + 1;
    }
    else 
    {
        printf("fatal: [ %s] error psize %d\n", target->id.c_str(), psize);
        exit(1);
    }
    printf("stack: size of [ %s] is %d\n", target->id.c_str(), target->ss);
    return target->ss;
}

void super_node::cut(super_node* target)
{
    sns::iterator it;
    // record data dependency of the cut node
    (*(cas.begin()))->cuts.insert(*(target->cas.end()-1));
    printf("%d cuts %d\n", (*(cas.begin()))->id, (*(target->cas.end()-1))->id);

    sucs.erase(target);
    target->pres.erase(this);
}

void super_node::schedule(vector<node*>& ret)
{
    if(this->done)
        return;
    assert(this->t);

    // schedule operations
    for(int i = this->cas.size()-1; i >= 0; i--)
    {
        node* n = this->cas[i];

        // marked as the first node or record the next node
        if(!start)
            start = n;
        else
            previous->next = n;

        // allocate rs
        if(!n->pres.first) // leaf
        {
            n->rs.first = allocate();
            n->rs.second = allocate();
        }
        else if(!n->pres.second)
        {
            n->rs.second = allocate(); 
        }

        // the node W/O sucs is the final result
        if(n->sucs.size() == 0)
        {
            n->rd = allocate(); 
        }
            
        // update the rs of the following node
        if(i > 0)
        {
            node* next = this->cas[i-1];
            if(next->pres.first == n)
                next->rs.first = n->op;
            else if(next->pres.second == n)
                next->rs.second = n->op;
            else
            {
                printf("fatal: node not match in itra tree bypass\n");
                exit(1);
            }
        }
        if(!n->done)
            ret.push_back(n);

        previous = n;
    }
    this->done = 1;

}
void super_node::inv_rec_schedule(vector<node*>& ret)
{
    assert(this->pres.size() == 2 || this->pres.size() == 0);
    if (this->pres.size() == 2)
    {
        super_node* left = *pres.begin(); 
        super_node* right = *pres.rbegin(); 
        right->inv_rec_schedule(ret);
        left->inv_rec_schedule(ret);
    }
    this->schedule(ret);
}

void super_node::rec_schedule(vector<node*>& ret)
{
    if(this->done == 1)
        return;
    assert(this->pres.size() == 2 || this->pres.size() == 0);
    if(this->pres.size() == 2)
    {
        sns::iterator it = this->pres.begin();
        super_node* left = *it;
        super_node* right = *(++it);
        
        // set rs & rd for each op
        // data only flow in SIU in this case
        if(left == this->t->early)
        {
            right->rec_schedule(ret);
        }
        else if(right == this->t->early)
        {
            left->rec_schedule(ret);
        }
        else if(left->ss > right->ss)  // rec_schedule() larger stack first
        {
            left->cas.front()->ops = left->cas.front()->ops == POP ?
                WRITE : PUSH; // push
            this->cas.back()->rs.first = STACK;
            left->rec_schedule(ret);

            right->cas.front()->ops = right->cas.front()->ops == PUSH?
                WRITE : POP; // pop
            this->cas.back()->rs.second = right->cas.front()->op;
            right->rec_schedule(ret); 
        }
        else
        {
            right->cas.front()->ops = right->cas.front()->ops == POP ?
                WRITE : PUSH; // push
            this->cas.back()->rs.second = STACK;
            right->rec_schedule(ret);

            left->cas.front()->ops = left->cas.front()->ops == PUSH?
                WRITE : POP; // pop
            this->cas.back()->rs.first = left->cas.front()->op;
            left->rec_schedule(ret); 
        }
    }
    else if (this->pres.size() == 0)
    {
        assert(this->sucs.size() <= 1);
    }
    this->schedule(ret);
}

void node::update_reg()
{
    //assert(sucs.size() > 1);
    set<node*>::iterator it;
    for(it = sucs.begin(); it != sucs.end(); ++it)
    {
        if( (*it)->pres.first == this )
            (*it)->rs.first = rd;
        else if( (*it)->pres.second == this )
            (*it)->rs.second = rd;
        else
        {
            printf("fatal: not match when update_reg\n");
            exit(1);
        }
    }
}


void node::push(tree* get)
{
    set<node*>::iterator it;
    for(it = sucs.begin(); it != sucs.end(); ++it)
    {
        // match push condition
        if( (*it)->wrap->t == get ) 
        {
            if( (*it)->pres.first == this )
                (*it)->rs.first = STACK;
            else if( (*it)->pres.second == this)
                (*it)->rs.second = STACK;
            else
            {
                printf("fatal: not match when push\n");
                exit(1);
            }
            get->early = (*it)->wrap;
            break;
        }
    }
}

int allocate()
{
    int ret = rcnt;
    rcnt++;
    if(rcnt == RF_SIZE) 
    {
        printf("Ping Pong: refresh\n");
        rcnt = 0; 
    }
    return ret;
}


void node::reset()
{
    node* n = this;
    super_node* reset_sn = n->wrap;
    reset_sn->done = 0;
    reset_sn->ss = -1;

    tree* reset_tree = reset_sn->t;
    reset_tree->done = 0;

    n->ops = NOP;
    n->rd = n->rs.first = n->rs.second = -1;
}

void node::process(int show)
{
    node* n = this;
    string op_name, rd, rs1, rs2, ops;
    // set op name
    switch(n->op)
    {
        case ADD:
            op_name = "add"; break;
        case MUL:
            op_name = "mul"; break;
        case SHI:
            op_name = "shi"; break;
        default:
            printf("fatal: no such type of op code %d\n", n->op);
            exit(1);
    }
    // set rd, rs
    if(n->rd == -1)
        rd = "(no WB)";

    switch(n->rs.first)
    {
        case ADD:   rs1 = "(ADD)";    break;
        case MUL:   rs1 = "(MUL)";    break;
        case SHI:   rs1 = "(SHI)";    break;
        case STACK: rs1 = "(STACK)";  break;
    }
    switch(n->rs.second)
    {
        case ADD:   rs2 = "(ADD)";    break;
        case MUL:   rs2 = "(MUL)";    break;
        case SHI:   rs2 = "(SHI)";    break;
        case STACK: rs2 = "(STACK)";  break;
    }   

    switch(n->ops)
    {
        case PUSH: 
            ops = "PUSH";
            break;
        case POP: 
            ops = "POP";
            break;
        case WRITE:
            ops = "WRITE";
            break;
        case NOP: 
            ops = "NOP";
            break;
        default:
            printf(", undefined stack operation\n");
            exit(1);
    } 

    if(show)
            printf("schedule %*d: %s, %%%*d%s, %%%*d%s, %%%*d%s, %s\n", 3, n->id, op_name.c_str(), 2, n->rd, rd.c_str(), 2, n->rs.first, rs1.c_str(), 2, n->rs.second, rs2.c_str(), ops.c_str());
}


