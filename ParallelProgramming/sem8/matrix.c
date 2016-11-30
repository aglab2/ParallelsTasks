#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <mpi.h>
#include <omp.h>

void cache_clear(){
       FILE *fp = fopen ("/proc/sys/vm/drop_caches", "w");
       fprintf (fp, "3");
       fclose (fp);
}

#define SIZE 2048
#define MIN(x,y) (x<y ? x: y)

typedef int array_t;

void array_set(array_t ** arr, array_t (*f)(int, int)){
       int i, j;
       for (i = 0; i < SIZE; i++){
              for (j = 0; j < SIZE; j++){
                     arr[i][j] = f(i, j);
              }
       }
}

array_t array_set_zero(int i, int j){
       return 0;
}

array_t array_set_1(int i, int j){
       return i+j;
}

array_t array_set_2(int i, int j){
       return i-j;
}

array_t array_cmp(array_t ** arr1, array_t ** arr2) {
       int i, j;
       for (i = 0; i < SIZE; i++){
              for (j = 0; j < SIZE; j++){
                     if (arr1[i][j] != arr2[i][j]){
                            printf("%d %d\n", i, j);
                            return arr1[i][j] - arr2[i][j];
                     }
              }
       }
       return 0;
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s NUM_OPENMP\n", argv[0]);
		exit(1);
	}

	//TODO: No error detection
	int num_proc_openmp = atoi(argv[1]);

	if (num_proc_openmp <= 0) {
		fprintf(stderr, "Number of procs should be positive!\n");
		exit(2);
	}

	int num_proc_mpi;
	int iter;

	//Setup threads
	omp_set_num_threads(num_proc_openmp);
	MPI_Init(NULL, NULL);
	MPI_Comm_size(MPI_COMM_WORLD, &num_proc_mpi);

	array_t ** A = (array_t **) malloc(SIZE*sizeof(array_t *));
	for (iter = 0; iter < SIZE; iter++){
		A[iter] = (array_t *) malloc(SIZE*sizeof(array_t));
	}

	array_t ** B = (array_t **) malloc(SIZE*sizeof(array_t *));
	for (iter = 0; iter < SIZE; iter++){
		B[iter] = (array_t *) malloc(SIZE*sizeof(array_t));
	}

	array_t ** C1 = (array_t **) malloc(SIZE*sizeof(array_t *));
	for (iter = 0; iter < SIZE; iter++){
		C1[iter] = (array_t *) malloc(SIZE*sizeof(array_t));
	}

	array_t ** C2 = (array_t **) malloc(SIZE*sizeof(array_t *));
	for (iter = 0; iter < SIZE; iter++){
		C2[iter] = (array_t *) malloc(SIZE*sizeof(array_t));
	}

	if (SIZE % (num_proc_mpi*num_proc_openmp)) {
		fprintf(stderr, "SIZE(%d) should be divided by num of executors!\n", SIZE);
		exit(3);
	}

       array_set(A, array_set_1);
       array_set(B, array_set_2);
	array_set(C1, array_set_zero);
	array_set(C2, array_set_zero);

	if (SIZE % (num_proc_mpi*num_proc_openmp)) {
		fprintf(stderr, "Specify number of proc, that divide %d\n", SIZE);
		MPI_Finalize();
		exit(1);
	}

	int BLOCKSIZE = SIZE / num_proc_mpi / num_proc_openmp;

	//BLOCKSIZE is set so there will be exactly num_proc_mpi*num_proc_openmp blocks
	//Inside one MPI_proc divide for each openmp_proc

	//long clock_time = clock();
	double mpi_time = MPI_Wtime();
	double omp_time = omp_get_wtime();

	int mpi_id;
	MPI_Comm_rank(MPI_COMM_WORLD, &mpi_id);
	#pragma omp parallel
	{
		int openmp_id = omp_get_thread_num();
		int global_id = mpi_id*num_proc_openmp + openmp_id;
		int block_j = global_id * BLOCKSIZE;

		int i, j, k;
	       int block_k;
	       int size_j, size_k;

		for (block_k = 0; block_k < SIZE; block_k += BLOCKSIZE) { //X-dimension of A or Y-dimension of B
                     for (i = 0; i < SIZE; i++) { //X-dimension of C
                            size_j = MIN(block_j + BLOCKSIZE, SIZE);
                            size_k = MIN(block_k + BLOCKSIZE, SIZE);
                            for (j = block_j; j < size_j; j++) { //Y-dimension of C
                                   array_t sum = 0;
                                   for (k = block_k; k < size_k; k++) { //Mult dimension of block
                                          sum += A[i][k] * B[k][j];
                                   }
					#pragma omp critical
                                   C2[i][j] += sum;
                            }
                     }
              }
	}
	for (iter = 0; iter < SIZE; iter++){
		MPI_Allreduce(C2[iter], C1[iter], SIZE, MPI_INTEGER, MPI_SUM, MPI_COMM_WORLD);
	}


       //if (array_cmp(C1, C2)) //TODO: Return me
       //       printf("Matrix multiplication gave wrong result!\n");

	MPI_Finalize();

	if (!mpi_id){
		mpi_time = MPI_Wtime() - mpi_time;
		omp_time = omp_get_wtime() - omp_time;
		printf("MPI:%lg\n", mpi_time);
		printf("OMP:%lg\n", omp_time);
	}


	return 0;
}
