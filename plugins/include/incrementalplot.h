/*
 IncrementalPlot, derived from QwtPlot, contains a container class for growing data and allows you
 to plot data asynchronously of any size from a single (x,y) point to a series of points. It will 
 also draw lines, e.g. data fits, holding line data in a separate structure.
 */

#ifndef _INCREMENTALPLOT_H_
#define _INCREMENTALPLOT_H_ 1

#include "basicplot.h"
#include <qwt-qt3/qwt_array.h>
#include <qwt-qt3/qwt_plot.h>
#include <qwt-qt3/qwt_symbol.h>

class QwtPlotCurve;

class CurveData
{
  // A container class for growing data
public:

  CurveData();

  void
  append(double *x, double *y, int count);

  int
  count() const;
  int
  size() const;
  const double *
  x() const;
  const double *
  y() const;

private:
  int d_count;
  QwtArray<double> d_x;
  QwtArray<double> d_y;
};

class IncrementalPlot : public BasicPlot
{
Q_OBJECT

public:
  IncrementalPlot(QWidget *parent = NULL);
  virtual
  ~IncrementalPlot();

  void
  appendData(double x, double y); // draws data point with default symbol style
  void
  appendData(double x, double y, QwtSymbol::Style s); // draws data point with specified symbol style
  void
  appendData(double *x, double *y, int size); // draws multiple data points
  void
  appendData(double *x, double *y, int size, QwtSymbol::Style s);

  void
  removeData(); // clears all data and lines
  const double *
  xData(); // returns data points (not points defining lines)
  const double *
  yData();
  const uint
  dataSize(); // returns number of data points
  bool
  dataExists(); // returns whether plot contains data
  void
  nextSymbol(); // sets symbol to the next QwtSymbol style

public slots:

  void
  appendLine(double *x, double *y, int size); // draws a line through given points

private:
  CurveData *d_data; // holds data points
  QwtPlotCurve *d_curve;
  CurveData *l_data; // holds points defining lines
  QwtPlotCurve *l_curve;
};

#endif // _INCREMENTALPLOT_H_
