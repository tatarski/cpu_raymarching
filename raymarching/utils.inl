#include<cmath>

namespace Utils {

	template<size_t N, size_t M, size_t K>
	Matrix<N, K> multiply(const Matrix<N, M>& a, const Matrix<M, K>& b)
	{
		Matrix<N, K> res;

		for (size_t i = 0; i < N; i++) {
			for (size_t j = 0; j < K; j++) {
				double dot = 0;
				for (size_t l = 0; l < M; l++) {
					dot += a[i][l] * b[l][j];
				}
				res[i][j] = dot;
			}
		}
		return res;
	}

	template<size_t M>
	double dot_product(const Vector<M>& a, const Vector<M>& b)
	{
		double res = 0;
		for (size_t i = 0; i < M; i++) {
			res += a[i] * b[i];
		}
		return res;
	}

	template<size_t N, size_t M>
	Vector<N> multiply(const Matrix<N, M>& A, const Vector<M> v) {
		Vector<N> res;
		for (int i = 0; i < M; i++) {
			res[i] = dot_product(A[i], v);
		}
		return res;
	}

	template<size_t M>
	Vector<M> multiply(const Vector<M>& v, double c) {
		Vector<M> res;
		for (int i = 0; i < M; i++) {
			res[i] = v[i]*c;
		}
		return res;
	}

	template<size_t M>
	double Vector<M>::len() const
	{
		return sqrt(dot_product(*this, *this));
	}

	template<size_t M>
	inline Vector<M> Vector<M>::normalize() const
	{
		return multiply(*this, 1. / len());
	}

	template<size_t M>
	Vector<M> add(const Vector<M>& v1, const Vector<M>& v2) {
		Vector<M> res;
		for (int i = 0; i < M; i++) {
			res[i] = v1[i] + v2[i];
		}
		return res;
	}
}