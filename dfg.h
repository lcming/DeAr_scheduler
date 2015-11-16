#ifndef _DFG_H
#define _DFG_H
#include "node.h"
#include <fstream>
#include <vector>
#include <stdio.h>
#include <set>


vector<node*> init_dfg(char* filename);
void init_super_dfg(vector<node*> list);
super_node* build_super(node* target);

#endif
