#ifndef _ARBITER_H
#define _ARBITER_H

#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <algorithm>
#include "node.h"
#include "dfg.h"

using namespace std;

enum
{
    UP,
    LEFT,
    DIA
};

/**
 * 
 *
*/
class thread
{
    public:
        int id;
        thread(int _id);

        vector<node*> cyc;
        vector<tree*> *forest;

        // the current tree this thread is on
        tree* current;

        // tree might be put in wait queue later
        set<tree*> candidates;

        vector<node*> wait;

        // pick up a candidate from candi set
        void schedule_from_cand(tree* cand);

        // pick up any free tree from dfg
        void schedule_from_dfg();
};

void dy_pgm(thread* t0, thread* t1, set<tree*>& trigger);
void trigger_trees(thread* t0, thread* t1, set<tree*> trigger, vector<tree*>& vforest);
super_node* inv_dy_pgm(thread* t0, thread* t1);

/** 0: t0 remaining
 *  1: t1 remaining
 *  2: complete
 */
tree* inter_tree_schedule(thread* t0, thread* t1, vector<tree*>& vforest); 
void intra_tree_schedule(thread* t0, thread* t1, super_node* remain_roots);
void show_vector(const vector<node*>& shown);

#endif
