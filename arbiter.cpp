#include "arbiter.h"
#include "dfg.h"

extern node* start;

run::run(tree* free)
{
    source = free; 
}

void run::finalize(int show)
{

}

void run::initialize()
{
    source->dispatch(); 
    node* n = start;
    while(n)
    {
        n->process(1);
        ready.push_back(n);
        //n->reset();

        // reset node chain
        node* prev = n;
        n = n->next;
        prev->next = NULL;
    }
    start = NULL;

    printf("run: ready size = %d\n", ready.size());

}
