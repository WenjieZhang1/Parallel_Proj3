#include "mpi.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#define NUM_POINTS 524288 

unsigned int X_axis[NUM_POINTS];
unsigned int Y_axis[NUM_POINTS];
 
void swap(unsigned int array[], int i, int j);
unsigned int find_median(unsigned int *start_x, int range, unsigned int *start_y);
int numprocs;
int myid;
int num_quadrants;


void find_quadrants (num_quadrants)
{
  /* YOU NEED TO FILL IN HERE */
  if(myid == 0) {
    bool cut_x = true;
    int quadrants = 1;
    int high[num_quadrants];
    int low[num_quadrants];
    int left[num_quadrants];
    int right[num_quadrants];
    int partition[num_quadrants];

    while(num_quadrants > quadrants){
      int num_points = NUM_POINTS/quadrants;
      if(cut_x) {
        for(int i = 0; i < quadrants; ++i){
          int x_partition = find_median(X_axis + i * num_points, num_points, Y_axis + i * num_points);
          partition[i + quadrants - 1] = x_partition;
        }
        cut_x = false;
      } else {
        for(int i = 0; i < quadrants; ++i){
          int y_partition = find_median(Y_axis + i * num_points, num_points, X_axis + i * num_points);
          partition[i + quadrants -1] = y_partition;
        }
        cut_x = true;
      }
      quadrants *= 2;
    }




  }






}

int main(argc,argv)
  int argc;
 char *argv[];
{
  int num_quadrants;
  int myid, numprocs;
  int  namelen;
  char processor_name[MPI_MAX_PROCESSOR_NAME];
  double global_cost = 0; 
  double startwtime = 0.0;
  double endwtime;
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
    }

  MPI_Bcast(&X_axis, NUM_POINTS, MPI_INT, 0, MPI_COMM_WORLD);  
  MPI_Bcast(&Y_axis, NUM_POINTS, MPI_INT, 0, MPI_COMM_WORLD);  

  find_quadrants (num_quadrants);
  
  MPI_Barrier(MPI_COMM_WORLD);

  if(myid == 0) {
    endwtime = MPI_Wtime();
    printf("\n elapsed_time = %f\n", endwtime - startwtime);
    printf("\n Total cost is : %f \n", global_cost);
  }

  MPI_Finalize();

  return 0;
}
  

