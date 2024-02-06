// raymarching.cpp : Defines the entry point for the application.
//

#include <iostream>
#include<chrono>
#include <xmmintrin.h>  // SSE
#include <emmintrin.h>  // SSE2
#include <smmintrin.h>  // SSE4.1
#include <nmmintrin.h>  // SSE4.2
#include <cstdlib>

#include "utils.h"
#include "Image.h"
#include "raymarching.h"
#include <functional>

using namespace std::chrono;
using namespace std;
using namespace Utils;
using namespace Raymarching;


#define curT duration_cast<milliseconds>(system_clock::now().time_since_epoch());

Image test_render() {
	// Init output image
	const size_t w = 500, h = 500;
	Image ppmimg(w, h, 3);

	// Render ppmimage using raymarching
	for (int i = 0; i < ppmimg.w; i++) {
		for (int j = 0; j < ppmimg.h; j++) {
			// Main color function - returns the color of a given pixel
			Vector<3> col = get_color(i, j, w, h);

			// Set rgb channels
			ppmimg.set(i, j, 0, std::clamp(col[0], 0., 255.));
			ppmimg.set(i, j, 1, std::clamp(col[1], 0., 255.));
			ppmimg.set(i, j, 2, std::clamp(col[2], 0., 255.));
		}
	}
	return ppmimg;
}
double test_dot_simd(size_t k = 100) {
	double sum = 0;
	for (int i = 0; i < k; i++) {
		float arr1[3] = { (rand()%100)/100., 2., 3.};
		float arr2[3] = { 1., 2., 3. };

		
		// Load 4 floats into simd registers
		__m128 vec1 = _mm_load_ps(arr1);
		__m128 vec2 = _mm_load_ps(arr2);

		__m128 res = _mm_dp_ps(vec1, vec2, 0b01110111);
		float dot_res;
		_mm_store_ss(&dot_res, res);

		sum += dot_res;
	}
	return sum;
}

int j = 0;
// Lazy initialization of std::vector of random vectors
const Vector<3>& randomvector(size_t index = 0) {
	static bool init = false;
	static size_t N = 100;
	static std::vector<Vector<3>> vectors = std::vector<Vector<3>>();
	if (!init) {
		init = true;
		for (size_t i = 0; i < N; i++) {
			vectors.push_back({ (rand() % 100) / 100.,
								(rand() % 100) / 100.,
								(rand() % 100) / 100. });
		}
	}
	size_t i = j++ % N;
	return vectors[i];
}
__m128 load_vector(const Vector<3>& vec) {
	float arr[4] = { vec[0], vec[1], vec[2], 0. };
	__m128 res = _mm_load_ps(arr);
	return res;
}
Vector<3> vector_from_register(__m128 v) {
	float res[4];
	_mm_store_ps(res, v);

	return { res[0], res[1], res[2]};
}
Vector<3> test_simd_color_calc(Vector<3> ambient, Vector<3> diffuse, Vector<3> normal, Vector<3> light_pos, Vector<3> p) {
	/*ambient* ca +
		diffuse * ((p - light1_pos).normalize() * normal) * cd +
		specular * pow(reflected * (dir * -1.), alpha) * cs;*/

	float ca = 0.7, cd = 0.2, cs = 0.5, alpha = 5.;

	__m128 ambient_reg = load_vector(ambient),
		diffuse_reg = load_vector(diffuse),
		normal_reg = load_vector(normal),
		light_pos_reg = load_vector(light_pos),
		p_reg = load_vector(p);

	//cout << "SIMD: " << vector_from_register(ambient_reg) << " " <<
		//vector_from_register(diffuse_reg) << " " <<
		//vector_from_register(normal_reg) << endl;

	// formula
	// final_color = 
	//  ambient* ca +
	//	diffuse * ((p-lightpos).normalize() * normal) * cd +
	//	specular * pow(reflected * (dir * -1.), alpha) * cs;


	__m128 ambient_scalar_reg = _mm_set_ps1(ca);
	__m128 diffuse_scalar_reg = _mm_set_ps1(cd);
	
	//cout << "SIMD scalar vectors: " << vector_from_register(ambient_scalar_reg) << " " <<
		//vector_from_register(diffuse_scalar_reg) << endl;


	// Final ambient color
	ambient_reg = _mm_mul_ps(ambient_reg, ambient_scalar_reg);
	
	// ambient * dc
	diffuse_reg = _mm_mul_ps(diffuse_reg, diffuse_scalar_reg);

	// p - light_pos
	__m128 light_dir = _mm_sub_ps(light_pos_reg, p_reg);
	
	// Length of light_dir vector
	// result is [len*len, len*len, len*len, 0]
	__m128 vec_len_reg = _mm_dp_ps(light_dir, light_dir, 0b01110111);

	// normalize: scalar multiplication by (1/sqrt(len*len))
	__m128 light_dir_norm = _mm_mul_ps(light_dir, _mm_rsqrt_ps(vec_len_reg));

	float light_coef;
	// dot(light_dir_normailized, normal)
	__m128 light_coef_reg = _mm_dp_ps(light_dir_norm, normal_reg, 0b01110111);
	
	//cout << "SIMD LIGHT COEF1: " << vector_from_register(light_coef_reg) << endl;

	diffuse_scalar_reg = _mm_mul_ps(diffuse_scalar_reg, light_coef_reg);

	//cout << "SIMD TOTAL diffuse COEF: " << vector_from_register(diffuse_scalar_reg) << endl;
	//cout << "SIMD diffuse Color: " << vector_from_register(diffuse_reg) << endl;

	// Final diffuse color
	diffuse_reg = _mm_mul_ps(diffuse_reg, light_coef_reg);

	//cout << "SIMD: " << vector_from_register(ambient_reg) << " " << vector_from_register(diffuse_reg) << endl;
	// Final color
	__m128 final_color_reg = _mm_add_ps(ambient_reg, diffuse_reg);

	Vector<3> final_color = vector_from_register(final_color_reg);
	return final_color;
}
Vector<3> test_simple_color_calc(Vector<3> ambient, Vector<3> diffuse, Vector<3> normal, Vector<3> light_pos, Vector<3> p) {
	/*ambient* ca +
		diffuse * ((p - light1_pos).normalize() * normal) * cd +
		specular * pow(reflected * (dir * -1.), alpha) * cs;*/

	//cout << "SIMPLE:" << ambient << " " << diffuse << " " << normal << endl;
	float ca = 0.7, cd = 0.2, cs = 0.5, alpha = 5.;

	//cout << "SIMPLE LIGHT COEF 1: " << (p - light_pos).normalize() * normal << endl;
	//cout << "SIMPLE TOTAL LIGHT COEF: " << (p - light_pos).normalize() * normal*cd << endl;

	//cout << "SIMPLE: " << ambient * ca << " " << diffuse * ((p - light_pos).normalize() * normal) * cd << endl;
	return ambient * ca + 
		diffuse * ((p - light_pos).normalize() * normal) * cd;
}
double test_dot_iter(size_t k = 100) {
	double sum = 0;
	for (int i = 0; i < k; i++) {
		float arr1[3] = { (rand() % 100) / 100., 2., 3. };
		float arr2[3] = { 1., 2., 3. };
		sum += arr1[0]*arr2[0] + arr1[1]*arr2[1] + arr1[2]*arr2[2];
	}
	return sum;
}
int main()
{
	std::srand(0);
	const int N = 3;
	milliseconds avg_time = milliseconds(0);

	// Init data in advance
	vector<Vector<3>> vecs;
	for (int k = 0; k < N; k++) {
		vecs.push_back(randomvector());
	}
	// N tests
	for (int k = 0; k < N; k++) {
		milliseconds start_time = curT;
		//Image ppmimg = test_render();

		Vector<3> sum= { 0., 0., 0. };
		//cout << "SUM IS: " << test_dot_iter(100000000) << endl;
		for (size_t i = 0; i < 100000000; i++) {

			// Simple random vector selection
			Vector<3> ambient = vecs[i%N],
				diffuse = vecs[i%N],
				normal = vecs[(i+1)%N],
				light_pos = vecs[(i + 2) % N],
				p = vecs[(i + 3) % N];

			sum = sum + test_simple_color_calc(ambient, diffuse, normal, light_pos, p);
		}

		milliseconds end_time = curT;

		cout << "Total time " << k << " :" << end_time - start_time << "\t\t " << sum << endl;
		avg_time += end_time - start_time;

		// Dont include writing time in benchmark
		//writePPM("res" + to_string(k) + ".ppm", ppmimg);

	}
	cout << avg_time / (double)N;
	return 0;
}
