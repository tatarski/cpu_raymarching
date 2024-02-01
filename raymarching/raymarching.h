
#include "utils.h"
#include "Image.h"

using namespace Utils;
namespace Raymarching {
	const double epsilon = 0.0001, epsilon_normal = 0.00001;

	// Core of raymarching algorithm
	// Min distance from a point to the scene
	// Positive number if point is outside of the scane
	// Negative number if point is inside of the scene
	double dist_to_scene(const Vector<3>& p) {
		return smin(p[1] + 1.,  // Union
			smax(		// Subtract
				-((Vector<3>{-0.4, -0.5, 4.4} - p).len() - 0.6),
				smin(	// Union
					(Vector<3>{0., 0., 5.} - p).len() - 1.,
					(Vector<3>{0.4, -0.3, 5.} - p).len() - 0.8,
					0.001),
				0.001),
			0.2
		);
	}
	// Get intersection point with sufrace when:
	// Beginning from point c
	// Moving along a ray with direction dir
	// If stopNearEdges is true - stop once distance to scene stops getting smaller (and starts getting bigger)
	// If stopNearEdges is true - edgeFound will be set to true if an edge has been found after completion
	bool stopNearEdges = true, edgeFound = false;
	double edge_threshold = 0.02;

	Vector<3> get_intersection(const Vector<3>& dir, const Vector<3>& c = { 0., 0., 0. }, double near = 0.05, double far = 7.) {
		edgeFound = false;
		double t = near,
			prevDist = INFINITY;

		while (t >= near && t < far) {
			Vector<3> p = c + dir * t;

			double dist = dist_to_scene(p);

			// if distance has started to get bigger - an edge has been found
			if (stopNearEdges && dist - prevDist > 0. && dist < edge_threshold) {
				edgeFound = true;
				return { 0., 0., 0. };
			}

			// Normal intersection with scene (distance is really small)
			if (dist < epsilon) {
				return p;
			}
			// March along the ray - we can safely move along the ray with 
			// a vector with a length of at least dist.
			// dist = min distance to scene in any direction
			t += dist;

			prevDist = dist;
		}
		return c + dir * t;
	}

	// Find normal of surface at intersection point p
	// Cast ray with slightly different direction from otiginal dir
	// (dir.x is slightly changed, dir.y is slightly changed)
	Vector<3> find_normal(const Vector<3>& dir, const Vector<3>& p) {
		Vector<3> dir_dx = dir + Vector<3>{-epsilon_normal, 0., 0.},
			dir_dy = dir + Vector<3>{0., epsilon_normal, 0.};

		// vec_dx and vec_dy "almost" lie on the surface
		Vector<3> vec_dx = p - get_intersection(dir_dx),
			vec_dy = p - get_intersection(dir_dy);
		// Normal is cross product two vectors that almost lie on the surface

		return cross(vec_dx, vec_dy).normalize();
	}
	Vector<3> get_color(size_t x, size_t y, size_t w, size_t h) {

		// Eye is positioned at (0,0,0)
		// Image is a plane perpendicular to the z zxis at coordinates (x, y, 1.)
		// X,Y - indexes of pixels in table
		// get direction from (0,0,0) towards pixel on image
		Vector<3> dir({ (double)x / w - 0.5, (double)y / w - 0.5 * ((double)h / w), 1. });

		dir[1] = -dir[1];

		// Normalize marching direction
		dir.normalize();

		double near = 0.05, far = 7.,
			// t - parametrization. Current distance along ray.
			t = near;

		stopNearEdges = true;

		// get intersection point of ray with scene
		Vector<3> p = get_intersection(dir);

		// Edges (from the point of view of the camera) will be black
		if (edgeFound) {
			return { 1., 1., 1. };
		}

		// Distance from p to closest object in scene
		double dist = dist_to_scene(p);

		if (dist < epsilon) {
			// Intersection has been found

			Vector<3> normal = find_normal(dir, p);

			// Ambient, diffuse and specular light colors
			Vector<3> ambient = { 1., 0., 0. },
				diffuse = { 1., 1., 1. },
				specular = { 1., 1., 1. };

			// Coefficients for ambiend, diffuse and specular lighting
			double ca = 0.7,
				cd = 0.5,
				cs = 0.2,
				alpha = 5.;

			// Position of light in scene
			Vector<3> light1_pos = { 3., 1., 5.1 },
				dir_to_light = (p - light1_pos).normalize() * -1;

			Vector<3> reflected = normal * 2. * (dir_to_light * normal) - dir_to_light;
			reflected.normalize();

			double shadow_threshold = 0.1;

			stopNearEdges = false;
			// Raymarch from intersection point in the direction of the point light
			Vector<3> lightIntersect = get_intersection((light1_pos - p).normalize(), p, 0.05, 10.);

			// Shadows if no direct light from point light 1
			if ((lightIntersect - p).len() > shadow_threshold) {
				ca = 0.6;
				cd = 0.3;
				cs = 0.05;
			}

			return ambient * ca +
				diffuse * ((p - light1_pos).normalize() * normal) * cd +
				specular * pow(reflected * (dir * -1.), alpha) * cs;

		}
		else {
			return Vector<3>{0., 0., 0.};
		}
	}
}