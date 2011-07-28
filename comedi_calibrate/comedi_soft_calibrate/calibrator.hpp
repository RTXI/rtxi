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

#ifndef _CALIBRATOR_HPP
#define _CALIBRATOR_HPP

#include <boost/shared_ptr.hpp>
#include "calibration_set.hpp"
#include <comedilib.hpp>
#include <string>
#include <vector>

/* Abstract base class for hardware-specific classes.  Calibrators
 * determine the proper soft calibration coefficients for a piece of hardware
 * using its onboard calibration references.  */
class Calibrator
{
public:
	Calibrator() {}
	virtual ~Calibrator() {}
	virtual std::string supportedDriverName() const = 0;
	virtual std::vector<std::string> supportedDeviceNames() const = 0;
	virtual CalibrationSet calibrate(const comedi::device &dev) = 0;
private:
};

#endif	// _CALIBRATOR_HPP
