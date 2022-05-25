#ifndef RTOS_H
#define RTOS_H

#include <functional>
#include <thread>

#include "debug.hpp"

namespace RT
{
namespace OS
{

const int SECONDS_TO_NANOSECONDS = 1000000000;  // Conversion from sec to nsec
const int64_t DEFAULT_PERIOD = 1000000;  // Default period is set to 1 msec

struct Task
{
  int64_t period = DEFAULT_PERIOD;
  int64_t next_t = 0;
  bool task_finished = false;
  std::thread rt_thread;
};

/*!
 * Initializes the real-time resources. This is done by locking memory
 * Pages and storing real-time identification variables
 *
 * \return 0 if successful, error code otherwise
 */
int initiate();

/*!
 * Releases real-time resources from the operating system. Called when rtxi
 * is closing.
 */
void shutdown();


/*!
 * terminates task in real-time loop
 *
 * \param task Object holding metadata for real-time task
 */
void deleteTask(std::unique_ptr<Task> & task);

/*!
 * Set the period for the real-time task
 *
 * \param task Object holding metadata for real-time task
 * \param period The new period to set the real-time loop to
 *
 * \returns 0 if successful, -1 otherwise
 */
int setPeriod(std::unique_ptr<Task> & task, int64_t period);

/*!
 * Uses real-time core to sleep until the next periodic wakeup.
 * It uses the timestep given in task to determine next waekup.
 *
 * \param task Object holding the timestep data for the task
 */
void sleepTimestep(std::unique_ptr<Task> & task);

/*!
 * CHecks whether the calling thread is in real time. Important
 * for functions that should not have access to real-time
 * services else risk blocking the whole computer system.
 *
 * \returns true if the calling thread is real-time, false otherwise
 */
bool isRealtime();

/*!
 * Returns the current CPU time in nanoseconds. In general
 *   this is really only useful for determining the time
 *   between two events.
 *
 * \return The current CPU time.
 */
int64_t getTime();

/*!
 * Returns the percentage of Cpu being used by the Real-Time
 * Thread. Should not be run from the real-time thread
 * directly as this can cause high latency.
 *
 * \return CPU_TIME Percentage of time the cpu is spent on
 *      real-time thread calculations. In multicore systems it
 *      is the percent of Cpu for the specific processor running
 *      the real-time task.
 */
double getCpuUsage();

/*!
 * Creates a real-time task. This task will be associated with a system object
 * to manage it.
 *
 * \param task Object holding metadata for real-time task
 * \param entry Callable function that will run the real-time loop
 * \param arg Reference to RT::System object that will manage this real-time loop
 *
 * \returns 0 if successful, -1 otherwise
 */
template <typename T>
int createTask(std::unique_ptr<Task> & task,
               std::function<void(T&)> func,
               T & arg)
{
  // Should not be creating real-time tasks from another real-time task
  if (RT::OS::isRealtime()) {
    ERROR_MSG("RT::OS::createTask : Task cannot be created from rt context");
    return -1;
  }
  int resval = 0;
  auto wrapper = [&func, &arg, &resval](){
    resval = RT::OS::initiate();
    if (resval != 0) {
      ERROR_MSG("There was a problem creating the real-time thread");
      return;
    }
    func(arg);
    RT::OS::shutdown();
  };
  std::thread thread_obj(wrapper);
  task->rt_thread = std::move(thread_obj);
  return resval;
}

/*!
Simple FIFO(First In First Out) for data transfer between RTXI threads

This data structure is a fundamental component to the inter-process 
communication between threads spawned by RTXI. In particular this is used
for communication between the real-time thread and non-realtime threads.
This is platform and interface dependent, so the FIFO primitive used in
posix interface will be different than Xenomai's evl interface
*/
class Fifo
{
public:
  /*!
   * FIFO constructor. Builds a FIFO object.
   *
   * \param s Size of the FIFO
   */
  Fifo(size_t size) : fifo_size(size) { }

  /*!
   * FIFO Destructor
   */
  virtual ~Fifo();

  /*!
   * Read the data stored in the FIFO written by RT thread.
   *
   * \param buffer The buffer where the data from the buffer should be
   *     written to
   * \param size The size of the data to read from the buffer
   * \param blocking Whether the thread should expect to be blocked or not
   * \return n Number of elements read. Same as size.
   */
  virtual size_t read(void* event, size_t event_size, bool blocking = true);

  /*!
   * Write to the FIFO storage for the RT thread.
   *
   * \param buffer The buffer holding the data to write to the FIFO.
   * \param size The size of the data to read from the buffer
   * \return n Number of elements written. Same as size.
   */
  virtual size_t write(const void* event, size_t event_size);

  /*!
   * Read the data stored in the FIFO written by non-RT thread.
   *
   * \param buffer The buffer where the data from the buffer should be
   *     written to
   * \param size The size of the data to read from the buffer
   * \param blocking Whether the thread should expect to be blocked or not
   * \return n Number of elements read. Same as size.
   */
  virtual size_t readRT(void* event, size_t event_size, bool blocking = true);

  /*!
   * Write to the FIFO storage for the non-RT thread.
   *
   * \param buffer The buffer holding the data to write to the FIFO.
   * \param size The size of the data to read from the buffer
   * \return n Number of elements written. Same as size.
   */
  virtual size_t writeRT(void* event, size_t event_size);

  size_t fifo_size;
};

int getFifo(std::unique_ptr<Fifo> & fifo);

}  // namespace OS
}  // namespace RT

#endif