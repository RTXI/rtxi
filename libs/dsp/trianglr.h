/*
 * Triangular window
 */

#ifndef _TRIANGLR_H_
#define _TRIANGLR_H_

#include "gen_win.h"

class TriangularWindow : public GenericWindow
{
public:
  TriangularWindow(int length, int zero_ends);
  void GenerateWindow(int length, int zero_ends);
};

#endif
