/*
	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "calibrator_misc.hpp"

#include <cmath>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_multifit.h>
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_vector.h>
#include <iostream>
#include <sstream>
#include <stdexcept>

double estimateMean(const std::vector<double> &samples)
{
	return gsl_stats_mean(&samples.at(0), 1, samples.size());
}

double estimateStandardDeviation(const std::vector<double> &samples, double mean)
{
	double value = gsl_stats_variance_m(&samples.at(0), 1, samples.size(), mean);
	return std::sqrt(value);
}

double estimateStandardDeviationOfMean(const std::vector<double> &samples, double mean)
{
	double value = gsl_stats_variance_m(&samples.at(0), 1, samples.size(), mean);
	return std::sqrt(value / samples.size());
}

// returns polynomial coefficients for polynomial fit to data y = f(x)
std::vector<double> fitPolynomial(const std::vector<double> &x, const std::vector<double> &y,
	double expansionOrigin, unsigned order)
{
	const int NUM_COEFFICIENTS = order + 1;
	if(x.size() != y.size())
	{
		std::ostringstream message;
		message << __FUNCTION__ << ": x and y sizes are different.\n";
		throw std::invalid_argument(message.str());
	}
	gsl_matrix *covariance = gsl_matrix_alloc(NUM_COEFFICIENTS, NUM_COEFFICIENTS);
	gsl_multifit_linear_workspace *work = gsl_multifit_linear_alloc(x.size(), NUM_COEFFICIENTS);
	double chisq;
	gsl_matrix *m = gsl_matrix_alloc(x.size(), NUM_COEFFICIENTS);
	unsigned i;
	for(i = 0; i < x.size(); ++i)
	{
		gsl_matrix_set(m, i, 0, 1.);
		int j;
		for(j = 1; j < NUM_COEFFICIENTS; ++j)
		{
			gsl_matrix_set(m, i, j, gsl_matrix_get(m, i, j - 1) * (x.at(i) - expansionOrigin));
		}
	}
	gsl_vector_const_view b = gsl_vector_const_view_array(&y.at(0), y.size());
	std::vector<double> coefficients(NUM_COEFFICIENTS);
	gsl_vector_view result = gsl_vector_view_array(&coefficients.at(0), NUM_COEFFICIENTS);
	gsl_multifit_linear(m, &b.vector, &result.vector, covariance, &chisq, work);
	gsl_matrix_free(m);
	gsl_matrix_free(covariance);
	gsl_multifit_linear_free(work);
	return coefficients;
}

Polynomial::Polynomial(): expansionOrigin(0.)
{}

double Polynomial::operator()(double input) const
{
	double value = 0.;
	double term = 1.;
	unsigned i;
	for(i = 0; i < coefficients.size(); ++i)
	{
		value += coefficients.at(i) * term;
		term *= input - expansionOrigin;
	}
	return value;
}

unsigned Polynomial::order() const
{
	if(coefficients.size() < 1) throw std::invalid_argument(__FUNCTION__);
	return coefficients.size() - 1;
}

void printPolynomial(const Polynomial &polynomial)
{
	std::cout << "Polynomial:\n";
	std::cout << "\torder = " << polynomial.order() << "\n";
	std::cout << "\texpansion origin = " << polynomial.expansionOrigin << "\n";
	unsigned j;
	for(j = 0; j < polynomial.coefficients.size(); ++j)
		std::cout << "\torder " << j << " coefficient = " << polynomial.coefficients.at(j) << "\n";
	std::cout << std::flush;
}

