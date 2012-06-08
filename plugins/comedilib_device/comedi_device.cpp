/*
 * Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College
 *               2012 University of Bristol, UK
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <comedi_device.h>
#include <debug.h>
#include <math.h>
#include <sstream>
#include <iostream>

using namespace DAQ;

ComediDevice::ComediDevice(comedi_t *d,std::string name,IO::channel_t *chan,size_t size)
    : DAQ::Device(name,chan,size), device(d) {    

    // If board uses soft calibration, retrieve calibration file    
    int aiSubDevice = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AI,0);
    int aoSubDevice = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AO,0);
    
    if( (comedi_get_subdevice_flags(device,aiSubDevice) & SDF_SOFT_CALIBRATED) > 0
            || (comedi_get_subdevice_flags(device,aoSubDevice) & SDF_SOFT_CALIBRATED) > 0 ) { // If board uses software calibration
            char *calibpath = comedi_get_default_calibration_path(device);
            calibration = comedi_parse_calibration_file( calibpath ); // Returns NULL if failed
            free( calibpath );
            if( calibration == NULL )
                ERROR_MSG("ComediDevice::ComediDevice : Failed to parse comedi soft calibration file/n");
    }

    if((subdevice[AI].id = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AI,0)) >= 0) {
        subdevice[AI].active = 0;
        subdevice[AI].count = comedi_get_n_channels(device,subdevice[AI].id);
        subdevice[AI].chan = new channel_t[subdevice[AI].count];
        if(!subdevice[AI].chan)
            subdevice[AI].count = 0;
        else
            for(size_t i=0;i<subdevice[AI].count;++i) {
                subdevice[AI].chan[i].active = false;
                subdevice[AI].chan[i].analog.maxdata = comedi_get_maxdata(device,subdevice[AI].id,i);
                setAnalogGain(AI,i,1.0);
                setAnalogRange(AI,i,0);
                setAnalogZeroOffset(AI,i,0);
                setAnalogReference(AI,i,0);
                setAnalogUnits(AI,i,0);
                setAnalogCalibration(AI,i);
                setAnalogCalibrationActive(AI,i,getAnalogCalibrationState(AI,i)); // If device is calibrated, set it to use calibration by default
            }
    } else {
        subdevice[AI].active = 0;
        subdevice[AI].count = 0;
        subdevice[AI].chan = NULL;
    }
    if((subdevice[AO].id = comedi_find_subdevice_by_type(device,COMEDI_SUBD_AO,0)) >= 0) {
        subdevice[AO].active = 0;
        subdevice[AO].count = comedi_get_n_channels(device,subdevice[AO].id);
        subdevice[AO].chan = new channel_t[subdevice[AO].count];
        if(!subdevice[AO].chan)
            subdevice[AO].count = 0;
        else
            for(size_t i=0;i<subdevice[AO].count;++i) {
                subdevice[AO].chan[i].active = false;
                subdevice[AO].chan[i].analog.maxdata = comedi_get_maxdata(device,subdevice[AO].id,i);
                setAnalogGain(AO,i,1.0);
                setAnalogZeroOffset(AI,i,0);
                setAnalogRange(AO,i,0);
                setAnalogReference(AO,i,0);
                setAnalogUnits(AO,i,0);
                setAnalogCalibration(AO,i);
                setAnalogCalibrationActive(AO,i,getAnalogCalibrationState(AO,i)); // If device is calibrated, set it to use calibration by default
            }
    } else {
        subdevice[AO].active = 0;
        subdevice[AO].count = 0;
        subdevice[AO].chan = NULL;
    }

    if((subdevice[DIO].id = comedi_find_subdevice_by_type(device,COMEDI_SUBD_DIO,0)) >= 0) {
        subdevice[DIO].active = 0;
        subdevice[DIO].count = comedi_get_n_channels(device,subdevice[DIO].id);
        subdevice[DIO].chan = new channel_t[subdevice[DIO].count];
        if(!subdevice[DIO].chan)
            subdevice[DIO].count = 0;
        else
            for(size_t i=0;i<subdevice[DIO].count;++i) {
                subdevice[DIO].chan[i].active = false;
                subdevice[DIO].chan[i].digital.previous_value = 0;
                setDigitalDirection(i,DAQ::INPUT);
            }
    } else {
        subdevice[DIO].active = 0;
        subdevice[DIO].count = 0;
        subdevice[DIO].chan = NULL;
    }
        
    setActive(true);
}

ComediDevice::~ComediDevice(void) {
    if(subdevice[AI].chan) delete[] subdevice[AI].chan;
    if(subdevice[AO].chan) delete[] subdevice[AO].chan;
    if(subdevice[DIO].chan) delete[] subdevice[DIO].chan;
    if(device) comedi_close(device);
    if(calibration != NULL) comedi_cleanup_calibration(calibration);
}

bool ComediDevice::analog_exists(type_t type,index_t count) const {
    if(type == AI && count < subdevice[AI].count ||
       type == AO && count < subdevice[AO].count)
        return true;
    return false;
}

size_t ComediDevice::getChannelCount(type_t type) const
{
    if(type != AI && type != AO && type != DIO)
        return 0;

    return subdevice[type].count;
}

bool ComediDevice::getChannelActive(type_t type,index_t channel) const
{
    if(channel >= getChannelCount(type))
        return false;

    return subdevice[type].chan[channel].active;    
}

int ComediDevice::setChannelActive(type_t type,index_t channel,bool state)
{
    if(channel >= getChannelCount(type))
        return -EINVAL;

    if(type == AI)
        output(channel) = 0.0;

    if(subdevice[type].chan[channel].active && !state)
        --subdevice[type].active;
    else if(!subdevice[type].chan[channel].active && state)
        ++subdevice[type].active;

    subdevice[type].chan[channel].active = state;
    return 0;
}

size_t ComediDevice::getAnalogRangeCount(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0;

    return static_cast<index_t>(comedi_get_n_ranges(device,subdevice[type].id,static_cast<unsigned long>(channel)));
}

size_t ComediDevice::getAnalogReferenceCount(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0;

    return 4;
}

size_t ComediDevice::getAnalogUnitsCount(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0;

    return 2;
}

std::string ComediDevice::getAnalogRangeString(type_t type,index_t channel,index_t index) const
{
    if(!analog_exists(type,channel) || !((index >= 0) && (index < getAnalogRangeCount(type,channel))))
        return "";

    std::ostringstream rangeString;

    comedi_range range;
    range = *comedi_get_range(device,subdevice[type].id,channel,index);

    rangeString << (double)range.min << " to " << (double)range.max;
    return rangeString.str();
}

std::string ComediDevice::getAnalogReferenceString(type_t type,index_t channel,index_t index) const
{
    if(!analog_exists(type,channel) || !((index >= 0) && (index < getAnalogReferenceCount(type,channel))))
        return "";

    switch(index) {
      case 0:
          return "Ground";
      case 1:
          return "Common";
      case 2:
          return "Differential";
      case 3:
          return "Other";
      default:
          return "";
    }
}

std::string ComediDevice::getAnalogUnitsString(type_t type,index_t channel,index_t index) const
{
    if(!analog_exists(type,channel) || !((index >= 0) && (index < getAnalogUnitsCount(type,channel))))
        return "";

    switch(index) {
      case 0:
          return "Volts";
      case 1:
          return "Amps";
      default:
          return "";
    }
}

index_t ComediDevice::getAnalogRange(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0-1;

    return subdevice[type].chan[channel].analog.range;
}

index_t ComediDevice::getAnalogReference(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0-1;

    return subdevice[type].chan[channel].analog.reference;
}

index_t ComediDevice::getAnalogUnits(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0-1;

    return subdevice[type].chan[channel].analog.units;
}

index_t ComediDevice::getAnalogOffsetUnits(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0-1;

    return subdevice[type].chan[channel].analog.offsetunits;
}

double ComediDevice::getAnalogGain(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0;

    return subdevice[type].chan[channel].analog.gain;
}

double ComediDevice::getAnalogZeroOffset(type_t type,index_t channel) const
{
    if(!analog_exists(type,channel))
        return 0;

    return subdevice[type].chan[channel].analog.zerooffset;
}

int ComediDevice::setAnalogRange(type_t type,index_t channel,index_t index)
{
    if(!analog_exists(type,channel) || !((index >= 0) && (index < getAnalogRangeCount(type,channel))))
        return -EINVAL;

    channel_t *chan = &subdevice[type].chan[channel];
    comedi_range range;

    range = *comedi_get_range(device,subdevice[type].id,channel,index);

    chan->analog.range = index;
    chan->analog.conv = (range.max-range.min)/chan->analog.maxdata;
    chan->analog.offset = -range.min/chan->analog.conv;

    /*
     * Save ourselves an extra division each timestep in the fast path.
     */
    if(type == AO)
        chan->analog.conv = 1/chan->analog.conv;

    return 0;
}

int ComediDevice::setAnalogReference(type_t type,index_t channel,index_t index)
{
    if(!analog_exists(type,channel) || !((index >= 0) && (index < getAnalogReferenceCount(type,channel))))
        return -EINVAL;

    subdevice[type].chan[channel].analog.reference = index;
    return 0;
}

int ComediDevice::setAnalogUnits(type_t type,index_t channel,index_t index)
{
    if(!analog_exists(type,channel) || !((index >= 0) && (index < getAnalogUnitsCount(type,channel))))
        return -EINVAL;

    subdevice[type].chan[channel].analog.units = index;
    return 0;
}

int ComediDevice::setAnalogOffsetUnits(type_t type,index_t channel,index_t index)
{
    if(!analog_exists(type,channel) || !((index >= 0) && (index < getAnalogUnitsCount(type,channel))))
        return -EINVAL;

    subdevice[type].chan[channel].analog.offsetunits = index;
    return 0;
}

int ComediDevice::setAnalogZeroOffset(type_t type,index_t channel,double offset)
{
    if(!analog_exists(type,channel))
        return -EINVAL;

    subdevice[type].chan[channel].analog.zerooffset = offset;
    return 0;
}

int ComediDevice::setAnalogGain(type_t type,index_t channel,double gain)
{
    if(!analog_exists(type,channel))
        return -EINVAL;
    subdevice[type].chan[channel].analog.gain = gain;
    return 0;
}

bool ComediDevice::getAnalogCalibrationActive(type_t type, index_t channel) const {    
    if(!analog_exists(type,channel))
        return false;

    return subdevice[type].chan[channel].analog.calibrationActive;
}

int ComediDevice::setAnalogCalibrationActive(type_t type,index_t channel,bool state) {
    if(!analog_exists(type,channel))
        return -EINVAL;

    subdevice[type].chan[channel].analog.calibrationActive = state;
    return 0;
}

bool ComediDevice::getAnalogCalibrationState(type_t type, index_t channel) const {    
    if(!analog_exists(type,channel))
        return false;

    return subdevice[type].chan[channel].analog.calibrated;
}

int ComediDevice::setAnalogCalibration(type_t type,index_t channel) {
    if(!analog_exists(type,channel))
        return -EINVAL;

    analog_channel_t *chanPtr = &subdevice[type].chan[channel].analog;
    comedi_polynomial_t polynomial;
    int retval = -1;
    
    if( calibration != NULL ) { // Board was soft calibrated
        DEBUG_MSG("ComediDevice::setAnalogCalibration - Soft calibration used for [subd,channel] [%i,%i]\n",
                  subdevice[type].id,channel);
        
        if( type == AI )
            retval = comedi_get_softcal_converter(subdevice[type].id,channel,chanPtr->range,
                                                  COMEDI_TO_PHYSICAL,calibration,&polynomial);        
        else if( type == AO )
            retval = comedi_get_softcal_converter(subdevice[type].id,channel,chanPtr->range,
                                                  COMEDI_FROM_PHYSICAL,calibration,&polynomial);
        else
            ERROR_MSG("ComediDevice::setAnalogCalibration : invalid type\n");

        if( retval < 0 ) {
            ERROR_MSG("ComediDevice::setAnalogCalibration : unable to retrieve softcal converter\n");
            chanPtr->calibrated = false;
        }
        else { // Calibration retrieval successful
            chanPtr->calibrated = true;
            chanPtr->order = polynomial.order;
            chanPtr->expansionOrigin = polynomial.expansion_origin;

            for( int i = 0; i < 4; i++ ) // Max 4 coefficients
                chanPtr->coefficients[i] = polynomial.coefficients[i];
        }
    }
    else { // Check if board is hard calibrated
        DEBUG_MSG("ComediDevice::setAnalogCalibration - Soft calibration used for [subd,channel] [%i,%i]\n",
                  subdevice[type].id,channel);
        
        if( type == AI )
            retval = comedi_get_hardcal_converter(device,subdevice[type].id,channel,chanPtr->range,
                                                  COMEDI_TO_PHYSICAL,&polynomial);
        else if( type == AO )
            retval = comedi_get_hardcal_converter(device,subdevice[type].id,channel,chanPtr->range,
                                                  COMEDI_FROM_PHYSICAL,&polynomial);
        else
            ERROR_MSG("ComediDevice::setAnalogCalibration : invalid type\n");
        
        if( retval < 0 ) {
            ERROR_MSG("ComediDevice::setAnalogCalibration : unable to retrieve calibration, no calibration is being used\n");
            chanPtr->calibrated = false;
        }
        else { // Calibration retrieval successful
            chanPtr->calibrated = true;
            chanPtr->order = polynomial.order;
            chanPtr->expansionOrigin = polynomial.expansion_origin;

            for( int i = 0; i < 4; i++ ) // Max 4 coefficients
                chanPtr->coefficients[i] = polynomial.coefficients[i];
        }
    }

    return 0;
}

direction_t ComediDevice::getDigitalDirection(index_t channel) const {
    if(channel >= subdevice[DIO].count)
        return DAQ::INPUT;

    return subdevice[DIO].chan[channel].digital.direction;
}

int ComediDevice::setDigitalDirection(index_t channel,direction_t direction) {
    if(channel >= subdevice[DIO].count)
        return -EINVAL;

    subdevice[DIO].chan[channel].digital.direction = direction;

    if(direction == DAQ::INPUT)
        return comedi_dio_config(device,subdevice[DIO].id,channel,0);
    if(direction == DAQ::OUTPUT)
        return comedi_dio_config(device,subdevice[DIO].id,channel,1);

    return -EINVAL;
}

void ComediDevice::read(void)
{
    lsampl_t sample;
    analog_channel_t *channel;

    for(size_t i=0;i < subdevice[AI].count;++i)
        if(subdevice[AI].chan[i].active) {
            channel = &subdevice[AI].chan[i].analog;
            comedi_data_read(device,subdevice[AI].id,i,channel->range,channel->reference,&sample);
            if( channel->calibrationActive ) { // Use polynomial fitting if channel is active
                double value = 0.;
                double term = 1.;
                
                for(unsigned j = 0; j <= channel->order; ++j) {
                        value += channel->coefficients[j] * term;
                        term *= sample - channel->expansionOrigin;
                    }
                output(i) = channel->gain * value + channel->zerooffset;
            }
            else // No calibration is used
                output(i) = channel->gain * channel->conv * (sample-channel->offset) + channel->zerooffset;
        }

    unsigned int data;
    size_t offset = getChannelCount(AI);

    for(size_t i=0;i < subdevice[DIO].count;++i)
        if(subdevice[DIO].chan[i].active && subdevice[DIO].chan[i].digital.direction == DAQ::INPUT) {
            comedi_dio_read(device,subdevice[DIO].id,i,&data);
            output(i+offset) = data;
        }
}

void ComediDevice::write(void)
{
    {
        double value;
        lsampl_t sample;
        analog_channel_t *channel;

        for(size_t i=0;i < subdevice[AO].count;++i)
            if(subdevice[AO].chan[i].active) {
                channel = &subdevice[AO].chan[i].analog;
                
                if( channel->calibrationActive ) { // Use polynomial fitting if channel calibration is active
                    double in = channel->gain * ( input(i) + channel->zerooffset ); // Physical value

                    value = 0.;
                    double term = 1.;
                
                    for(unsigned j = 0; j <= channel->order; ++j) {
                            value += channel->coefficients[j] * term;
                            term *= in - channel->expansionOrigin;
                        }
                    
                    value = round( value ); 
                }
                else // No calibration is used
                    value = round(channel->gain*channel->conv*(input(i)+channel->zerooffset)+channel->offset);

                /*
                 * Prevent wrap around in the data units.
                 */
                if(value > channel->maxdata)
                    value = channel->maxdata;
                else if(value < 0.0)
                    value = 0.0;
                
                sample = static_cast<sampl_t>(value);

                comedi_data_write(device,subdevice[AO].id,i,channel->range,channel->reference,sample);
            }
    }

    {
        size_t offset = getChannelCount(AO);
        int value;

        for(size_t i=0;i < subdevice[DIO].count;++i) {
            value = input(i+offset) != 0.0;
            if(subdevice[DIO].chan[i].active && subdevice[DIO].chan[i].digital.direction == DAQ::OUTPUT && subdevice[DIO].chan[i].digital.previous_value != value) {
                subdevice[DIO].chan[i].digital.previous_value = value;
                comedi_dio_write(device,subdevice[DIO].id,i,value);
            }
        }
    }
}

void ComediDevice::doLoad(const Settings::Object::State &s) {
    for(size_t i = 0;i < subdevice[AI].count && i < static_cast<size_t>(s.loadInteger("AI Count"));++i) {
        std::ostringstream str;
        str << i;
        setChannelActive(AI,i,s.loadInteger(str.str()+" AI Active"));
        setAnalogCalibrationActive(AI,i,s.loadInteger(str.str()+" AI Calibration Active"));
        setAnalogRange(AI,i,s.loadInteger(str.str()+" AI Range"));
        setAnalogReference(AI,i,s.loadInteger(str.str()+" AI Reference"));
        setAnalogUnits(AI,i,s.loadInteger(str.str()+" AI Units"));
        setAnalogGain(AI,i,s.loadDouble(str.str()+" AI Gain"));
        setAnalogZeroOffset(AI,i,s.loadDouble(str.str()+" AI Zero Offset"));
        setAnalogCalibration(AI,i);        
    }

    for(size_t i = 0;i < subdevice[AO].count && i < static_cast<size_t>(s.loadInteger("AO Count"));++i) {
        std::ostringstream str;
        str << i;
        setChannelActive(AO,i,s.loadInteger(str.str()+" AO Active"));
        setAnalogCalibrationActive(AO,i,s.loadInteger(str.str()+" AO Calibration Active"));
        setAnalogRange(AO,i,s.loadInteger(str.str()+" AO Range"));
        setAnalogReference(AO,i,s.loadInteger(str.str()+" AO Reference"));
        setAnalogUnits(AO,i,s.loadInteger(str.str()+" AO Units"));
        setAnalogGain(AO,i,s.loadDouble(str.str()+" AO Gain"));
        setAnalogZeroOffset(AO,i,s.loadDouble(str.str()+" AO Zero Offset"));
        setAnalogCalibration(AO,i);                
    }

    for(size_t i = 0;i < subdevice[DIO].count && i < static_cast<size_t>(s.loadInteger("DIO Count"));++i) {
        std::ostringstream str;
        str << i;
        setChannelActive(DIO,i,s.loadInteger(str.str()+" DIO Active"));
        setDigitalDirection(i,static_cast<DAQ::direction_t>(s.loadInteger(str.str()+" DIO Direction")));
    }
}

void ComediDevice::doSave(Settings::Object::State &s) const {
    s.saveInteger("AI Count",subdevice[AI].count);
    for(size_t i = 0;i < subdevice[AI].count;++i) {
        std::ostringstream str;
        str << i;
        s.saveInteger(str.str()+" AI Active",getChannelActive(AI,i));
        s.saveInteger(str.str()+" AI Calibration Active",getAnalogCalibrationActive(AI,i));
        s.saveInteger(str.str()+" AI Range",getAnalogRange(AI,i));
        s.saveInteger(str.str()+" AI Reference",getAnalogReference(AI,i));
        s.saveInteger(str.str()+" AI Units",getAnalogUnits(AI,i));
        s.saveDouble(str.str()+" AI Gain",getAnalogGain(AI,i));
        s.saveDouble(str.str()+" AI Zero Offset",getAnalogZeroOffset(AI,i));
    }

    s.saveInteger("AO Count",subdevice[AO].count);
    for(size_t i = 0;i < subdevice[AO].count;++i) {
        std::ostringstream str;
        str << i;
        s.saveInteger(str.str()+" AO Active",getChannelActive(AO,i));
        s.saveInteger(str.str()+" AO Calibration Active",getAnalogCalibrationActive(AO,i));
        s.saveInteger(str.str()+" AO Range",getAnalogRange(AO,i));
        s.saveInteger(str.str()+" AO Reference",getAnalogReference(AO,i));
        s.saveInteger(str.str()+" AO Units",getAnalogUnits(AO,i));
        s.saveDouble(str.str()+" AO Gain",getAnalogGain(AO,i));
        s.saveDouble(str.str()+" AO Gain",getAnalogZeroOffset(AO,i));
    }

    s.saveInteger("DIO Count",subdevice[DIO].count);
    for(size_t i = 0;i < subdevice[DIO].count;++i) {
        std::ostringstream str;
        str << i;
        s.saveInteger(str.str()+" DIO Active",getChannelActive(DIO,i));
        s.saveInteger(str.str()+" DIO Direction",subdevice[DIO].chan[i].digital.direction);
    }
}
