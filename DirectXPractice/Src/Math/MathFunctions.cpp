#include "MathFunctions.h"
#include <math.h>
#include <algorithm>

bool FloatEqual(float a, float b, float epsilon) {
	float diff = fabsf(a - b);
	if (diff < epsilon) {
		return true;
	}
	float relativeEpsilon = (std::max(fabsf(a), fabsf(b))) * epsilon;
	return diff < relativeEpsilon;
}

bool DoubleEqual(double a, double b, double epsilon) {
	double diff = fabs(a - b);
	if (diff < epsilon) {
		return true;
	}
	double relativeEpsilon = (std::max(fabs(a), fabs(b))) * epsilon;
	return diff < relativeEpsilon;
}

