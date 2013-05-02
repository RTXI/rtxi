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

#ifndef _NI_M_SERIES_CALIBRATOR_HPP
#define _NI_M_SERIES_CALIBRATOR_HPP

#include "calibrator.hpp"
#include "calibrator_misc.hpp"
#include <map>
#include <string>
#include <vector>

namespace NIMSeries
{
	class References
	{
	public:
		static const int positive_cal_shift = 7;
		static const int negative_cal_shift = 10;
		enum PositiveCalSource
		{
			POS_CAL_REF = 2 << positive_cal_shift,
			POS_CAL_PWM_500mV = 3 << positive_cal_shift,
			POS_CAL_PWM_2V = 4 << positive_cal_shift,
			POS_CAL_PWM_10V = 5 << positive_cal_shift,
			POS_CAL_GROUND = 6 << positive_cal_shift,
			POS_CAL_AO = 7 << positive_cal_shift
		};
		enum NegativeCalSource
		{
			NEG_CAL_1V = 2 << negative_cal_shift,
			NEG_CAL_1mV = 3 << negative_cal_shift,
			NEG_CAL_GROUND = 5 << negative_cal_shift,
			NEG_CAL_GROUND2 = 6 << negative_cal_shift,
			NEG_CAL_PWM_10V = 7 << negative_cal_shift,
		};
		References(const comedi::device &dev);
		void setPWM(unsigned high_ns, unsigned low_ns, unsigned *actual_high_ns = 0, unsigned *actual_low_ns = 0);
		void setReference(enum PositiveCalSource posSource, enum NegativeCalSource NegSource);
		void setReference(unsigned AOChannel);
		std::vector<lsampl_t> readReference(unsigned numSamples, unsigned samplePeriodNanosec, unsigned inputRange, unsigned settleNanosec) const;
		std::vector<double> readReferenceDouble(unsigned numSamples, unsigned samplePeriodNanosec, unsigned inputRange, unsigned settleNanosec) const;
		unsigned getMinSamplePeriodNanosec() const;
	private:
		void setReferenceBits(unsigned bits);

		comedi::device _dev;
	};

	/* Calibrator for National Instruments M-Series boards. */
	class Calibrator: public ::Calibrator
	{
	public:
		Calibrator();
		virtual std::string supportedDriverName() const {return "ni_pcimio";}
		virtual std::vector<std::string> supportedDeviceNames() const;
		virtual CalibrationSet calibrate(const comedi::device &dev);
	private:
		static const unsigned numSamples = 15000;
		static const unsigned settleNanosec = 1000000;
		static const unsigned baseRange = 0;
		static const unsigned masterClockPeriodNanosec = 50;
		static const unsigned minimumPWMPulseTicks = 0x20;
		static const unsigned TargetPWMPeriodTicks = 20 * minimumPWMPulseTicks;

		const SubdeviceCalibration calibrateAISubdevice();
		Polynomial calibrateAINonlinearity(const std::map<unsigned, double> &PWMCharacterization);
		// calibrate the one range that can actually read the onboard voltage reference directly
		Polynomial calibrateAIBaseRange(const Polynomial &nonlinearityCorrection);
		Polynomial calibrateAIRange(const Polynomial &PWMCalibration, const Polynomial &nonlinearityCorrection,
			enum NIMSeries::References::PositiveCalSource posSource, unsigned range);
		Polynomial calibratePWM(const std::map<unsigned, double> &PWMCharacterization,
			const Polynomial &baseRangeCalibration);
		Polynomial calibrateAIGainAndOffset(const Polynomial &nonlinearityCorrection,
			enum NIMSeries::References::PositiveCalSource posReferenceSource, double referenceVoltage, unsigned range);
		void setPWMUpTicks(unsigned upTicks);
		std::map<unsigned, double> characterizePWM(enum NIMSeries::References::PositiveCalSource posReferenceSource, unsigned ADRange);
		unsigned smallestCalibratedAIRangeContaining(const std::vector<bool> &calibrated, double rangeThreshold);
		void calibrateAIRangesAboveThreshold(const Polynomial &PWMCalibration, const Polynomial &nonlinearityCorrection,
			enum NIMSeries::References::PositiveCalSource posReferenceSource,
			SubdeviceCalibration *AICalibration, std::vector<bool> *calibrated, double maxRangeThreshold);
		// round numSamples so we sample over an integer number of PWM periods
		unsigned PWMRoundedNumSamples(unsigned numSamples, unsigned samplePeriodNS) const;
		void checkAIBufferSize();
		unsigned PWMPeriodTicks() const;
		const SubdeviceCalibration calibrateAOSubdevice(const SubdeviceCalibration &AICalibration);
		Polynomial calibrateAOChannelAndRange(const Polynomial &AICalibration,
			unsigned AIRange, unsigned AOChannel, unsigned AORange);
		unsigned findAIRangeForAO(unsigned AORange) const;
		// returns an AO code that will produce a high voltage inside the specified ai input range
		lsampl_t highCode(unsigned AIRange, unsigned AORange) const;
		void dumpAICalibrationSources();

		comedi::device _dev;
		boost::shared_ptr<References> _references;
	};

	class EEPROM
	{
	public:
		EEPROM(const comedi::device &dev);
		float referenceVoltage() const;
	private:
		enum CalibrationAreaOffsets
		{
			voltageReferenceOffset = 12
		};
		unsigned calibrationAreaBaseAddress() const;
		unsigned readByte(unsigned address) const;
		unsigned readUInt16(unsigned startAddress) const;
		float readFloat(unsigned startAddress) const;
		comedi::device _dev;
	};
};

#endif	// _NI_M_SERIES_CALIBRATOR_HPP
