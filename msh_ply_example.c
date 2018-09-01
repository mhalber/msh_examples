/*
  Author: Maciej Halber
  Date : Jul 18, 2018
  License: CC0
 
  Compilation: gcc -std=c99 -I<path_to_msh_libraries> msh_ply_example.c -o msh_ply_example
  Usage:       msh_ply_example <path_to_ply_file
  Description: This is a simple program showcasing basic functionality of msh_ply.h library. It
               saves a simple cube file to disk, and reads it back. Program also showcases a 
               function that deliberately misuses the api in order to showcase error reporting.
*/

#define MSH_PLY_IMPLEMENTATION
#define MSH_PLY_INCLUDE_HEADERS
#include "msh_ply.h"

typedef struct Vec3f
{
  float x,y,z;
} Vec3f;

typedef struct Vec3i
{
  int x,y,z;
} Vec3i;

typedef struct TriMeshSimple
{
  Vec3f* vertices;
  Vec3i* faces;
  int n_vertices;
  int n_faces;
} TriMeshSimple;

typedef struct TriMesh
{
  Vec3f* positions;
  Vec3i* faces;
  int n_vertices;
  int n_faces;
} TriMesh;

void create_cube_simple( TriMeshSimple* mesh )
{
  mesh->n_vertices = 8;
  mesh->n_faces    = 12;
  mesh->vertices   = (Vec3f*)malloc( mesh->n_vertices * sizeof(Vec3f) );
  mesh->faces      = (Vec3i*)malloc( mesh->n_faces * sizeof(Vec3i) );

  mesh->vertices[0] = (Vec3f){ -1.0f, -1.0f, -1.0f };
  mesh->vertices[1] = (Vec3f){  1.0f, -1.0f, -1.0f };
  mesh->vertices[2] = (Vec3f){ -1.0f,  1.0f, -1.0f };
  mesh->vertices[3] = (Vec3f){  1.0f,  1.0f, -1.0f };
  mesh->vertices[4] = (Vec3f){ -1.0f, -1.0f,  1.0f };
  mesh->vertices[5] = (Vec3f){  1.0f, -1.0f,  1.0f };
  mesh->vertices[6] = (Vec3f){ -1.0f,  1.0f,  1.0f };
  mesh->vertices[7] = (Vec3f){  1.0f,  1.0f,  1.0f };

  mesh->faces[0]  = (Vec3i){ 0, 2, 1 };
  mesh->faces[1]  = (Vec3i){ 1, 2, 3 };
  mesh->faces[2]  = (Vec3i){ 0, 1, 4 };
  mesh->faces[3]  = (Vec3i){ 1, 5, 4 };
  mesh->faces[4]  = (Vec3i){ 0, 4, 2 };
  mesh->faces[5]  = (Vec3i){ 2, 4, 6 };
  mesh->faces[6]  = (Vec3i){ 2, 7, 3 };
  mesh->faces[7]  = (Vec3i){ 2, 6, 7 };
  mesh->faces[8]  = (Vec3i){ 3, 7, 1 };
  mesh->faces[9]  = (Vec3i){ 1, 7, 5 };
  mesh->faces[10] = (Vec3i){ 4, 5, 6 };
  mesh->faces[11] = (Vec3i){ 5, 7, 6 };
}

void
write_example_simple( const char* filename, TriMeshSimple* mesh )
{

  // Describe_simple the data and assign pointers
  msh_ply_desc_t verts_desc = { .element_name = "vertex",
                                .property_names = (const char*[]){"x", "y", "z"},
                                .num_properties = 3,
                                .data_type = MSH_PLY_FLOAT,
                                .data = &mesh->vertices,
                                .data_count = &mesh->n_vertices };

  msh_ply_desc_t faces_desc = { .element_name = "face",
                                .property_names = (const char*[]){"vertex_indices"},
                                .num_properties = 1,
                                .data_type = MSH_PLY_INT32,
                                .list_type = MSH_PLY_UINT8,
                                .data = &mesh->faces,
                                .data_count = &mesh->n_faces,
                                .list_size_hint = 3 };

  // Create ply file, add descriptors and write
  msh_ply_t* out_ply = msh_ply_open( filename, "wb" );
  msh_ply_add_descriptor( out_ply, &verts_desc );
  msh_ply_add_descriptor( out_ply, &faces_desc );
  msh_ply_write( out_ply );
  msh_ply_close( out_ply ); 
}

void
read_example_simple( const char* filename, TriMeshSimple *mesh )
{
  // Describe the data and assign pointers
  msh_ply_desc_t verts_desc = { .element_name = "vertex",
                                .property_names = (const char*[]){"x", "y", "z"},
                                .num_properties = 3,
                                .data_type = MSH_PLY_FLOAT,
                                .data = &mesh->vertices,
                                .data_count = &mesh->n_vertices };

  msh_ply_desc_t faces_desc = { .element_name = "face",
                                .property_names = (const char*[]){"vertex_indices"},
                                .num_properties = 1,
                                .data_type = MSH_PLY_INT32,
                                .list_type = MSH_PLY_UINT8,
                                .data = &mesh->faces,
                                .data_count = &mesh->n_faces,
                                .list_size_hint = 3 };

  // Create ply file, add descriptors and read
  msh_ply_t* in_ply = msh_ply_open( filename, "rb" );
  msh_ply_add_descriptor( in_ply, &verts_desc );
  msh_ply_add_descriptor( in_ply, &faces_desc );
  msh_ply_read( in_ply );
  msh_ply_close( in_ply ); 
}

int 
error_report_example( const char* filename, TriMeshSimple *mesh )
{
  msh_ply_t* fply = msh_ply_open( filename, "rb" );
  msh_ply_desc_t vertex_desc = { .element_name = "vertex",
                                 .property_names = (const char*[]){"x", "y", "z"},
                                 .num_properties = 3,
                                 .data = &mesh->vertices,
                                 /*.data_count = &mesh->n_vertices,  <== Missing data count */ 
                                 .data_type = MSH_PLY_FLOAT };
  
  int err = MSH_PLY_NO_ERRORS;
  err = msh_ply_add_descriptor( fply, &vertex_desc );
  if( err ) 
  { 
    printf( "%s\n", msh_ply_get_error_string( err) ); 
    msh_ply_close( fply ); 
    return 1; 
  }
  
  err = msh_ply_read( fply ); 
  if( err ) 
  {
    printf( "%s\n", msh_ply_get_error_string( err ) ); 
    msh_ply_close( fply ); 
    return 1;
  }

  msh_ply_close( fply );
  
  return 0;

}

int main( int argc, char** argv )
{
  if( argc < 2 ) { printf("Please provide path to the ply file!\n"); return 0; }

  TriMeshSimple cube_0 = {0};
  TriMeshSimple cube_1 = {0};
  TriMeshSimple cube_2 = {0};
  create_cube_simple( &cube_0 );

  // Simple example for reading and writing
  write_example_simple( argv[1], &cube_0 );
  printf( "Wrote cube data to ply file %s.\n"
          "N.Verts: %d; N. Faces: %d\n\n", argv[1], cube_0.n_vertices, cube_0.n_faces );

  read_example_simple( argv[1], &cube_1 );
  printf( "Read back cube stored in file %s.\n"
          "N.Verts: %d; N. Faces: %d\n\n", argv[1], cube_1.n_vertices, cube_1.n_faces );

  // Error example
  error_report_example( argv[1], &cube_2 );

  return 1;
}