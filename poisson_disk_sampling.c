#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_ARGPARSE_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#define MSH_HASH_GRID_IMPLEMENTATION

#include "msh/msh_std.h"
#include "msh/msh_argparse.h"
#include "msh/msh_vec_math.h"
#include "msh/msh_hash_grid.h"

typedef struct options
{
  char* input_ply_filename;
  char* output_ply_filename;

  real32_t subsample_radius;
} opts_t;


/* TODO(maciej): Move this to a separate msh_ lib? */
typedef struct indexed_mesh
{
  real32_t* positions;
  int32_t* face_indices;

  size_t n_vertices;
  size_t n_faces;
} indexed_mesh_t;

typedef enum indexed_mesh_error_codes
{
  INDEXED_MESH_NO_ERR,
  INDEXED_MESH_INIT_FAILURE_ERR
} indexed_mesh_error_t;

int
indexed_mesh_init( indexed_mesh_t* mesh, size_t n_vertices, size_t n_faces )
{
  mesh->n_vertices   = n_vertices;
  mesh->n_faces      = n_faces;
  mesh->positions    = (real32_t*)malloc( n_vertices * 3 * sizeof(real32_t) );
  mesh->face_indices = (int32_t*)malloc( n_faces * 3 * sizeof(real32_t) );
  if( !mesh->positions || !mesh->face_indices ) { return INDEXED_MESH_INIT_FAILURE_ERR; }
  return INDEXED_MESH_NO_ERR;
}

void
indexed_mesh_free( indexed_mesh_t* mesh, size_t n_vertices, size_t n_faces )
{
  mesh->n_vertices = 0;
  mesh->n_faces    = 0;
  free( mesh->positions );
  free( mesh->face_indices );
}

int
indexed_mesh_load( indexed_mesh_t mesh, const char* filename )
{

}

char*
indexed_mesh_get_error_msg( int32_t error_code )
{
  switch( error_code )
  {
    case INDEXED_MESH_NO_ERR:
      return "No Errors";
    case INDEXED_MESH_INIT_FAILURE_ERR:
      return "Failed to initialize memory for indexed mesh.";
    default:
      return "No Errors";
  }
}


// void
// mesh_uniform_sampling( const trimesh_t* input_mesh, trimesht_t* output_mesh, real32_t density )
// {

// }

// void
// pointcloud_poisson_disk_sampling( const trimesh_t* input_pointcloud, trimesh_t* output_pointcloud, 
//                                   real32_t radius )
// {

// }

// void
// mesh_poisson_disk_sampling( const trimesh_t* input_mesh, trimesh_t* output_mesh, real32_t radius )
// {
//   trimesh_t* tmp_pointcloud = {0};
//   mesh_uniform_sampling( input_mesh, tmp_pointcloud, density );
//   pointcloud_poisson_disk_sampling( tmp_pointcloud, output_mesh, radius );

// }

int8_t
parse_arguments( int argc, char** argv, opts_t* opts )
{

}

int32_t 
main( int32_t argc, char** argv )
{

}
