// raymarching.cpp : Defines the entry point for the application.
//

#include <iostream>
#include "utils.h"
#include "Image.h"

using namespace std;
using namespace Utils;

// Linear interpolation
double mix(double a, double b, double k) {
	return a * (1. - k) + b * k;
}


double smin(double a, double b, double k, double s = -1.) {
	// when a is near b - more significant offset
	// when a is far from b - very small offset
	return 0.5 * (a + b + s * sqrt((a - b) * (a - b) + k));
}
double smax(double a, double b, double k) {
	return smin(a, b, k, 1.);
}
const double epsilon = 0.0001, epsilon_normal = 0.00001;
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
bool stopNearEdges = true, edgeFound = false;
double edge_threshold = 0.02;


Vector<3> get_intersection(const Vector<3>& dir, const Vector<3>& c = {0., 0., 0.}, double near = 0.05, double far = 7.) {
	edgeFound = false;
	double t = near,
		prevDist = INFINITY;

	while (t >= near && t < far) {
		Vector<3> p = c + dir * t;

		double dist = dist_to_scene(p);

		if (stopNearEdges && dist - prevDist > 0. && dist < edge_threshold) {
			edgeFound = true;
			return { 0., 0., 0. };
		}
		if (dist < epsilon) {
			return p;
		}
		t += dist;

		prevDist = dist;
	}
	return c + dir * t;
}

Vector<3> find_normal(const Vector<3>& dir, const Vector<3>& p) {
	Vector<3> dir_dx = dir + Vector<3>{-epsilon_normal, 0., 0.},
			  dir_dy = dir + Vector<3>{0., epsilon_normal, 0.};
	Vector<3> vec_dx = p - get_intersection(dir_dx),
		      vec_dy = p - get_intersection(dir_dy);
	return cross(vec_dx, vec_dy).normalize();
}
Vector<3> get_color(size_t x, size_t y, size_t w, size_t h) {

	Vector<3> dir({ (double)x / w - 0.5, (double)y / w - 0.5*((double)h/w), 1.});

	dir[1] = -dir[1];

	dir.normalize();

	double near = 0.05, far = 7.,
		t = near;

	stopNearEdges = true;
	Vector<3> p = get_intersection(dir);

	if (edgeFound) {
		return { 1., 1., 1. };
	}

	double dist = dist_to_scene(p);
	
	if (dist < epsilon) {
		Vector<3> normal = find_normal(dir, p);
		Vector<3> ambient = { 1., 0., 0. },
			diffuse = { 1., 1., 1. },
			specular = {1., 1., 1. };

		Vector<3> light1_pos = { 3., 1., 5.1 },
			dir_to_light = (p - light1_pos).normalize()*-1;

		Vector<3> reflected = normal*2.*(dir_to_light * normal) - dir_to_light;
		reflected.normalize();

		double ca = 0.7,
			cd = 0.5,
			cs = 0.2,
			alpha = 5.;

		double shadow_threshold = 0.1;

		stopNearEdges = false;
		Vector<3> lightIntersect = get_intersection((light1_pos - p).normalize(), p, 0.05, 10.);
		//cout << lightIntersect << endl;
		//cout << (p - light1_pos).normalize() << endl;
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
int main()
{
	//Matrix<3, 3> A = { {1., 0., 0.}, {0., 1., 0.}, {0., 0., 0.} };
	//Matrix<3, 3> B = { {1., 1., 1.}, {4., 5., 6.}, {7., 8., 9.} };

	//Vector<3> v1 = { 0, 5, 0 };
	//Vector<3> v2 = { 3, 4, 5 };
	//cout << dot_product(v1, v2) << endl;

	//cout << (A * B) * v1 << endl;
	//cout << v1*(1/(v1.len())) << endl;

	// Init output image
	const size_t w = 500, h = 500;
	Image ppmimg(w, h, 3);
	//for (int i = 0; i < ppmimg.w; i++) {
	//	for (int j = 0; j < ppmimg.h; j++) {
	//		ppmimg.set(i, j, 0, 255.*i/ppmimg.w);
	//		ppmimg.set(i, j, 1, 255.*j/ppmimg.h);
	//		ppmimg.set(i, j, 2, i % 255);
	//	}
	//}

	for (int i = 0; i < ppmimg.w; i++) {
		for (int j = 0; j < ppmimg.h; j++) {
			Vector<3> col = get_color(i, j, w, h);

			ppmimg.set(i, j, 0, std::clamp(col[0], 0., 255.));
			ppmimg.set(i, j, 1, std::clamp(col[1], 0., 255.));
			ppmimg.set(i, j, 2, std::clamp(col[2], 0., 255.));
		}
	}

	writePPM("res.ppm", ppmimg);
	return 0;
}
