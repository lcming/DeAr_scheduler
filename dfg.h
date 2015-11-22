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
void test_single_thread(int show);
void process_node(node* n, int show);
void reset_node(node* n);
void reset_dfg();

#endif
