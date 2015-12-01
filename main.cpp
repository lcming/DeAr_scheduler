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

    thread* t0 = new thread(0);
    thread* t1 = new thread(1);
    int run = 0;
    int* hit = (int*)calloc(sizeof(int), nodes.size());

    // insert all free trees
    while(1)
    {
        vector<tree*> vforest;
        for( auto &it : forest)
        {
            if(it->pres.size() == 0 && !it->done)
                vforest.push_back(it);
        }

        printf("vforest size = %d\n", vforest.size());
        if(vforest.size() == 0)
            break;

        // bind forest
        t0->forest = &vforest;
        t1->forest = &vforest;



        tree* remain_tree = inter_tree_schedule(t0, t1, vforest);
            

        int reverse_head = t0->cyc.end() - t0->cyc.begin();
        intra_tree_schedule(t0, t1, remain_tree->root);
        int reverse_tail = t0->cyc.end() - t0->cyc.begin();

        remain_tree->done = 1;
        // update from intra schedule
        for( auto &t : remain_tree->sucs)
        {
            set<tree*>::iterator it = t->pres.find(remain_tree);
            t->pres.erase(it);
        }


        reverse(t0->cyc.begin()+reverse_head, t0->cyc.begin()+reverse_tail);
        reverse(t1->cyc.begin()+reverse_head, t1->cyc.begin()+reverse_tail);
    }
        // final result
        assert(t0->cyc.size() == t1->cyc.size());
        for(int i = 0; i < t0->cyc.size(); i++)
        {
            printf("cyc %d\n", run);
            assert(t0->cyc[run] || t1->cyc[run]);
            if(t0->cyc[run])
            {
                t0->cyc[run]->process(1);
                if(t0->cyc[run])
                {
                    hit[t0->cyc[run]->id] = 1;
                }
            }
            if(t1->cyc[run])
            {
                t1->cyc[run]->process(1);
                if(t1->cyc[run])
                {
                    hit[t1->cyc[run]->id] = 1;
                }
            }
            run++;
        }

    printf("nodes not done:");
    for(int i = 0; i < nodes.size(); i++)
    {
        if(!hit[i]) 
            printf("%d ", i);
    }
    printf("\n");

    float opc = (float)(nodes.size()) / (float)(t0->cyc.size());
    printf("ops = %f\n", opc);

    

    return 0; 
}

