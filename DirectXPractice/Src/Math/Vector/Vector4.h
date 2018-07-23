#pragma once
#include "VectorAccuracy.h"
#include "Vector3.h"
#include <iostream>

namespace mff {


#define VEC4_ONE_ARG_OPERATOR(op)\
Vector4& operator op (const Vector4& arg){\
	x op arg.x;\
	y op arg.y;\
	z op arg.z;\
	w op arg.w;\
	return *this;\
}

#define VEC4_ONE_ARG_OTHER_OPERATOR(op)\
template<typename U>\
Vector4& operator op (const Vector4<U>& arg) {\
	x op static_cast<T>(arg.x);\
	y op static_cast<T>(arg.y);\
	z op static_cast<T>(arg.z);\
	w op static_cast<T>(arg.w);\
	return *this;\
}

	template<typename T>
	struct Vector4 {
		Vector4() :x(0), y(0), z(0), w(1) {}
		Vector4(T v) : x(v), y(v), z(v), w(v) {}
		Vector4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

		template<typename U>
		Vector4(const Vector3<T>& v, U w = 0) : x(v.x), y(v.y), z(v.z), w(static_cast<T>(w)) {}

		template<typename U>
		Vector4<T>& operator=(const Vector4<U>& arg) {
			x = static_cast<T>(arg.x);
			y = static_cast<T>(arg.y);
			z = static_cast<T>(arg.z);
			w = static_cast<T>(arg.w);
			return *this;
		}

		VEC4_ONE_ARG_OPERATOR(+= );
		VEC4_ONE_ARG_OPERATOR(-= );
		VEC4_ONE_ARG_OPERATOR(*= );
		VEC4_ONE_ARG_OPERATOR(/= );

		VEC4_ONE_ARG_OTHER_OPERATOR(+= );
		VEC4_ONE_ARG_OTHER_OPERATOR(-= );
		VEC4_ONE_ARG_OTHER_OPERATOR(*= );
		VEC4_ONE_ARG_OTHER_OPERATOR(/= );

		Vector4& operator *= (T scaler) {
			x *= scaler;
			y *= scaler;
			z *= scaler;
			w *= scaler;
			return *this;
		}
		Vector4& operator /= (T scaler) {
			x /= scaler;
			y /= scaler;
			z /= scaler;
			w /= scaler;
			return *this;
		}

		T& operator[](int idx) {
			return m[idx];
		}

		const T& operator[](int idx) const {
			return m[idx];
		}

		union {
			T m[4];
			struct { T x, y, z, w; };
			struct { T r, g, b, a; };
			struct { T s, t, u, v; };
		};
	};

#define VEC4_TWO_ARG_OPERATOR(op)\
template<typename T>\
Vector4<T> operator op (const Vector4<T>& lhs, const Vector4<T>& rhs){\
	return Vector4<T>(lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z, lhs.w op rhs.w);\
}
#define VEC4_TWO_ARG_OTHERS_OPERATOR(op)\
template<typename T, typename U>\
Vector4<typename HighAccuracy<T,U>::type> operator op (const Vector4<T>& lhs, const Vector4<U>& rhs){\
	return Vector4<typename HighAccuracy<T,U>::type>(lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z, lhs.w op rhs.w);\
}

#define VEC4_SCALER_OPERATION(op)\
template<typename T>\
Vector4<T> operator op (const Vector4<T>& vec, T scaler){\
	return Vector4<T>(vec.x op scaler, vec.y op scaler, vec.z op scaler, vec.w op scaler);\
}\
template<typename T>\
Vector4<T> operator op (T scaler, const Vector4<T>& vec){\
	return Vector4<T>(scaler op vec.x, scaler op vec.y, scaler op vec.z, scaler op vec.w);\
}

#define VEC4_OTHER_SCALER_OPERATION(op)\
template<typename T, typename U>\
Vector4<typename HighAccuracy<T,U>::type> operator op (const Vector4<T>& vec, U scaler){\
	return Vector4<typename HighAccuracy<T,U>::type>(vec.x op scaler, vec.y op scaler, vec.z op scaler, vec.w op scaler);\
}\
template<typename T, typename U>\
Vector4<typename HighAccuracy<T,U>::type> operator op (U scaler, const Vector4<T>& vec) {\
	return Vector4<typename HighAccuracy<T,U>::type>(scaler op vec.x, scaler op vec.y, scaler op vec.z, scaler op vec.w);\
}

	VEC4_TWO_ARG_OPERATOR(+);
	VEC4_TWO_ARG_OPERATOR(-);
	VEC4_TWO_ARG_OPERATOR(*);
	VEC4_TWO_ARG_OPERATOR(/ );

	VEC4_TWO_ARG_OTHERS_OPERATOR(+);
	VEC4_TWO_ARG_OTHERS_OPERATOR(-);
	VEC4_TWO_ARG_OTHERS_OPERATOR(*);
	VEC4_TWO_ARG_OTHERS_OPERATOR(/);

	VEC4_SCALER_OPERATION(*);
	VEC4_SCALER_OPERATION(/);

	VEC4_OTHER_SCALER_OPERATION(*);
	VEC4_OTHER_SCALER_OPERATION(/);

	template<typename T>
	bool operator==(const Vector4<T>& lhs, const Vector4<T>& rhs) {
		return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w);
	}

	template<>
	inline bool operator==(const Vector4<float>& lhs, const Vector4<float>& rhs) {
		return (FloatEqual(lhs.x, rhs.x) && FloatEqual(lhs.y, rhs.y) && FloatEqual(lhs.z, rhs.z) && FloatEqual(lhs.w, rhs.w));
	}

	template<>
	inline bool operator==(const Vector4<double>& lhs, const Vector4<double>& rhs) {
		return (DoubleEqual(lhs.x, rhs.x) && DoubleEqual(lhs.y, rhs.y) && DoubleEqual(lhs.z, rhs.z) && DoubleEqual(lhs.w, rhs.w));
	}

	template<typename T>
	void print(const Vector4<T>& v) {
		std::cout << "x = " << v.x << ", y = " << v.y << ", z = " << v.z << ", w = " << v.w << std::endl;
	}

	template<typename T>
	T dot(const Vector4<T>& lhs, const Vector4<T>& rhs) {
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	template<typename T>
	Vector4<T> Normalize(const Vector4<T>& vec) {
		return vec / sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z + vec.w * vec.w);
	}
} // namespace mff