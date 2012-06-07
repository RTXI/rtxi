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

#ifndef DAQ_DEVICE_H
#define DAQ_DEVICE_H

#include <io.h>
#include <map>
#include <rt.h>

//! DAQ Oriented Classes
/*!
 * Objects contained within this namespace are responsible for providing
 *   a standardized interface for dealing with DAQ cards.
 */
namespace DAQ {

    /*!
     * Used to specify the interface type.
     */
    enum type_t {
        AI,   /*!< Analog Input Interface         */
        AO,   /*!< Analog Output Interface        */
        DIO,  /*!< Digital Input/Output Interface */
        DI,  /*!< Digital Input Interface         */
        DO,  /*!< Digital Output Interface        */
    };
    /*!
     * Used to specify indexes for channel number, range, reference, and units.
     */
    typedef unsigned long index_t;
    /*!
     * Flag to indicate error from a function returning an index.
     */
    static const index_t INVALID = (0-1);
    /*!
     * Used to specify  digital interface direction.
     */
    enum direction_t {
        INPUT,  /*!< Digital Input  */
        OUTPUT, /*!< Digital Output */
    };

    class Device;
    class Driver;

    /*!
     * Provides a central meeting point for interfacing with all DAQ objects
     *   available in the system.
     *
     * \sa DAQ::Device
     * \sa DAQ::Driver
     */
    class Manager {

        friend class Device;
        friend class Driver;

    public:

        /*!
         * Manager is a Singleton, which means that there can only be one
         *   instance. This function returns a pointer to that single instance.
         *
         * \return The instance of Manager.
         */
        static Manager *getInstance(void);

        /*!
         * Loop through each Device and execute a callback.
         * The callback takes two parameters, a Device pointer and param,
         *   the second parameter to  foreachDevice.
         *
         * \param callback The callback function
         * \param param A parameter to the callback function.
         *
         * \sa DAQ::Device
         */
        void foreachDevice(void (*callback)(Device *,void *),void *param);

        /*!
         * Function for creating a device from the specified driver.
         *
         * \param driver The driver for the device.
         * \param params Parameters to the driver.
         *
         * \sa DAQ::Device
         * \sa DAQ::Driver
         */
        Device *loadDevice(const std::string &driver,const std::list<std::string> &params);

    private:

        Manager(void) : mutex(Mutex::RECURSIVE) {};
        ~Manager(void) {};
        Manager(const Manager &) {};
        Manager &operator=(const Manager &) { return *getInstance(); };

        static Manager *instance;

        void insertDevice(Device *);
        void removeDevice(Device *);

        void registerDriver(Driver *,const std::string &);
        void unregisterDriver(const std::string &);

        Mutex mutex;
        std::list<Device *> deviceList;
        std::map<std::string,Driver *> driverMap;

    }; // class Manager

    /*!
     * Object that represents a single DAQ card.
     */
    class Device : public RT::Device, public IO::Block {

    public:

        /*!
         * The constructor needs to be provided with a specification of its
         *   channels that will be passed to the inherited IO::Block.
         *
         * \sa IO::Block
         */
        Device(std::string,IO::channel_t *,size_t);
        virtual ~Device(void);

        /*!
         * Get the number of channels of the specified type.
         *
         * \param type The type of the channels to be counted.
         * \return The number of channels of the specified type.
         */
        virtual size_t getChannelCount(type_t type) const=0;
        /*!
         * Get the channel's active state.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return  The channel's active state.
         */
        virtual bool getChannelActive(type_t type,index_t index) const=0;        
        /*!
         * Set the channel's active state.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param state The channel's new state.
         */
        virtual int setChannelActive(type_t type ,index_t index,bool state)=0;        

        /*!
         * Get the channel's active state of using its calibration.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The channel's active state of using its calibration.
         */
        virtual bool getAnalogCalibrationActive(type_t type,index_t index) const=0;
        /*!
         * Get the channel's state of calibration.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The channel's active state of using its calibration.
         */
        virtual bool getAnalogCalibrationState(type_t type,index_t index) const=0;
        /*!
         * Get the number of available ranges for the specified channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The number of available ranges for the channel.
         */
        virtual size_t getAnalogRangeCount(type_t type,index_t index) const=0;
        /*!
         * Get the number of available reference for the specified channel.
         *
         * \param type The channel's type.
         * \patam index The channel's index.
         * \return The number of available references for the channel.
         */
        virtual size_t getAnalogReferenceCount(type_t type,index_t index) const=0;
        /*!
         * Get the number of available units for the channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The number of available units for the channel.
         */
        virtual size_t getAnalogUnitsCount(type_t type,index_t index) const=0;
        /*!
         * Get a string representation of the specified range.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param range The index of the channel's range.
         * \return The string representation of the selected range.
         */
        virtual std::string getAnalogRangeString(type_t type,index_t index,index_t range) const=0;
        /*!
         * Get a string representation of the specified reference.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param reference The index of the channel's reference.
         * \return The string representation of the selected reference.
         */
        virtual std::string getAnalogReferenceString(type_t type,index_t index,index_t reference) const=0;
        /*!
         * Get a string representation of the specifed units.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param units The index of the channel's units.
         * \return The string representation of the selected units.
         */
        virtual std::string getAnalogUnitsString(type_t type,index_t index,index_t units) const=0;
        /*!
         * Get the gain of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The gain of the selected channel.
         */
        virtual double getAnalogGain(type_t type,index_t index) const=0;
        /*!
         * Get the offset of the selected channel that makes the signal zero
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The zero offset of the selected channel.
         */
        virtual double getAnalogZeroOffset(type_t type,index_t index) const=0;
        /*!
         * Get the index of the range for the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The index of the channel's range or INVALID on error.
         */
        virtual index_t getAnalogRange(type_t type,index_t index) const=0;
        /*!
         * Get the index of the reference for the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The index of the channel's reference or INVALID on error.
         */
        virtual index_t getAnalogReference(type_t type,index_t index) const=0;
        /*!
         * Get the index of the units for the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The index of the channel's units or INVALID on error.
         */
        virtual index_t getAnalogUnits(type_t type,index_t index) const=0;
        /*!
         * Get the index of the units for the selected channel zero offset.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return The index of the channel's offset units or INVALID on error.
         */
        virtual index_t getAnalogOffsetUnits(type_t type,index_t index) const=0;        
        /*!
         * Set the gain of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param gain The channel's new gain.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogGain(type_t type,index_t index,double gain)=0;
        /*!
         * Set the range of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param range The channel's new range index.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogRange(type_t type,index_t index,index_t range)=0;
        /*!
         * Set the zero offset of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param offset The channel's new zero offset index.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogZeroOffset(type_t type,index_t index,double offset)=0;
        /*!
         * Set the reference of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param reference The channel's new reference index.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogReference(type_t type,index_t index,index_t reference)=0;
        /*!
         * Set the units of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param units The channel's new units index.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogUnits(type_t type,index_t index,index_t units)=0;
        /*!
         * Set the offset units of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \param units The channel's new offset units index.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogOffsetUnits(type_t type,index_t index,index_t units)=0;
        /*!
         * Set the calibration of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogCalibration(type_t type,index_t index)=0;
        /*!
         * Set the calibration active state of the selected channel.
         *
         * \param type The channel's type.
         * \param index The channel's index.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setAnalogCalibrationActive(type_t type,index_t index, bool state)=0;
        /*!
         * Get the direction of the specified digital channel.
         *
         * \param index The digital channel's index.
         * \return The direction of the digital channel.
         */
        virtual direction_t getDigitalDirection(index_t index) const=0;
        /*!
         * Set the direction of the specified digital channel.
         *
         * \param index The digital channel's index.
         * \param direction The digital channel's new direction.
         * \return 0 if successful or a negative value on error.
         */
        virtual int setDigitalDirection(index_t index,direction_t direction)=0;

    }; // class Device

    /*!
     * Acts as a device factory for a specific class of DAQ::Devices.
     *
     * \sa DAQ::Device
     */
    class Driver {

    public:

        /*!
         * The constructor needs to be provided with the name of this driver.
         *
         * \param name The name of the driver.
         */
        Driver(const std::string &name);
        virtual ~Driver(void);

        /*!
         * A factory function for create a DAQ::Device with the provided args.
         *
         * \param args Arguments to the new DAQ::Device.
         * \return A new DAQ::Device.
         *
         * \sa DAQ::Device
         */
        virtual Device *createDevice(const std::list<std::string> &args)=0;

    private:

        std::string name;

    }; // class Driver

}; // namespace DAQ

#endif // DAQ_DEVICE_H
