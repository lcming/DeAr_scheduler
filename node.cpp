
#include "node.h"
extern sns root;
extern sns leaf;
extern sns cut;
extern int rcnt;
extern int tcnt;

super_cas::super_cas(super_node* _sc)
{
    sc = _sc;
    child.first = child.second = NULL;
}

node::node(int _id, int _op)
{
    id = _id; 
    assert(_op >= ADD && _op <= SHI);
    op = _op;
    wrap = NULL;
    pres.first = pres.second = NULL;
    rd = ops = rs.first = rs.second = -1;
}

void connect(node* src, node* dst)
{
    src->sucs.insert(dst);

    //===================================
    // To avoid reversing to operands, 
    // we must know which pre comes first
    //===================================
    if(dst->pres.first == NULL) // not used yet
        dst->pres.first = src;
    else if(dst->pres.second == NULL)
    {
        dst->pres.second = src;
        assert(dst->pres.first != dst->pres.second);
    }
    else
    {
        printf("fatal: more than 2 pres\n");
        exit(1);
    }
}
void connect(super_node* src, super_node* dst)
{
    sns::iterator it;
    for(it = dst->pres.begin(); it != dst->pres.end(); ++it) // search all sibs
    {
        src->sibs.insert(*it);
        (*it)->sibs.insert(src);
    }
    src->sucs.insert(dst);
    dst->pres.insert(src);
}

super_node::super_node()
{
    ss = -1;
    done = 0;
    dest = -1;
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
        root.erase(target);
        root.insert(this);
    }
    delete target;
}
super_node::~super_node()
{

}
int analyze_stack(super_node* target)
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

void schedule(super_node* target)
{
    assert(target->pres.size() == 2 || target->pres.size() == 0);
    if(target->pres.size() == 2)
    {
        sns::iterator it = target->pres.begin();
        super_node* left = *it;
        super_node* right = *(++it);

        // set rs & rd for each op
        // data only flow in SIU in this case
        if(left->ss > right->ss)  // schedule larger stack first
        {
            left->cas.front()->ops = PUSH; // push
            target->cas.back()->rs.first = PUSH;
            schedule(left);

            right->cas.front()->ops = POP; // pop
            target->cas.back()->rs.second = right->cas.front()->op;
            schedule(right); 
        }
        else
        {
            right->cas.front()->ops = PUSH; // push
            target->cas.back()->rs.second = PUSH;
            schedule(right);

            left->cas.front()->ops = POP; // pop
            target->cas.back()->rs.first = left->cas.front()->op;
            schedule(left); 
        }
        // todo: set rn, rs for target
    }
    else if (target->pres.size() == 0)
    {
        // a leaf node
        assert(target->sucs.size() <= 1);
        // we simplify register allocation, so assume rs are already determinded

    }

    // schedule operations
    char* op_name;
    for(int i = target->cas.size()-1; i >= 0; i--)
    {
        node* n = target->cas[i];
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
        if(n->cuts.size() > 0) // cut node must write back temp result
        {
            n->rd = tcnt++;
            printf("schedule %d: %s, %%t%d, %%%d, %%%d", n->id, op_name, n->rd, n->rs.first, n->rs.second);
        }
        else if(n->sucs.size() == 0) // final result
        {
            n->rd = rcnt++;
            printf("schedule %d: %s, %%r%d, %%%d, %%%d", n->id, op_name, n->rd, n->rs.first, n->rs.second);
        }
        else
            printf("schedule %d: %s, %%%d, %%%d, %%%d", n->id, op_name, n->rd, n->rs.first, n->rs.second);
        if(n->ops == PUSH)
            printf(", PUSH");
        else if(n->ops == POP)
            printf(", POP");
        printf("\n");

        // update the rs of the following node
        if(i > 0)
        {
            node* next = target->cas[i-1];
            if(n == next->pres.first)
                next->rs.first = n->op;
            else if(n == next->pres.second)
                next->rs.second = n->op;
            else
            {
                printf("fatal: scheduler doesn't match pre\n");
                exit(1);
            }
        }
            
        
    }
}


