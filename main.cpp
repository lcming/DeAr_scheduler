#include "node.h"
#include "arbiter.h"
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

    for(it = result.begin(); it != result.end(); ++it)
    {
        //tree* rt = new tree(*it, 1);
        tree* rt = build_tree(*it);
        (*it)->t = rt;
        printf("tree: tree pres = %d, sucs = %d\n", rt->pres.size(), rt->sucs.size());
    }
/*
    for(int i = 0; i < 10; i++)
    {
        test_single_thread(0);
        reset_dfg();
    }
    test_single_thread(1);
    reset_dfg();
    */
    set<tree*>::iterator tit;
    set<run*> run_pool;
    for(tit = forest.begin(); tit != forest.end(); ++tit)
    {
        if( (*tit)->pres.size() == 0 && (*tit)->done != 1) 
        {
            run* input = new run(*tit);
            run_pool.insert(input);
        }
        reset_dfg();
    }

    set<run*>::iterator rit;
    for(rit = run_pool.begin(); rit != run_pool.end(); ++rit)
    {
        printf("run: ");
        (*rit)->dispatch_ready(1);
    }





    return 0; 
}

