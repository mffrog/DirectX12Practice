#pragma once
#include <math.h>
namespace mff {
	struct CharType {
		using type = char;
	};
	struct IntType {
		using type = int;
	};
	struct FloatType {
		using type = float;
	};
	struct DoubleType {
		using type = double;
	};


	template<typename T, typename U>
	struct HighAccuracy : DoubleType {};

#define DECLARE_HIGH_ACCURACY(T,U,Super)\
template<>\
struct HighAccuracy<T, U> : Super {};

	//char
	DECLARE_HIGH_ACCURACY(char, int, IntType);
	DECLARE_HIGH_ACCURACY(int, char, IntType);
	DECLARE_HIGH_ACCURACY(char, float, FloatType);
	DECLARE_HIGH_ACCURACY(float, char, FloatType);
	DECLARE_HIGH_ACCURACY(char, double, DoubleType);
	DECLARE_HIGH_ACCURACY(double, char, DoubleType);

	//int
	DECLARE_HIGH_ACCURACY(int, float, FloatType);
	DECLARE_HIGH_ACCURACY(float, int, FloatType);
	DECLARE_HIGH_ACCURACY(int, double, DoubleType);
	DECLARE_HIGH_ACCURACY(double, int, DoubleType);

} // namespace mff