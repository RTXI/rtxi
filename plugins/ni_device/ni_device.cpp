/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <errno.h>
#include <rt.h>
#include <sstream>

#include <ni_device.h>

#include <ai.h>
#include <ao.h>
#include <common.h>

using namespace DAQ;

NIDevice::AIConfig::AIConfig(tMSeries *b,ni_device_t *i,u8 *e,subdevice_t *s,index_t c,bool a)
    : board(b), info(i), eepromMemory(e), subd(s), channel(c), active(a) {}

NIDevice::AIConfig::~AIConfig(void) {}

int NIDevice::AIConfig::callback(void) {    
    if(channel < subd->count)
        subd->chan[channel].analog.active = active;

    // ---- AI Reset ----
    //

    configureTimebase (board);
    pllReset (board);
    analogTriggerReset (board);

    aiReset (board);
    if(info->select_active_high)
        aiPersonalize (board, tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_High);
    else
        aiPersonalize (board, tMSeries::tAI_Output_Control::kAI_CONVERT_Output_SelectActive_Low);
    aiClearFifo (board);

    if(info->adc_reset)
        adcReset(board);

    // ---- End of AI Reset ----

    // ---- Start AI task ----

    aiDisarm (board);
    aiClearConfigurationMemory (board);

    // fill configuration FIFO 
    // Note: It is not necessary for the channel numbers to be ordered
    subd->active = 0;
    for(size_t i=0;i<subd->count;++i)
        if(subd->chan[i].analog.active)
            ++subd->active;
    size_t n = subd->active;
    for (size_t i=0;i<subd->count && n;++i)
        if(subd->chan[i].analog.active)
            aiConfigureChannel (board,
                                i,  // channel number
                                info->ai_gains[subd->chan[i].analog.range].gain_idx,  // gain -- check ai.h for allowed values
                                tMSeries::tAI_Config_FIFO_Data::kAI_Config_PolarityBipolar,
                                tMSeries::tAI_Config_FIFO_Data::kAI_Config_Channel_TypeDifferential, 
                                (--n)?kFalse:kTrue); // last channel?

    aiSetFifoRequestMode (board);    
    
    aiEnvironmentalize (board);
    
    aiHardwareGating (board);
    
    aiTrigger (board,
               tMSeries::tAI_Trigger_Select::kAI_START1_SelectPulse,
               tMSeries::tAI_Trigger_Select::kAI_START1_PolarityRising_Edge,
               tMSeries::tAI_Trigger_Select::kAI_START2_SelectPulse,
               tMSeries::tAI_Trigger_Select::kAI_START2_PolarityRising_Edge);
    
    aiSampleStop (board, 
                  (subd->active > 1)?kTrue:kFalse); // multi channel?

    aiNumberOfSamples (board,   
                       1,      // posttrigger samples
                       0,      // pretrigger samples
                       kTrue); // continuous?

    aiSampleStart (board, 
                   0,
                   0,
                   tMSeries::tAI_START_STOP_Select::kAI_START_SelectPulse,
                   tMSeries::tAI_START_STOP_Select::kAI_START_PolarityRising_Edge);

    aiConvert (board, 
               info->period_divisor,     // convert period divisor
               info->delay_divisor,     // convert delay divisor
               kFalse); // external sample clock?

    aiClearFifo (board);

    // check scale.h for scaling index values
    for(size_t i=0;i<subd->count;++i)
        aiGetScalingCoefficients(eepromMemory,info->ai_gains[subd->chan[i].analog.range].scale_idx,0,i,&subd->chan[i].analog.scale);

    aiArm (board, kFalse); 
    aiStart (board);

    return 0;
}

NIDevice::AOConfig::AOConfig(tMSeries *b,ni_device_t *i,u8 *e,subdevice_t *s,index_t c,bool a)
    : board(b), info(i), eepromMemory(e), subd(s), channel(c), active(a) {}

NIDevice::AOConfig::~AOConfig(void) {}

int NIDevice::AOConfig::callback(void) {
    if(channel < subd->count)
        subd->chan[channel].analog.active = active;

    configureTimebase (board);
    pllReset (board);
    analogTriggerReset (board);

    aoReset (board);
    aoPersonalize (board);
    aoResetWaveformChannels (board);
    aoClearFifo (board);

    // unground AO reference
    board->AO_Calibration.writeAO_RefGround (kFalse);

    // ---- End of AO Reset ----

    // ---- Start A0 task ---

    subd->active = 0;
    for(size_t i=0;i<subd->count;++i) {
        if(subd->chan[i].analog.active) ++subd->active;
        aoGetScalingCoefficients(eepromMemory,0,0,i,&subd->chan[i].analog.scale);
        aoConfigureDAC (board,
                        i,
                        0xF,
                        tMSeries::tAO_Config_Bank::kAO_DAC_PolarityBipolar,
                        tMSeries::tAO_Config_Bank::kAO_Update_ModeImmediate);
    }

    return 0;
}

NIDevice::DIOConfig::DIOConfig(tMSeries *b,ni_device_t *i,subdevice_t *s,index_t c,bool a,direction_t d)
    : board(b), info(i), subd(s), channel(c), active(a), direction(d) {}

NIDevice::DIOConfig::~DIOConfig(void) {}

int NIDevice::DIOConfig::callback(void) {
    if(channel < subd->count) {
        subd->chan[channel].digital.active = active;
        subd->chan[channel].digital.direction = direction;
    }

    subd->active = 0;
    u32 dio_direction = 0;
    for(u32 i=0;i<subd->count;++i) {
        if(subd->chan[i].digital.active) ++subd->active;
        if(subd->chan[i].digital.direction == OUTPUT)
            dio_direction |= 1<<i;
    }

    board->DIO_Direction.writeRegister(dio_direction);

    return 0;
}

NIDevice::NIDevice(IO::channel_t *channels,size_t channel_count,const char *name,ni_device_t *device_info,u32 *bar,size_t bar_count)
    : Device(device_info->name,channels,channel_count), deviceName(name), info(device_info) {

    subdevice[AI].count = device_info->ai_count;
    subdevice[AI].active = 0;
    subdevice[AI].chan = new channel_t[subdevice[AI].count];
    for(size_t i=0;i<subdevice[AI].count;++i) {
        subdevice[AI].chan[i].analog.active = false;
        subdevice[AI].chan[i].analog.gain = 1.0;
        subdevice[AI].chan[i].analog.range = 0;
        subdevice[AI].chan[i].analog.reference = 0;
        subdevice[AI].chan[i].analog.units = 0;
        subdevice[AI].chan[i].analog.zerooffset = 0;
        subdevice[AI].chan[i].analog.offsetunits = 0;
    }
    subdevice[AO].count = device_info->ao_count;
    subdevice[AO].active = 0;
    subdevice[AO].chan = new channel_t[subdevice[AO].count];
    for(size_t i=0;i<subdevice[AO].count;++i) {
        subdevice[AO].chan[i].analog.active = false;
        subdevice[AO].chan[i].analog.gain = 1.0;
        subdevice[AO].chan[i].analog.range = 0;
        subdevice[AO].chan[i].analog.reference = 0;
        subdevice[AO].chan[i].analog.units = 0;
        subdevice[AO].chan[i].analog.zerooffset = 0;
        subdevice[AO].chan[i].analog.offsetunits = 0;
    }
    subdevice[DIO].count = device_info->dio_count > 32 ? 32 : device_info->dio_count;
    subdevice[DIO].active = 0;
    subdevice[DIO].chan = new channel_t[subdevice[DIO].count];
    for(size_t i=0;i<subdevice[AI].count;++i) {
        subdevice[DIO].chan[i].digital.active = false;
        subdevice[DIO].chan[i].digital.direction = DAQ::INPUT;
    }

    bus = acquireBoard(bar[0],bar[1]);

    {
        tAddressSpace  bar0;
        u32 physicalBar1;

        bar0 = bus->createAddressSpace(kPCI_BAR0);
        physicalBar1 = bus->get(kBusAddressPhysical,kPCI_BAR1);
        bar0.write32(0xC0, (physicalBar1 & 0xffffff00L) | 0x80);
        bus->destroyAddressSpace(bar0);
    }

    eepromReadMSeries (bus,eepromMemory,sizeof(eepromMemory));   

    bar1 = bus->createAddressSpace(kPCI_BAR1);
    board = new tMSeries(bar1);

    setActive(true);
}

NIDevice::~NIDevice(void) {
    setActive(false);

    aiDisarm (board);

    delete board;
    bus->destroyAddressSpace(bar1);
}

bool NIDevice::analog_exists(type_t type,index_t chan) const {
    if((type == AI || type == AO) && chan < subdevice[type].count)
        return true;
    return false;
}

size_t NIDevice::getChannelCount(type_t type) const {
    if(type == AI || type == AO || type == DIO)
        return subdevice[type].count;
    return 0;
}

bool NIDevice::getChannelActive(type_t type,index_t chan) const {
    if(type == DIO && chan < subdevice[DIO].count)
        return subdevice[type].chan[chan].digital.active;
    if(analog_exists(type,chan))
        return subdevice[type].chan[chan].analog.active;
    return false;
}

int NIDevice::setChannelActive(type_t type,index_t chan,bool state) {
    int retval;

    if(type == DIO && chan < subdevice[DIO].count) {
        printf("subdevice[DIO].active = %d\n",subdevice[DIO].active);
        printf("direction = %d\n",subdevice[DIO].chan[chan].digital.direction);
        DIOConfig event(board,info,&subdevice[DIO],chan,state,subdevice[DIO].chan[chan].digital.direction);
        retval = RT::System::getInstance()->postEvent(&event);
        printf("subdevice[DIO].active = %d\n",subdevice[DIO].active);
        printf("direction = %d\n",subdevice[DIO].chan[chan].digital.direction);
    } else if(analog_exists(type,chan)) {
        if(type == AI) {
            output(chan) = 0.0;
            AIConfig event(board,info,eepromMemory,&subdevice[AI],chan,state);
            retval = RT::System::getInstance()->postEvent(&event);
        } else if(type == AO) {
            AOConfig event(board,info,eepromMemory,&subdevice[AO],chan,state);
            retval = RT::System::getInstance()->postEvent(&event);
        }
    } else
        return -EINVAL;
    return 0;
}

size_t NIDevice::getAnalogRangeCount(type_t type,index_t chan) const {
    if(!analog_exists(type,chan))
        return 0;
    if(type == AI)
        return info->ai_gain_count;
    else
        return 1;
}

size_t NIDevice::getAnalogReferenceCount(type_t type,index_t chan) const {
    if(!analog_exists(type,chan))
        return 0;
    return 1;
}

size_t NIDevice::getAnalogUnitsCount(type_t type,index_t chan) const {
    if(!analog_exists(type,chan))
        return 0;
    return 2;
}

std::string NIDevice::getAnalogRangeString(type_t type,index_t chan,index_t idx) const {
    if(!analog_exists(type,chan) || idx >= getAnalogRangeCount(type,chan))
        return "";
    if(type == AI)
        return info->ai_gains[idx].name;
    else
        return "-10 to 10";
}

std::string NIDevice::getAnalogReferenceString(type_t type,index_t chan,index_t idx) const {
    if(!analog_exists(type,chan) || idx >= getAnalogReferenceCount(type,chan))
        return "";
    return "Default";
}

std::string NIDevice::getAnalogUnitsString(type_t type,index_t chan,index_t idx) const {
    if(!analog_exists(type,chan) || idx >= getAnalogUnitsCount(type,chan))
        return "";

    switch(idx) {
      case 0:
          return "Volts";
      case 1:
          return "Amps";
      default:
          return "";
    }
}

double NIDevice::getAnalogGain(type_t type,index_t chan) const {
    if(!analog_exists(type,chan))
        return 0.0;
    return subdevice[type].chan[chan].analog.gain;
}

index_t NIDevice::getAnalogRange(type_t type,index_t chan) const {
    if(!analog_exists(type,chan))
        return 0-1;
    return subdevice[type].chan[chan].analog.range;
}

index_t NIDevice::getAnalogReference(type_t type,index_t chan) const {
    if(!analog_exists(type,chan))
        return 0-1;
    return subdevice[type].chan[chan].analog.reference;
}

index_t NIDevice::getAnalogUnits(type_t type,index_t chan) const {
    if(!analog_exists(type,chan))
        return 0-1;
    return subdevice[type].chan[chan].analog.units;
}

index_t NIDevice::getAnalogOffsetUnits(type_t type,index_t chan) const
{
    if(!analog_exists(type,chan))
        return 0-1;

    return subdevice[type].chan[chan].analog.offsetunits;
}

double NIDevice::getAnalogZeroOffset(type_t type,index_t chan) const
{
    if(!analog_exists(type,chan))
        return 0;

    return subdevice[type].chan[chan].analog.zerooffset;
}

int NIDevice::setAnalogGain(type_t type,index_t chan,double value) {
    if(!analog_exists(type,chan))
        return -EINVAL;
    subdevice[type].chan[chan].analog.gain = value;
    return 0;
}

int NIDevice::setAnalogRange(type_t type,index_t chan,index_t value) {
    if(!analog_exists(type,chan))
        return -EINVAL;
    subdevice[type].chan[chan].analog.range = value;

    if(type == AI) {
        AIConfig event(board,info,eepromMemory,&subdevice[AI],0-1,0);
        RT::System::getInstance()->postEvent(&event);
    }

    return 0;
}

int NIDevice::setAnalogReference(type_t type,index_t chan,index_t value) {
    if(!analog_exists(type,chan))
        return -EINVAL;
    subdevice[type].chan[chan].analog.reference = value;
    return 0;
}

int NIDevice::setAnalogUnits(type_t type,index_t chan,index_t value) {
    if(!analog_exists(type,chan))
        return -EINVAL;
    subdevice[type].chan[chan].analog.units = value;
    return 0;
}

int NIDevice::setAnalogOffsetUnits(type_t type,index_t chan,index_t index)
{
    if(!analog_exists(type,chan) || !((index >= 0) && (index < getAnalogUnitsCount(type,chan))))
        return -EINVAL;

    subdevice[type].chan[chan].analog.offsetunits = index;
    return 0;
}

int NIDevice::setAnalogZeroOffset(type_t type,index_t chan,double offset)
{
    if(!analog_exists(type,chan))
        return -EINVAL;

    subdevice[type].chan[chan].analog.zerooffset = offset;
    return 0;
}

direction_t NIDevice::getDigitalDirection(index_t chan) const {
    if(chan >= subdevice[DIO].count)
        return DAQ::INPUT;
    return subdevice[DIO].chan[chan].digital.direction;
}

int NIDevice::setDigitalDirection(index_t chan,direction_t direction) {
    if(chan >= subdevice[DIO].count || (direction != DAQ::INPUT && direction != DAQ::OUTPUT))
        return -EINVAL;

    printf("setting %d\n",direction);
    printf("direction = %d\n",subdevice[DIO].chan[chan].digital.direction);

    DIOConfig event(board,info,&subdevice[DIO],chan,subdevice[DIO].chan[chan].digital.active,direction);
    int retval = RT::System::getInstance()->postEvent(&event);

    printf("direction = %d\n",subdevice[DIO].chan[chan].digital.direction);
    return retval;
}

void NIDevice::read(void) {
    size_t count;

    if(subdevice[AI].active) {
        aiStartOnDemand (board);
        while (board->Joint_Status_2.readAI_Scan_In_Progress_St()) {}

        i32 value;
        f32 scaled;

        count = 0;
        for(size_t i=0;i<getChannelCount(AI);++i) {
            if(getChannelActive(AI,i)) {
                value = board->AI_FIFO_Data.readRegister ();
                aiPolynomialScaler (&value,&scaled,&subdevice[AI].chan[i].analog.scale);
                output(i) = subdevice[AI].chan[i].analog.gain*scaled+subdevice[AI].chan[i].analog.zerooffset;

                if(++count >= subdevice[AI].active) break;
            }
        }
    }

    if(subdevice[DIO].active) {
        size_t offset;
        unsigned int data = board->Static_Digital_Input.readRegister();

        count = 0;
        offset = getChannelCount(AI);
        for(size_t i=0;count < getChannelCount(DIO);++i)
            if(getChannelActive(DIO,i)) {
                if(subdevice[DIO].chan[i].digital.direction == DAQ::INPUT) {
                    if(data & 1<<i)
                        output(i+offset) = 5.0;
                    else
                        output(i+offset) = 0.0;
                } else {
                    if(input(i+getChannelCount(AO)) > 2.0)
                        output(i+offset) = 5.0;
                    else
                        output(i+offset) = 0.0;
                }

                if(++count >= subdevice[DIO].active) break;
            }
    }
}

void NIDevice::write(void) {
    size_t count;

    if(subdevice[AO].active) {
        i32 value;
        f32 voltage;

        count = 0;
        for(size_t i=0;i<getChannelCount(AO);++i)
            if(getChannelActive(AO,i)) {
	        voltage = (input(i)-subdevice[AO].chan[i].analog.zerooffset)*subdevice[AO].chan[i].analog.gain;
                aoLinearScaler(&value,&voltage,&subdevice[AO].chan[i].analog.scale);
                board->DAC_Direct_Data[i].writeRegister(value);

                if(++count >= subdevice[AO].active) break;
            }
    }

    if(subdevice[DIO].active) {
        size_t offset;
        unsigned int data = 0;

        count = 0;
        offset = getChannelCount(AO);
        for(size_t i=0;count < getChannelCount(DIO);++i)
            if(subdevice[DIO].chan[i].digital.active) {
                ++count;
                if(input(i+offset) > 2.0)
                    data |= 1<<i;
            }
        board->Static_Digital_Output.writeRegister(data);
    }
}

void NIDevice::doLoad(const Settings::Object::State &s) {
    for(size_t i = 0;i < subdevice[AI].count && i < static_cast<size_t>(s.loadInteger("AI Count"));++i) {
        std::ostringstream str;
        str << i;
        setAnalogRange(AI,i,s.loadInteger(str.str()+" AI Range"));
        setAnalogReference(AI,i,s.loadInteger(str.str()+" AI Reference"));
        setAnalogUnits(AI,i,s.loadInteger(str.str()+" AI Units"));
        setAnalogGain(AI,i,s.loadDouble(str.str()+" AI Gain"));
        setAnalogZeroOffset(AI,i,s.loadDouble(str.str()+" AI Zero Offset"));
        setChannelActive(AI,i,s.loadInteger(str.str()+" AI Active"));
    }

    for(size_t i = 0;i < subdevice[AO].count && i < static_cast<size_t>(s.loadInteger("AO Count"));++i) {
        std::ostringstream str;
        str << i;
        setAnalogRange(AO,i,s.loadInteger(str.str()+" AO Range"));
        setAnalogReference(AO,i,s.loadInteger(str.str()+" AO Reference"));
        setAnalogUnits(AO,i,s.loadInteger(str.str()+" AO Units"));
        setAnalogGain(AO,i,s.loadDouble(str.str()+" AO Gain"));
        setAnalogZeroOffset(AO,i,s.loadDouble(str.str()+" AO Zero Offset"));
        setChannelActive(AO,i,s.loadInteger(str.str()+" AO Active"));
    }

    for(size_t i = 0;i < subdevice[DIO].count && i < static_cast<size_t>(s.loadInteger("DIO Count"));++i) {
        std::ostringstream str;
        str << i;
        setDigitalDirection(i,static_cast<DAQ::direction_t>(s.loadInteger(str.str()+" DIO Direction")));
        setChannelActive(DIO,i,s.loadInteger(str.str()+" DIO Active"));
    }
}

void NIDevice::doSave(Settings::Object::State &s) const {
    s.saveInteger("AI Count",subdevice[AI].count);
    for(size_t i = 0;i < subdevice[AI].count;++i) {
        std::ostringstream str;
        str << i;
        s.saveInteger(str.str()+" AI Active",getChannelActive(AI,i));
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
        s.saveInteger(str.str()+" DIO Direction",getDigitalDirection(i));
    }
}
