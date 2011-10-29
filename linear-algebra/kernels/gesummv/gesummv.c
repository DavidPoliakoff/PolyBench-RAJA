/**
 * gesummv.c: This file is part of the PolyBench 3.0 test suite.
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
#include "gesummv.h"


/* Array initialization. */
static
void init_array(int n,
		DATA_TYPE *alpha,
		DATA_TYPE *beta,
		DATA_TYPE POLYBENCH_2D(A,N,N),
		DATA_TYPE POLYBENCH_2D(B,N,N),
		DATA_TYPE POLYBENCH_1D(x,N))
{
  int i, j;

  *alpha = 43532;
  *beta = 12313;
  for (i = 0; i < n; i++)
    {
      x[i] = ((DATA_TYPE) i) / n;
      for (j = 0; j < n; j++) {
	A[i][j] = ((DATA_TYPE) i*j) / n;
	B[i][j] = ((DATA_TYPE) i*j) / n;
      }
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_1D(y,N))

{
  int i;

  for (i = 0; i < n; i++) {
    fprintf (stderr, DATA_PRINTF_MODIFIER, y[i]);
    if (i % 20 == 0) fprintf (stderr, "\n");
  }
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_gesummv(int n,
		    DATA_TYPE alpha,
		    DATA_TYPE beta,
		    DATA_TYPE POLYBENCH_2D(A,N,N),
		    DATA_TYPE POLYBENCH_2D(B,N,N),
		    DATA_TYPE POLYBENCH_1D(tmp,N),
		    DATA_TYPE POLYBENCH_1D(x,N),
		    DATA_TYPE POLYBENCH_1D(y,N))
{
  int i, j;

#pragma scop
  for (i = 0; i < n; i++)
    {
      tmp[i] = 0;
      y[i] = 0;
      for (j = 0; j < n; j++)
	{
	  tmp[i] = A[i][j] * x[j] + tmp[i];
	  y[i] = B[i][j] * x[j] + y[i];
	}
      y[i] = alpha * tmp[i] + beta * y[i];
    }
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int n = N;

  /* Variable declaration/allocation. */
  DATA_TYPE alpha;
  DATA_TYPE beta;
#ifdef POLYBENCH_HEAP_ARRAYS
  /* Heap arrays use variable 'n' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(A, n, n);
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(B, n, n);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(tmp, n);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(x, n);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(y, n);
  A = POLYBENCH_ALLOC_2D_ARRAY(n, n, DATA_TYPE);
  B = POLYBENCH_ALLOC_2D_ARRAY(n, n, DATA_TYPE);
  tmp = POLYBENCH_ALLOC_1D_ARRAY(n, DATA_TYPE);
  x = POLYBENCH_ALLOC_1D_ARRAY(n, DATA_TYPE);
  y = POLYBENCH_ALLOC_1D_ARRAY(n, DATA_TYPE);
#else
  /* Stack arrays use the numerical value 'N' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(A, N, N);
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(B, N, N);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(tmp, N);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(x, N);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(y, N);
#endif


  /* Initialize array(s). */
  init_array (n, &alpha, &beta,
	      POLYBENCH_ARRAY(A),
	      POLYBENCH_ARRAY(B),
	      POLYBENCH_ARRAY(x));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_gesummv (n, alpha, beta,
		  POLYBENCH_ARRAY(A),
		  POLYBENCH_ARRAY(B),
		  POLYBENCH_ARRAY(tmp),
		  POLYBENCH_ARRAY(x),
		  POLYBENCH_ARRAY(y));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(n, POLYBENCH_ARRAY(y)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(B);
  POLYBENCH_FREE_ARRAY(tmp);
  POLYBENCH_FREE_ARRAY(x);
  POLYBENCH_FREE_ARRAY(y);

  return 0;
}