#pragma once
#include "../Vector/Vector4.h"
//メモリレイアウト　→　v[i] がそれぞれ行
//データレイアウト　↓
/*
	t = translation, r = rotation, s = scale

	sr, sr, sr, t
	sr, sr, sr, t
	sr, sr, sr, t
	 0,  0,  0, 1
*/
namespace mff {
	template<typename T>
	struct Matrix4x4 {
		Matrix4x4(T v = static_cast<T>(1)) : v{ Vector4<T>(0),Vector4<T>(0),Vector4<T>(0),Vector4<T>(0) } {
			m[0] = v;
			m[5] = v;
			m[10] = v;
			m[15] = v;
		}
		Matrix4x4(const Vector4<T>& v1, const Vector4<T>& v2, const Vector4<T>& v3, const Vector4<T>& v4) {
			v[0] = v1;
			v[1] = v2;
			v[2] = v3;
			v[3] = v4;
		}

		Matrix4x4& operator+=(const Matrix4x4& arg) {
			v[0] += arg.v[0];
			v[1] += arg.v[1];
			v[2] += arg.v[2];
			v[3] += arg.v[3];
		}

		Matrix4x4& operator-=(const Matrix4x4& arg) {
			v[0] -= arg.v[0];
			v[1] -= arg.v[1];
			v[2] -= arg.v[2];
			v[3] -= arg.v[3];
		}

		Matrix4x4& operator*=(const Matrix4x4& arg) {
			for (int i = 0; i < 4; ++i) {
				Vector4<T> tmp(arg[0][i],arg[1][i],arg[2][i],arg[3][i]);
				for (int j = 0; j < 4; ++j) {
					v[j][i] = dot(v[j], tmp);
				}
			}
		}

		Vector4<T>& operator[](int idx) {
			return v[idx];
		}

		const Vector4<T>& operator[](int idx) const {
			return v[idx];
		}

		union {
			T m[16];
			Vector4<T> v[4];
		};
	};

	template<typename T>
	Matrix4x4<T> operator+(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs) {
		return Matrix4x4<T>(lhs.v[0] + rhs.v[0], lhs.v[1] + rhs.v[1], lhs.v[2] + rhs.v[2], lhs.v[3] + rhs.v[3]);
	}

	template<typename T>
	Matrix4x4<T> operator-(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs) {
		return Matrix4x4<T>(lhs.v[0] - rhs.v[0], lhs.v[1] - rhs.v[1], lhs.v[2] - rhs.v[2], lhs.v[3] - rhs.v[3]);
	}
	
	template<typename T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& lhs, const Matrix4x4<T>& rhs) {
		Matrix4x4<T> ret(static_cast<T>(0));
		for (int column = 0; column < 4; ++column) {
			Vector4<T> tmp(rhs.v[0][column], rhs.v[1][column], rhs.v[2][column], rhs.v[3][column]);
			for (int row = 0; row < 4; ++row) {
				ret[row][column] = dot(lhs[row], tmp);
			}
		}
		return ret;
	}

	//template<typename T>
	//Vector4<T> operator*(const Vector4<T>& vec, const Matrix4x4<T>& mat) {
	//	return Vector4<T>(
	//		dot(vec, Vector4<T>(mat[0][0], mat[1][0], mat[2][0], mat[3][0])),
	//		dot(vec, Vector4<T>(mat[0][1], mat[1][1], mat[2][1], mat[3][1])),
	//		dot(vec, Vector4<T>(mat[0][2], mat[1][2], mat[2][2], mat[3][2])),
	//		dot(vec, Vector4<T>(mat[0][3], mat[1][3], mat[2][3], mat[3][3]))
	//		);
	//}

	template<typename T>
	Vector4<T> operator*(const Matrix4x4<T>& mat, const Vector4<T>& vec) {
		return Vector4<T>(
			dot(mat[0], vec),
			dot(mat[1], vec),
			dot(mat[2], vec),
			dot(mat[3], vec)
			);
	}

	template<typename T>
	Matrix4x4<T> operator*(const Matrix4x4<T>& mat, T scaler){
		return Matrix4x4<T>(mat[0] * scaler, mat[1] * scaler, mat[2] * scaler, mat[3] * scaler);
	}

	template<typename T>
	Matrix4x4<T> operator*(T scaler, const Matrix4x4<T>& mat) {
		return Matrix4x4<T>(scaler * mat[0], scaler * mat[1], scaler * mat[2], scaler * mat[3]);
	}

	template<typename T>
	void print(const Matrix4x4<T>& mat) {
		for (int i = 0; i < 4; ++i) {
			for (int j = 0; j < 4; ++j) {
				std::cout << "m[" << i * 4 + j << "] = " << mat[i][j] << " ";
			}
			std::cout << std::endl;
		}
	}
} //namespace mff