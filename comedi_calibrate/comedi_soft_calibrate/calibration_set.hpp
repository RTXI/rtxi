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

#ifndef _CALIBRATION_SET_HPP
#define _CALIBRATION_SET_HPP

#include "calibrator_misc.hpp"
#include <map>
#include <utility>
#include <string>
#include <vector>


class SubdeviceCalibration
{
public:
	SubdeviceCalibration(bool toPhys = true): _toPhys(toPhys) {}
	static const unsigned allChannels = static_cast<unsigned>(-1);
	static const unsigned allRanges = static_cast<unsigned>(-1);
	const Polynomial& polynomial(unsigned channel = allChannels, unsigned range = allRanges) const;
	void insertPolynomial(const Polynomial &polynomial, unsigned channel = allChannels, unsigned range = allRanges);
	bool toPhys() const {return _toPhys;}
	const std::map<std::pair<unsigned, unsigned>, Polynomial>& polynomials() const {return _polynomials;}
private:
	/* channelRangeMatch is passed to std::find so we can find
	* a matching calibration, taking into account the possibility
	* of allChannels and allRanges */
	static bool channelRangeMatch(unsigned channel, unsigned range,
		const std::pair<std::pair<unsigned, unsigned>, Polynomial> &calibration);
	std::map<std::pair<unsigned, unsigned>, Polynomial> _polynomials;
	bool _toPhys;
};

/* A complete set of calibration coefficients for a device */
typedef std::map<unsigned, SubdeviceCalibration> CalibrationSet;

#endif	// _CALIBRATION_SET_HPP
