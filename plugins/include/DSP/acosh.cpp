/*
 * Acosh, inverse hyperbolic cosine
 */

#include <stdlib.h>
#include <math.h>
#include "acosh.h"

double acosh(double x) {
	double result;
	result = log(x + sqrt(x * x - 1.0));
	return (result);
}

