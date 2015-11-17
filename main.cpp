#include "node.h"
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
        forest.insert(rt);
    }

    set<tree*>::iterator tit;
    for(tit = forest.begin(); tit != forest.end(); ++tit)
    {
        // find ready tree
        if( (*tit)->pres.size() == 0) 
        {
            if((*tit)->done != 1)
                (*tit)->dispatch();
            printf("main dispatch\n");
        }
    }

    return 0; 
}

