#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include "omp_apps.h"

int main() {
	// Generate input vectors and destination vector
	double *x = gen_array(ARRAY_SIZE);
	double *y = gen_array(ARRAY_SIZE);
	double *z = (double*) malloc(ARRAY_SIZE*sizeof(double));

	// Test framework that sweeps the number of threads and times each run
	double start_time, run_time;
	int num_threads = omp_get_max_threads();

	double fast_adjacent_time = 1000;
	double fast_chunks_time = 1000;


	for (int i=1; i<=num_threads; i++) {
		omp_set_num_threads(i);		
	  	start_time = omp_get_wtime();
		for(int j=0; j<REPEAT; j++)
			v_add_optimized_adjacent(x,y,z);
		run_time = omp_get_wtime() - start_time;
	    if(!verify(x,y, v_add_optimized_adjacent)){
	      printf("v_add optimized adjacent does not match reference.\n");
	      return -1; 
	    }
		if (run_time < fast_adjacent_time) {
			fast_adjacent_time = run_time;
		}
	    printf("Optimized adjacent: %d thread(s) took %f seconds\n",i,run_time);
     }


	for (int i=1; i<=num_threads; i++) {
		omp_set_num_threads(i);		
	  	start_time = omp_get_wtime();
		for(int j=0; j<REPEAT; j++)
			v_add_optimized_chunks(x,y,z);
		run_time = omp_get_wtime() - start_time;
	    if(!verify(x,y, v_add_optimized_chunks)){
	      printf("v_add optimized chunks does not match reference.\n");
	      return -1; 
	    }
		if (run_time < fast_chunks_time) {
			fast_chunks_time = run_time;
		}
	    printf("Optimized chunks: %d thread(s) took %f seconds\n",i,run_time);
	  }

	double fast_naive_time = 1000;

	for(int i=1; i<=num_threads; i++) {
		omp_set_num_threads(i);		
		start_time = omp_get_wtime();
		for(int j=0; j<REPEAT; j++)
			v_add_naive(x,y,z);
		run_time = omp_get_wtime() - start_time;
		if (run_time < fast_naive_time) {
			fast_naive_time = run_time;
		}
  		printf("Naive: %d thread(s) took %f seconds\n",i,run_time);
    }
	printf("\n");
	printf("==> A %.2lfx speedup from the naive benchmark to the fastest adjacent runtime\n",
	fast_naive_time / fast_adjacent_time);
	printf("==> A %.2lfx speedup from the naive benchmark to the fastest chunks runtime\n",
	fast_naive_time / fast_chunks_time);

    return 0;
}

