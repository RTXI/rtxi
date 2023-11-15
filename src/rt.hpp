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

#ifndef RT_H
#define RT_H

#include <unordered_map>
#include <variant>
#include <vector>

#include "event.hpp"
#include "fifo.hpp"
#include "io.hpp"
#include "rtos.hpp"

// forward declaration
namespace Widgets
{
class Component;
}  // namespace Widgets

/*!
 * Objects contained within this namespace are responsible
 *   for managing realtime execution.
 */
namespace RT
{

namespace State
{

/*!
 * Value used to store internal RT::Thread state
 */
enum state_t : int8_t
{
  INIT, /*!< The parameters need to be initialized.         */
  EXEC, /*!< The module is in execution mode                */
  MODIFY, /*!< The parameters have been modified by the user. */
  PERIOD, /*!< The system period has changed.                 */
  PAUSE, /*!< The Pause button has been activated            */
  UNPAUSE, /*!< When the pause button has been deactivated     */
  EXIT, /*!< When the module has been told to exit        */
};
}  // namespace State

/*!
 * Telemitry structure used to check RT::System state during runtime
 */
namespace Telemitry
{
typedef int response_t;
constexpr response_t RT_PERIOD_UPDATE = 0; /*!< The period has been updated*/
constexpr response_t RT_THREAD_LIST_UPDATE = 1; /*!< Thread list was updated*/
constexpr response_t RT_DEVICE_LIST_UPDATE = 2; /*!< Device list was updated*/
constexpr response_t RT_NOOP = 3; /*!< A No-Operation was performed*/
constexpr response_t RT_SHUTDOWN =
    4; /*!< RTXI Real-TIme system is shutting down*/
constexpr response_t RT_WIDGET_PARAM_UPDATE =
    5; /*!< A widget parameter was updated*/
constexpr response_t IO_LINK_UPDATED =
    6; /*!< A interblock connection was updated*/
constexpr response_t RT_WIDGET_STATE_UPDATE =
    7; /*!< The state of a widget was updated*/
constexpr response_t RT_ERROR =
    -1; /*!< There was an error with the last event handling*/
constexpr response_t NO_TELEMITRY = -2; /*!< No Telemitry (placeholder)*/

/*!
 * Response structure representing state changes in RT::System
 *
 * Response data structure is sent over the event fifo from the
 * realtime thread to the ui threads. It holds information such as
 * the telemitry code, as well as the command pointer associated with the
 * response. The command pointer is then used by the telemitry processor
 * to wake sleeping threads waiting on the success (or failure) of
 * such command.
 */
struct Response
{
  response_t type = NO_TELEMITRY;
  Event::Object* cmd = nullptr;
};
}  // namespace Telemitry

/*!
 * Base class for devices that are to interface with System.
 *
 * Classes who inherit this base class are defined as blocks
 * independent of other blocks, and define device interface methods
 * read() and write().
 *
 * \sa RT::System
 */
class Device : public IO::Block
{
public:
  Device(std::string n, const std::vector<IO::channel_t>& c)
      : IO::Block(std::move(n), c, /*isdependent=*/false)
  {
  }
  Device(const Device&) = default;  // copy constructor
  Device& operator=(const Device&) = default;  // copy assignment operator
  Device(Device&&) = delete;  // move constructor
  Device& operator=(Device&&) = delete;  // move assignment operator
  ~Device() override = default;

  /*! \fn virtual void read(void)
   * Function called by the realtime task at the beginning of each period.
   *
   * \sa RT::System
   */
  virtual void read() = 0;

  /*! \fn virtual void write(void)
   * Function called by the realtime task at the end of each period.
   *
   * \sa RT::System
   */
  virtual void write() = 0;
};  // class Device

/*!
 * Base class for objects that are to interface with System.
 *
 * Classes who inherit this base class are defined as blocks
 * dependent of other blocks, and define thread interface method
 * execute().
 *
 * \sa RT::System
 */
class Thread : public IO::Block
{
public:
  Thread(std::string n, const std::vector<IO::channel_t>& c)
      : IO::Block(std::move(n), c, /*isdependent=*/true)
  {
  }
  Thread(const Thread& connector) = default;  // copy constructor
  Thread& operator=(const Thread& connector) =
      default;  // copy assignment operator
  Thread(Thread&&) = delete;  // move constructor
  Thread& operator=(Thread&&) = delete;  // move assignment operator
  ~Thread() override = default;

  /*! \fn virtual void execute(void)
   * Function called periodically by the realtime task.
   *
   * \sa RT::System
   */
  virtual void execute() = 0;
};  // class Thread

/*!
 * Information about the connections between blocks.
 *
 * This is meant to represent connection information about blocks and channels.
 * It also defines equality and inequality functions for determining whether a
 * connection is the same as another, and convenient for existence tests.
 *
 * \param src IO::Block pointer representing the source of data
 * \param src_port_type IO::flags_t for source. Either IO::INPUT, IO::OUTPUT or
 *                      IO::UNKNOWN
 * \param src_port Index of the source channel generating the output
 * \param dest IO::Block pointer representing who to send the output to
 * \param dest_port Index of the destination channel taking the output as input
 */
typedef struct block_connection_t
{
  IO::Block* src = nullptr;
  IO::flags_t src_port_type = IO::UNKNOWN;
  size_t src_port = 0;
  IO::Block* dest = nullptr;
  size_t dest_port = 0;
  bool operator==(const block_connection_t& rhs) const
  {
    return (this->src == rhs.src) && (this->src_port_type == rhs.src_port_type)
        && (this->src_port == rhs.src_port) && (this->dest == rhs.dest)
        && (this->dest_port == rhs.dest_port);
  }
  bool operator!=(const block_connection_t& rhs) const
  {
    return !operator==(rhs);
  }
} block_connection_t;

/*!
 * Class that manages connections between blocks.
 *
 * The connector class, which should not be confused with the connector plugin,
 * is used by RT::System class during runtime to correctly connect and schedule
 * blocks before execution. Most of the connector class functionality is used
 * outside of real-time, but some can and should be used in the real-time
 * context. this class can also differentiate between RT::Device and RT::Thread
 * and keeps a registry of all connections. FInally, Connector class is
 * responsible for moving outputs of blocks to their appropriate inputs during
 * runtime.
 *
 * The connector plugin communicates with RT::System through events to query and
 * establish block connections.
 *
 * \sa IO::Block
 * \sa Connector::Plugin
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
   * Create a connection between source and destination block.
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
  int connect(block_connection_t connection);

  /*!
   * Break a connection between source and destination blocks.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block
   * \sa IO::Block::writeinput()
   * \sa IO::Block::readoutput()
   */
  void disconnect(block_connection_t connection);

  /*!
   * Determine whether source and destination blocks are connected.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block::connect()
   * \sa IO::Block::disconnect()
   */
  bool connected(block_connection_t connection);

  /*!
   * Register the block in order to access connection services
   *
   * The caller of this function must provide a pointer to the memory
   * that the real-time thread will use to store connections. This is
   * an optimization that avoids memory allocations in rt-thread as
   * much as possible
   *
   * \param thread Pointer to block object to register
   * \param block_connections pointer to the vector connections memory to use
   */
  void insertBlock(IO::Block* block,
                   std::vector<RT::block_connection_t>& block_connections);

  /*!
   * Unregister the block from the registry
   *
   * \param block Pointer to block to unregister
   */
  void removeBlock(IO::Block* block);

  /*!
   * Checks whether block is registered with the connector
   *
   * \param IO::Block Pointer to block
   * \returns true if registered, false otherwise
   */
  bool isRegistered(IO::Block* block);

  /*!
   * Get the list of devices that are registered with connector class.
   *
   * To the connector class devices are io blocks that are independent
   * of other blocks when connected.
   *
   * \returns vector of RT::Device pointers representing registered devices
   */
  std::vector<RT::Device*> getDevices();

  /*!
   * Get a list of threads that are registered with connector class.
   *
   * To the connector class threads are blocks that are dependent of other
   * blocks when connected. They are topologically sorted.
   *
   * \returns vector of RT::Thread pointers representing registered threads
   */
  std::vector<RT::Thread*> getThreads();

  /*!
   * Returns a list of output connections for the given block
   *
   * \param src Source IO::Block pointer to find the connections for
   * \returns A vector of RT::block_connection_t containing connection info
   * \sa RT::block_connection_t
   */
  std::vector<RT::block_connection_t> getOutputs(IO::Block* src);

  /*!
   * Copies outputs of the given block
   *
   * This function is used in the real-time loop to propagate values
   * from one block to another. It works by using the assigned index of
   * the block to quickly access the block, then iterates through all
   * connections of the block and copies output values to the input values
   * of the next block.
   *
   * \param Pointer to block that is the source of the output.
   */
  void propagateBlockConnections(IO::Block* block);

  /*!
   * Destroys all connections for a given block
   *
   * \param block The IO::Block object pointer to remove connections from
   */
  void clearAllConnections(IO::Block* block);

  /*!
   * Query all registered blocks
   *
   * \return A vector of IO::Block pointers that are registered with the
   * connector
   */
  std::vector<IO::Block*> getRegisteredBlocks();

  /*!
   * Query all connections in RTXI
   *
   * \return A vector of RT::block_connection_t representing all connections in
   *         the connector class registry
   *
   * \sa RT::block_connection_t
   */
  std::vector<RT::block_connection_t> getAllConnections();

private:
  int find_cycle(RT::block_connection_t conn, IO::Block* ref_block);
  std::vector<RT::Thread*> topological_sort();
  std::vector<IO::Block*> block_registry;
  std::vector<std::vector<RT::block_connection_t>> connections;
};  // class Connector

/*!
 * Variant used internally by RT::System to receive commands and
 * updates
 */
using command_param_t = std::variant<std::monostate,
                                     int64_t,
                                     int64_t*,
                                     uint64_t,
                                     double,
                                     RT::Thread*,
                                     std::vector<RT::Thread*>*,
                                     RT::Device*,
                                     std::vector<RT::Device*>*,
                                     IO::Block*,
                                     RT::block_connection_t,
                                     Widgets::Component*,
                                     State::state_t,
                                     std::string>;
/*!
 * Manages the RTOS as well as all objects that require
 *   realtime execution.
 *
 * The System class is responsible for providing the execution context,
 * closed real-time loop, and connection processes. Instantiation of this
 * class automatically creates the real-time thread, and sets up communication
 * with the Event::Manager class. Note that this does not initiate the
 * telemitry system, and this should be created separately. The only real
 * way to manage the real-time thread is by sending events, which system is
 * programmed to transform into commands it can understand internally. This
 * is designed to prevent use of non-realtime concurrency primitives that can
 * slow down the system.
 */
class System : public Event::Handler
{
public:
  explicit System(Event::Manager* em, RT::Connector* rtc);
  System(const System& system) = delete;  // copy constructor
  System& operator=(const System& system) = delete;  // copy assignment operator
  System(System&&) = delete;  // move constructor
  System& operator=(System&&) = delete;  // move assignment operator
  ~System() override;

  /*!
   * Obtain the real-time period (in nanoseconds) of the system
   *
   * \param The Period of the system in nanoseconds
   */
  int64_t getPeriod();

  /*!
   * Extracts telemitry from the system running in real-time
   *
   * \return A vector of RT::Telemitry::Response structs
   */
  std::vector<RT::Telemitry::Response> getTelemitry();

  /*!
   * Creates a worker thread that reads telemitry from real-time thread
   *
   * The passing of messages to the real-time thread using
   * RT::System::receiveEvent is not enough to change system state as the system
   * itself should not interact directly with concurrency primitives. The
   * solution is to create a thread that will read telemitry struct and wakeup
   * any sleeping threads associated with the given internal command. Without
   * this, any caller that sends events to the system class will block
   * indefinitely.
   *
   * \sa RT::System::getTelemitry()
   */
  void createTelemitryProcessor();

  /*!
   * Processes Event and sends appropriate command to real-time thread
   *
   * This function not only processes the event object, but it also performs
   * computations that do not need to be run in real-time. This offloads heavy
   * computations such as topological sort of threads or allocation of resources
   * to non-realtime threads and allows the real-time thread to run fast fast
   * fast.
   *
   * \param event An Event::Object object for the system to process
   * \sa Event::Manager
   * \sa Event::Handler
   */
  void receiveEvent(Event::Object* event) override;

private:
  // We want our cmd class to be private. the only way to access
  // RT::System functions is through its event handler.
  class CMD : public Event::Object
  {
  public:
    explicit CMD(Event::Type et);
    command_param_t getRTParam(const std::string_view& param_name);
    void setRTParam(const std::string_view& param_name,
                    const command_param_t& value);

  private:
    struct rt_param
    {
      std::string_view name;
      RT::command_param_t value;
    };
    std::vector<rt_param> rt_params;
  };

  void insertDevice(Event::Object* event);
  void removeDevice(Event::Object* event);
  void insertThread(Event::Object* event);
  void removeThread(Event::Object* event);
  // void blockActivityChange(Event::Object* event);
  void threadActivityChange(Event::Object* event);
  void deviceActivityChange(Event::Object* event);
  void ioLinkChange(Event::Object* event);
  void connectionsInfoRequest(Event::Object* event);
  void allConnectionsInfoRequest(Event::Object* event);
  void blockInfoRequest(Event::Object* event);
  void setPeriod(Event::Object* event);
  void shutdown(Event::Object* event);
  void NOOP(Event::Object* event);
  void getPeriodValues(Event::Object* event);
  void provideTimetickPointers(Event::Object* event);
  void changeWidgetParameters(Event::Object* event);
  void changeWidgetState(Event::Object* event);

  void executeCMD(CMD* cmd);
  void updateDeviceList(CMD* cmd);
  void updateThreadList(CMD* cmd);
  void ioLinkUpdateCMD(CMD* cmd);
  void updateBlockActivity(CMD* cmd);
  void setPeriod(CMD* cmd);
  void shutdown(CMD* cmd);
  void getPeriodTicksCMD(CMD* cmd);
  void changeWidgetParametersCMD(CMD* cmd);
  void changeWidgetStateCMD(CMD* cmd);

  void postTelemitry(RT::Telemitry::Response telemitry);

  static void execute(void* sys);

  int64_t periodStartTime = 1;
  int64_t periodEndTime = 1;
  int64_t lastperiodStartTime = 1;

  // System owns the task object and the pipe used to communicate with it
  std::unique_ptr<RT::OS::Task> task;
  std::unique_ptr<RT::OS::Fifo> eventFifo;
  std::thread telemitry_processing_thread;
  std::atomic<bool> telemitry_processing_thread_running = true;

  // system doesn't own any of the below variables. That's why they are
  // only pointers.
  Event::Manager* event_manager;
  RT::Connector* rt_connector;

  // System's real-time loop maintains copy of device and thread pointers
  std::vector<RT::Device*> devices;
  std::vector<RT::Thread*> threads;
};  // class System
}  // namespace RT
#endif  // RT_H
