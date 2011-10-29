/**
 * covariance.c: This file is part of the PolyBench 3.0 test suite.
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
#include "covariance.h"


/* Array initialization. */
static
void init_array (int n,
		 DATA_TYPE *float_n,
		 DATA_TYPE POLYBENCH_2D(data,M,N))
{
  int i, j;

  *float_n = 1.2;

  for (i = 0; i < M; i++)
    for (j = 0; j < N; j++)
      data[i][j] = ((DATA_TYPE) i*j) / M;
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_2D(symmat,M,M))

{
  int i, j;

  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      fprintf (stderr, DATA_PRINTF_MODIFIER, symmat[i][j]);
      if ((i * n + j) % 20 == 0) fprintf (stderr, "\n");
    }
  fprintf (stderr, "\n");
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_covariance(int m, int n,
		       DATA_TYPE float_n,
		       DATA_TYPE POLYBENCH_2D(data,M,N),
		       DATA_TYPE POLYBENCH_2D(symmat,M,M),
		       DATA_TYPE POLYBENCH_1D(mean,M))
{
  int i, j, j1, j2;

#pragma scop
  /* Determine mean of column vectors of input data matrix */
  for (j = 0; j < m; j++)
    {
      mean[j] = 0.0;
      for (i = 0; i < n; i++)
        mean[j] += data[i][j];
      mean[j] /= float_n;
    }

  /* Center the column vectors. */
  for (i = 0; i < n; i++)
    for (j = 0; j < m; j++)
      data[i][j] -= mean[j];

  /* Calculate the m * m covariance matrix. */
  for (j1 = 0; j1 < m; j1++)
    for (j2 = j1; j2 < m; j2++)
      {
        symmat[j1][j2] = 0.0;
        for (i = 0; i < n; i++)
	  symmat[j1][j2] += data[i][j1] * data[i][j2];
        symmat[j2][j1] = symmat[j1][j2];
      }
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int n = N;
  int m = M;

  /* Variable declaration/allocation. */
  DATA_TYPE float_n;
#ifdef POLYBENCH_HEAP_ARRAYS
  /* Heap arrays use variable 'n' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(data, m, n);
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(symmat, m, m);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(mean, m);
  data = POLYBENCH_ALLOC_2D_ARRAY(m, n, DATA_TYPE);
  symmat = POLYBENCH_ALLOC_2D_ARRAY(m, m, DATA_TYPE);
  mean = POLYBENCH_ALLOC_1D_ARRAY(m, DATA_TYPE);
#else
  /* Stack arrays use the numerical value 'N' for the size. */
  DATA_TYPE POLYBENCH_2D(data,M,N);
  DATA_TYPE POLYBENCH_2D(symmat,M,M);
  DATA_TYPE POLYBENCH_1D(mean,M);
#endif


  /* Initialize array(s). */
  init_array (n, &float_n, POLYBENCH_ARRAY(data));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_covariance (m, n, float_n,
		     POLYBENCH_ARRAY(data),
		     POLYBENCH_ARRAY(symmat),
		     POLYBENCH_ARRAY(mean));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(m, POLYBENCH_ARRAY(symmat)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(data);
  POLYBENCH_FREE_ARRAY(symmat);
  POLYBENCH_FREE_ARRAY(mean);

  return 0;
}