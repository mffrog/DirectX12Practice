#pragma once

#include "VectorAccuracy.h"
#include "../MathFunctions.h"
#include <iostream>

namespace mff {
#define VEC2_ONE_ARG_OPERATOR(op) \
Vector2& operator op (const Vector2& arg){ \
	m[0] op arg.m[0];\
	m[1] op arg.m[1];\
	return *this;\
}

#define VEC2_ONE_ARG_OTHRES_OPERATOR(op)\
template<typename U>\
Vector2& operator op (const Vector2<U>& arg){\
	m[0] op static_cast<T>(arg.m[0]);\
	m[1] op static_cast<T>(arg.m[1]);\
	return *this;\
}

#define VEC2_TWO_ARG_OPERATOR(op)\
template<typename T>\
Vector2<T> operator op (const Vector2<T>& lhs, const Vector2<T>& rhs){\
	return Vector2<T>(lhs.x op rhs.x,lhs.y op rhs.y);\
}

	template<typename T>
	struct Vector2 {
		using type = T;
		Vector2(T v = 0) : x(v), y(v) {}
		Vector2(T x, T y) : x(x), y(y) {}

		template<typename U>
		Vector2& operator=(const Vector2<U>& arg) {
			printf("template operator\n");
			x = static_cast<T>(arg.x);
			y = static_cast<T>(arg.y);
			return *this;
		}

		VEC2_ONE_ARG_OPERATOR(+= );
		VEC2_ONE_ARG_OPERATOR(-= );
		VEC2_ONE_ARG_OPERATOR(*= );
		VEC2_ONE_ARG_OPERATOR(/= );

		VEC2_ONE_ARG_OTHRES_OPERATOR(+= );
		VEC2_ONE_ARG_OTHRES_OPERATOR(-= );
		VEC2_ONE_ARG_OTHRES_OPERATOR(*= );
		VEC2_ONE_ARG_OTHRES_OPERATOR(/= );

		Vector2& operator *= (T scaler) {
			x *= scaler;
			y *= scaler;
			return *this;
		}

		Vector2& operator /= (T scaler) {
			x /= scaler;
			y /= scaler;
			return *this;
		}

		T& operator[](int idx) {
			return m[idx];
		}

		const T& operator[](int idx) const {
			return m[idx];
		}

		union {
			T m[2];
			struct { T x, y; };
			struct { T s, t; };
		};
	};

	VEC2_TWO_ARG_OPERATOR(+);
	VEC2_TWO_ARG_OPERATOR(-);
	VEC2_TWO_ARG_OPERATOR(*);
	VEC2_TWO_ARG_OPERATOR(/ );

	template<typename T>
	Vector2<T> operator - (const Vector2<T>& arg) {
		return Vector2<T>(-arg.x, -arg.y);
	}

	template<typename T>
	Vector2<T> operator * (const Vector2<T>& vec, T scaler) {
		return Vector2<T>(vec.x * scaler, vec.y * scaler);
	}
	template<typename T>
	Vector2<T> operator * (T scaler, const Vector2<T>& vec) {
		return Vector2<T>(vec.x * scaler, vec.y * scaler);
	}

	template<typename T>
	Vector2<T> operator / (const Vector2<T>& vec, T scaler) {
		return Vector2<T>(vec.x / scaler, vec.y / scaler);
	}
	template<typename T>
	Vector2<T> operator / (T scaler, const Vector2<T>& vec) {
		return Vector2<T>(scaler / vec.x, scaler / vec.y);
	}

#define VEC2_TWO_ARG_OTHERS_OPERATOR(op)\
template<typename T, typename U>\
Vector2<typename HighAccuracy<T,U>::type> operator op (const Vector2<T>& lhs, const Vector2<U>& rhs){\
	return Vector2<typename HighAccuracy<T,U>::type>(lhs.x op rhs.x, lhs.y op rhs.y);\
}

#define VEC2_OTHER_SCALER_OPERATION(op)\
template<typename T, typename U>\
Vector2<typename HighAccuracy<T,U>::type> operator op (const Vector2<T>& vec, U scaler){\
	return Vector2<typename HighAccuracy<T,U>::type>(vec.x op scaler, vec.y op scaler);\
}\
template<typename T, typename U>\
Vector2<typename HighAccuracy<T,U>::type> operator op (U scaler, const Vector2<T>& vec) {\
	return Vector2<typename HighAccuracy<T,U>::type>(scaler op vec.x, scaler op vec.y);\
}

	VEC2_TWO_ARG_OTHERS_OPERATOR(+);
	VEC2_TWO_ARG_OTHERS_OPERATOR(-);
	VEC2_TWO_ARG_OTHERS_OPERATOR(*);
	VEC2_TWO_ARG_OTHERS_OPERATOR(/ );

	VEC2_OTHER_SCALER_OPERATION(*);
	VEC2_OTHER_SCALER_OPERATION(/);

	template<typename T>
	bool operator==(const Vector2<T>& lhs, const Vector2<T>& rhs) {
		return (lhs.x == rhs.x && lhs.y == rhs.y);
	}

	template<>
	inline bool operator==(const Vector2<float>& lhs, const Vector2<float>& rhs) {
		return (FloatEqual(lhs.x, rhs.x) && FloatEqual(lhs.x, rhs.x));
	}

	template<>
	inline bool operator==(const Vector2<double>& lhs, const Vector2<double>& rhs) {
		return (DoubleEqual(lhs.x, rhs.x) && DoubleEqual(lhs.y, rhs.y));
	}

	template<typename T>
	void print(const Vector2<T>& v) {
		std::cout << "x = " << v.x << ", y = " << v.y << std::endl;
	}

	template<typename T>
	T dot(const Vector2<T>& lhs, const Vector2<T>& rhs) {
		return lhs.x * rhs.x + lhs.y * rhs.y;
	}

	template<typename T>
	T cross(const Vector2<T>& lhs, const Vector2<T>& rhs) {
		return lhs.x * rhs.y - lhs.y * rhs.x;
	}

	template<typename T>
	Vector2<T> Normalize(const Vector2<T>& vec) {
		return vec / sqrt(vec.x * vec.x + vec.y * vec.y);
	}
} // namespace mff