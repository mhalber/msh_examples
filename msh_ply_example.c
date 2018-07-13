#define MSH_STD_IMPLEMENTATION
#define MSH_PLY_IMPLEMENTATION

#include <msh/msh_std.h>
#include <msh/experimental/msh_ply.h>

int main( int argc, char** argv )
{
  if( argc < 2 ) { return 0; }

  float verts[8*3] = { -1.0f, -1.0f, -1.0f, /* 0 */
                        1.0f, -1.0f, -1.0f, /* 1 */
                       -1.0f,  1.0f, -1.0f, /* 2 */
                        1.0f,  1.0f, -1.0f, /* 3 */

                       -1.0f, -1.0f,  1.0f, /* 4 */
                        1.0f, -1.0f,  1.0f, /* 5 */
                       -1.0f,  1.0f,  1.0f, /* 6 */
                        1.0f,  1.0f,  1.0f  /* 7 */ };

  msh_ply_property_desc_t verts_desc = { .element_name = "vertex",
                                         .property_names = (const char*[]){"x", "y", "z"},
                                         .num_properties = 3,
                                         .data_type = MSH_PLY_FLOAT,
                                         .data = verts,
                                         .data_count = 8 };
  uint8_t colors[8*3] = { 0,  0,  0, 
                          0, 255, 0,
                          0, 0, 255,
                          0, 255, 255,
                          255, 0, 0,
                          255, 255, 0,
                          255, 0, 255,
                          255, 255, 255 };

  msh_ply_property_desc_t colors_desc = { .element_name = "vertex",
                                         .property_names = (const char*[]){"red", "green", "blue"},
                                         .num_properties = 3,
                                         .data_type = MSH_PLY_UINT8,
                                         .data = colors,
                                         .data_count = 8 };
  int32_t faces[12*3] = { 0, 2, 1, 1, 2, 3,
                          0, 1, 4, 1, 5, 4,
                          0, 4, 2, 2, 4, 6,
                          2, 7, 3, 2, 6, 7,
                          3, 7, 1, 1, 7, 5,
                          4, 5, 6, 5, 7, 6 };

  msh_ply_property_desc_t faces_desc = { .element_name = "face",
                                         .property_names = (const char*[]){"vertex_indices"},
                                         .num_properties = 1,
                                         .data_type = MSH_PLY_INT32,
                                         .list_type = MSH_PLY_UINT8,
                                         .data = faces,
                                         .data_count = 12,
                                         .list_size_hint = 3 };


  msh_ply_property_desc_t descriptors[3] = { verts_desc, colors_desc, faces_desc };

  msh_ply_t* out_ply = msh_ply_open( argv[1], "wb" );
  for( int i = 0 ; i < msh_count_of( descriptors ); ++i )
  {
    msh_ply_add_descriptor( out_ply, &descriptors[i] );
  }
  msh_ply_write( out_ply );
  msh_ply_close( out_ply ); 

  msh_ply_t* in_ply = msh_ply_open( argv[1], "rb" );
  for( int i = 0 ; i < msh_count_of( descriptors ); ++i )
  {
    msh_ply_add_descriptor( in_ply, &descriptors[i] );
  }
  msh_ply_read( in_ply );
  msh_ply_print_header( in_ply );
  msh_ply_close( in_ply ); 
  return 1;
}