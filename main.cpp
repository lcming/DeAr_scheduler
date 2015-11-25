#include "node.h"
#include "thread.h"
#include "dfg.h"
#include <fstream>
#include <vector>
#include <stdio.h>
#include <set>

using namespace std;


int gid = 0;
int rcnt = 0; // cnt for temp result
sns leaf; // global vec to store leaf super node
sns result; // global vec to store leaf super node
set<tree*> forest;
set<node*> nodes;
set<super_node*> super_nodes;
node* start = NULL;
node* previous = NULL;
set<ptree> restore_tree;



int main(int argc, char* argv[])
{
    sns::iterator it;
    vector<node*> list = init_dfg(argv[1]);
    init_super_dfg(list);
    
    printf("overview: # of leaf = %d, result = %d\n", leaf.size(), result.size());

    for( auto &it : result)
    {
        //tree* rt = new tree(*it, 1);
        tree* rt = build_tree(it);
        it->t = rt;
        printf("tree: tree pres = %d, sucs = %d\n", rt->pres.size(), rt->sucs.size());
    }


/*
    for(int i = 0; i < 10; i++)
    {
        test_single_thread(0);
        reset_dfg();
    }
    test_single_thread(1);
*/

    vector<tree*> vforest;

    // insert all free trees
    for( auto &it : forest)
    {
        if(it->pres.size() == 0)
            vforest.push_back(it);
    }

    printf("vforest size = %d\n", vforest.size());

    thread* t0 = new thread(0, vforest);
    thread* t1 = new thread(1, vforest);

    super_node* remain = inter_tree_schedule(t0, t1, vforest);
    



    return 0; 
}

