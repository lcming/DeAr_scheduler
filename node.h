#ifndef _NODE_H
#define _NODE_H


#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <set>

#define RF_SIZE 16

using namespace std;


enum
{
    ADD,
    MUL,
    SHI
};

enum
{
    PUSH = -3,
    POP = -2
};

class super_node;

typedef pair<int, int> pint;
class node
{
    public:
        int id;
        int op;
        int ops; // stack operation
        pint rs;
        int rd;
        node* first;
        set<node*> pres;
        set<node*> sucs;
        set<node*> cuts;
        super_node* wrap; // node wrapper

        node(int _id, int _op);
        void dbg();
        ~node();

};

class super_node
{
    public: 
        string id;
        int ss; // stack size
        int done; // done flag
        int dest; // result destination
        int level;
        vector<node*> cas;
        set<super_node*> pres;
        set<super_node*> sucs;
        set<super_node*> sibs;


        super_node();
        void merge();
        void cut();
        void cut(super_node* target);
        void add_node(node* n);
        void dbg();

        ~super_node();
};

typedef set<super_node*> sns;
typedef set<node*> nds;
typedef pair<node*, node*> ndp;
typedef pair<super_node*, super_node*> snp;

void connect(node* src, node* dst);
void connect(super_node* src, super_node* dst);
void update_stack(super_node* target);
int analyze_stack(super_node* target);
void schedule(super_node* target);

#endif
