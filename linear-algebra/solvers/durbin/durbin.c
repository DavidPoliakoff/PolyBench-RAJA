/**
 * durbin.c: This file is part of the PolyBench 3.0 test suite.
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
#include "durbin.h"


/* Array initialization. */
static
void init_array (int n,
		 DATA_TYPE POLYBENCH_2D(y,N,N),
		 DATA_TYPE POLYBENCH_2D(sum,N,N),
		 DATA_TYPE POLYBENCH_1D(alpha,N),
		 DATA_TYPE POLYBENCH_1D(beta,N),
		 DATA_TYPE POLYBENCH_1D(r,N))
{
  int i, j;

  for (i = 0; i < n; i++)
    {
      alpha[i] = i;
      beta[i] = (i+1)/n/2.0;
      r[i] = (i+1)/n/4.0;
      for (j = 0; j < n; j++) {
	y[i][j] = ((DATA_TYPE) i*j) / n;
	sum[i][j] = ((DATA_TYPE) i*j) / n;
      }
    }
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int n,
		 DATA_TYPE POLYBENCH_1D(out,N))

{
  int i;

  for (i = 0; i < n; i++) {
    fprintf (stderr, DATA_PRINTF_MODIFIER, out[i]);
    if (i % 20 == 0) fprintf (stderr, "\n");
  }
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_durbin(int n,
		   DATA_TYPE POLYBENCH_2D(y,N,N),
		   DATA_TYPE POLYBENCH_2D(sum,N,N),
		   DATA_TYPE POLYBENCH_1D(alpha,N),
		   DATA_TYPE POLYBENCH_1D(beta,N),
		   DATA_TYPE POLYBENCH_1D(r,N),
		   DATA_TYPE POLYBENCH_1D(out,N))
{
  int i, k;

#pragma scop
  y[0][0] = r[0];
  beta[0] = 1;
  alpha[0] = r[0];
  for (k = 1; k < n; k++)
    {
      beta[k] = beta[k-1] - alpha[k-1] * alpha[k-1] * beta[k-1];
      sum[0][k] = r[k];
      for (i = 0; i <= k - 1; i++)
	sum[i+1][k] = sum[i][k] + r[k-i-1] * y[i][k-1];
      alpha[k] = -sum[k][k] * beta[k];
      for (i = 0; i <= k-1; i++)
	y[i][k] = y[i][k-1] + alpha[k] * y[k-i-1][k-1];
      y[k][k] = alpha[k];
    }
  for (i = 0; i < n; i++)
    out[i] = y[i][N-1];
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int n = N;

  /* Variable declaration/allocation. */
#ifdef POLYBENCH_HEAP_ARRAYS
  /* Heap arrays use variable 'n' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(y, n, n);
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(sum, n, n);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(alpha, n);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(beta, n);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(r, n);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(out, n);
  y = POLYBENCH_ALLOC_2D_ARRAY(n, n, DATA_TYPE);
  sum = POLYBENCH_ALLOC_2D_ARRAY(n, n, DATA_TYPE);
  alpha = POLYBENCH_ALLOC_1D_ARRAY(n, DATA_TYPE);
  beta = POLYBENCH_ALLOC_1D_ARRAY(n, DATA_TYPE);
  r = POLYBENCH_ALLOC_1D_ARRAY(n, DATA_TYPE);
  out = POLYBENCH_ALLOC_1D_ARRAY(n, DATA_TYPE);
#else
  /* Stack arrays use the numerical value 'N' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(y, N, N);
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(sum, N, N);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(alpha, N);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(beta, N);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(r, N);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(out, N);
#endif


  /* Initialize array(s). */
  init_array (n,
	      POLYBENCH_ARRAY(y),
	      POLYBENCH_ARRAY(sum),
	      POLYBENCH_ARRAY(alpha),
	      POLYBENCH_ARRAY(beta),
	      POLYBENCH_ARRAY(r));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_durbin (n,
		 POLYBENCH_ARRAY(y),
		 POLYBENCH_ARRAY(sum),
		 POLYBENCH_ARRAY(alpha),
		 POLYBENCH_ARRAY(beta),
		 POLYBENCH_ARRAY(r),
		 POLYBENCH_ARRAY(out));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(n, POLYBENCH_ARRAY(out)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(y);
  POLYBENCH_FREE_ARRAY(sum);
  POLYBENCH_FREE_ARRAY(alpha);
  POLYBENCH_FREE_ARRAY(beta);
  POLYBENCH_FREE_ARRAY(r);
  POLYBENCH_FREE_ARRAY(out);

  return 0;
}
