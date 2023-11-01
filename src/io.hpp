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

#ifndef IO_H
#define IO_H

#include <array>
#include <limits>
#include <string>
#include <vector>

//! Connection Oriented Classes
/*!
 * Objects contained within this namespace are responsible
 * for managing data sharing between various RTXI entities 
 * called blocks. This is different to the Fifo used for
 * interprocess communication between rtxi and its realtime
 * thread.
 *
 * \sa RT::OS::Fifo
 */
namespace IO
{

constexpr size_t INVALID_BLOCK_ID = std::numeric_limits<size_t>::max();

/*!
 * Variable used to specify the type of a channel.
 *
 * Blocks can either have INPUT or OUTPUT channels to other blocks.
 * These channels are usually default constructed to UNKNOWN, which
 * is an error if used as such
 */
enum flags_t : size_t
{
  OUTPUT = 0,
  INPUT,
  UNKNOWN
};

/*!
 * Structure used to pass information to an IO::Block upon creation.
 * It is a structure critical for describing the block's ports.
 *
 * \param name The name of the channel
 * \param description short description of the channel
 * \param flags whether the channel is IO::INPUT or IO::OUTPUT type
 *
 * \sa IO::Block::Block()
 */
typedef struct channel_t
{
  std::string name;
  std::string description;
  IO::flags_t flags = IO::UNKNOWN;  // IO::INPUT or IO::OUTPUT
} channel_t;

/*!
 * Interface class for IO data between RTXI devices and plugins.
 */
class Block
{
public:
  /*!
   * The constructor needs to be provided with a specification of the channels
   * that will be embedded in this block in the channels parameter. Fields could
   * be IO::INPUT, IO::OUTPUT. It also needs to specify whether the block is
   * dependent on other blocks for real-time scheduling. This base class is not
   * meant to be inherited directly and Widget::Component is recomended instead
   *
   * \param blockname The name of the block.
   * \param channels The lis of channel specifications for this block.
   *
   * \sa Widgets::Component
   * \sa IO::channel_t
   */
  Block(std::string blockname,
        const std::vector<channel_t>& channels,
        bool isdependent);  // default constructor
  Block(const Block& block) = default;  // copy constructor
  Block& operator=(const Block& block) = default;  // copy assignment operator
  Block(Block&&) = delete;  // move constructor
  Block& operator=(Block&&) = delete;  // move assignment operator
  virtual ~Block() = default;

  /*!
   * Get the name of the block.
   *
   * \return The name of the block.
   */
  std::string getName() const { return name; }

  /*!
   * Get the number of channels of the specified type.
   *
   * \param type The type of the channels to be counted.
   * \return The number of channels of the specified type.
   */
  size_t getCount(flags_t type) const;

  /*!
   * Get the name of the specified channel.
   *
   * \param type Port type.
   * \param index The channel's index.
   * \return The name of the channel.
   */
  std::string getChannelName(IO::flags_t type, size_t index) const;

  /*!
   * Get the description of the specified channel.
   *
   * \param index The channel's index.
   * \return The description of the channel.
   */
  std::string getChannelDescription(IO::flags_t type, size_t index) const;

  /*!
   * write the values of the specified input channel.
   *
   * This function is responsible for writting external values to
   * the specified input channel. Additional writes to input result in adding  
   * to existing values in the channel, and is only reset to zero during 
   * readinput functin call.
   *
   * \param index The input channel's index.
   * \param data the data to push into the block
   *
   * \return The value of the specified input channel.
   *
   * \sa IO::Block::readinput()
   */
  void writeinput(size_t index, const double& data);

  /*!
   * Get the values of the specified output channel.
   *
   * This function can read values from both input and output types. This 
   * does not reset the values on the channel
   *
   * \param index The output channel's index.
   * \param direction The channel direction represented by IO::flags_t
   * \return The value of the specified output channel.
   */
  const double& readPort(IO::flags_t direction, size_t index);

  /*!
   * Returns the dependency property of the block
   *
   * Some blocks are dependent on the output of others. It is crucial for 
   * these blocks to be scheduled in real-time to follow their dependencies, 
   * or else risk large errors in time dependent calculations.
   *
   * \return True for a dependent IO::Block, false otherwise
   */
  bool dependent() const { return this->isInputDependent; }

  /*!
   * Returns activity state of the block
   *
   * \return True for active block, false otherwise
   */
  bool getActive() const { return this->active; }

  /*!
   * Sets the activity state of the block. Only used by Real-Time thread
   *
   * /param act The activity state to set the block to. True if the block
   *            is in the active state. False otherwise.
   */
  void setActive(bool act) { this->active = act; }

  /*!
   * Assign unique ID to the block. Used by the Real-Time thread
   *
   * \param block_id The id to assign to the block
   */
  void assignID(size_t block_id) { this->id = block_id; }

  /*!
   * Retrieve the assigned id of the block
   *
   * \return The id of the block
   */
  size_t getID() const { return this->id; }

protected:
  /*!
   * Read the input sent to this block. Only the block itself has access.
   *
   * Reads the buffered values from all external inputs and resets the buffer
   * to zero. This function should only be accessed once per scope or risk 
   * getting previously read values overwirten. This function assumes reading
   * input channels.
   *
   * \param index The channel to read the sent input from
   * \returns A reference to input value written
   *
   * \sa IO::Block::writeinput()
   */
  double& readinput(size_t index);

  /*!
   * Writes output to specified channel. Only the block itself has access.
   *
   * \param index The channel to write the output to
   * \param data A reference to value to send 
   */
  void writeoutput(size_t index, const double& data);

private:
  size_t id = INVALID_BLOCK_ID;
  typedef struct port_t
  {
    double buff_value = 0.0;
    double value = 0.0;
    IO::channel_t channel_info;
  } port_t;
  std::string name;
  bool isInputDependent;
  std::array<std::vector<port_t>, IO::UNKNOWN> ports;
  bool active = false;
};  // class Block

/*!
 * Structure used to pass information to plugins about a connection.
 *
 * This structure holds details that identifies a connection end. it is
 * guaranteed to be unique and has comparison functions implemented to
 * allow for ease of equality checks in higher level algorithms
 *
 * \param block Pointer to the block object
 * \param port The index of the channel this object represents
 * \param direction An IO::flags_t value stating the direction. possible
 *                  values are: IO::INPUT, IO::OUTPUT, IO::UNKNOWN (default)
 *
 * \sa IO::Block::Block()
 */
typedef struct endpoint
{
  IO::Block* block = nullptr;
  size_t port = 0;
  IO::flags_t direction = IO::UNKNOWN;
  bool operator==(const endpoint& rhs) const
  {
    return (this->block == rhs.block) && (this->port == rhs.port)
        && (this->direction == rhs.direction);
  }
  bool operator!=(const endpoint& rhs) const { return !operator==(rhs); }
} endpoint;

}  // namespace IO

#endif  // IO_H
