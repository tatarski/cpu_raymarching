#pragma once
#include <string>
#include <vector>
#include <cassert>
#include <string>
#include <fstream>
#include <cerrno>
#include<iostream>
class Image {
private:
	std::vector<double> channels;
public:
	// Width, height, number of channels
	const size_t w, h, c;
	Image(size_t w, size_t h, size_t c) : channels(std::vector<double>( w * h * c)), w(w), h(h), c(c) {}

	void set(size_t x, size_t y, size_t c, double g) {
		channels[c * w * h + w * y + x] = g;
	}
	double get(size_t x, size_t y, size_t c) const {
		return channels[c * w * h + w * y + x];
	}
	
};

void writePPM(std::string name, const Image& img) {
	assert(img.c == 3);
	std::ofstream os(name, std::ios_base::trunc);

	if (!os.is_open()) {
		std::cerr << errno << "Error opening file: " << errno << name << std::endl;
	}

	os << "P3\n";
	os << img.w << " " << img.h << "\n";
	os << "255\n";

	for (size_t i = 0; i < img.h; i++) {
		for (size_t j = 0; j < img.w; j++) {
			for (size_t c = 0; c < img.c; c++) {
				os << std::clamp((int)(img.get(j, i, c)*255), 0, 255) << " ";
			}
		}
	}
}