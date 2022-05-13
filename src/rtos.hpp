#ifndef RTOS_H
#define RTOS_H

#include <thread>

namespace RT
{
namespace OS
{

const int SECONDS_TO_NANOSECONDS = 1000000000; // Conversion from sec to nsec
const int64_t DEFAULT_PERIOD = 1000000; // Default period is set to 1 msec

struct Task
{
  int64_t period = DEFAULT_PERIOD;
  int64_t next_t = 0;
  bool task_finished = false;
  std::shared_ptr<std::thread> rt_thread;
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
 * Creates a real-time task
 *
 * \param task Object holding metadata for real-time task
 * \param entry Pointer to function that will be run in real-time loop
 * \param arg Arguments to pass to function that will run in real-time loop
 * \param prio Priority of the real-time thread running the function. Currently
 * unused.
 *
 * \returns 0 if successful, -1 otherwise
 */
int createTask(Task* task, void* (*entry)(void*), void* arg, int prio = 0);

/*!
 * terminates task in real-time loop
 *
 * \param task Object holding metadata for real-time task
 */
void deleteTask(Task* task);

/*!
 * Set the period for the real-time task
 *
 * \param task Object holding metadata for real-time task
 * \param period The new period to set the real-time loop to
 *
 * \returns 0 if successful, -1 otherwise
 */
int setPeriod(Task* task, int64_t period);

/*!
 * Uses real-time core to sleep until the next periodic wakeup.
 * It uses the timestep given in task to determine next waekup.
 *
 * \param task Object holding the timestep data for the task
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

}  // namespace OS
}  // namespace RT

#endif