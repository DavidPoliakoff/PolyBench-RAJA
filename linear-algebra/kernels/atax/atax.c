/**
 * atax.c: This file is part of the PolyBench 3.0 test suite.
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
#include "atax.h"


/* Array initialization. */
static
void init_array (int nx, int ny,
		 DATA_TYPE POLYBENCH_2D(A,NX,NY),
		 DATA_TYPE POLYBENCH_1D(x,NY))
{
  int i, j;

  for (i = 0; i < ny; i++)
      x[i] = i * M_PI;
  for (i = 0; i < nx; i++)
    for (j = 0; j < ny; j++)
      A[i][j] = ((DATA_TYPE) i*(j+1)) / nx;
}


/* DCE code. Must scan the entire live-out data.
   Can be used also to check the correctness of the output. */
static
void print_array(int nx,
		 DATA_TYPE POLYBENCH_1D(y,NX))

{
  int i;

  for (i = 0; i < nx; i++) {
    fprintf (stderr, DATA_PRINTF_MODIFIER, y[i]);
    if (i % 20 == 0) fprintf (stderr, "\n");
  }
  fprintf (stderr, "\n");
}


/* Main computational kernel. The whole function will be timed,
   including the call and return. */
static
void kernel_atax(int nx, int ny,
		 DATA_TYPE POLYBENCH_2D(A,NX,NY),
		 DATA_TYPE POLYBENCH_1D(x,NY),
		 DATA_TYPE POLYBENCH_1D(y,NX),
		 DATA_TYPE POLYBENCH_1D(tmp,NY))
{
  int i, j;

#pragma scop
  for (i= 0; i < nx; i++)
    y[i] = 0;
  for (i = 0; i < ny; i++)
    {
      tmp[i] = 0;
      for (j = 0; j < ny; j++)
	tmp[i] = tmp[i] + A[i][j] * x[j];
      for (j = 0; j < ny; j++)
	y[j] = y[j] + A[i][j] * tmp[i];
    }
#pragma endscop

}


int main(int argc, char** argv)
{
  /* Retrieve problem size. */
  int nx = NX;
  int ny = NY;

  /* Variable declaration/allocation. */
#ifdef POLYBENCH_HEAP_ARRAYS
  /* Heap arrays use variable 'n' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(A, nx, ny);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(x, ny);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(y, nx);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(tmp, ny);
  A = POLYBENCH_ALLOC_2D_ARRAY(nx, ny, DATA_TYPE);
  x = POLYBENCH_ALLOC_1D_ARRAY(ny, DATA_TYPE);
  y = POLYBENCH_ALLOC_1D_ARRAY(nx, DATA_TYPE);
  tmp = POLYBENCH_ALLOC_1D_ARRAY(ny, DATA_TYPE);
#else
  /* Stack arrays use the numerical value 'N' for the size. */
  DATA_TYPE POLYBENCH_2D_ARRAY_DECL(A, NX, NY);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(x, NY);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(y, NX);
  DATA_TYPE POLYBENCH_1D_ARRAY_DECL(tmp, NY);
#endif

  /* Initialize array(s). */
  init_array (nx, ny, POLYBENCH_ARRAY(A), POLYBENCH_ARRAY(x));

  /* Start timer. */
  polybench_start_instruments;

  /* Run kernel. */
  kernel_atax (nx, ny,
	       POLYBENCH_ARRAY(A),
	       POLYBENCH_ARRAY(x),
	       POLYBENCH_ARRAY(y),
	       POLYBENCH_ARRAY(tmp));

  /* Stop and print timer. */
  polybench_stop_instruments;
  polybench_print_instruments;

  /* Prevent dead-code elimination. All live-out data must be printed
     by the function call in argument. */
  polybench_prevent_dce(print_array(nx, POLYBENCH_ARRAY(y)));

  /* Be clean. */
  POLYBENCH_FREE_ARRAY(A);
  POLYBENCH_FREE_ARRAY(x);
  POLYBENCH_FREE_ARRAY(y);
  POLYBENCH_FREE_ARRAY(tmp);

  return 0;
}