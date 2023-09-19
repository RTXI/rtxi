/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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


#include <cmath>
#include <cstddef>
#include <map>

#include "io.hpp"
#include "rt.hpp"

//! DAQ Oriented Classes
/*!
 * Objects contained within this namespace are responsible for providing
 *   a standardized interface for dealing with DAQ cards.
 */
namespace DAQ
{

namespace ChannelType {
/*!
 * Used to specify the interface type.
 */
enum type_t : size_t
{
  AI=0, /*!< Analog Input Interface         */
  AO, /*!< Analog Output Interface        */
  DI, /*!< Digital Input Interface         */
  DO, /*!< Digital Output Interface        */
  UNKNOWN
};
} // namespace ChannelType

/*!
 * Used to specify indexes for channel number, range, reference, and units.
 */
typedef uint64_t index_t;

/*!
 * Flag to indicate error from a function returning an index.
 */
static const index_t INVALID = static_cast<index_t>(-1);

/*!
 * Used to specify  digital interface direction.
 */
enum direction_t
{
  INPUT, /*!< Digital Input  */
  OUTPUT, /*!< Digital Output */
};

namespace Reference{
enum reference_t : size_t
{
  GROUND=0,
  COMMON,
  DIFFERENTIAL,
  OTHER,
  UNKNOWN
};
} // namespace Reference

/*!
 * Object that represents a single DAQ card.
 */
class Device : public RT::Device
{
public:
  /*!
   * The constructor needs to be provided with a specification of its
   *   channels that will be passed to the inherited IO::Block.
   *
   * \sa IO::Block
   */
  Device(const std::string& dev_name,
         const std::vector<IO::channel_t>& channels)
      : RT::Device(dev_name, channels)
  {
  }

  /*!
   * Get the number of channels of the specified type.
   *
   * \param type The type of the channels to be counted.
   * \return The number of channels of the specified type.
   */
  virtual size_t getChannelCount(ChannelType::type_t type) const = 0;

  /*!
   * Get the channel's active state.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return  The channel's active state.
   */
  virtual bool getChannelActive(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Set the channel's active state.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param state The channel's new state.
   */
  virtual int setChannelActive(ChannelType::type_t type, index_t index, bool state) = 0;

  /*!
   * Get the number of available ranges for the specified channel.
   *
   * \param index The channel's index.
   * \return The number of available ranges for the channel.
   */
  virtual size_t getAnalogRangeCount(index_t index) const = 0;

  /*!
   * Get the number of available reference for the specified channel.
   *
   * \patam index The channel's index.
   * \return The number of available references for the channel.
   */
  virtual size_t getAnalogReferenceCount(index_t index) const = 0;

  /*!
   * Get the number of available units for the channel.
   *
   * \param index The channel's index.
   * \return The number of available units for the channel.
   */
  virtual size_t getAnalogUnitsCount(index_t index) const = 0;
  virtual size_t getAnalogDownsample(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Get a string representation of the specified range.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param range The index of the channel's range.
   * \return The string representation of the selected range.
   */
  virtual std::string getAnalogRangeString(ChannelType::type_t type,
                                           index_t index,
                                           index_t range) const = 0;

  /*!
   * Get a string representation of the specified reference.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param reference The index of the channel's reference.
   * \return The string representation of the selected reference.
   */
  virtual std::string getAnalogReferenceString(ChannelType::type_t type,
                                               index_t index,
                                               index_t reference) const = 0;

  /*!
   * Get a string representation of the specified units.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param units The index of the channel's units.
   * \return The string representation of the selected units.
   */
  virtual std::string getAnalogUnitsString(ChannelType::type_t type,
                                           index_t index,
                                           index_t units) const = 0;

  /*!
   * Get the gain of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The gain of the selected channel.
   */
  virtual double getAnalogGain(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Get the offset of the selected channel that makes the signal zero
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The zero offset of the selected channel.
   */
  virtual double getAnalogZeroOffset(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Get the index of the range for the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The index of the channel's range or INVALID on error.
   */
  virtual index_t getAnalogRange(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Get the index of the reference for the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The index of the channel's reference or INVALID on error.
   */
  virtual index_t getAnalogReference(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Get the index of the units for the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The index of the channel's units or INVALID on error.
   */
  virtual index_t getAnalogUnits(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Get the index of the units for the selected channel zero offset.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The index of the channel's offset units or INVALID on error.
   */
  virtual index_t getAnalogOffsetUnits(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Set the gain of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param gain The channel's new gain.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogGain(ChannelType::type_t type, index_t index, double gain) = 0;

  /*!
   * Set the range of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param range The channel's new range index.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogRange(ChannelType::type_t type, index_t index, index_t range) = 0;

  /*!
   * Set the zero offset of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param offset The channel's new zero offset index.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogZeroOffset(ChannelType::type_t type,
                                  index_t index,
                                  double offset) = 0;

  /*!
   * Set the reference of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param reference The channel's new reference index.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogReference(ChannelType::type_t type,
                                 index_t index,
                                 index_t reference) = 0;

  /*!
   * Set the units of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param units The channel's new units index.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogUnits(ChannelType::type_t type, index_t index, index_t units) = 0;

  /*!
   * Set the offset units of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param units The channel's new offset units index.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogOffsetUnits(ChannelType::type_t type,
                                   index_t index,
                                   index_t units) = 0;
  virtual int setAnalogDownsample(ChannelType::type_t type,
                                  index_t index,
                                  size_t downsample) = 0;
  virtual int setAnalogCounter(ChannelType::type_t type, index_t index) = 0;

  /*!
   * Set the calibration of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \param value The calibration value.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogCalibrationValue(ChannelType::type_t type,
                                        index_t index,
                                        double value) = 0;

  /*!
   * Get the calibration of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return the calibration value for the channel.
   */
  virtual double getAnalogCalibrationValue(ChannelType::type_t type,
                                           index_t index) const = 0;

  /*!
   * Set the calibration active state of the selected channel.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setAnalogCalibrationActive(ChannelType::type_t type,
                                         index_t index,
                                         bool state) = 0;

  /*!
   * Get the channel's active state of using its calibration.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The channel's active state of using its calibration.
   */
  virtual bool getAnalogCalibrationActive(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Get the channel's state of calibration.
   *
   * \param type The channel's type.
   * \param index The channel's index.
   * \return The channel's active state of using its calibration.
   */
  virtual bool getAnalogCalibrationState(ChannelType::type_t type, index_t index) const = 0;

  /*!
   * Set the direction of the specified digital channel.
   *
   * \param index The digital channel's index.
   * \param direction The digital channel's new direction.
   * \return 0 if successful or a negative value on error.
   */
  virtual int setDigitalDirection(index_t index, direction_t direction) = 0;

};  // class Device

/*!
 * Acts as a device factory for a specific class of DAQ::Devices.
 *
 * \sa DAQ::Device
 */
class Driver
{
public:
  /*!
   * The constructor needs to be provided with the name of this driver.
   *
   * \param name The name of the driver.
   */
  explicit Driver(std::string dev_name)
      : name(std::move(dev_name))
  {
  }
  Driver(const Driver&) = default;  // copy constructor
  Driver& operator=(const Driver&) = default;  // copy assignment operator
  Driver(Driver&&) = default;  // move constructor
  Driver& operator=(Driver&&) = default;  // move assignment operator
  virtual ~Driver() = default;

  virtual void loadDevices() = 0;
  virtual void unloadDevices() = 0;

  virtual std::vector<DAQ::Device*> getDevices() = 0;
  std::string getDriverName() { return this->name; }

private:
  std::string name;
};  // class Driver

} // namespace DAQ

#endif  // DAQ_DEVICE_H
