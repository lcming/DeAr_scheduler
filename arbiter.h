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

/**
 * 
 *
*/
class run
{
    public:
        run(tree* free);
        tree* source;
        set<tree*> merges;
        vector<node*> ready;
        vector<node*> wait;
        void initialize();
        void finalize(int show);
        void receive(tree* merge);
};

void dy_pgm(run* first, run* second);


#endif
