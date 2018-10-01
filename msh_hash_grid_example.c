// msh_hash_grid_example.c
// compile: gcc -I../dev msh_hash_grid_example.c -o ../bin/msh_hash_grid_example.ply -lglfw3 -lopengl32 -lglew32 -lnanovg

#define MSH_STD_INCLUDE_LIBC_HEADERS
#define MSH_STD_IMPLEMENTATION
#define MSH_HASH_GRID_IMPLEMENTATION
#define MSH_VEC_MATH_IMPLEMENTATION
#define GLFW_INCLUDE_GLEXT
#define NANOVG_GL3_IMPLEMENTATION

#include <GL/glew.h> // TODO: replace with flextgl or smth
#include <GLFW/glfw3.h>

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

int main( int argc, char** argv )
{
  GLFWwindow* window;
  NVGcontext* vg = NULL;
  double prevt = 0;

  if (!glfwInit()) {
    printf("Failed to init GLFW.");
    return -1;
  }

  // glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, 1);
  // glfwWindowHint(GLFW_SAMPLES, 4);
  window = glfwCreateWindow( 512, 512, "Poisson Disk Sampling", NULL, NULL);

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

  style_t database_style = { .stroke_size  = 1.0f,
                             .stroke_color = nvgRGBAf(0.6f, 0.6f, 0.6f, 1.0f),
                             .fill_color   = nvgRGBAf(0.8f, 0.8f, 0.8f, 1.0f) };
  style_t result_style = { .stroke_size  = 1.0f,
                           .stroke_color = nvgRGBAf(0.6f, 0.6f, 0.8f, 1.0f),
                           .fill_color   = nvgRGBAf(0.8f, 0.8f, 1.0f, 1.0f) };
  style_t query_style = { .stroke_size  = 2.0f,
                          .stroke_color = nvgRGBAf(0.3f, 0.3f, 0.3f, 1.0f),
                          .fill_color   = nvgRGBAf(0.5f, 0.5f, 0.5f, 1.0f) };
  msh_vec2_t origin = msh_vec2(256, 256);
  int n_pts = 2000;
  msh_vec2_t* pts = generate_random_points_within_a_circle(origin, 256, n_pts );

  msh_vec3_t* pts_3d = malloc( n_pts * sizeof(msh_vec3_t) );
  for( int i = 0; i < n_pts; ++i )
  {
    pts_3d[i] = msh_vec3(pts[i].x, pts[i].y, 0.0f);
  }

  msh_hash_grid_t search_grid = {0};
  msh_hash_grid_init( &search_grid, (float*)&pts_3d[0], n_pts, 16.0f );
  int max_n_neigh = 100;
  msh_hash_grid_search_desc_t search_opts = { .radius = 32.0f,
                                              .max_n_neigh = max_n_neigh,
                                              .n_query_pts = 1,
                                              .sort = 0,
                                              .distances_sq = malloc( max_n_neigh * sizeof(float) ),
                                              .indices = malloc( max_n_neigh * sizeof(int32_t) ),
                                              .n_neighbors = malloc( max_n_neigh *sizeof(size_t) ) };
  

  float query_theta = 0;
  float query_r = 128;
  while (!glfwWindowShouldClose(window))
  {
    uint64_t t1 = msh_time_now();
    int win_width, win_height;
    int fb_width, fb_height;
    float px_ratio;

    msh_vec2_t query_pt = msh_vec2( origin.x + query_r * cos( query_theta ), 
                                    origin.y + query_r * sin( query_theta ) );
    search_opts.query_pts = (float*)&(msh_vec3_t){ .x = query_pt.x, .y = query_pt.y, .z = 0.0f };
    int n_results = msh_hash_grid_radius_search_mt( &search_grid, &search_opts );
    // int n_results = msh_hash_grid_knn_search_mt( &search_grid, &search_opts );
    msh_vec2_t *result_pts = malloc( n_results * sizeof(msh_vec2_t) );
    for( int i = 0; i < n_results; ++i )
    {
      result_pts[i] = pts[ search_opts.indices[i] ];
    }

    // Calculate pixel ration for hi-dpi devices.
    glfwGetWindowSize(window, &win_width, &win_height);
    glfwGetFramebufferSize(window, &fb_width, &fb_height);

    px_ratio = (float)fb_width / (float)win_width;

    // Update and render
    glViewport(0, 0, fb_width, fb_height);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


    nvgBeginFrame(vg, win_width, win_height, px_ratio);

    draw_points( vg, pts, n_pts, 2.0f, database_style );
    draw_points( vg, result_pts, n_results, 4.0f, result_style );
    draw_points( vg, &query_pt, 1, 6.0f, query_style );

    nvgEndFrame(vg);

    glfwSwapBuffers(window);
    glfwPollEvents();
    uint64_t t2 = msh_time_now();
    float elapsed_time = msh_time_diff( MSHT_SECONDS, t2, t1);
    query_theta += elapsed_time;
    char buf[1024];
    sprintf(buf, "TEST: %fms.", msh_time_diff(MSHT_MILLISECONDS, t2, t1) );
    glfwSetWindowTitle( window, buf );
  }

  nvgDeleteGL3(vg);
  glfwTerminate();
  return 0;
}