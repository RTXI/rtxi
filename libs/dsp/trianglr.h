/*
 * Triangular window
 */

#ifndef _TRIANGLR_H_
#define _TRIANGLR_H_

#include "gen_win.h"

/*!
 * Triangular Window class
 *
 * Generates a Triangular window used for DFT input windowing
 */
class TriangularWindow : public GenericWindow
{
public:
  /*!
   * Creates a triangular window
   *
   * Triangular windows can have zero at each end, or the user
   * can choose to only have nonzero values for the window.
   *
   * \param length The size of the window
   * \param zero_ends set to 1 if you wish to append a zero at both ends, 0
   *                  otherwise
   */
  TriangularWindow(int length, int zero_ends);

  /*!
   * Generate the triangular window
   *
   * This creates the half of the lag window buffer needed to fully specify
   * the triangular window.
   *
   * \param length The size of the window
   * \param zero_ends set to 1 if you wish to append a zero at both ends, 0
   *                  otherwise
   */
  void GenerateWindow(int length, int zero_ends);
};

#endif
