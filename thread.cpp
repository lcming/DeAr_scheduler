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
        if( !t->done ) 
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

void dy_pgm(thread* t0, thread* t1)
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

    if(max_opc == 1.0)
    {
        // special case here
        // this implies 2 thread cannot help at all
        // so do it using single thread instead
        t0->wait.insert(t0->wait.begin(), t1->wait.begin(), t1->wait.end());
        for(int i = 0; i < t0->wait.size(); i++)
        {
            t0->cyc.push_back(t0->wait[i]);
            t1->cyc.push_back(NULL);
        }
        t0->wait.clear();
        t1->wait.clear();
        return;
    }

    printf("dp: best dispatch size for thread 0 = %d, thread 1 = %d\n", max_y, max_x);

    int stride = 0;

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
        stride++;
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
            t0->wait[i_t0]->done = t1->wait[i_t1]->done = 1;
            t0->cyc[base+i] = t0->wait[i_t0++];
            t1->cyc[base+i] = t1->wait[i_t1++];
        }
        else if(condi == LEFT)
        {
            t1->wait[i_t1]->done = 1;
            t1->cyc[base+i] = t1->wait[i_t1++];
        }
        else if(condi == UP)
        {
            t0->wait[i_t0]->done = 1;
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

    /*
    assert(t0->cyc.size() == t1->cyc.size());
    int run = 0;
    for(int i = 0; i < stride; i++)
    {
        printf("cyc\n");
        assert(t0->cyc[base] || t1->cyc[base]);
        if(t0->cyc[base])
        {
            printf("cyc %d: ", base);
            t0->cyc[base]->process(1);
            run++;
        }
        if(t1->cyc[base])
        {
            printf("cyc %d: ", base);
            t1->cyc[base]->process(1);
            run++;
        }
        base ++;
    }
    printf("cyc: this round %d\n", run);
    */
    
}

void show_vector(const vector<node*>& shown)
{
    for( auto &it : shown)
    {
        printf("%d ", it->id);
    }
    printf("\n");
}

super_node* inter_tree_schedule(thread* t0, thread* t1, vector<tree*>& vforest)
{
    int cyc_cnt = 0;
    while(vforest.size() > 0)
    {
        if(t0->wait.size() == 0)
            t0->schedule_from_dfg();
        if(t1->wait.size() == 0)
            t1->schedule_from_dfg();

        // any of thread has no work, break
        if(t0->wait.size() == 0 || t1->wait.size() == 0)
            break;

        dy_pgm(t0, t1);
/*
        assert(t0->cyc.size() == t1->cyc.size());
        int run = 0;
        for(int i = 0; i < stride; i++)
        {
            printf("cyc\n");
            assert(t0->cyc[cyc_cnt] || t1->cyc[cyc_cnt]);
            if(t0->cyc[cyc_cnt])
            {
                printf("cyc %d: ", cyc_cnt);
                t0->cyc[cyc_cnt]->process(1);
                run++;
            }
            if(t1->cyc[cyc_cnt])
            {
                printf("cyc %d: ", cyc_cnt);
                t1->cyc[cyc_cnt]->process(1);
                run++;
            }
            cyc_cnt ++;
        }
        printf("cyc: this round %d\n", run);
        */
    }
    printf("remaining: t0: %d, t1: %d\n", t0->wait.size(), t1->wait.size());

    assert(t0->wait.size() == 0 || t1->wait.size() == 0);

    super_node* ret; 
    if(t0->wait.size() == 0 && t1->wait.size() == 0)
        ret =  NULL;
    else if(t0->wait.size() == 0)
    {
        for( auto &it : t1->wait)
        {
            it->wrap->done = 0;
        }
        ret =  t1->wait.back()->wrap;
        t1->wait.clear();
    }
    else if(t1->wait.size() == 0)
    {
        for( auto &it : t0->wait)
        {
            it->wrap->done = 0;
        }
        ret =  t0->wait.back()->wrap;
        t0->wait.clear();
    }
    else 
    {
        printf("inter_tree_schedule return error\n");
    }
    return ret;
}

void intra_tree_schedule(thread* t0, thread* t1, super_node* sn)
{
    assert(t0->wait.size() == 0 && t1->wait.size() == 0);

    vector<node*> n_list;
    // only want to analyze this sn instead of recursion
    sn->schedule(n_list);

    // r_sn must be done sequentially
    for(int i = 0; i < n_list.size() ; i--)
    {
        // reverse order 
        t0->cyc.push_back(n_list[i]);
        t1->cyc.push_back(NULL);
        n_list[i]->done = 1;
    }


    if(sn->pres.size() == 2)
    {
        super_node* left = *sn->pres.begin();
        super_node* right = *sn->pres.rbegin();
        vector<node*> r_left, r_right;
        left->inv_rec_schedule(r_left);
        right->inv_rec_schedule(r_right);

        printf("intra: %s, left size = %d, right size = %d\n", sn->id.c_str(), r_left.size(), r_right.size());
        // fix false done
        for(auto &it : r_left)
            it->wrap->done = 0;
        for(auto &it : r_right)
            it->wrap->done = 0;

        if(r_left.size() == 0 && r_right.size() == 0)
        {
            printf("inra: no more work in r_left and r_right\n");
            // nothing 
        }
        else if(r_left.size() > 0 && r_right.size() > 0)
        {
            // put on wait queue
            t0->wait.insert(t0->wait.begin(), r_left.begin(), r_left.end());
            t1->wait.insert(t1->wait.begin(), r_right.begin(), r_right.end());

            super_node* next_sn = inv_dy_pgm(t0, t1);
            if(t0->wait.size() == 0 && t1->wait.size() == 0)
                return;
            else if(t0->wait.size() > 0)
            {
                t0->wait.clear();
                intra_tree_schedule(t0, t1, next_sn);
            }
            else if(t1->wait.size() > 0)
            {
                t1->wait.clear();
                intra_tree_schedule(t0, t1, next_sn);
            }
            else
            {
                printf("fatal\n");
                exit(1);
            }
        }
        else
        {
            // recursive intra_schedule 
            super_node* next_sn = r_left.size() > 0 ? left : right;
            intra_tree_schedule(t0, t1, next_sn);
        }
    }



}

super_node* inv_dy_pgm(thread* t0, thread* t1)
{
    reverse(t0->wait.begin(), t0->wait.end());
    reverse(t1->wait.begin(), t1->wait.end());

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

    if(max_opc == 1.0)
    {
        // special case here
        // this implies 2 thread cannot help at all
        // so do it using single thread instead
        t0->wait.insert(t0->wait.begin(), t1->wait.begin(), t1->wait.end());
        for(int i = 0; i < t0->wait.size(); i++)
        {
            t0->cyc.push_back(t0->wait[i]);
            t1->cyc.push_back(NULL);
        }
        t0->wait.clear();
        t1->wait.clear();
        return NULL;
    }

    printf("dp: best dispatch size for thread 0 = %d, thread 1 = %d\n", max_y, max_x);

    int stride = 0;

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
        stride++;
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
            t0->wait[i_t0]->done = t1->wait[i_t1]->done = 1;
            // update wrap done
            if(t0->wait[i_t0] == t0->wait[i_t0]->wrap->cas.back())
                t0->wait[i_t0]->wrap->done = 1;
            if(t1->wait[i_t1] == t1->wait[i_t1]->wrap->cas.back())
                t1->wait[i_t1]->wrap->done = 1;
            t0->cyc[base+i] = t0->wait[i_t0++];
            t1->cyc[base+i] = t1->wait[i_t1++];
        }
        else if(condi == LEFT)
        {
            t1->wait[i_t1]->done = 1;

            if(t1->wait[i_t1] == t1->wait[i_t1]->wrap->cas.back())
                t1->wait[i_t1]->wrap->done = 1;

            t1->cyc[base+i] = t1->wait[i_t1++];
        }
        else if(condi == UP)
        {
            t0->wait[i_t0]->done = 1;

            if(t0->wait[i_t0] == t0->wait[i_t0]->wrap->cas.back())
                t0->wait[i_t0]->wrap->done = 1;

            t0->cyc[base+i] = t0->wait[i_t0++];
        }
    }
    t0->wait.erase(t0->wait.begin(), t0->wait.begin()+ready_cnt_t0);
    t1->wait.erase(t1->wait.begin(), t1->wait.begin()+ready_cnt_t1);

    printf("thread 0: wait: ");
    show_vector(t0->wait);

    printf("thread 1: wait: ");
    show_vector(t1->wait);

    if(t0->wait.size() == 0 && t1->wait.size() == 0)
    {
        printf("inv: both zero\n");
        return NULL;
    }
    else 
    {
        vector<node*> find = t0->wait.size() > 0 ? t0->wait : t1->wait;
        vector<super_node*> ret;
        for(auto &it : find)
        {
            assert(it->sucs.size() == 1);
            node* suc = *(it->sucs.begin());
            if( suc->done )      
            {
                ret.push_back(it->wrap);
            }
        }
        if(ret.size() == 1)
        {
            printf("inv: %s\n", ret[0]->id.c_str());
            return ret[0];
        }
        /*
        else if(ret.size() == 2)
        {
            assert( *ret[0]->sucs.begin() == *ret[0]->sucs.begin() );
        
            return *ret[0]->sucs.begin();
        }
        */
        else
        {
            //printf("fatal: inv_dy_pgm return error, ret size = %d\n", ret.size());
            //exit(1);
            int hit;
            int min_level = 999;
            for(int i = 0; i < ret.size(); i++)
            {
                if( ret[i]->get_level() < min_level) 
                {
                    hit = i; 
                }
            }
            return *ret[hit]->sucs.begin();
        }
        
    }

}


