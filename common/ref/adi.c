/* adi.c: this file is part of PolyBench/C */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
/* Include polybench common header. */
#include <polybench.h>
/* Include benchmark-specific header. */
#include "adi.h"

static void init_array(int n, double u[N][N]) {
  int i, j;
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      u[i][j] = (double)(i + n - j) / n;
    }
}

static void print_array(int n, double u[N][N]) {
  int i, j;
  fprintf(stderr, "==BEGIN DUMP_ARRAYS==\n");
  fprintf(stderr, "begin dump: %s", "u");
  for (i = 0; i < n; i++)
    for (j = 0; j < n; j++) {
      if ((i * n + j) % 20 == 0) fprintf(stderr, "\n");
      fprintf(stderr, "%0.2lf ", u[i][j]);
    }
  fprintf(stderr, "\nend   dump: %s\n", "u");
  fprintf(stderr, "==END   DUMP_ARRAYS==\n");
}
# 49 "adi.c"

static void kernel_adi(int tsteps,
                       int n,
                       double u[N][N],
                       double v[N][N],
                       double p[N][N],
                       double q[N][N]) {
  int t, i, j;
  double DX, DY, DT;
  double B1, B2;
  double mul1, mul2;
  double a, b, c, d, e, f;
#pragma scop
  DX = 1.0 / (double)n;
  DY = 1.0 / (double)n;
  DT = 1.0 / (double)tsteps;
  B1 = 2.0;
  B2 = 1.0;
  mul1 = B1 * DT / (DX * DX);
  mul2 = B2 * DT / (DY * DY);
  a = -mul1 / 2.0;
  b = 1.0 + mul1;
  c = a;
  d = -mul2 / 2.0;
  e = 1.0 + mul2;
  f = d;
  for (t = 1; t <= tsteps; t++) {
    for (i = 1; i < n - 1; i++) {
      v[0][i] = 1.0;
      p[i][0] = 0.0;
      q[i][0] = v[0][i];
      for (j = 1; j < n - 1; j++) {
        p[i][j] = -c / (a * p[i][j - 1] + b);
        q[i][j] =
            (-d * u[j][i - 1] + (1.0 + 2.0 * d) * u[j][i] - f * u[j][i + 1]
             - a * q[i][j - 1])
            / (a * p[i][j - 1] + b);
      }
      v[n - 1][i] = 1.0;
      for (j = n - 2; j >= 1; j--) {
        v[j][i] = p[i][j] * v[j + 1][i] + q[i][j];
      }
    }
    for (i = 1; i < n - 1; i++) {
      u[i][0] = 1.0;
      p[i][0] = 0.0;
      q[i][0] = u[i][0];
      for (j = 1; j < n - 1; j++) {
        p[i][j] = -f / (d * p[i][j - 1] + e);
        q[i][j] =
            (-a * v[i - 1][j] + (1.0 + 2.0 * a) * v[i][j] - c * v[i + 1][j]
             - d * q[i][j - 1])
            / (d * p[i][j - 1] + e);
      }
      u[i][n - 1] = 1.0;
      for (j = n - 2; j >= 1; j--) {
        u[i][j] = p[i][j] * u[i][j + 1] + q[i][j];
      }
    }
  }
#pragma endscop
}

int main(int argc, char** argv) {
  int n = N;
  int tsteps = TSTEPS;
  double(*u)[N][N];
  u = (double(*)[N][N])polybench_alloc_data((N) * (N), sizeof(double));
  double(*v)[N][N];
  v = (double(*)[N][N])polybench_alloc_data((N) * (N), sizeof(double));
  double(*p)[N][N];
  p = (double(*)[N][N])polybench_alloc_data((N) * (N), sizeof(double));
  double(*q)[N][N];
  q = (double(*)[N][N])polybench_alloc_data((N) * (N), sizeof(double));
  init_array(n, *u);
  polybench_timer_start();
  kernel_adi(tsteps, n, *u, *v, *p, *q);
  polybench_timer_stop();
  polybench_timer_print();
  if (argc > 42 && !strcmp(argv[0], "")) print_array(n, *u);
  free((void*)u);
  free((void*)v);
  free((void*)p);
  free((void*)q);
  return 0;
}
