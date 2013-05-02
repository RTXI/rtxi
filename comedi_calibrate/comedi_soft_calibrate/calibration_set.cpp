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

#include "calibration_set.hpp"

#include <algorithm>
#include <boost/bind.hpp>
#include <cstdlib>
#include <sstream>
#include <stdexcept>
#include <cstdlib>

// SubdeviceCalibration

const Polynomial& SubdeviceCalibration::polynomial(unsigned channel, unsigned range) const
{
	std::pair<unsigned, unsigned> channelRange;
	channelRange.first = channel;
	channelRange.second = range;
	std::map<std::pair<unsigned, unsigned>, Polynomial>::const_iterator it =
		std::find_if(_polynomials.begin(), _polynomials.end(), boost::bind(&SubdeviceCalibration::channelRangeMatch, channel, range, _1));
	if(it == _polynomials.end())
	{
		std::ostringstream message;
		message << __FUNCTION__ << ": failed to find calibration polynomial for channel " << channel << ", range " << range << " .\n";
		throw std::invalid_argument(message.str());
	}
	return it->second;
}

void SubdeviceCalibration::insertPolynomial(const Polynomial &polynomial, unsigned channel, unsigned range)
{
	std::pair<unsigned, unsigned> channelRange;
	channelRange.first = channel;
	channelRange.second = range;
	_polynomials[channelRange] = polynomial;
};

// private functions

bool SubdeviceCalibration::channelRangeMatch(unsigned channel, unsigned range,
	const std::pair<std::pair<unsigned, unsigned>, Polynomial> &calibration)
{
	std::pair<unsigned, unsigned> channelRange1;
	channelRange1.first = channel;
	channelRange1.second = range;
	const std::pair<unsigned, unsigned> &channelRange2 = calibration.first;
	if(channelRange1.first == channelRange2.first || channelRange1.first == allChannels || channelRange2.first == allChannels)
	{
		if(channelRange1.second == channelRange2.second || channelRange1.second == allRanges || channelRange2.second == allRanges)
		{
			return true;
		}
	}
	return false;
}
