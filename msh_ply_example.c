/*
 Author: Maciej Halber
 Date : Jul 18, 2018
 License: Public Domain
 To compile: gcc -I<path to folder with msh_libs> msh_ply_example.c -o msh_example
*/

#define MSH_PLY_IMPLEMENTATION
#define MSH_PLY_INCLUDE_HEADERS
#include <msh/msh_ply.h>

typedef struct Vec3f
{
  float x,y,z;
} Vec3f;

typedef struct Vec3i
{
  int x,y,z;
} Vec3i;

typedef struct TriMesh
{
  Vec3f* vertices;
  Vec3i* faces;
  int n_vertices;
  int n_faces;
} TriMesh;

void create_cube( TriMesh* mesh )
{
  mesh->n_vertices = 8;
  mesh->n_faces    = 12;
  mesh->vertices   = malloc( mesh->n_vertices * sizeof(Vec3f) );
  mesh->faces      = malloc( mesh->n_faces * sizeof(Vec3i) );

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
write_example( const char* filename, TriMesh* mesh )
{

  // Describe the data and assign pointers
  msh_ply_property_desc_t verts_desc = { .element_name = "vertex",
                                         .property_names = (const char*[]){"x", "y", "z"},
                                         .num_properties = 3,
                                         .data_type = MSH_PLY_FLOAT,
                                         .data = &mesh->vertices,
                                         .data_count = &mesh->n_vertices };

  msh_ply_property_desc_t faces_desc = { .element_name = "face",
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
read_example( const char* filename, TriMesh *mesh )
{
  // Describe the data and assign pointers
  msh_ply_property_desc_t verts_desc = { .element_name = "vertex",
                                         .property_names = (const char*[]){"x", "y", "z"},
                                         .num_properties = 3,
                                         .data_type = MSH_PLY_FLOAT,
                                         .data = &mesh->vertices,
                                         .data_count = &mesh->n_vertices };

  msh_ply_property_desc_t faces_desc = { .element_name = "face",
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
  msh_ply_print_header( in_ply );
  msh_ply_close( in_ply ); 
}

int main( int argc, char** argv )
{
  if( argc < 2 ) { printf("Please provide path to the ply file!\n"); return 0; }

  TriMesh cube1;
  create_cube( &cube1 );
  write_example( argv[1], &cube1 );

  TriMesh cube2;
  read_example( argv[1], &cube2 );

  printf( "Wrote and read back cube stored in file %s.\n"
          "N.Verts: %d; N. Faces: %d\n", argv[1], cube2.n_vertices, cube2.n_faces );
  return 1;
}