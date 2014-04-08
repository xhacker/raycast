#include "sphere.h"
#include <stdlib.h>
#include <math.h>
#include <float.h>

/**********************************************************************
 * This function intersects a ray with a given sphere 'sph'. You should
 * use the parametric representation of a line and do the intersection.
 * The function should return the parameter value for the intersection,
 * which will be compared with others to determine which intersection
 * is closest. The value -1.0 is returned if there is no intersection
 *
 * If there is an intersection, the point of intersection should be
 * stored in the "hit" variable
 **********************************************************************/
float intersect_sphere(Point o, Vector ray, Spheres *sph, Point* hit)
{
    double a = vec_dot(ray, ray);
    double b = 2 * vec_dot(ray, get_vec(sph->center, o));
    double c = vec_dot(get_vec(sph->center, o), get_vec(sph->center, o));
    c -= sph->radius * sph->radius;

    double dt = b * b - 4 * a * c;

    if (dt < 0) {
        return -1.0;
    }
    else {
        double t0 = (-b - sqrt(dt)) / (a * 2);
        if (t0 < 0) {
            return -1.0;
        }

        hit->x = o.x + t0 * ray.x;
        hit->y = o.y + t0 * ray.y;
        hit->z = o.z + t0 * ray.z;

        Vector v = { hit->x - o.x, hit->y - o.y, hit->z - o.z };
        return vec_len(v);
    }
}

bool intersect_shadow(Point o, Vector ray, Spheres *spheres)
{
    normalize(&ray);

    while (spheres) {
        double a = vec_dot(ray, ray);
        double b = vec_dot(vec_scale(get_vec(spheres->center, o), 2), ray);
        double c = vec_dot(get_vec(spheres->center, o), get_vec(spheres->center, o));
        c -= spheres->radius * spheres->radius;

        double discriminant = b * b - 4 * a * c;

        double t1 = (-b + sqrtf(discriminant)) / 2 * a;
        double t2 = (-b - sqrtf(discriminant)) / 2 * a;

        if (discriminant > 0 && t1 > 0 && t2 > 0) {
            return true;
        }

        spheres = spheres->next;
    }
    return false;
}

/*********************************************************************
 * This function returns a pointer to the sphere object that the
 * ray intersects first; NULL if no intersection. You should decide
 * which arguments to use for the function. For exmaple, note that you
 * should return the point of intersection to the calling function.
 **********************************************************************/
Spheres * intersect_scene(Point o, Vector ray, Spheres *spheres, Point *hit)
{
    Spheres *closest = NULL;

    double min_distance = DBL_MAX;

    while (spheres) {
        double distance = intersect_sphere(o, ray, spheres, hit);
        if ((distance != -1.0) && (distance < min_distance)) {
            min_distance = min_distance;
            closest = spheres;
        }
        spheres = spheres->next;
    }

    return closest;
}

/*****************************************************
 * This function adds a sphere into the sphere list
 *
 * You need not change this.
 *****************************************************/
Spheres * add_sphere(Spheres *slist, Point ctr, float rad, float amb[],
                    float dif[], float spe[], float shine,
                    float refl, bool transparent, int sindex)
{
    Spheres* new_sphere;

    new_sphere = (Spheres*)malloc(sizeof(Spheres));
    new_sphere->index = sindex;
    new_sphere->center = ctr;
    new_sphere->radius = rad;
    (new_sphere->mat_ambient)[0] = amb[0];
    (new_sphere->mat_ambient)[1] = amb[1];
    (new_sphere->mat_ambient)[2] = amb[2];
    (new_sphere->mat_diffuse)[0] = dif[0];
    (new_sphere->mat_diffuse)[1] = dif[1];
    (new_sphere->mat_diffuse)[2] = dif[2];
    (new_sphere->mat_specular)[0] = spe[0];
    (new_sphere->mat_specular)[1] = spe[1];
    (new_sphere->mat_specular)[2] = spe[2];
    new_sphere->mat_shineness = shine;
    new_sphere->reflectance = refl;
    new_sphere->transparent = transparent;
    new_sphere->next = NULL;

    if (slist == NULL) { // first object
        slist = new_sphere;
    }
    else { // insert at the beginning
        new_sphere->next = slist;
        slist = new_sphere;
    }

    return slist;
}

/******************************************
 * computes a sphere normal - done for you
 ******************************************/
Vector sphere_normal(Point q, Spheres* sph)
{
    Vector rc;

    rc = get_vec(sph->center, q);
    normalize(&rc);
    return rc;
}
