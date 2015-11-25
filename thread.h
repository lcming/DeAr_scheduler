#ifndef _ARBITER_H
#define _ARBITER_H

#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
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
        thread(int _id, vector<tree*>& _forest);

        vector<node*> cyc;
        vector<tree*> *forest;

        // the current tree this thread is on
        tree* current;

        // tree might be put in wait queue later
        set<tree*> candidates;

        vector<node*> ready;
        vector<node*> wait;

        // pick up a candidate from candi set
        void schedule_from_cand(tree* cand);

        // pick up any free tree from dfg
        void schedule_from_dfg();
};

int dy_pgm(thread* t0, thread* t1);
void show_vector(const vector<node*>& shown);

#endif
