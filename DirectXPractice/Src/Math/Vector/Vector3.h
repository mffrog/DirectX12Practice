#pragma once
#include "VectorAccuracy.h"
#include <iostream>

namespace mff {

#define VEC3_ONE_ARG_OPERATOR(op)\
Vector3& operator op (const Vector3& arg){\
	x op arg.x;\
	y op arg.y;\
	z op arg.z;\
	return *this;\
}

#define VEC3_ONE_ARG_OTHER_OPERATOR(op)\
template<typename U>\
Vector3& operator op (const Vector3<U>& arg){\
	x op static_cast<T>(arg.x);\
	y op static_cast<T>(arg.y);\
	z op static_cast<T>(arg.z);\
	return *this;\
}

	template<typename T>
	struct Vector3 {
		Vector3(T v = 0) :x(v), y(v), z(v) {}
		Vector3(T x, T y, T z) : x(x), y(y), z(z) {}

		template<typename U>
		Vector3<T>& operator=(const Vector3<U>& arg) {
			x = static_cast<T>(arg.x);
			y = static_cast<T>(arg.y);
			z = static_cast<T>(arg.z);
			return *this;
		}

		T Length() const {
			return sqrt(x * x + y * y + z * z);
		}
		T LengthSq() const {
			return dot(*this, *this);
		}
		VEC3_ONE_ARG_OPERATOR(+= );
		VEC3_ONE_ARG_OPERATOR(-= );
		VEC3_ONE_ARG_OPERATOR(*= );
		VEC3_ONE_ARG_OPERATOR(/= );

		VEC3_ONE_ARG_OTHER_OPERATOR(+= );
		VEC3_ONE_ARG_OTHER_OPERATOR(-= );
		VEC3_ONE_ARG_OTHER_OPERATOR(*= );
		VEC3_ONE_ARG_OTHER_OPERATOR(/= );

		Vector3& operator *= (T scaler) {
			x *= scaler;
			y *= scaler;
			z *= scaler;
			return *this;
		}

		Vector3& operator /= (T scaler) {
			x /= scaler;
			y /= scaler;
			z /= scaler;
			return *this;
		}

		union {
			T m[3];
			struct { T x, y, z; };
			struct { T r, g, b; };
			struct { T s, t, u; };
		};
	};

#define VEC3_TWO_ARG_OPERATOR(op)\
template<typename T>\
Vector3<T> operator op (const Vector3<T>& lhs, const Vector3<T>& rhs){\
	return Vector3<T>(lhs.x op rhs.x, lhs.y op rhs.y, lhs.z op rhs.z);\
}
#define VEC3_TWO_ARG_OTHERS_OPERATOR(op)\
template<typename T, typename U>\
Vector3<typename HighAccuracy<T,U>::type> operator op (const Vector3<T>& lhs, const Vector3<U>& rhs){\
	return Vector3<typename HighAccuracy<T,U>::type>(lhs.x op rhs.x,lhs.y op rhs.y,lhs.z op rhs.z);\
}
#define VEC3_SCALER_OPERATION(op)\
template<typename T>\
Vector3<T> operator op (const Vector3<T>& vec, T scaler){\
	return Vector3<T>(vec.x op scaler, vec.y op scaler, vec.z op scaler);\
}\
template<typename T>\
Vector3<T> operator op (T scaler, const Vector3<T>& vec){\
	return Vector3<T>(scaler op vec.x, scaler op vec.y, scaler op vec.z);\
}

#define VEC3_OTHER_SCALER_OPERATION(op)\
template<typename T, typename U>\
Vector3<typename HighAccuracy<T,U>::type> operator op (const Vector3<T>& vec, U scaler){\
	return Vector3<typename HighAccuracy<T,U>::type>(vec.x op scaler, vec.y op scaler, vec.z op scaler);\
}\
template<typename T, typename U>\
Vector3<typename HighAccuracy<T,U>::type> operator op (U scaler, const Vector3<T>& vec) {\
	return Vector3<typename HighAccuracy<T,U>::type>(scaler op vec.x, scaler op vec.y, scaler op vec.z);\
}

	VEC3_TWO_ARG_OPERATOR(+);
	VEC3_TWO_ARG_OPERATOR(-);
	VEC3_TWO_ARG_OPERATOR(*);
	VEC3_TWO_ARG_OPERATOR(/);

	VEC3_TWO_ARG_OTHERS_OPERATOR(+);
	VEC3_TWO_ARG_OTHERS_OPERATOR(-);
	VEC3_TWO_ARG_OTHERS_OPERATOR(*);
	VEC3_TWO_ARG_OTHERS_OPERATOR(/);

	VEC3_SCALER_OPERATION(*);
	VEC3_SCALER_OPERATION(/);

	VEC3_OTHER_SCALER_OPERATION(*);
	VEC3_OTHER_SCALER_OPERATION(/);

	template<typename T>
	bool operator==(const Vector3<T>& lhs, const Vector3<T>& rhs) {
		return (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z);
	}

	template<>
	inline bool operator==(const Vector3<float>& lhs, const Vector3<float>& rhs) {
		return (FloatEqual(lhs.x, rhs.x) && FloatEqual(lhs.y, rhs.y) && FloatEqual(lhs.z, rhs.z));
	}

	template<>
	inline bool operator==(const Vector3<double>& lhs, const Vector3<double>& rhs) {
		return (DoubleEqual(lhs.x, rhs.x) && DoubleEqual(lhs.y, rhs.y) && DoubleEqual(lhs.z, rhs.z));
	}

	template<typename T>
	void print(const Vector3<T>& v) {
		std::cout << "x = " << v.x << ", y = " << v.y << ", z = " << v.z << std::endl;
	}

	template<typename T>
	T dot(const Vector3<T>& lhs, const Vector3<T>& rhs) {
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z;
	}

	template<typename T>
	Vector3<T> cross(const Vector3<T>& lhs, const Vector3<T>& rhs) {
		return Vector3<T>(lhs.y * rhs.z - lhs.z * rhs.y, lhs.z * rhs.x - lhs.x * rhs.z, lhs.x * rhs.y - lhs.y * rhs.x);
	}

	template<typename T>
	Vector3<T> Normalize(const Vector3<T>& vec) {
		return vec / sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
	}
} // namespace mff