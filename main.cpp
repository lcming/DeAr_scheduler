#include "node.h"
#include <fstream>
#include <vector>
#include <stdio.h>
#include <set>

using namespace std;


int gid = 0;
int rcnt = 0; // cnt for temp result
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
    
    printf("overview: # of leaf = %d, root = %d\n", leaf.size(), root.size());
    //vector<super_node*> test_leaf = get_partial_leaf(root);

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
    while(target->pres.first != NULL && target->pres.second == NULL && target->pres.first->sucs.size() == 1)
    {
        target = target->pres.first;
        super->add_node(target);
    }
    printf("\n");

    // a leaf node w/o pre
    if(target->pres.first == NULL)
    {
        super->level = 0;
        leaf.insert(super);
        return super;
    }

    if(target->pres.first != NULL) // got a pre to grow new sn
    {
        int level = 0;

        super_node* super_pre;
        if(target->pres.first->wrap == NULL)  // node not create yet, build
            super_pre = build_super(target->pres.first);
        else
        {
            super_pre = target->pres.first->wrap;
            cut.insert(super_pre);
        }
        // connect
        printf("regress: super [ %s] connect to [ %s]\n", super_pre->id.c_str(), super->id.c_str());
        connect(target->pres.first->wrap, super);
        if(super_pre->level > level)
            level = super_pre->level;

        super->level = level + 1;
    }

    if(target->pres.second != NULL) // got a pre to grow new sn
    {
        int level = 0;

        super_node* super_pre;
        if(target->pres.second->wrap == NULL)  // node not create yet, build
            super_pre = build_super(target->pres.second);
        else
        {
            super_pre = target->pres.second->wrap;
            cut.insert(super_pre);
        }
        // connect
        printf("regress: super [ %s] connect to [ %s]\n", super_pre->id.c_str(), super->id.c_str());
        connect(target->pres.second->wrap, super);
        if(super_pre->level > level)
            level = super_pre->level;

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

