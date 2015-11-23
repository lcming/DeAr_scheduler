#include "arbiter.h"
#include "dfg.h"

extern node* start;

run::run(tree* free)
{
    source = free; 
    ready = source->dispatch();
    printf("run: ready size = %d\n", ready.size());
}


void run::dispatch_ready(int show)
{
    for(int i = 0; i < ready.size(); i++)
        ready[i]->process(show);
}

void run::finalize(int show)
{

}

