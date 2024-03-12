/*
 * Hann window, not to be confused with Hanning window
 */

#ifndef _HANN_H_
#define _HANN_H_

#include "gen_win.h"

/*!
  * Hann Window class
  *
  * Generates a Hann window used for DFT input windowing
  */
class HannWindow : public GenericWindow
{
public:

  /*!
   * Creates a Hann window
   *
   * Hann windows can have zero at each end, or the user
   * can choose to only have nonzero values for the window.
   *
   * \param num_taps The size of the window
   * \param zero_ends set to 1 if you wish to append a zero at both ends, 0
   *                  otherwise
   */
  HannWindow(int num_taps, int zero_ends);
  
  /*!
   * Generate the hann window
   *
   * This creates the half of the lag window buffer needed to fully specify
   * the hann window.
   *
   * \param length The size of the window
   * \param zero_ends set to 1 if you wish to append a zero at both ends, 0
   *                  otherwise
   */
  void GenerateWindow(int length, int zero_ends);
};

#endif
