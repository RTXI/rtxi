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
#include <vector>
#include <variant>

#include "event.hpp"
#include "fifo.hpp"
#include "io.hpp"
#include "rtos.hpp"

// forward declaration
namespace Modules {
class Component;
}; // namespace Modules

/*!
 * Objects contained within this namespace are responsible
 *   for managing realtime execution.
 */
namespace RT
{


namespace Telemitry
{
typedef int response_t;
const response_t RT_PERIOD_UPDATE = 0;
const response_t RT_THREAD_LIST_UPDATE = 1;
const response_t RT_DEVICE_LIST_UPDATE = 2;
const response_t RT_NOOP = 3;
const response_t RT_SHUTDOWN = 4;
const response_t RT_MODULE_PARAM_UPDATE = 5;
const response_t RT_ERROR = -1;
const response_t NO_TELEMITRY = -2;

struct Response {
  response_t type = NO_TELEMITRY;
  Event::Object* cmd = nullptr;
};
};  // namespace Telemitry

/*!
 * Base class for devices that are to interface with System.
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
  Device(const Device& connector) = default;  // copy constructor
  Device& operator=(const Device& connector) =
      default;  // copy assignment operator
  Device(Device&&) = delete;  // move constructor
  Device& operator=(Device&&) = delete;  // move assignment operator
  virtual ~Device() = default;

  /*! \fn virtual void read(void)
   * Function called by the realtime task at the beginning of each period.
   *
   * \sa RT::System
   */
  void read();

  /*! \fn virtual void write(void)
   * Function called by the realtime task at the end of each period.
   *
   * \sa RT::System
   */
  void write();

protected:
  void bind_read_callback(std::function<void(void)> callback);
  void bind_write_callback(std::function<void(void)> callback);

private:
  std::function<void(void)> read_callback;
  std::function<void(void)> write_callback;
};  // class Device

/*!
 * Base class for objects that are to interface with System.
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
  virtual ~Thread() = default;

  /*! \fn virtual void execute(void)
   * Function called periodically by the realtime task.
   *
   * \sa RT::System
   */
  void execute();

protected:
  void bind_execute_callback(std::function<void(void)> callback);

private:
  std::function<void(void)> execute_callback;
};  // class Thread

typedef struct
{
  RT::Device* dest;
  size_t dest_port;
} device_connection_t;

typedef struct
{
  RT::Thread* dest;
  size_t dest_port;
} thread_connection_t;

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
  std::vector<thread_connection_t> output_threads;
  std::vector<device_connection_t> output_devices;
} outputs_info;

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
   * Create a connection between source thread and destination thread.
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
  int connect(RT::Thread* src, size_t out, RT::Thread* dest, size_t in);

  /*!
   * Create a connection between source thread and destination device.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \returns 0 if successfully connected, -1 otherwise
   *
   * \sa IO::Block
   */
  int connect(RT::Thread* src, size_t out, RT::Device* dest, size_t in);

  /*!
   * Create a connection between source device and destination thread.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \returns 0 if successfully connected, -1 otherwise
   *
   * \sa IO::Block
   */
  int connect(RT::Device* src, size_t out, RT::Thread* dest, size_t in);

  /*!
   * Create a connection between source device and destination device.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \returns 0 if successfully connected, -1 otherwise
   *
   * \sa IO::Block
   */
  int connect(RT::Device* src, size_t out, RT::Device* dest, size_t in);

  /*!
   * Break a connection between source thread and destination thread.
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
  void disconnect(RT::Thread* src, size_t out, RT::Thread* dest, size_t in);

  /*!
   * Break a connection between source thread and destination device.
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
  void disconnect(RT::Thread* src, size_t out, RT::Device* dest, size_t in);

  /*!
   * Break a connection between source device and destination thread.
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
  void disconnect(RT::Device* src, size_t out, RT::Thread* dest, size_t in);

  /*!
   * Break a connection between source device and destination device.
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
  void disconnect(RT::Device* src, size_t out, RT::Device* dest, size_t in);

  /*!
   * Determine whether source thread and destination thread are connected.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block::connect()
   * \sa IO::Block::disconnect()
   */
  bool connected(RT::Thread* src, size_t out, RT::Thread* dest, size_t in);

  /*!
   * Determine whether source thread and destination device are connected.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block::connect()
   * \sa IO::Block::disconnect()
   */
  bool connected(RT::Thread* src, size_t out, RT::Device* dest, size_t in);

  /*!
   * Determine whether source device and destination thread are connected.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block::connect()
   * \sa IO::Block::disconnect()
   */
  bool connected(RT::Device* src, size_t out, RT::Thread* dest, size_t in);

  /*!
   * Determine whether source device and destination device are connected.
   *
   * \param src The source of the data.
   * \param out The source channel of the data.
   * \param dest The destination of the data.
   * \param in The destination channel of the data.
   *
   * \sa IO::Block::connect()
   * \sa IO::Block::disconnect()
   */
  bool connected(RT::Device* src, size_t out, RT::Device* dest, size_t in);

  /*!
   * Register the thread in order to access connection services
   *
   * \param thread Pointer to block object to register
   */
  void insertBlock(RT::Thread* thread);

  /*!
   * Register the device in order to access connection services
   *
   * \param device Pointer to block object to register
   */
  void insertBlock(RT::Device* device);

  /*!
   * Unregister the thraed from the registry
   *
   * \param thread Pointer to block to unregister
   */
  void removeBlock(RT::Thread* thread);

  /*!
   * Unregister the device from the registry
   *
   * \param device Pointer to block to unregister
   */
  void removeBlock(RT::Device* device);

  /*!
   * Checks whether thread is registered with the connector
   *
   * \param thread Pointer to thread
   * \returns true if registered, false otherwise
   */
  bool isRegistered(RT::Thread* thread);

  /*!
   * Checks whether device is registered with the connector
   *
   * \param device Pointer to thread
   * \returns true if registered, false otherwise
   */
  bool isRegistered(RT::Device* device);

  /*!
   * Get the list of devices that are registered with connector class.
   * To the connector class devices are io blocks that are independent
   * of other blocks when connected.
   *
   * \returns List of RT::Device pointers representing registered devices
   */
  std::vector<RT::Device*> getDevices();

  /*!
   * Get a lsit of threads that are registered with connector class. To
   * the connector class threads are blocks that are dependent of other
   * blocks when connected. They are topologically sorted.
   *
   * \returns List of RT::Thread pointers representing registered threads
   */
  std::vector<RT::Thread*> getThreads();

  /*!
   * Returns a list of outputs for the input thread
   *
   * \param src Source RT::Thread pointer to find the outputs for
   * \returns A vector of RT::outputs_info containing connection info
   * \sa RT::outputs_info
   */
  std::vector<RT::outputs_info> getOutputs(RT::Thread* src);

  /*!
   * Returns a list of outputs for the input device
   *
   * \param src Source RT::Device pointer to find the outputs for
   * \returns A vector of RT::outputs_info containing connection info
   * \sa RT::outputs_info
   */
  std::vector<RT::outputs_info> getOutputs(RT::Device* src);

  /*!
   * Copies outputs of the given device object to the inputs of
   * connected thread and device objects
   *
   * \param device Pointer to device that is the source of the output.
   */
  void propagateDeviceConnections(RT::Device* device);

  /*!
   * Copies outputs of the given thread object to the inputs of
   * connected thread and device objects
   *
   * \param thread Pointer to thraed that is the source of the output.
   */
  void propagateThreadConnections(RT::Thread* thread);

  void assignID(IO::Block* block);

private:
  typedef struct
  {
    RT::Thread* thread_ptr;
    std::vector<outputs_info> channels_outbound_con;
  } thread_entry_t;

  typedef struct
  {
    RT::Device* device_ptr;
    std::vector<outputs_info> channels_outbound_con;
  } device_entry_t;

  std::vector<RT::Thread*> topological_sort();
  std::vector<thread_entry_t> thread_registry;
  std::vector<device_entry_t> device_registry;
};  // class Connector

using command_param_t = std::variant<std::monostate, 
                                     int64_t, int64_t*, uint64_t, double,
                                     RT::Thread*, std::vector<RT::Thread*>*,
                                     RT::Device*, std::vector<RT::Device*>*,
                                     Modules::Component*, std::string>;
/*!
 * Manages the RTOS as well as all objects that require
 *   realtime execution.
 */
class System : public Event::Handler
{
public:
  explicit System(Event::Manager* em, RT::Connector* rtc);
  System(const System& system) = delete;  // copy constructor
  System& operator=(const System& system) = delete;  // copy assignment operator
  System(System&&) = delete;  // move constructor
  System& operator=(System&&) = delete;  // move assignment operator
  ~System();

  int64_t getPeriod();
  RT::Telemitry::Response getTelemitry();

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
    struct rt_param {
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
  void setPeriod(Event::Object* event);
  void shutdown(Event::Object* event);
  void NOOP(Event::Object* event);
  void getPeriodValues(Event::Object* event);
  void provideTimetickPointers(Event::Object* event);
  void changeModuleParameters(Event::Object* event);

  void executeCMD(CMD* cmd);
  void updateDeviceList(CMD* cmd);
  void updateThreadList(CMD* cmd);
  void updateBlockActivity(CMD* cmd);
  void setPeriod(CMD* cmd);
  void shutdown(CMD* cmd);
  void getPeriodTicksCMD(CMD* cmd);
  void changeModuleParametersCMD(CMD* cmd);

  void postTelemitry(RT::Telemitry::Response telemitry);
  void createTelemitryProcessor();

  static void execute(void* sys);

  int64_t periodStartTime = 1;
  int64_t periodEndTime = 1;
  int64_t lastperiodStartTime = 1;

  // System owns the task object and the pipe used to communicate with it
  std::unique_ptr<RT::OS::Task> task;
  std::unique_ptr<RT::OS::Fifo> eventFifo;

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
