#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void cache_clear(){
       FILE *fp = fopen ("/proc/sys/vm/drop_caches", "w");
       fprintf (fp, "3");
       fclose (fp);
}

#define SIZE 1500
#define ATTEMPT_NUM 1
#define MIN(x,y) (x<y ? x: y)

typedef __int128 array_t;

array_t A[SIZE][SIZE];
array_t B[SIZE][SIZE];

array_t C1[SIZE][SIZE];
array_t C2[SIZE][SIZE];

void array_set(array_t arr[SIZE][SIZE], array_t (*f)(int, int)){
       int i, j;
       for (i = 0; i < SIZE; i++){
              for (j = 0; j < SIZE; j++){
                     arr[i][j] = f(i, j);
              }
       }
}

array_t array_func_zero(int i, int j){
       return 0;
}

array_t array_func_1(int i, int j){
       return i+j;
}

array_t array_func_2(int i, int j){
       return i-j;
}

array_t array_cmp(array_t arr1[SIZE][SIZE], array_t arr2[SIZE][SIZE]) {
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
       array_set(A, array_func_1);
       array_set(B, array_func_2);

       register int BLOCKSIZE;
       register int i, j, k;
       register int block_k, block_j;
       register array_t sum;
       register int size_j, size_k;

       int attempt;
       clock_t start, diff, total = 0;

       for (attempt = 0; attempt < ATTEMPT_NUM; attempt++){
              cache_clear();
              array_set(C1, array_func_zero);
              start = clock();
              for (i = 0; i < SIZE; i++)
                     for (j = 0; j < SIZE; j++)
                            for (k = 0; k < SIZE; k++)
                                   C1[i][j] += A[i][k] * B[k][j];

              diff = (clock() - start) * 1000 / CLOCKS_PER_SEC;
              total += diff;
       }
       printf("Regular: %ld\n", total / ATTEMPT_NUM);


       //Blocked multiplication
       for (BLOCKSIZE = 1; BLOCKSIZE < 1024; BLOCKSIZE ++){
       total = 0;
       for (attempt = 0; attempt < ATTEMPT_NUM; attempt++){
              array_set(C2, array_func_zero);
              cache_clear();
              start = clock();
              for (block_j = 0; block_j < SIZE; block_j += BLOCKSIZE) { //X-dimension of B
                     for (block_k = 0; block_k < SIZE; block_k += BLOCKSIZE) { //X-dimension of A or Y-dimension of B
                            for (i = 0; i < SIZE; i++) { //X-dimension of C
                                   size_j = MIN(block_j + BLOCKSIZE, SIZE);
                                   size_k = MIN(block_k + BLOCKSIZE, SIZE);
                                   for (j = block_j; j < size_j; j++) { //Y-dimension of C
                                          sum = 0;
                                          for (k = block_k; k < size_k; k++) { //Mult dimension of block
                                                 sum += A[i][k] * B[k][j];
                                          }
                                          C2[i][j] += sum;
                                   }
                            }
                     }
              }
              diff = (clock() - start) * 1000 / CLOCKS_PER_SEC;
              total += diff;
              if (array_cmp(C1, C2))
                     printf("Fuck\n");
       }
       printf("%d %ld\n", BLOCKSIZE, total / ATTEMPT_NUM);
       }
       return 0;
}
