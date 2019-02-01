#pragma once

#include <cmath>

#define MATH_PI			(float)3.14159265358979323846
#define MATH_2PI		MATH_PI * 2.0f
#define MATH_E			(float)2.71828182845904523536
#define MATH_SQRT2		(float)1.41421356237309504880

#define MATH_DEG_TO_RAD(deg) (MATH_PI / 180.0f) * deg
#define MATH_RAD_TO_DEG(rad) (180.0f / MATH_PI) * rad

#define MATH_MIN(n1, n2) n1 <= n2 ? n1 : n2
#define MATH_MAX(n1, n2) n1 >= n2 ? n1 : n2
#define MATH_CLAMP(x, upper, lower) MATH_MIN(upper, MATH_MAX(lower, x))

#define MATH_LERP(a, b, c)  a + c * (b - a)
