
#include "node.h"
extern sns result;
extern sns leaf;
extern sns cut;
extern set<tree*> forest;
extern int rcnt;



tree::tree(super_node* _root, int _wb)
{
    root = _root;
    wb  = _wb;
    done = 0;
    early = NULL;
    if(root->pres.size() == 2)
    {
        super_node* n_left = *(root->pres.begin());
        super_node* n_right = *(++root->pres.begin());
        prev = root;
        grow(n_left);
        grow(n_right);
    }
    else if(root->pres.size() == 1)
    {
        prev = root;
        super_node* n_left = *(root->pres.begin());
        grow(n_left);
    }
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
        t_ready.back()->root->cas[0]->ops = t_ready.back()->root->cas[0]->ops == PUSH ?
            WRITE : POP;
    }

    // cut the dependency of all sucs
    for(it = sucs.begin(); it != sucs.end(); ++it)
    {
        it2 = (*it)->pres.find(this);
        (*it)->pres.erase(it2);
    }

    return t_ready;
}

void tree::early_schedule()
{
    if(early == NULL || early == root)
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
    early->schedule();
}

void tree::dispatch()
{

    // main part of scheduling
    analyze_stack(root);
    vector<tree*> t_ready = initialize();
    early_schedule();
    root->rec_schedule();
    done = 1;
    for(int i = 0; i < t_ready.size(); i++)
    {
        t_ready[i]->dispatch();
    }
    if(sucs.size() == 0)
        printf("RESULT\n");
}

void tree::grow(super_node* sn)
{
    printf("tree: on %s\n", sn->id.c_str());
    if(sn->sucs.size() > 1 && sn->t == NULL)   // build a new tree
    {
        // create new tree and pass
        tree* t_new = new tree(sn, 0);
        sn->t = t_new;
        forest.insert(t_new);
        pres.insert(t_new);
        t_new->sucs.insert(this);
        sn->cut(prev);
        printf("tree: tree %X with root %s\n", t_new, sn->id.c_str());
        printf("tree: pres = %d, sucs = %d\n", t_new->pres.size(), t_new->sucs.size());
    }
    else if(sn->t != NULL) // connect tree
    {
        printf("tree: already a built tree\n");
        pres.insert(sn->t);
        sn->t->sucs.insert(this);
        sn->cut(prev);
    }
    else
    {
        super_node* par = *(sn->sucs.begin());
        super_node* sib = *par->pres.begin() == sn ? 
            *(++par->pres.begin()) : *par->pres.begin();
        if(sib->t == NULL)
        {
            if(sib->sucs.size() > 1)        
            {
                tree* t_new = new tree(sn, 0);
                sn->t = t_new;
                forest.insert(t_new);
                pres.insert(t_new);
                t_new->sucs.insert(this);
                sn->cut(prev);
                printf("tree: tree %X with root %s\n", t_new, sn->id.c_str());
                printf("tree: pres = %d, sucs = %d\n", t_new->pres.size(), t_new->sucs.size());
            }
            else
            {
                sn->t = this; 
                if(sn->pres.size() == 2)
                {
                    super_node* n_left = *(sn->pres.begin());
                    super_node* n_right = *(++sn->pres.begin());
                    prev = sn;
                    grow(n_left);
                    grow(n_right);
                }
            }
        }
        else
        {
            if(sib->t != this)
            {
                tree* t_new = new tree(sn, 0);
                sn->t = t_new;
                forest.insert(t_new);
                pres.insert(t_new);
                t_new->sucs.insert(this);
                sn->cut(prev);
                printf("tree: tree %X with root %s\n", t_new, sn->id.c_str());
                printf("tree: pres = %d, sucs = %d\n", t_new->pres.size(), t_new->sucs.size());
            }
            else
            {
                sn->t = this; 
                if(sn->pres.size() == 2)
                {
                    super_node* n_left = *(sn->pres.begin());
                    super_node* n_right = *(++sn->pres.begin());
                    prev = sn;
                    grow(n_left);
                    grow(n_right);
                }
            }

        }
    }
}

node::node(int _id, int _op)
{
    id = _id; 
    assert(_op >= ADD && _op <= SHI);
    op = _op;
    ops = NOP;
    wrap = NULL;
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
    if(dst->pres.first == NULL) // not used yet
        dst->pres.first = this;
    else if(dst->pres.second == NULL)
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
    dest = -1;
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

void super_node::merge()
{
    assert(sucs.size() == 1);
    sns::iterator it;
    super_node* target = *(sucs.begin());
    if(target->pres.size() > 1) // cannot merge
        return;
    if(t != NULL)
        return;

    printf("merge:[ %s] [ %s]\n", id.c_str(), target->id.c_str());
    sucs.clear();

    // iterate all sucs of mergee
    for(it = target->sucs.begin(); it != target->sucs.end(); ++it)
    {
        // inherit sucs of mergee
        sucs.insert(*it);
        // replace the mergee by the merger
        (*it)->pres.erase(target); 
        (*it)->pres.insert(this);
    }
    // concat cas
    for(int i = 0; i < cas.size(); i++)
    {
        target->cas.push_back(cas[i]);
    }
    cas = target->cas;
    // concat id
    id = target->id + id;
    if(target->sucs.size() == 0) // a root to be merged
    {
        result.erase(target);
        result.insert(this);
    }
    delete target;
}
super_node::~super_node()
{

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

    super_node* merger = *(target->pres.begin());
    if(target->pres.size() == 1 && merger->sucs.size() == 1)
        merger->merge();
}

void super_node::cut() // split cut node
{
    sns::iterator it;
    int maxlv = 0;
    int id = -1;
    super_node* mergee;

    // find the max level
    for(it = sucs.begin(); it != sucs.end(); ++it)
    {
        if( (*it)->level > maxlv || ( (*it)->level == maxlv && (*it)->cas[0]->id > id) ) 
        {
            maxlv = (*it)->level;
            mergee = *it;
            id = mergee->cas[0]->id;
        }
    }

    for(it = sucs.begin(); it != sucs.end(); ++it)
    {
        if(*it != mergee)  
            cut(*it); // specify target to cut
    }
    merge();
}

void super_node::schedule()
{
    assert(this->t != NULL);
    this->done = 1;
    // schedule operations
    char* op_name;
    for(int i = this->cas.size()-1; i >= 0; i--)
    {
        node* n = this->cas[i];
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

        // allocate rs
        if(n->pres.first == NULL) // leaf
        {
            n->rs.first = allocate();
            n->rs.second = allocate();
        }
        else if(n->pres.second == NULL)
        {
            n->rs.second = allocate(); 
        }

        // string that increase readability of assembly
        string rd, rs1, rs2;
#ifdef READABLE
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
#endif


        printf("schedule %*d: %s, %%%*d%s, %%%*d%s, %%%*d%s", 3, n->id, op_name, 2, n->rd, rd.c_str(), 2, n->rs.first, rs1.c_str(), 2, n->rs.second, rs2.c_str());

        switch(n->ops)
        {
            case PUSH: 
                printf(", PUSH");
                break;
            case POP: 
                printf(", POP");
                break;
            case WRITE: 
                printf(", WRITE");
                break;
            case NOP: 
                printf(", NOP");
                break;
            default:
                printf(", undefined stack operation\n");
                exit(1);
        } 
        printf("\n");

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
    }

}

void super_node::rec_schedule()
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
            right->rec_schedule();
        }
        else if(right == this->t->early)
        {
            left->rec_schedule();
        }
        else if(left->ss > right->ss)  // rec_schedule() larger stack first
        {
            left->cas.front()->ops = left->cas.front()->ops == POP ?
                WRITE : PUSH; // push
            this->cas.back()->rs.first = STACK;
            left->rec_schedule();

            right->cas.front()->ops = right->cas.front()->ops == PUSH?
                WRITE : POP; // pop
            this->cas.back()->rs.second = right->cas.front()->op;
            right->rec_schedule(); 
        }
        else
        {
            right->cas.front()->ops = right->cas.front()->ops == POP ?
                WRITE : PUSH; // push
            this->cas.back()->rs.second = STACK;
            right->rec_schedule();

            left->cas.front()->ops = left->cas.front()->ops == PUSH?
                WRITE : POP; // pop
            this->cas.back()->rs.first = left->cas.front()->op;
            left->rec_schedule(); 
        }
    }
    else if (this->pres.size() == 0)
    {
        assert(this->sucs.size() <= 1);
    }
    this->schedule();
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
