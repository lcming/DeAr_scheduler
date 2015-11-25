#include "thread.h"

extern node* start;

thread::thread(int _id, vector<tree*>& _forest)
{
    id = _id;
    forest = &_forest;
}


void thread::schedule_from_cand(tree* cand)
{

}

void thread::schedule_from_dfg()
{
    // find a free tree from forest
    int cnt = 0;
    for( auto &t : *forest)
    {
        if( t->pres.size() == 0 && !t->done ) 
        {
            vector<node*> new_work = t->dispatch();
            (*forest).erase((*forest).begin()+cnt);
            printf("thread %d: get new work: ", id);
            for( auto &n : new_work)
                printf("%d ", n->id);
            printf("\n");

            wait.insert(wait.end(), new_work.begin(), new_work.end());

            printf("thread %d: wait queue: ", id);
            for( auto &n : wait)
                printf("%d ", n->id);
            printf("\n");

            // find and break
            break;
        }
        cnt++;
    }
}

int dy_pgm(thread* t0, thread* t1)
{
    vector< vector<int> > mat;

    // init matrix
    int height = t0->wait.size() + 1;
    int width = t1->wait.size() + 1;
    mat.resize(height);
    for( auto &it : mat)
    {
        it.resize(width);
    }

    // init data
    mat[0][0] = 0;
    for(int i = 1; i < width; i++)
    {
        mat[0][i] = i;
    }
    for(int i = 0; i < height; i++)
    {
        mat[i][0] = i;
    }

    // fill the matrix
    for(int i = 1; i < height; i++)
    {
        printf("dp: ");
        for(int j = 1; j < width; j++) 
        {
            if(t0->wait[i-1]->op != t1->wait[j-1]->op) 
            {
                mat[i][j] = mat[i-1][j-1] + 1;
            }
            else
            {
                mat[i][j] = mat[i-1][j] < mat[i][j-1] ? mat[i-1][j] + 1 : mat[i][j-1] + 1;
            }
            printf("%3d ", mat[i][j]);
        }
        printf("\n");
    }

    // find max opc as the backtracking point
    float max_opc = -1.0;
    int max_x;
    int max_y;
    for(int i = 0; i < height; i++)
    {
        int x = width - 1;
        int y = height - 1 - i;
        int op_cnt = x + y;
        int cyc_cnt = mat[y][x];
        float opc = float(op_cnt) / float(cyc_cnt);
        if( opc > max_opc ) 
        {
            max_opc = opc; 
            max_x = x;
            max_y = y;
        }
            
    }

    for(int i = 0; i < width; i++)
    {
        int x = width - 1 - i;
        int y = height - 1;
        int op_cnt = x + y;
        int cyc_cnt = mat[y][x];
        float opc = float(op_cnt) / float(cyc_cnt);
        if( opc > max_opc ) 
        {
            max_opc = opc; 
            max_x = x;
            max_y = y;
        }
    }

    printf("dp: best dispatch size for thread 0 = %d, thread 1 = %d\n", max_y, max_x);

    int ret = 0;

    // baktracking
    int ready_cnt_t0 = max_y;
    int ready_cnt_t1 = max_x;

    vector<int> path;
    while( max_x != 0 || max_y != 0)
    {
        if( max_x >= 1 && max_y >= 1 && t0->wait[max_y-1]->op != t1->wait[max_x-1]->op )  
        {
            path.push_back(DIA);
            max_x --;
            max_y --;
        }
        else if( max_y >= 1 && (mat[max_y][max_x] - 1 == mat[max_y-1][max_x]))
        {
            path.push_back(UP);
            max_y--;
        }
        else if( max_x >= 1 && (mat[max_y][max_x] - 1 == mat[max_y][max_x-1]))
        {
            path.push_back(LEFT);
            max_x--;
        }
        else
        {
            printf("fatal: backtracking\n");
            exit(1);
        }
        ret++;
    }

    printf("dp: backtracking: ");
    for( auto &it : path)
    {
        if(it == UP) 
            printf("<-t0");
        else if(it == LEFT) 
            printf("<-t1");
        else if(it == DIA)
            printf("<-both");
    }
    printf("\n");

    int base = t0->cyc.size();
    int i_t0 = 0;
    int i_t1 = 0;
    for(int i = 0; i < path.size(); i++)
    {
        t0->cyc.push_back(NULL);
        t1->cyc.push_back(NULL);
        int condi = path[path.size()-1-i];
        if(condi == DIA) 
        {
            t0->cyc[base+i] = t0->wait[i_t0++];
            t1->cyc[base+i] = t1->wait[i_t1++];
        }
        else if(condi == LEFT)
        {
            t1->cyc[base+i] = t1->wait[i_t1++];
        }
        else if(condi == UP)
        {
            t0->cyc[base+i] = t0->wait[i_t0++];
        }
    }
    t0->wait.erase(t0->wait.begin(), t0->wait.begin()+ready_cnt_t0);
    t1->wait.erase(t1->wait.begin(), t1->wait.begin()+ready_cnt_t1);

    /*
    first->ready.insert(first->ready.begin(), first->wait.begin(), first->wait.begin() + max_y);
    first->wait.erase(first->wait.begin(), first->wait.begin() + max_y);

    */


    printf("thread 0: wait: ");
    show_vector(t0->wait);

    printf("thread 1: wait: ");
    show_vector(t1->wait);


    return ret;

    
}

void show_vector(const vector<node*>& shown)
{
    for( auto &it : shown)
    {
        printf("%d ", it->id);
    }
    printf("\n");
}

