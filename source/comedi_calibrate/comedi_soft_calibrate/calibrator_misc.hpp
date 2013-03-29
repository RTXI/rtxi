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

#ifndef _CALIBRATOR_MISC_HPP
#define _CALIBRATOR_MISC_HPP

#include <vector>

double estimateMean(const std::vector<double> &samples);
double estimateStandardDeviation(const std::vector<double> &samples, double mean);
double estimateStandardDeviationOfMean(const std::vector<double> &samples, double mean);
std::vector<double> fitPolynomial(const std::vector<double> &x, const std::vector<double> &y, double expansionOrigin, unsigned order);

class Polynomial
{
public:
	Polynomial();
	double operator()(double input) const;
	unsigned order() const;

	std::vector<double> coefficients;
	double expansionOrigin;
private:
};

void printPolynomial(const Polynomial &polynomial);

#endif	// _CALIBRATOR_MISC_HPP
