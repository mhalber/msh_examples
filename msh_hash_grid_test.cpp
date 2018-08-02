#define MSH_STD_IMPLEMENTATION
#define MSH_PLY_IMPLEMENTATION
#define MSH_HASH_GRID_IMPLEMENTATION
#include <cmath>
#include "flann/flann.hpp"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include "msh/msh_std.h"
#include "msh/msh_ply.h"
#include "msh/experimental/msh_hash_grid.h"

typedef flann::Index<flann::L2_Simple<float>> flann_index_t;

typedef struct pointcloud
{
  int n_pts;
  msh_hg_v3_t* pts;
} PointCloud;

void read_ply( const char* filename, PointCloud* pc )
{
  msh_ply_t* pf = msh_ply_open( filename, "rb");
  if( pf )
  {
    const char* positions_names[] = { "x", "y", "z" };
    msh_ply_desc_t vertex_desc = { .element_name = (char*)"vertex",
                                   .property_names = positions_names,
                                   .num_properties = 3,
                                   .data_type = MSH_PLY_FLOAT,
                                   .list_type = MSH_PLY_INVALID,
                                   .data = &pc->pts,
                                   .list_data = 0,
                                   .data_count = &pc->n_pts,
                                   .list_size_hint = 0 };
    msh_ply_add_descriptor( pf, &vertex_desc );
    int test = msh_ply_read(pf);
  }
  msh_ply_close(pf);
}

int main( int argc, char** argv )
{
  float radius = 20.1f;
  printf("This program is an example of a spatial hashtable use\n");
  uint64_t t2, t1;
  t1 = msh_time_now();
  PointCloud pc = {};
  if( argc <= 1 ) { printf("Please priovide path to a .ply file to read\n"); }
  read_ply( argv[argc-1], &pc );
  t2 = msh_time_now();
  printf("Reading ply file with %d pts took %fms.\n", pc.n_pts, msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  msh_hash_grid_t spatial_table;
  t1 = msh_time_now();
  msh_hash_grid_init( &spatial_table, (float*)&pc.pts[0], pc.n_pts, radius );
  t2 = msh_time_now();
  printf("Table creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  t1 = msh_time_now();
  flann::Matrix<float> pts_mat((float*)&pc.pts[0], pc.n_pts, 3);
  flann_index_t* kdtree = new flann_index_t( pts_mat, flann::KDTreeSingleIndexParams(16));
  kdtree->buildIndex();
  t2 = msh_time_now();
  printf("KD-Tree creation: %fms.\n", msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  int n_query_pts = 10;
  float* query_pts = (float*)malloc( n_query_pts * sizeof(float) * 3 );
  for( int i = 0; i < n_query_pts; ++i )
  {
    int idx = rand() % pc.n_pts;
    query_pts[3*i+0] = pc.pts[idx].x;
    query_pts[3*i+1] = pc.pts[idx].y;
    query_pts[3*i+2] = pc.pts[idx].z;
  }

  float radius_sq = radius*radius;
  t1 = msh_time_now();
  int total_neigh_count_a = 0;
  int total_neigh_count_b = 0;
  int* neigh_count_a = (int*)malloc(n_query_pts * sizeof(int));
  int* neigh_count_b = (int*)malloc(n_query_pts * sizeof(int));
  #define MAX_N_NEIGH 2000
  int nn_inds[MAX_N_NEIGH] = {-1}; float nn_dists[MAX_N_NEIGH] = {1e9};
  int flann_inds[MAX_N_NEIGH] = {-1}; float flann_dists[MAX_N_NEIGH] = {1e9};\

  flann::SearchParams params = flann::SearchParams(128, 0, 1);
  params.max_neighbors = MAX_N_NEIGH;

  double timers[2] = {0.0, 0.0};

  for( int i = 0 ; i < n_query_pts; ++i )
  {
    uint64_t ct1, ct2;
    ct1 = msh_time_now();
    float* pt = &query_pts[3*i];
    neigh_count_a[i] = msh_hash_grid_radius_search( &spatial_table, pt, radius, nn_dists, nn_inds, MAX_N_NEIGH, 1 );
    total_neigh_count_a += neigh_count_a[i];
    ct2 = msh_time_now();
    timers[0] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );

    ct1 = msh_time_now();
    float *query = pt;
    flann::Matrix<float> query_mat(&query[0], 1, 3);
    flann::Matrix<int> indices_mat(&flann_inds[0], 1, MAX_N_NEIGH);
    flann::Matrix<float> dists_mat(&flann_dists[0], 1, MAX_N_NEIGH);
    neigh_count_b[i] = kdtree->radiusSearch( query_mat, indices_mat,
                                             dists_mat, radius_sq,
                                             params );
    total_neigh_count_b += neigh_count_b[i];
    ct2 = msh_time_now();
    timers[1] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );
  }
  t2 = msh_time_now();
  printf("RADIUS: Finding %d points using hash_grid.: %fms.\n", total_neigh_count_a, timers[0]);
  printf("RADIUS: Finding %d points using flann.: %fms.\n", total_neigh_count_b, timers[1]);

  total_neigh_count_a = 0;
  total_neigh_count_b = 0;
  int knn = 16;
  timers[0] = timers[1] = 0;
  for( int i = 0 ; i < n_query_pts; ++i )
  {
    uint64_t ct1, ct2;
    ct1 = msh_time_now();
    float* pt = &query_pts[3*i];
    neigh_count_a[i] = msh_hash_grid_knn_search( &spatial_table, pt, knn, nn_dists, nn_inds, 1 );
    total_neigh_count_a += neigh_count_a[i];
    ct2 = msh_time_now();
    timers[0] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );

    float *query = pt;
    flann::Matrix<float> query_mat(&query[0], 1, 3);
    flann::Matrix<int> indices_mat(&flann_inds[0], 1, MAX_N_NEIGH);
    flann::Matrix<float> dists_mat(&flann_dists[0], 1, MAX_N_NEIGH);
    ct1 = msh_time_now();
    neigh_count_b[i] = kdtree->knnSearch( query_mat, indices_mat,
                                          dists_mat, knn,
                                          params );
    total_neigh_count_b += neigh_count_b[i];
    ct2 = msh_time_now();
    timers[1] += msh_time_diff(MSHT_MILLISECONDS, ct2, ct1 );
    // if( neigh_count_b[i] != neigh_count_a[i] )
    // {
    //   printf("OOPS!\n");
    // }
    // else
    // {
    //   for( int k = 0; k < neigh_count_b[i]; ++k )
    //   {
    //     printf("(%d) %6d %6d | %f %f\n", (flann_inds[k]==nn_inds[k]),flann_inds[k], nn_inds[k], flann_dists[k], nn_dists[k] );
    //   }
    //   printf("----------\n");
    // }
  }
  printf("KNN: Finding %d points using hash_grid.: %fms.\n", total_neigh_count_a, timers[0]);
  printf("KNN: Finding %d points using flann.: %fms.\n", total_neigh_count_b, timers[1]);

  msh_hash_grid_term( &spatial_table );
  return 0;
}
