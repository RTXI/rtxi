/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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
#include <list>
#include <unordered_map>
#include <string>
#include <vector>

//! Connection Oriented Classes
/*!
 * Objects contained within this namespace are responsible
 * for managing data sharing between various experimental
 * entities. This is different to the Fifo used for
 * interprocess communication between rtxi and its realtime
 * thread.
 *
 * \sa RT::OS::Fifo
 */
namespace IO
{

/*!
 * Variable used to specify the type of a channel.
 */
enum flags_t : size_t
{
  INPUT = 0,
  OUTPUT = 1,
  UNKNOWN = 2
};

/*!
 * Structure used to pass information to an IO::Block upon creation.
 * It is a structure critical for describing the block's ports.
 *
 * \param name The name of the channel
 * \param description short description of the channel
 * \param flags whether the channel is IO::INPUT or IO::OUTPUT type
 * \param data_size accepted data length of the input/output
 *
 * \sa IO::Block::Block()
 */
typedef struct
{
  std::string name;
  std::string description;
  IO::flags_t flags = IO::UNKNOWN;  // IO::INPUT or IO::OUTPUT
  size_t data_size = 0;  // For those channels that accept arays of values
} channel_t;

/*!
 * Interface for IO data between RTXI devices and plugins.
 */
class Block
{
public:
  /*!
   * The constructor needs to be provided with a specification of the channels
   * that will be embedded in this block in the channels parameter. Fields that
   * are not of type INPUT or OUTPUT will be safely ignored. Size should be the
   * number of total fields in the channels parameter, regardless of type.
   *
   * \param name The name of the block.
   * \param channels The lis of channel specifications for this block.
   * \param size The number of channels in the specification.
   *
   * \sa IO::channel_t
   */
  Block(std::string name,
        const std::vector<channel_t>& channels,
        bool isdependent);  // default constructor
  Block(const Block& block) = default;  // copy constructor
  Block& operator=(const Block& block) = default;  // copy assignment operator
  Block(Block&&) = delete;  // move constructor
  Block& operator=(Block&&) = delete;  // move assignment operator
  ~Block() = default;

  /*!
   * Get the name of the block.
   *
   * \return Tbe name of the block.
   */
  std::string getName() const { return name; };

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
   * \param type Port type. Either ::IO::INPUT or ::IO::OUTPUT
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
   * \param index The input channel's index.
   * \param data the data to push into the block
   *
   * \return The value of the specified input channel.
   */
  void writeinput(size_t index, const std::vector<double>& data);

  /*!
   * Get the values of the specified output channel.
   *
   * \param index The output channel's index.
   * \return The value of the specified output channel.
   */
  const std::vector<double>& readoutput(size_t index);
    
  bool dependent() const {return this->isInputDependent;}

protected:
  // These functions are meant to be used by RT::Thread classes

  /*!
   * Read the input sent to this block. Only the block itself has access.
   *
   * \param index The channel to read the sent input from
   * \returns A vector of values
   */
  const std::vector<double>& readinput(size_t index);

  /*!
   * Writes output to specified channel. Only the block itself has access.
   *
   * \param index The channel to write the output to
   * \param data A vector of values to send to the channel
   */
  void writeoutput(size_t index, const std::vector<double>& data);


private:
  using port_t = struct
  {
    IO::channel_t channel_info;
    std::vector<double> values;
  };
  std::string name;
  bool isInputDependent;
  std::array<std::vector<port_t>, IO::UNKNOWN> ports;
};  // class Block

/*!
 * Information about the outputs of a particular block. This is
 * meant to be used along with a block pointer for the source and
 * should not be used alone.
 * 
 * \param src_port Index of the source channel generating the output
 * \param dest Pointer to IO::Block to send the output to
 * \param dest_port Index of the destination channel taking the input
 */
typedef struct
{
  size_t src_port;
  IO::Block* dest;
  size_t dest_port;
}outputs_info;

/*!
 * The structure representating the connection between two block devices
 *
 * \param src pointer to source block
 * \param outputs a list of outputs coming out from the block
 */
typedef struct
{
  IO::Block* src;
  std::vector<outputs_info> outputs;
}connections_t;

/*!
 * Acts as a central meeting point between Blocks. Provides
 *   interfaces for finding and connecting blocks.
 *
 * \sa IO::Block
 */
class Connector
{
public:
  Connector() = default;  // default constructor
  Connector(const Connector& connector) = delete;  // copy constructor
  Connector& operator=(const Connector& connector) =
      delete;  // copy assignment operator
  Connector(Connector&&) = delete;  // move constructor
  Connector& operator=(Connector&&) = delete;  // move assignment operator
  ~Connector() = default;

  /*!
   * Create a connection between the two specified Blocks.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \returns 0 if successfully connected, -1 if it found a cycle
   *
   * \sa IO::Block
   */
  int connect(IO::Block* src, size_t out, IO::Block* dest, size_t in);

  /*!
   * Break a connection between the two specified Blocks.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block
   * \sa IO::Block::input()
   * \sa IO::Block::output()
   */
  void disconnect(IO::Block* src, size_t out, IO::Block* dest, size_t in);

  /*!
   * Determine whether two channels are connected or not.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block::connect()
   * \sa IO::Block::disconnect()
   */
  bool connected(IO::Block* src, size_t out, IO::Block* dest, size_t in);

  /*!
   * Register the block in order to access connection services
   *
   * \param block Pointer to block object to register
   */
  void insertBlock(IO::Block* block);

  /*!
   * Unregister the block from the registry
   *
   * \param block Pointer to block to unregister
   */
  void removeBlock(IO::Block* block);

  /*!
   * Get the list of devices that are registered with connector class.
   * To the connector class devices are io blocks that are independent
   * of other blocks when connected.
   *
   * \returns List of IO::Block pointers representing registred devices
   */
  std::vector<IO::Block*> getDevices();

  /*!
   * Get a lsit of threads that are registered with connector class. To
   * the connector class threads are blocks that are dependent of other 
   * blocks when connected. They are topologically sorted.
   *
   * \returns List of IO::Block pointers representing registered threads
   */
  std::vector<IO::Block*> getThreads();

  /*!
   * Returns a list of outputs for the input block pointer
   *
   * \param src Input IO::Block pointer to find the outputs for
   * \returns A vector of IO::outputs_info containing connection info
   * \sa IO::outputs_info
   */
  std::vector<IO::outputs_info> getOutputs(IO::Block* src);
private:
  std::vector<IO::Block*> topological_sort();
  std::unordered_map<IO::Block*, std::vector<outputs_info>> registry;
};  // class Connector

}  // namespace IO

#endif  // IO_H
