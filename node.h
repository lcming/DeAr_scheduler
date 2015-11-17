#ifndef _NODE_H
#define _NODE_H


#include <vector>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <set>

#define RF_SIZE 32
#define READABLE

using namespace std;


enum
{
    ADD = RF_SIZE,
    MUL,
    SHI,
    STACK

};

enum
{
    PUSH,
    POP,
    WRITE,
    NOP
};

class super_node;
class tree;

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
        pair<node*, node*> pres;
        set<node*> sucs;
        set<node*> cuts;
        super_node* wrap; // node wrapper

        void update_reg();
        void bypass(tree* get);
        void connect(node* dst);
        void push(tree* get);

        node(int _id, int _op);
        //void dbg();
        ~node();

};

class super_node
{
    public: 
        tree*  t;
        string id;
        int ss; // stack size
        int done; // done flag
        int dest; // result destination
        int level;
        int ready;
        vector<node*> cas;
        set<super_node*> pres;
        set<super_node*> sucs;


        super_node();
        void cut(super_node* target);
        void add_node(node* n);
        void connect(super_node* dst);
        void rec_schedule();
        void schedule();
        //void dbg();

        ~super_node();
};

class tree
{
    public:
        super_node* root;
        super_node* prev;
        super_node* early;
        set<tree*> pres, sucs;
        tree(super_node* _root, int _wb);
        void dispatch();
        vector<tree*> initialize();
        int analyze_stack(super_node* target);
        void extend(super_node* sn);
        void early_schedule();
        void connect(tree* target);
        int done;
        int wb;
};

typedef set<super_node*> sns;
typedef set<node*> nds;
typedef pair<node*, node*> ndp;
typedef pair<super_node*, super_node*> snp;


tree* build_tree(super_node* sn);
int allocate();

#endif
