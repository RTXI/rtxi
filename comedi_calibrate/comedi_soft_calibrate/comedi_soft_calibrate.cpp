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

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>
#include "calibrator.hpp"
#include "../libcomedi_calibrate/comedi_calibrate_shared.h"
#include <comedilib.hpp>
#include <iostream>
#include "ni_m_series_calibrator.hpp"
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

void writeCalibrationSet(const CalibrationSet &calibration, const std::string &driverName,
	const std::string &boardName, const std::string &filePath)
{
	comedi_calibration_t *c_cal = static_cast<comedi_calibration_t *>(malloc(sizeof(comedi_calibration_t)));
	if(c_cal == 0) throw std::runtime_error("writeCalibrationSet: malloc failed\n");
	memset(c_cal, 0, sizeof(comedi_calibration_t));
	c_cal->driver_name = static_cast<char*>(malloc(driverName.size() + 1));
	strcpy(c_cal->driver_name, driverName.c_str());
	c_cal->board_name = static_cast<char*>(malloc(boardName.size() + 1));
	strcpy(c_cal->board_name, boardName.c_str());
	CalibrationSet::const_iterator it;
	for(it = calibration.begin(); it != calibration.end(); ++it)
	{
		const SubdeviceCalibration &subdeviceCalibration = it->second;
		std::map<std::pair<unsigned, unsigned>, Polynomial>::const_iterator jt;
		for(jt = it->second.polynomials().begin(); jt != it->second.polynomials().end(); ++jt)
		{
			comedi_calibration_setting_t *setting = sc_alloc_calibration_setting(c_cal);
			setting->subdevice = it->first;
			unsigned channel = jt->first.first;
			if(channel != SubdeviceCalibration::allChannels)
			{
				sc_push_channel(setting, channel);
			}
			unsigned range = jt->first.second;
			if(range != SubdeviceCalibration::allRanges)
			{
				sc_push_range(setting, range);
			}
			const Polynomial &polynomial = jt->second;
			comedi_polynomial_t *comediPolynomial = static_cast<comedi_polynomial_t*>(malloc(sizeof(comedi_polynomial_t)));
			assert(comediPolynomial);
			comediPolynomial->expansion_origin = polynomial.expansionOrigin;
			comediPolynomial->order = polynomial.order();
			unsigned i;
			for(i = 0; i < polynomial.coefficients.size(); ++i)
			{
				assert(i < COMEDI_MAX_NUM_POLYNOMIAL_COEFFICIENTS);
				comediPolynomial->coefficients[i] = polynomial.coefficients.at(i);
			}
			if(subdeviceCalibration.toPhys())
			{
				setting->soft_calibration.to_phys = comediPolynomial;
			}else
			{
				setting->soft_calibration.from_phys = comediPolynomial;
			}
		}
	}
	int retval = write_calibration_file(filePath.c_str(), c_cal);
	comedi_cleanup_calibration(c_cal);
	if(retval)
	{
		std::ostringstream message;
		message << __FUNCTION__ << ": write_calibration_file() failed.";
		throw std::runtime_error(message.str());
	}
}

class ComediSoftCalibrateApp
{
public:
	ComediSoftCalibrateApp(int argc, char **argv);
	virtual ~ComediSoftCalibrateApp();
	void exec();
private:
	boost::program_options::options_description desc;
	boost::program_options::variables_map vm;
	std::string _deviceFile;
	std::string _saveFile;
	comedi::device _comediDev;
	std::vector<boost::shared_ptr<Calibrator> > _calibrators;
};

ComediSoftCalibrateApp::ComediSoftCalibrateApp(int argc, char **argv):
	desc("Allowed options")
{
	desc.add_options()
		("help", "produce this help message and exit")
		("file,f", boost::program_options::value<typeof(_deviceFile)>(&_deviceFile)->default_value("/dev/comedi0"), "device file")
		("save-file,S", boost::program_options::value<typeof(_saveFile)>(&_saveFile)->default_value(""), "calibration save file")
	;
	try
	{
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
	}
	catch(const std::exception &err)
	{
		std::cerr << "Caught exception: " << err.what() << std::endl;
		std::cout << desc << std::endl;
		throw;
	}
	boost::program_options::notify(vm);
	_calibrators.push_back(boost::shared_ptr<Calibrator>(new NIMSeries::Calibrator()));
}

ComediSoftCalibrateApp::~ComediSoftCalibrateApp()
{}

void ComediSoftCalibrateApp::exec()
{
	if(vm.count("help"))
	{
		std::cout << desc << std::endl;
		return;
	}
	_comediDev.open(_deviceFile);
	std::vector<boost::shared_ptr<Calibrator> >::iterator it;
	for(it = _calibrators.begin(); it != _calibrators.end(); ++it)
	{
		if((*it)->supportedDriverName() != _comediDev.driver_name()) continue;
		std::vector<std::string> devices = (*it)->supportedDeviceNames();
		std::vector<std::string>::iterator dit;
		for(dit = devices.begin(); dit != devices.end(); ++dit)
		{
			if(*dit == _comediDev.board_name()) break;
		}
		if(dit == devices.end()) continue;
		break;
	}
	if(it == _calibrators.end())
	{
		std::ostringstream message;
		message << "Failed to find calibrator for " << _comediDev.driver_name() << " driver.";
		std::cerr << message.str() << std::endl;
		throw std::invalid_argument(message.str().c_str());
	}
	CalibrationSet calibration = (*it)->calibrate(_comediDev);
	if(_saveFile == "")
	{
		_saveFile = _comediDev.default_calibration_path();
	}
	writeCalibrationSet(calibration, _comediDev.driver_name(),
		_comediDev.board_name(), _saveFile);
}

int main(int argc, char **argv)
{
	try
	{
		ComediSoftCalibrateApp app(argc, argv);
		app.exec();
	}
	catch(const std::exception &err)
	{
		std::cerr << "Caught exception: " << err.what() << std::endl;
		return 1;
	}
	return 0;
}
