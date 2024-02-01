// raymarching.cpp : Defines the entry point for the application.
//

#include <iostream>
#include<chrono>
#include <immintrin.h>
#include <smmintrin.h>
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
	const int N = 10;
	milliseconds avg_time = milliseconds(0);
	// N test renders
	for (int k = 0; k < N; k++) {
		milliseconds start_time = curT;

		Image ppmimg = test_render();

		//cout << "SUM IS: " << test_dot_iter(100000000) << endl;

		milliseconds end_time = curT;


		cout << "Time for iter " << k << " :" << end_time - start_time << endl;
		
		avg_time += end_time - start_time;

		// Dont include writing time in benchmark
		writePPM("res" + to_string(k) + ".ppm", ppmimg);

	}
	cout << avg_time / (double)N;
	return 0;
}
