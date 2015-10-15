#include "node.h"
#include <fstream>
#include <vector>
#include <stdio.h>
#include <set>

using namespace std;


int gid = 0;
int gcnt = 1;
sns leaf; // global vec to store leaf super node
sns root; // global vec to store leaf super node
sns cut;
vector<node*> init_dfg(char* filename);
void init_super_dfg(vector<node*> list);
super_node* build_super(node* target);


int main(int argc, char* argv[])
{
    sns::iterator it;
    vector<node*> list = init_dfg(argv[1]);
    init_super_dfg(list);
    printf("regress: # of leaf = %d, root = %d\n", leaf.size(), root.size());

    //vector<super_node*> test_leaf = get_partial_leaf(root);
    for(it = root.begin(); it != root.end(); ++it)
        (*it)->dbg();

    for(it = cut.begin(); it != cut.end(); ++it)
    {
        (*it)->cut();
    }

    for(it = root.begin(); it != root.end(); ++it)
    {
        analyze_stack(*it);
    }

    for(it = root.begin(); it != root.end(); ++it)
    {
        schedule(*it);
    }


    return 0; 
}

super_node* build_super(node* target)
{
    printf("regress: build cas:");
    super_node* super = new super_node;
    super->add_node(target);

    // search until encounter a node with non-one pre, which implies the end of the super node
    while(target->pres.size() == 1 && (*target->pres.begin())->sucs.size() == 1)
    {
        target = *target->pres.begin();
        super->add_node(target);
    }
    printf("\n");

    int psize = target->pres.size();
    // more than one pre: growing new super nodes
    if(psize == 0)
    {
        super->level = 0;
        leaf.insert(super);
    }
    else
    {
        int level = 0;
        nds::iterator it;
        for(it = target->pres.begin(); it != target->pres.end(); ++it)
        {
            super_node* super_pre;
            if((*it)->wrap == NULL)  // node not create yet, build
                super_pre = build_super((*it));
            else
            {
                super_pre = (*it)->wrap;
                cut.insert(super_pre);
            }
            // connect
            printf("regress: super [ %s] connect to [ %s]\n", super_pre->id.c_str(), super->id.c_str());
            connect((*it)->wrap, super);
            if(super_pre->level > level)
                level = super_pre->level;
        }
        super->level = level + 1;

    }
    printf("[ %s] level = %d\n", super->id.c_str(), super->level);
    return super;
}

void init_super_dfg(vector<node*> list)
{
    // find all end nodes as terminate node's pre
    for(int i = 0; i < list.size(); i++)
    {
        if(list[i]->sucs.size() == 0)  // an end node
        {
            root.insert(build_super(list[i]));
        }
    }
}

vector<node*> init_dfg(char* filename)
{
    ifstream fin(filename);
    int total, id, op_id;
    string op;
    fin >> total;

    // initialize
    vector<node*> list;
    list.resize(total);
    for(int i = 0; i < total; i++)
    {
        fin >> id >> op;
        if(op == "ADD" || op == "add" || op == "SUB" || op == "sub")
            op_id = ADD;
        else if(op == "MUL" || op == "mul")
            op_id = MUL;
        else if(op == "shl" || op == "ashr" || op == "shr" || op == "SHI")
            op_id = SHI;
        else
        {
            printf("fatal: unknow op\n");
            op_id = -1; // unknown op
        }
        //op_id += 4; // important: to make rf allocation more intuitive
        list[id] = new node(id, op_id);
    }

    // interconnect
    int src, dst;
    while(fin >> src >> dst)
        connect(list[src], list[dst]);


    return list;
}

