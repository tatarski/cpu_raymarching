#pragma once
#include <initializer_list>
#include <algorithm>
#include <cassert>
#include <ostream>

namespace Utils {

	template<size_t M>
	class Vector {
	private:
		double arr[M];
	public:
		Vector() {};

		Vector(std::initializer_list<double> c) {
			assert(c.size() == M);
			std::copy(c.begin(), c.end(), arr);
		}
		const double& operator[] (size_t i) const {
			return arr[i];
		}
		double& operator[] (size_t i) {
			return arr[i];
		}
		friend std::ostream& operator<<(std::ostream& os, const Vector<M>& v) {
			for (size_t i = 0; i < M; i++) {
				os << v.arr[i] << " ";
			}
			return os;
		}
		double len() const;

		Vector<M> normalize() const;
	};

	template<size_t N, size_t M>
	class Matrix{
	private:
		Vector<M> rows[N];
	public:
		Matrix() {};
		Matrix(std::initializer_list<Vector<M>> c) {
			assert(c.size() == N);
			std::copy(c.begin(), c.end(), rows);
		}
		const Vector<M>& operator[](size_t i) const {
			return rows[i];
		}
		Vector<M>& operator[](size_t i) {
			return rows[i];
		}
		friend std::ostream& operator<<(std::ostream& os, const Matrix<N, M>& m) {
			for (int i = 0; i < N; i++) {
				os << m[i] << "\n";
			}
			return os;
		}
	};

	template<size_t M>
	double dot_product(const Vector<M>& a, const Vector<M>& b);

	template<size_t M>
	double operator*(const Vector<M>& a, const Vector<M>& b) {
		return dot_product(a, b);
	}


	template<size_t N, size_t M, size_t K>
	Matrix<N, K> multiply(const Matrix<N, M>& a, const Matrix<M, K>& b);

	template<size_t N, size_t M, size_t K>
	Matrix<N, K> operator*(const Matrix<N, M>& A, const Matrix<M, K>& B) {
		return multiply<N, M, K>(A, B);
	}

	template<size_t N, size_t M>
	Vector<N> multiply(const Matrix<N, M>& A, const Vector<M> v);

	template<size_t N, size_t M>
	Vector<N> operator*(const Matrix<N, M>& A, const Vector<M>& c) {
		return multiply<N, M>(A, c);
	}

	template<size_t M>
	Vector<M> multiply(const Vector<M>& v, double c);

	template<size_t M>
	Vector<M> operator*(const Vector<M>& v, double c) {
		return multiply<M>(v, c);
	}
	template<size_t M>
	Vector<M> add(const Vector<M>& v1, const Vector<M>& v2);
	
	template<size_t M>
	Vector<M> operator+(const Vector<M>& v1, const Vector<M>& v2) {
		return add(v1, v2);
	}
	template<size_t M>
	Vector<M> operator-(const Vector<M>& v1, const Vector<M>& v2) {
		return add(v1*-1., v2);
	}
	
	Vector<3> cross(const Vector<3>& v1, const Vector<3>& v2) {
		return Vector<3> {
			v1[1]*v2[2] - v1[2]*v2[1],
			v1[2]*v2[0] - v1[0]*v2[2],
			v1[0]*v2[1] - v1[1]*v2[0]
		};
	}
}

#include "utils.inl"