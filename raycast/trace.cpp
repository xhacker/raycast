#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <stdio.h>
#include <stdlib.h>
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

extern bool shadow_on;
extern bool reflection_on;
extern bool chessboard_on;
extern bool stochastic_on;
extern bool supersampling_on;
extern int step_max;

// chess board
Vector board_norm = {0, -3, 1};
Point board_p = {0, -3, -4};

/////////////////////////////////////////////////////////////////////

void validate_color(RGB_float *c)
{
    if (c->r < 0) {
        c->r = 0;
    }
    if (c->g < 0) {
        c->g = 0;
    }
    if (c->b < 0) {
        c->b = 0;
    }

    if (c->r > 1.0) {
        c->r = 1.0;
    }
    if (c->g > 1.0) {
        c->g = 1.0;
    }
    if (c->b > 1.0) {
        c->b = 1.0;
    }
}

/*********************************************************************
 * Phong illumination - you need to implement this!
 *********************************************************************/
RGB_float phong(Point p, Vector v, Vector surf_norm, Spheres* sph)
{
    Vector light_v = get_vec(p, light1);
    double d = vec_len(get_vec(p, light1));
    normalize(&light_v);

    RGB_float color    = {0, 0, 0};
    RGB_float ambient  = {0, 0, 0};

    double coeff = 1 / (decay_a + decay_b * d + decay_c * d * d);

    // diffuse
    double dot = vec_dot(surf_norm, light_v);
    color.r += coeff * light1_diffuse[0] * sph->mat_diffuse[0] * dot;
    color.g += coeff * light1_diffuse[1] * sph->mat_diffuse[1] * dot;
    color.b += coeff * light1_diffuse[2] * sph->mat_diffuse[2] * dot;

//    // global ambient
//    ambient.r += global_ambient[0] * sph->reflectance;
//    ambient.g += global_ambient[1] * sph->reflectance;
//    ambient.b += global_ambient[2] * sph->reflectance;

    // light ambient
    ambient.r += light1_ambient[0] * sph->mat_ambient[0];
    ambient.g += light1_ambient[1] * sph->mat_ambient[1];
    ambient.b += light1_ambient[2] * sph->mat_ambient[2];

    // specular
    double N = sph->mat_shineness;
    double cos_theta = vec_dot(surf_norm, light_v);
    if (cos_theta < 0) {
        cos_theta = 0.0;
    }
    Vector reflect_vec = vec_plus(light_v, vec_scale(surf_norm, -2 * cos_theta));
    normalize(&reflect_vec);
    color.r += coeff * (light1_specular[0] * sph->mat_specular[0] * pow(vec_dot(reflect_vec, v), N));
    color.g += coeff * (light1_specular[1] * sph->mat_specular[1] * pow(vec_dot(reflect_vec, v), N));
    color.b += coeff * (light1_specular[2] * sph->mat_specular[2] * pow(vec_dot(reflect_vec, v), N));

    if (shadow_on && intersect_shadow(p, light_v, scene)) {
        color = ambient;
    }

    return color;
}

bool intersect_chessboard(Point p, Vector ray, Point *hit)
{
    normalize(&board_norm);

    Vector vec;
    vec.x = p.x - board_p.x;
    vec.y = p.y - board_p.y;
    vec.z = p.z - board_p.z;

    if (vec_dot(board_norm, ray) == 0 && vec_dot(board_norm, vec)) {
        return false;
    }

    double t = vec_dot(board_norm, vec) / vec_dot(board_norm, ray) * -1;

    if (t > 0) {
        hit->x = p.x + t * ray.x;
        hit->y = p.y + t * ray.y;
        hit->z = p.z + t * ray.z;
        return true;
    }

    return false;
}

RGB_float board_color(Point p)
{
    RGB_float color;

    int i = int(p.x + 100) - 100;
    int j = int(p.z + 100) - 100;

    if (i >= 4 || i < -4 || j >= 2 || j < -6) {
        return background_clr;
    }

    if ((i % 2 == 0 && j % 2 == 0) ||
        (i % 2 != 0 && j % 2 != 0)) {
        color = {0, 0, 0};
    }
    else {
        color = {1, 1, 1};
    }

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

    Point board_hit;
    if (chessboard_on && intersect_chessboard(p, ray, &board_hit))
    {
        Vector eye_vec = get_vec(board_hit, eye_pos);
        normalize(&eye_vec);
        color = board_color(board_hit);
        Vector shadow_vec = get_vec(board_hit, light1);
        Spheres *sph = NULL;
        if (shadow_on && intersect_shadow(board_hit, shadow_vec, sph)) {
            color = clr_scale(color, 0.5);
        }
    }

    if (closest_sphere) {
        Vector eye_vec = get_vec(hit, eye_pos);
        Vector surf_norm = sphere_normal(hit, closest_sphere);
        Vector light_ray = get_vec(hit, p);
        normalize(&surf_norm);
        normalize(&eye_vec);
        normalize(&light_ray);

        color = phong(hit, eye_vec, surf_norm, closest_sphere);

        if (reflection_on && cur_step <= step_max) {
            normalize(&ray);
            Vector reflected_ray = vec_plus(vec_scale(surf_norm, -2 * vec_dot(surf_norm, ray)), ray);

            RGB_float reflected_color = recursive_ray_trace(reflected_ray, hit, cur_step + 1);

            if (stochastic_on) {
                for (int i = 0; i < NUM_RANDOM_RAYS; ++i) {
                    double random_x = (double)rand() / RAND_MAX / 2;
                    double random_y = (double)rand() / RAND_MAX / 2;
                    double random_z = (double)rand() / RAND_MAX / 2;

                    Vector random_ray = reflected_ray;
                    random_ray.x += random_x;
                    random_ray.y += random_y;
                    random_ray.z += random_z;
                    normalize(&random_ray);

                    RGB_float random_reflected_color = recursive_ray_trace(random_ray, hit, cur_step + 1);
                    reflected_color = clr_add(reflected_color, clr_scale(random_reflected_color, RANDOM_SCALE));
                }
                reflected_color = clr_scale(reflected_color, 1.0 / (1.0 + RANDOM_SCALE * NUM_RANDOM_RAYS));
            }

            color = clr_add(color, clr_scale(reflected_color, closest_sphere->reflectance));
        }
    }

    validate_color(&color);

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
    double x_grid_size = image_width / double(win_width);
    double y_grid_size = image_height / double(win_height);
    double x_start = -0.5 * image_width;
    double y_start = -0.5 * image_height;
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

            ret_color = recursive_ray_trace(ray, eye_pos, 1);

            if (supersampling_on) {
                for (int x = -1; x <= 1; x += 2) {
                    for (int y = -1; y <= 1; y += 2) {
                        Point new_pixel_pos = cur_pixel_pos;
                        new_pixel_pos.x += x * x_grid_size / 4;
                        new_pixel_pos.y += y * y_grid_size / 4;

                        Vector new_ray = get_vec(eye_pos, new_pixel_pos);

                        ret_color = clr_add(ret_color, recursive_ray_trace(new_ray, eye_pos, 1));
                    }
                }
                ret_color = clr_scale(ret_color, 1.0 / 5);
            }

            frame[i][j][0] = GLfloat(ret_color.r);
            frame[i][j][1] = GLfloat(ret_color.g);
            frame[i][j][2] = GLfloat(ret_color.b);

            cur_pixel_pos.x += x_grid_size;
        }

        cur_pixel_pos.y += y_grid_size;
        cur_pixel_pos.x = x_start;
    }
}
