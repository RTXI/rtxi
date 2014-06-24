#include "scatterplot.h"

ScatterPlot::ScatterPlot(QWidget *parent) :
  IncrementalPlot(parent)
{
  setAxes(0, 1000, 0, 100); // initial view
}

void
ScatterPlot::appendPoint(double x, double y)
{
  appendData(x, y);
}

void
ScatterPlot::appendPoint(double x, double y, QwtSymbol::Style s)
{
  appendData(x, y, s);
}

void
ScatterPlot::clear()
{
  removeData();
  replot();
}
