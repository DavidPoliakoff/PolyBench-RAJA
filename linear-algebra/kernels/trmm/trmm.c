/**
 * trmm.c: This file is part of the PolyBench 3.0 test suite.
 *
 *
 * Contact: Louis-Noel Pouchet <pouchet@cse.ohio-state.edu>
 * Web address: http://polybench.sourceforge.net
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include polybench common header. */
#include <polybench.h>

/* Include benchmark-specific header. */
/* Default data type is double, default size is 4000. */
#include "trmm.h"


/* Array initialization. */
static
void init_array(int ni,
		DATA_TYPE *alpha,
		DATA_TYPE POLYBENCH_2D(A,NI,NI),
		DATA_TYPE POLYBENCH_2D(B,NI,NI))
{
  int i, j;

  *alpha = 32412;
  for (i = 0; i < ni; i++)
    for (j = 0; j < ni; j++) {
      A[i][j] = ((DATA_TYPE) i*j) / ni;
      B[i][j] = ((DATA_TYPE) i*j) / ni;
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int ni,
		 DATA_TYPE POLYBENCH_2D(B,NI,NI))
{
  int i, j;

  for (i = 0; i < ni; i++)
    for (j = 0; j < ni; j++) {
	fprintf (stderr, DATA_PRINTF_MODIFIER, B[i][j]);
	if (i % 20 == 0) fprintf (stderr, "\n");
    }
  fprintf (stderr, "\n");
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_trmm(int ni,
		 DATA_TYPE alpha,
		 DATA_TYPE POLYBENCH_2D(A,NI,NI),
		 DATA_TYPE POLYBENCH_2D(B,NI,NI))
{
  int i, j, k;

#pragma scop
  /*  B := alpha*A'*B, A triangular */
  for (i = 1; i < ni; i++)
    for (j = 0; j < ni; j++)
      for (k = 0; k < i; k++)
        B[i][j] += alpha * A[i][k] * B[j][k];
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int ni = NI;

  /* Variable declaration/allocation. */
  DATA_TYPE alpha;
#ifdef POLYBENCH_HEAP_ARRAYS
  /* Heap arrays use variable 'n' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(A, ni, ni);
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(B, ni, ni);
  A = POLYBENCH_ALLOC_2D_ARRAY(ni, ni, DATA_TYPE);
  B = POLYBENCH_ALLOC_2D_ARRAY(ni, ni, DATA_TYPE);
#else
  /* Stack arrays use the numerical value 'N' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(A,NI,NI);
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(B,NI,NI);
#endif

  /* Initialize array(s). */
  init_array (ni, &alpha, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(B));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_trmm (ni, alpha, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(B));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(ni, POLYBENCH_ARRAY(B)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(B);

  return 0;
}