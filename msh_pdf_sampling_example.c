/*
  Author: Maciej Halber
  Date : Sep 1, 2018
  License: CC0
 
  Compilation: gcc -std=c99 -I<path_to_msh_libraries> msh_pdf_sampling_example.c -o msh_pdf_sampling_example
  Usage:       msh_pdf_sampling_example <path_to_ply_file
  Description: This program showcases different ways in which it is possible to sample discrete 
               distributions using msh libraries. The problem we try to tackle is essentially
               simulating a loaded dice - given set of weights describing likelihood of rolling
               specific side of a dice we wish to obtain a random index that follows the same
               distribution as our likelihoods. This extends to an ability to sample from a 
               discrete probability distribution.

               msh_std.h implements three ways to do this:
               
               1) Linear search - requires no setup, other than normalizing the distribution.
               Generating the random sample from a distribution can take O(n) time. Use only
               for very small distributions.
               
               2) Inverted CDF - Works by computing an approximation of inverted cumulative 
               distribution function. Setup takes O(m) time, where m is discretization of 
               probabilites. Sampling takes O(1) time. Issue with this method is the discretization
               of probabilities to integers. Depending on your distribution m might need to be 
               very large. To see this method fail, decrease A_INVCDF_N_BINS and inspect the printed
               values

               3) Alias method - preferable way to do sampling. Requires O(n) setup, generation 
               takes O(1) time (although we need to compute two random numbers instead of one).
               Implementation of Vose's algorithm, after description by Keith Schwartz: 
               http://www.keithschwarz.com/darts-dice-coins/

               This program will perform simulation of loaded dice and sampling from a mixture of 
               gaussian distribution, using all three methods. Timings will also be performed.
*/


#define MSH_STD_INCLUDE_HEADERS
#define MSH_STD_IMPLEMENTATION
#include "msh_std.h"

enum { A_N_ELEMS = 10,   A_N_SAMPLES = 1000000, A_INVCDF_N_BINS = 4096 };
enum { B_N_ELEMS = 8196, B_N_BINS = 64, B_N_SAMPLES = 100000, B_INVCDF_N_BINS = 8196 };

void print_histogram( double* hist, int n_bins )
{
  double max = -1e9;
  for( int i = 0; i < n_bins; ++i ) {  max = msh_max(max, hist[i]); }
  for( int i = 0; i < n_bins; ++i ) { hist[i] /= max; }

  int n_rows = 24;
  int n_cols = n_bins;
  for( int r = 0; r < n_rows; ++r )
  {
    for( int c = 0; c < n_cols; ++c )
    {
      int h = n_rows * hist[c];
      if( h > n_rows - r) printf("#", h);
      else printf(" ");
    }
    printf("\n");
  }
}

void print_array_with_title( double* arr, int n_elems, char* title )
{
  printf("  %s ", title );
  for( int i = 0; i < A_N_ELEMS; ++i )
  {
    printf("%8.4f ", arr[i] );
  }
  printf("\n");
}

void print_bin_counts( int* arr, int n_elems )
{
  printf("  Bin counts:  ");
  for( int i = 0; i < A_N_ELEMS; ++i )
  {
    printf("%8d ", arr[i] );
  }
  printf("\n");
}

void print_bin_counts_as_weights( int* arr, int n_elems, int n_samples )
{
  printf("  Bin weights: ");
  for( int i = 0; i < A_N_ELEMS; ++i )
  {
    printf("%8.4f ", arr[i]/(float)n_samples );
  }
  printf("\n");
}

int main( void )
{
  uint64_t t1, t2;
  double te = 0;
  msh_rand_ctx_t rand_gen = {0};
  msh_rand_init(&rand_gen, 7123ULL);

  int dice_rolls[6] = {0};

  const int n_rolls = 100000;
  t1 = msh_time_now();
  for( int i = 0 ; i < n_rolls; ++i )
  {
    int side_idx = (int) (msh_rand_nextf( &rand_gen ) * 6);
    dice_rolls[side_idx]++;
  }
  t2 = msh_time_now();
  te = msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
  printf("Simulating %d fair 6-sided dice rolls: %fms\n", n_rolls, te );

  printf("Fair dice roll distribution: ");
  for( int i = 0; i < 6; ++i )
  {
    printf("%5.3f%% ", dice_rolls[i]/(float)n_rolls * 100.0f );
  }
  printf("\n\n");


  printf( "Simulating a loaded %d sided loaded dice with following weights:\n", A_N_ELEMS );
  double weights[ A_N_ELEMS ] = {0};
  double distrib[ A_N_ELEMS ] = {0};
  weights[0] = 0.01;
  weights[1] = 1;
  weights[2] = 0.001;
  weights[3] = 1;
  weights[4] = 0;
  weights[5] = 1;
  weights[6] = 20.0;
  weights[7] = 100.0;
  weights[8] = 45.0;
  weights[9] = 1;
  msh_distrib2pdf( weights, distrib, A_N_ELEMS );
  print_array_with_title( weights, A_N_ELEMS, "Weights:      " );
  print_array_with_title( distrib, A_N_ELEMS, "Distribution: " );
  printf("\n");

//----

  msh_discrete_distrib_t sampling_ctx = {0};
  msh_discrete_distribution_init( &sampling_ctx, distrib, A_N_ELEMS, 7123ULL );
  t1 = msh_time_now();
  int bin_counts_alias[A_N_ELEMS] = {0};
  for( int i = 0; i < A_N_SAMPLES; ++i )
  {
    int idx = msh_discrete_distribution_sample( &sampling_ctx );
    bin_counts_alias[idx]++;
  }
  t2 = msh_time_now();
  te = msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
  printf("  Alias sampling (%fms):\n", te);
  print_bin_counts( bin_counts_alias, A_N_ELEMS );
  print_bin_counts_as_weights( bin_counts_alias, A_N_ELEMS, A_N_SAMPLES );
  printf("\n");

  msh_discrete_distribution_free( &sampling_ctx );

//----

  double pdf[A_N_ELEMS] = {0};
  msh_distrib2pdf( distrib, pdf, A_N_ELEMS );

  t1 = msh_time_now();
  int bin_counts_linear[A_N_ELEMS] = {0};
  for( int i = 0; i < A_N_SAMPLES; ++i )
  {
    double p = msh_rand_nextf( &rand_gen );
    int idx = msh_pdfsample_linear( pdf, p, A_N_ELEMS );
    bin_counts_linear[idx]++; 
  }
  t2 = msh_time_now();
  te = msh_time_diff(MSHT_MILLISECONDS,t2,t1);
  printf("  Linear sampling (%fms):\n", te);
  print_bin_counts( bin_counts_linear, A_N_ELEMS );
  print_bin_counts_as_weights( bin_counts_linear, A_N_ELEMS, A_N_SAMPLES );
  printf("\n");

//----

  double cdf[A_N_ELEMS] = {0};
  double invcdf[A_INVCDF_N_BINS] = {0};
  msh_pdf2cdf( pdf, cdf, A_N_ELEMS );
  msh_invert_cdf( cdf, A_N_ELEMS, invcdf, A_INVCDF_N_BINS );
  t1 = msh_time_now();
  int bin_counts_invcdf[A_N_ELEMS] = {0};
  for( int i = 0; i < A_N_SAMPLES; ++i )
  {
    double p = msh_rand_nextf( &rand_gen );
    int idx = msh_pdfsample_invcdf( invcdf, p, A_INVCDF_N_BINS );
    bin_counts_invcdf[idx]++;
  }
  t2 = msh_time_now();
  te = msh_time_diff(MSHT_MILLISECONDS,t2,t1);
  printf("  Inv. CDF sampling (%fms):\n", te);
  print_bin_counts( bin_counts_invcdf, A_N_ELEMS );
  print_bin_counts_as_weights( bin_counts_invcdf, A_N_ELEMS, A_N_SAMPLES );
  printf("\n");

//=============================

  printf("\nSampling from a discretized mixture of gaussians:\n");
  double f[B_N_ELEMS] = {0};
  for( int i = 0; i < B_N_ELEMS; ++i )
  {
    double a = msh_gauss1d( i, B_N_ELEMS/2, 1500 );
    double b = msh_gauss1d( i, B_N_ELEMS/4, 250 );
    double c = msh_gauss1d( i, 7*B_N_ELEMS/8, 125 );
    f[i] = a + b + c;
  }

  msh_discrete_distribution_init( &sampling_ctx, f, B_N_ELEMS, 7123ULL );
  t1 = msh_time_now();
  double hist_alias[B_N_BINS] = {0};
  for( int i = 0; i < B_N_SAMPLES; ++i )
  {
    int idx = msh_discrete_distribution_sample( &sampling_ctx );
    int b = ((double)idx / (double)B_N_ELEMS) * B_N_BINS;
    hist_alias[b]++;
  }
  t2 = msh_time_now();
  te = msh_time_diff( MSHT_MILLISECONDS, t2, t1 );
  printf("Alias sampling (%fms):\n", te);
  print_histogram( hist_alias, B_N_BINS );
  printf("\n");

  msh_discrete_distribution_free( &sampling_ctx );

//----

  double f_pdf[B_N_ELEMS] = {0};
  msh_distrib2pdf( f, f_pdf, B_N_ELEMS );

  t1 = msh_time_now();
  double hist_linear[B_N_BINS] = {0};
  for( int i = 0; i < B_N_SAMPLES; ++i )
  {
    double p = msh_rand_nextf( &rand_gen );
    int idx = msh_pdfsample_linear( f_pdf, p, B_N_ELEMS );
    int b = ((double)idx / (double)B_N_ELEMS) * B_N_BINS;
    hist_linear[b]++; 
  }
  t2 = msh_time_now();
  te = msh_time_diff(MSHT_MILLISECONDS,t2,t1);
  printf("Linear sampling (%fms):\n", te);
  print_histogram( hist_linear, B_N_BINS );
  printf("\n");

//----

  double f_cdf[B_N_ELEMS] = {0};
  double f_invcdf[B_INVCDF_N_BINS] = {0};
  msh_pdf2cdf( f_pdf, f_cdf, B_N_ELEMS );
  msh_invert_cdf( f_cdf, B_N_ELEMS, f_invcdf, B_INVCDF_N_BINS );
  t1 = msh_time_now();
  double hist_invcdf[B_N_BINS] = {0};
  for( int i = 0; i < B_N_SAMPLES; ++i )
  {
    double p = msh_rand_nextf( &rand_gen );
    int idx = msh_pdfsample_invcdf( f_invcdf, p, B_INVCDF_N_BINS );
    int b = ((double)idx / (double)B_N_ELEMS) * B_N_BINS;
    hist_invcdf[b]++; 
  }
  t2 = msh_time_now();
  te = msh_time_diff(MSHT_MILLISECONDS,t2,t1);
  printf("Inv. CDF sampling (%fms):\n", te);
  print_histogram( hist_invcdf, B_N_BINS );
  printf("\n");


}
