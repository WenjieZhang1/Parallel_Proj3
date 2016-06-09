#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define NUM_POINTS 524288

unsigned int X_axis[NUM_POINTS];
unsigned int Y_axis[NUM_POINTS];
unsigned int qselect(unsigned int *v,  unsigned int *y, int n, int k);

void find_quadrants_2 (int num_quadrants, int quadrants, int start, int end) {
    if(quadrants == num_quadrants) {
        return;
    } 
    unsigned int x_breaking;
    unsigned int y_breaking;
    int MinX = X_axis[start];
    int MaxX = X_axis[start];
    int MinY = Y_axis[start];
    int MaxY = Y_axis[start];
    int i;
    for (i = start+1; i < end; i++) {
    	MinX = X_axis[i] < MinX ? X_axis[i] : MinX;
    	MaxX = X_axis[i] > MaxX ? X_axis[i] : MaxX;
    	MinY = Y_axis[i] < MinY ? Y_axis[i] : MinY;
    	MaxY = Y_axis[i] > MaxY ? Y_axis[i] : MaxY;
    }

    int X_cut;
    X_cut = ((MaxX - MinX) > (MaxY - MinY)) ? 1 : 0;
    if (X_cut) {
        x_breaking = qselect(X_axis + start, Y_axis + start, end-start, (end-start)/2);
        find_quadrants_2(num_quadrants, quadrants*2, start, (end+start)/2);
        find_quadrants_2(num_quadrants, quadrants*2, (end+start)/2, end);
        if(num_quadrants == 2* quadrants){
            // left side 
            printf(" (%d,%d) ", MinX, MaxY);
            printf(" (%u,%d) ", x_breaking, MaxY);
            printf(" (%u,%d) ", x_breaking, MinY);
            printf(" (%d,%d) \n", MinX, MinY);
            // right side
            printf(" (%d,%d) ", x_breaking, MaxY);
            printf(" (%u,%d) ", MaxX, MaxY);
            printf(" (%u,%d) ", MaxX, MinY);
            printf(" (%d,%d) \n", x_breaking, MinY);  
        }
    } else {
        y_breaking = qselect(Y_axis + start, X_axis + start, end-start, (end-start)/2);
        find_quadrants_2(num_quadrants, quadrants*2, start, (end+start)/2);
        find_quadrants_2(num_quadrants, quadrants*2, (end+start)/2, end);
        if(num_quadrants == 2* quadrants){
            // down side 
            printf(" (%d,%d) ", MinX, MaxY);
            printf(" (%u,%d) ", MaxX, MaxY);
            printf(" (%u,%d) ", MaxX, y_breaking);
            printf(" (%d,%d) \n", MinX, y_breaking);
            // up side
            printf(" (%d,%d) ", MinX, y_breaking);
            printf(" (%u,%d) ", MaxX, y_breaking);
            printf(" (%u,%d) ", MaxX, MinY);
            printf(" (%d,%d) \n", MinX, MinY);  
        }
    }

}
void SWAP(int a, int b, unsigned int *v, unsigned int *w) { 
        unsigned int tmp = v[a]; 
        v[a] = v[b]; 
        v[b] = tmp; 
        tmp = w[a]; 
        w[a] = w[b]; 
        w[b] = tmp; 
}
unsigned int qselect(unsigned int *v, unsigned int* w, int len, int k)
{
    int i, st;
    for (st = i = 0; i < len - 1; i++) {
        if (v[i] > v[len-1]) continue;
        SWAP(i, st, v, w);
        st++;
    }
 
    SWAP(len-1, st, v, w);
 
    return k == st  ?v[st]
            :st > k ? qselect(v, w, st, k)
                : qselect(v + st, w + st, len - st, k - st);
}
int main(argc,argv)
int argc;
char *argv[];
{
    int  num_quadrants;
    int  namelen;
    int myid, numprocs;
    char processor_name[MPI_MAX_PROCESSOR_NAME];

    double startwtime = 0.0, endwtime;
    
    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD,&myid);
    MPI_Get_processor_name(processor_name,&namelen);
    
    if (argc != 2)
    {
        fprintf (stderr, "Usage: recursive_bisection <#of quadrants>\n");
        MPI_Finalize();
        exit (0);
    }
    
    fprintf (stderr,"Process %d on %s\n", myid, processor_name);
    
    num_quadrants = atoi (argv[1]);
    
    if (myid == 0)
        fprintf (stdout, "Extracting %d quadrants with %d processors \n", num_quadrants, numprocs);
        
        if (myid == 0)
        {
            int i;
            
            srand (10000);
            
            for (i = 0; i < NUM_POINTS; i++)
                X_axis[i] = (unsigned int)rand();

            
            for (i = 0; i < NUM_POINTS; i++)
                Y_axis[i] = (unsigned int)rand();

            //start timer at process 0
            printf("\nComputing Parallely Using MPI.\n");
            startwtime = MPI_Wtime();
            // find_quadrants_1 (num_quadrants);
            find_quadrants_2 (num_quadrants, 1, 0, NUM_POINTS);
        }
    
    MPI_Bcast(&X_axis, NUM_POINTS, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&Y_axis, NUM_POINTS, MPI_INT, 0, MPI_COMM_WORLD); 
    
    int i, j, k;
    double global_cost = 0; 
    double local_cost = 0;
    for (i = myid; i < num_quadrants; i += numprocs)
    {
       int points = NUM_POINTS / num_quadrants;
       for (j = 0; j < points - 1; j++) {
            for (k = j+1; k < points; k++) {
                int x1 = points * i + j;
                int x2 = points * i + k;
                int y1 = points * i + j;
                int y2 = points * i + k;

                double diff_x = abs(X_axis[x1] - X_axis[x2]);
                double diff_y = abs(Y_axis[y1] - Y_axis[y2]);
                local_cost += sqrt((double)diff_x * diff_x + diff_y * diff_y);
            }
       }     
    }
    MPI_Reduce(&local_cost, &global_cost, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (myid == 0) {
        endwtime = MPI_Wtime();
        printf("\nelapsed time = %f\n", endwtime - startwtime);
        printf("\nTotal cost:  %lf \n", global_cost);

    }
    
    MPI_Finalize();


    return 0;
}
