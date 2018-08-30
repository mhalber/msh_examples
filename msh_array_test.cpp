#define MSH_STD_IMPLEMENTATION
#include "msh/msh_std.h"
#include <vector>

#define N_ELEMS 1024*1024*100

int 
main( int argc, char** argv )
{

  uint64_t t1, t2;

  t1 = msh_time_now();
  std::vector<int> vec;
  for( int i = 0 ; i < N_ELEMS; ++i )
  {
    vec.push_back(i);
  }
  t2 = msh_time_now();
  printf("std_vector: Pushing %d elems took: %fms\n", N_ELEMS, msh_time_diff(MSHT_MILLISECONDS, t2, t1));


  t1 = msh_time_now();
  msh_array(int) arr = 0;
  for( int i = 0 ; i < N_ELEMS; ++i )
  {
    msh_array_push(arr, i);
  }
  t2 = msh_time_now();
  printf("msh_array:  Pushing %d elems took: %fms\n", N_ELEMS, msh_time_diff(MSHT_MILLISECONDS, t2, t1));

  return 0;
}