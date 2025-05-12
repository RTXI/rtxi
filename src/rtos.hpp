#ifndef RTOS_H
#define RTOS_H

#include <any>
#include <thread>

#include <string.h>

namespace RT::OS
{

const int64_t SECONDS_TO_NANOSECONDS =
    1000000000;  // Conversion from sec to nsec
const int64_t DEFAULT_PERIOD = 1000000;  // Default period is set to 1 msec
const uint64_t DEFAULT_FIFO_SIZE = 255;  // Default Fifo size of 255 bytes
const size_t PROCESSOR_COUNT = std::thread::hardware_concurrency();
/*!
 * Object representation of a real-time loop
 *
 * \param period The period for the real-time loop in nanoseconds.
 * \param next_t Next wakeup time in absolute clock time (nanoseconds).
 * \param task_finished Bool field used by real-time loop to signal end.
 * \param rt_thread a std::thread object representing the rt loop.
 */
struct Task
{
  int64_t period = DEFAULT_PERIOD;
  int64_t next_t = 0;
  bool task_finished = false;
  std::thread rt_thread;
  std::any thread_id;
};

/*!
 * Initializes the real-time resources. This is done by locking memory
 * Pages and storing real-time identification variables
 *
 * \param task The real-time structure identifying the task
 * \return 0 if successful, error code otherwise
 *
 * \sa RT::OS::shutdown()
 */
int initiate(RT::OS::Task* task);

/*!
 * Releases real-time resources from the operating system. Called when rtxi
 * is closing.
 *
 * \param task the real-time structure identifying the task
 * \sa RT::OS::initiate()
 */
void shutdown(RT::OS::Task* task);

/*!
 * terminates task in real-time loop
 *
 * \param task Task object holding metadata for real-time task
 *
 * \sa RT::OS::createTask()
 */
void deleteTask(Task* task);

/*!
 * Set the period for the real-time task
 *
 * \param task Object holding metadata for real-time task
 * \param period The new period to set the real-time loop to
 *
 * \returns 0 if successful, -1 otherwise
 *
 * \sa RT::OS::sleepTimestep
 */
int setPeriod(Task* task, int64_t period);

/*!
 * Get the period for the real-time task
 *
 * This function can only be called from a real-time context and
 * will not function from other threads that are not real-time.
 * It is meant to provide the real-time period to Thread/Device
 * classes that require this value for their calculations in the
 * real-time loop. Available inside execute().
 *
 * \returns Period of the realtime thread the calling function is
 *    running from, -1 otherwise.
 *
 * \sa RT::OS::sleepTimestep
 */
int64_t getPeriod();

/*!
 * Uses real-time core to sleep until the next periodic wakeup.
 * It uses the timestep given in task to determine next waekup.
 *
 * \param task Object holding the timestep data for the task
 *
 * \sa RT::OS::setPeriod()
 */
void sleepTimestep(Task* task);

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
 * \param arg Reference to RT::System object that will manage this real-time
 * loop
 *
 * \returns 0 if successful, -1 otherwise
 */
int createTask(Task* task, void (*func)(void*), void* arg);

/*!
 * Renames a thread. This is useful for debugging and developer sanity
 *
 * \param thread Standard thread object
 * \param name String holding the name to assign to thread
 */
void renameOSThread(std::thread& thread, const std::string& name);
}  // namespace RT::OS

#endif
