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

#include <list>
#include <string>
#include <array>
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
enum flags_t: unsigned long
{
  INPUT = 0,
  OUTPUT
};

/*!
 * Structure used to pass information to an IO::Block upon creation.
 *
 * \sa IO::Block::Block()
 */
typedef struct
{
  std::string name;
  std::string description;
  IO::flags_t flags;  // IO::INPUT or IO::OUTPUT
  size_t data_size;  // For those channels that accept arays of values
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
  Block(std::string name, std::vector<channel_t> channels);
  ~Block();

  /*!
   * Get the name of the block.
   *
   * \return Tbe name of the block.
   */
  std::string getName() const
  {
    return name;
  };

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
   * \param index The channel's index.
   * \return The name of the channel.
   */
  std::string getChannelName(size_t index) const;

  /*!
   * Get the description of the specified channel.
   *
   * \param index The channel's index.
   * \return The description of the channel.
   */
  std::string getChannelDescription(size_t index) const;

  /*!
   * Get the value of the specified channel.
   *
   * \param index The channel's index.
   * \return The value of the channel.
   */
  std::vector<double> getChannelValue(size_t index) const;

  /*!
   * Get the value of the specified input channel.
   *
   * \param index The input channel's index.
   */
  void writeinput(size_t index, const std::vector<double>& data);
  const std::vector<double>& readinput(size_t index);

  /*!
   * Get the value of the specified output channel.
   *
   * \param index The output channel's index.
   * \return The value of the specified output channel.
   *
   * \sa IO::Block::output()
   */
  const std::vector<double>& readoutput(size_t index);
  void writeoutput(size_t index, const std::vector<double>& data);

private:
  struct port_t
  {
    IO::channel_t channel_info;
    std::vector<double> values;
  };
  std::string name;
  std::array<std::vector<port_t>, 2> ports;
};  // class Block

/*!
 * Acts as a central meeting point between Blocks. Provides
 *   interfaces for finding and connecting blocks.
 *
 * \sa IO::Block
 */
class Connector
{
public:
  Connector();
  ~Connector();

  /*!
   * Loop through each Block and execute a callback.
   * The callback takes two parameters, a Block pointer and param,
   *   the second parameter to foreachBlock.
   *
   * \param callback The callback function.
   * \param param A parameter to the callback function.
   *
   * \sa IO::Block
   */
  void foreachBlock(void (*callback)(Block*, void*), void* param);

  /*!
   * Loop through each Connection and execute callback.
   * The callback takes 5 parameters:
   *   The source block
   *   The source output.
   *   The destination block.
   *   The destination input.
   *   The last parameter to foreachConnection.
   *
   * \param callback The callback function.
   * \param param A parameter to the callback function.
   *
   * \sa IO::Block
   */
  void foreachConnection(
      void (*callback)(Block*, size_t, Block*, size_t, void*), void* param);

  /*!
   * Create a connection between the two specified Blocks.
   *
   * \param outputBlock The source of the data.
   * \param outputChannel The source channel of the data.
   * \param inputBlock The destination of the data.
   * \param inputChannel The destination channel of the data.
   *
   * \sa IO::Block
   * \sa IO::Block::input()
   * \sa IO::Block::output()
   */
  void connect(IO::Block* outputBlock,
               size_t outputChannel,
               IO::Block* inputBlock,
               size_t inputChannel);
  /*!
   * Break a connection between the two specified Blocks.
   *
   * \param outputBlock The source of the data.
   * \param outputChannel The source channel of the data.
   * \param inputBlock The destination of the data.
   * \param inputChannel The destination channel of the data.
   *
   * \sa IO::Block
   * \sa IO::Block::input()
   * \sa IO::Block::output()
   */
  void disconnect(IO::Block* outputBlock,
                  size_t outputChannel,
                  IO::Block* inputBlock,
                  size_t inputChannel);

  /*!
   * Determine whether two channels are connected or not.
   *
   * \param outputBlock The source of the data.
   * \param outputChannel THe source channel of the data.
   * \param inputBlock The destination of the data.
   * \param inputChannel The destination channel of the data.
   *
   * \sa IO::Block::connect()
   * \sa IO::Block::disconnect()
   */
  bool connected(IO::Block* outputBlock,
                 size_t outputChannel,
                 IO::Block* inputBlock,
                 size_t inputChannel);

  void insertBlock(Block*);
  void removeBlock(Block*);

private:

  std::list<Block*> blockList;
  std::list<
};  // class Connector

}  // namespace IO

#endif  // IO_H
