
#include "node.h"
extern sns root;
extern sns leaf;
extern sns cut;
extern int gcnt;

node::node(int _id, int _op)
{
    id = _id; 
    assert(_op >= ADD && _op <= SHI);
    op = _op;
    wrap = NULL;
    rd = ops = rs.first = rs.second = -1;
}

void node::dbg()
{
    /*
    printf("id:%d, op:%d", id, op);
    printf("\n");
    printf("pres:");
    for(int i = 0; i < pres.size(); i++)
        printf("%d, ", pres[i]->id);
    printf("\n");

    printf("sucs:");
    for(int i = 0; i < sucs.size(); i++)
        printf("%d, ", sucs[i]->id);
    printf("\n");
    */
}

void connect(node* src, node* dst)
{
    src->sucs.insert(dst);
    dst->pres.insert(src);
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

void super_node::dbg()
{
    /*
    printf("regress: id:[ %s]\n", id.c_str());
    printf("regress: pres:");
    for(int i = 0; i < pres.size(); i++)
        printf("[ %s]  ", pres[i]->id.c_str());
    printf("\n");

    printf("regress: sucs:");
    for(int i = 0; i < sucs.size(); i++)
        printf("[ %s]  ", sucs[i]->id.c_str());
    printf("\n");

    for(int i = 0; i < pres.size(); i++)
        pres[i]->dbg();
    */
}

/*
void update_stack(super_node* target) // fwd update
{
    if(target->pres.size() == 2)
    {
        printf("regress: update ss of [ %s] from %d", target->id.c_str(), target->ss);
        int left = target->pres[0]->ss;
        int right = target->pres[1]->ss;
        target->ss = (left > right)? right + 1 : left + 1;
        printf(" to %d\n", target->ss);
    }
    // fwd update stack
    for(int i = 0; i < target->sucs.size(); i++)
        update_stack(target->sucs[i]);
}
*/

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
    printf("regress: stack of [ %s] is %d\n", target->id.c_str(), target->ss);
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
            target->cas.back()->rs.first = right->cas.front()->op;
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
                op_name = "add"; break;
            case SHI:
                op_name = "add"; break;
            default:
                printf("fatal: no such type of op code %d\n", n->op);
                exit(1);
        }
        if(n->cuts.size() > 0)
            n->rd = gcnt++;
        printf("schedule %d:%s, %%%d, %%%d, %%%d, (%d)", n->id, op_name, n->rd, n->rs.first, n->rs.second, n->ops);
        if(n->ops == PUSH)
            printf(", PUSH");
        else if(n->ops == POP)
            printf(", POP");
        printf("\n");
    }
    /*
    // trigger sibs merging
    int ssize = target->sibs.size();
    if(ssize > 1)
    {
        for(int i = 0; i < ssize; i++) 
        {
            super_node* merger = target->sibs[i];
            if(merger->sucs.size() == 1 && !merger->done && !merger->sucs[0]->done) // sibling exist and it can be merged
                merger->merge();
        }
    }
    */
}


