#pragma once
#include "../Vector/Vector3.h"
#include "../Vector/Vector4.h"
namespace mff {
	template<typename T>
	struct Matrix4x3 {
		Matrix4x3(T v = static_cast<T>(1)) {
			m[0] = v;
			m[5] = v;
			m[10] = v;
		}

		Matrix4x3(const Vector4<T>& v1, const Vector4<T>& v2, const Vector4<T>& v3) {
			v[0] = v1;
			v[1] = v2;
			v[2] = v3;
		}

		Matrix4x3& operator+=(const Matrix4x3& arg) {
			v[0] += arg.v[0];
			v[1] += arg.v[1];
			v[2] += arg.v[2];
		}

		Matrix4x3& operator-=(const Matrix4x3& arg) {
			v[0] -= arg.v[0];
			v[1] -= arg.v[1];
			v[2] -= arg.v[2];
		}

		Matrix4x3& operator*=(T scaler) {
			v[0] *= scaler;
			v[1] *= scaler;
			v[2] *= scaler;
		}

		Matrix4x3& operator/=(T scaler) {
			v[0] /= scaler;
			v[1] /= scaler;
			v[2] /= scaler;
		}

		Vector4<T>& operator[](int idx) {
			return v[idx];
		}

		const Vector4<T>& operator[](int idx) {
			return v[idx];
		}

		union {
			T m[12];
			Vector4<T> v[3];
		};
	};

	template<typename T>
	Matrix4x3<T> operator+(const Matrix4x3<T>& lhs, const Matrix4x3<T>& rhs) {
		return Matrix4x3<T>(lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1], lhs.v[2] + rhs.v[2]);
	}

	template<typename T>
	Matrix4x3<T> operator-(const Matrix4x3<T>& lhs, const Matrix4x3<T>& rhs) {
		return Matrix4x3<T>(lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1], lhs.v[2] - rhs.v[2]);
	}

	template<typename T>
	Matrix4x3<T> operator*(const Matrix4x3<T>& mat, T scaler) {
		return Matrix4x3<T>(mat.v[0] * scaler, mat.v[1] * scaler, mat.v[2] * scaler);
	}

	template<typename T>
	Matrix4x3<T> operator*(T scaler, const Matrix4x3<T>& mat) {
		return Matrix4x3<T>(scaler * mat.v[0], scaler * mat.v[1], scaler * mat.v[2]);
	}

	template<typename T>
	Vector3<T> operator*(const Matrix4x3<T>& mat, const Vector4<T>& vec) {
		return Vector3<T>(dot(mat.v[0], vec), dot(mat.v[1], vec), dot(mat.v[2],vec));
	}
} // namespace mff