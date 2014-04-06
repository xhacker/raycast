#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <stdio.h>
#include <math.h>
#include "global.h"
#include "sphere.h"

//
// Global variables
//
extern int win_width;
extern int win_height;

extern GLfloat frame[WIN_HEIGHT][WIN_WIDTH][3];

extern float image_width;
extern float image_height;

extern Point eye_pos;
extern float image_plane;
extern RGB_float background_clr;
extern RGB_float null_clr;

extern Spheres *scene;

// light 1 position and color
extern Point light1;
extern float light1_ambient[3];
extern float light1_diffuse[3];
extern float light1_specular[3];

// global ambient term
extern float global_ambient[3];

// light decay parameters
extern float decay_a;
extern float decay_b;
extern float decay_c;

extern int shadow_on;
extern int step_max;

/////////////////////////////////////////////////////////////////////

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point q, Vector v, Vector surf_norm, Spheres* sph)
{
    //
    // do your thing here
    //
    RGB_float color;
    return color;
}

/************************************************************************
 * This is the recursive ray tracer - you need to implement this!
 * You should decide what arguments to use.
 ************************************************************************/
RGB_float recursive_ray_trace(Vector ray, Point p, int cur_step)
{
    RGB_float color = background_clr;

    Point hit;
    Spheres *closest_sphere = intersect_scene(p, ray, scene, &hit);

    if (closest_sphere) {
        Vector eye_vec = get_vec(hit, eye_pos);
        Vector surf_norm = sphere_normal(hit, closest_sphere);
        normalize(&surf_norm);
        normalize(&eye_vec);

        color = phong(hit, eye_vec, surf_norm, closest_sphere);
    }

    return color;
}

/*********************************************************************
 * This function traverses all the pixels and cast rays. It calls the
 * recursive ray tracer and assign return color to frame
 *
 * You should not need to change it except for the call to the recursive
 * ray tracer. Feel free to change other parts of the function however,
 * if you must.
 *********************************************************************/
void ray_trace()
{
    int i, j;
    float x_grid_size = image_width / float(win_width);
    float y_grid_size = image_height / float(win_height);
    float x_start = -0.5 * image_width;
    float y_start = -0.5 * image_height;
    RGB_float ret_color;
    Point cur_pixel_pos;
    Vector ray;

    // ray is cast through center of pixel
    cur_pixel_pos.x = x_start + 0.5 * x_grid_size;
    cur_pixel_pos.y = y_start + 0.5 * y_grid_size;
    cur_pixel_pos.z = image_plane;

    for (i = 0; i < win_height; i++) {
        for (j = 0; j < win_width; j++) {
            ray = get_vec(eye_pos, cur_pixel_pos);

            ret_color = recursive_ray_trace(ray, eye_pos, 0);

            // Parallel rays can be cast instead using below
            //
            // ray.x = ray.y = 0;
            // ray.z = -1.0;
            // ret_color = recursive_ray_trace(cur_pixel_pos, ray, 1);

            frame[i][j][0] = GLfloat(ret_color.r);
            frame[i][j][1] = GLfloat(ret_color.g);
            frame[i][j][2] = GLfloat(ret_color.b);

            cur_pixel_pos.x += x_grid_size;
        }

        cur_pixel_pos.y += y_grid_size;
        cur_pixel_pos.x = x_start;
    }
}
