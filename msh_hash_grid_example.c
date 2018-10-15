/*
  Author: Maciej Halber
  Date : Oct 15, 2018
  License: CC0
 
  Compilation: gcc -std=c99 -I<path_to_msh_libraries> msh_hash_grid_example.c -o msh_hash_grid_example -lglfw3 -lopengl32 -lglew32 -lnanovg
  Usage:       msh_hash_grid_example
  Description: This program showcases the usage of msh_hash_grid.h. It creates a window in which
               we visualize neighbors of a moving 2D point. Requires OpenGL, GLFW, GLEW and nanovg
               to build.
*/

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_HASH_GRID_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#define GLFW_INCLUDE_GLEXT
#define NANOVG_GL3_IMPLEMENTATION

#include <GL/glew.h>    // TODO: replace with flextgl or smth
#include <GLFW/glfw3.h> // TODO: replace with sokol_app.h

#include "msh/msh_std.h"
#include "msh/msh_hash_grid.h"
#include "msh/msh_vec_math.h"
#include "nanovg/nanovg.h"
#include "nanovg/nanovg_gl.h"


msh_vec2_t*
generate_random_points_within_a_circle( msh_vec2_t center, float radius, int n_pts )
{
  msh_rand_ctx_t rand_gen = {0};
  msh_rand_init( &rand_gen, 60123817 );
  msh_vec2_t* pts = malloc(sizeof(msh_vec2_t)*n_pts);
  for( int i = 0; i < n_pts; ++i )
  {
    float theta = MSH_TWO_PI * msh_rand_nextf( &rand_gen );
    float r = radius * sqrtf( msh_rand_nextf( &rand_gen ) );
    float x = r * cosf( theta );
    float y = r * sinf( theta );
    pts[i] = msh_vec2( x, y );
    pts[i] = msh_vec2_add( pts[i], center );
  }
  return pts;
}

typedef struct style
{
  float stroke_size;
  NVGcolor fill_color;
  NVGcolor stroke_color;
} style_t;

void 
draw_points( NVGcontext *vg, const msh_vec2_t* pts, size_t n_pts, float size, style_t style )
{
  nvgFillColor( vg, style.fill_color );
  nvgStrokeColor( vg, style.stroke_color );
  nvgStrokeWidth( vg, style.stroke_size );
  nvgBeginPath( vg );
  for( size_t i = 0; i < n_pts ; i++)
  {
    nvgCircle( vg, pts[i].x, pts[i].y, size );
  }
  nvgStroke( vg );
  nvgFill( vg );
}

void
draw_lines( NVGcontext* vg, const msh_vec2_t* lines, size_t n_lines, style_t style )
{
  nvgStrokeColor( vg, style.stroke_color );
  nvgStrokeWidth( vg, style.stroke_size );
  nvgBeginPath( vg );
  for( size_t i = 0; i < n_lines ; i++)
  {
    msh_vec2_t a = lines[2*i];
    msh_vec2_t b = lines[2*i+1];
    nvgMoveTo( vg, a.x, a.y );
    nvgLineTo( vg, b.x, b.y );
  }
  nvgStroke( vg );
}

void
draw_lines_alpha( NVGcontext* vg, const msh_vec2_t* lines, const float* alphas, size_t n_lines, style_t style )
{
  nvgStrokeWidth( vg, style.stroke_size );
  for( size_t i = 0; i < n_lines ; i++)
  {
    NVGcolor c = style.stroke_color;
    c.a = alphas[i];
    nvgBeginPath( vg );
    nvgStrokeColor( vg, c );
    msh_vec2_t a = lines[2*i];
    msh_vec2_t b = lines[2*i+1];
    nvgMoveTo( vg, a.x, a.y );
    nvgLineTo( vg, b.x, b.y );
    nvgStroke( vg );
  }
}

int main( int argc, char** argv )
{
  GLFWwindow* window;
  NVGcontext* vg = NULL;
  double prevt = 0;

  if (!glfwInit()) {
    printf("Failed to init GLFW.");
    return -1;
  }

  float window_size = 256;
  window = glfwCreateWindow( window_size, window_size, "Poisson Disk Sampling", NULL, NULL);

  if (!window) {
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  if(glewInit() != GLEW_OK) {
    printf("Failed to init GLEW.\n");
    return -1;
  }

  // GLEW generates GL error because it calls glGetString(GL_EXTENSIONS), we'll consume it here.
  glGetError();

  vg = nvgCreateGL3( NVG_ANTIALIAS | NVG_STENCIL_STROKES  );

  if (vg == NULL) {
    printf("Could not init nanovg.\n");
    return -1;
  }

  glfwSwapInterval(0);

  
  style_t lines_style = { .stroke_size  = 2.0f,
                             .stroke_color = nvgRGBAf(0.0f, 0.0f, 0.0f, 1.0f) };
  style_t database_style = { .stroke_size  = 3.0f,
                             .stroke_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f),
                             .fill_color   = nvgRGBA(78, 121, 161, 255) };
  style_t result_style = { .stroke_size  = 3.0f,
                           .stroke_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f),
                           .fill_color   = nvgRGBA(242, 142, 43, 255) };
  style_t query_style = { .stroke_size  = 3.0f,
                          .stroke_color = nvgRGBAf(1.0f, 1.0f, 1.0f, 1.0f),
                          .fill_color   = nvgRGBA(225, 87, 89, 255) };
  style_t domain_style = { .stroke_size  = 6.0f,
                           .stroke_color  = nvgRGBAf(0.8f, 0.8f, 0.8f, 1.0f),
                           .fill_color    = nvgRGBAf(1.0f, 1.0f, 1.0f, 0.0f) };

  msh_vec2_t domain_origin = msh_vec2( window_size/2, window_size/2 );
  float domain_radius = window_size/2 - 20;
  int n_pts = 250;
  msh_vec2_t* pts = generate_random_points_within_a_circle( domain_origin, 
                                                            domain_radius, 
                                                            n_pts );


  msh_hash_grid_t search_grid = {0};
  float search_radius = 64.0f;
  msh_hash_grid_init_2d( &search_grid, (float*)&pts[0], n_pts, search_radius );
  int max_n_neigh = 10;
  msh_hash_grid_search_desc_t search_opts = { .radius = search_radius,
                                              .max_n_neigh = max_n_neigh,
                                              .n_query_pts = 1,
                                              .sort = 1,
                                              .distances_sq = malloc( max_n_neigh * sizeof(float) ),
                                              .indices = malloc( max_n_neigh * sizeof(int32_t) ),
                                              .n_neighbors = malloc( max_n_neigh *sizeof(size_t) ) };
  

  float query_theta = 1.24f;
  float query_r = window_size/4.0f;
  while( !glfwWindowShouldClose(window) )
  {
    uint64_t t1 = msh_time_now();
    uint64_t lt1 = msh_time_now();
    int win_width, win_height;
    int fb_width, fb_height;
    float px_ratio;

    msh_vec2_t query_pt = msh_vec2( domain_origin.x + query_r * cos( query_theta ), 
                                    domain_origin.y + query_r * sin( query_theta ) );
    search_opts.query_pts = (float*)&query_pt;
    int n_results = msh_hash_grid_radius_search( &search_grid, &search_opts );
    // int n_results = msh_hash_grid_knn_search( &search_grid, &search_opts );
    msh_vec2_t *result_pts = malloc( n_results * sizeof(msh_vec2_t) );
    for( int i = 0; i < n_results; ++i )
    {
      result_pts[i] = pts[ search_opts.indices[i] ];
    }
    msh_vec2_t* connector_lines = malloc( n_results * 2 * sizeof(msh_vec2_t));
    float* lines_intensity = malloc( n_results *  sizeof(float));
    for( int i = n_results-1; i >= 0; --i )
    {
      connector_lines[2*i] = query_pt;
      connector_lines[2*i+1] = result_pts[i];
      lines_intensity[i] = 1.0 - sqrt(search_opts.distances_sq[i]) / search_radius;
    }
    uint64_t lt2 = msh_time_now();
    float logic_time = msh_time_diff(MSHT_MILLISECONDS, lt2, lt1);
    // Calculate pixel ration for hi-dpi devices.
    glfwGetWindowSize(window, &win_width, &win_height);
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    px_ratio = (float)fb_width / (float)win_width;

    // Update and render
    glViewport(0, 0, fb_width, fb_height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    lt1 = msh_time_now();
    nvgBeginFrame(vg, win_width, win_height, px_ratio);
    draw_points( vg, &domain_origin, 1, domain_radius + 0.5*domain_style.stroke_size, domain_style );
    draw_points( vg, pts, n_pts, 3.0f, database_style );
    draw_lines_alpha(  vg, connector_lines, lines_intensity, n_results, lines_style );
    draw_points( vg, result_pts, n_results, 3.0f, result_style );
    draw_points( vg, &query_pt, 1, 4.5f, query_style );
    lt2 = msh_time_now();
    float upload_time = msh_time_diff(MSHT_MILLISECONDS, lt2, lt1);

    nvgEndFrame(vg);

    free( result_pts );
    free( connector_lines );
    free( lines_intensity );


    glfwSwapBuffers(window);
    glfwPollEvents();
    uint64_t t2 = msh_time_now();
    float elapsed_time = msh_time_diff( MSHT_SECONDS, t2, t1);
    query_theta += elapsed_time;
    char buf[1024];
    sprintf(buf, "TEST: %fms.| %fms. | %fms.", logic_time, upload_time, msh_time_diff(MSHT_MILLISECONDS, t2, t1) );
    glfwSetWindowTitle( window, buf );
  }

  nvgDeleteGL3(vg);
  glfwTerminate();
  return 0;
}