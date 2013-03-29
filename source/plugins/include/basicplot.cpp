#include "basicplot.h"
#include <stdlib.h>
#include <qwt-qt3/qwt_plot_grid.h>
#include <qwt-qt3/qwt_plot_canvas.h>
#include <qwt-qt3/qwt_plot_layout.h>

BasicPlot::BasicPlot(QWidget *parent) :
  QwtPlot(parent)
{
  setFrameStyle(QFrame::NoFrame);
  setLineWidth(0);
  setCanvasLineWidth(2);
  plotLayout()->setAlignCanvasToScales(true);
  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->setMajPen(QPen(Qt::gray, 0, Qt::DotLine));
  grid->attach(this);
  setCanvasBackground(QColor(29, 100, 141)); // nice blue

  // enable zooming
  Zoomer *zoomer = new Zoomer(canvas());
  zoomer->setRubberBandPen(QPen(Qt::white, 2, Qt::DotLine));
  zoomer->setTrackerPen(QPen(Qt::white));
  QObject::connect(this, SIGNAL(setNewBase(QwtScaleDiv*,QwtScaleDiv*)), zoomer,
      SLOT(setNewBase(QwtScaleDiv*,QwtScaleDiv*)));

}

void
BasicPlot::setAxes(double xmin, double xmax, double ymin, double ymax)
{
  setAxisScale(xBottom, xmin, xmax);
  setAxisScale(yLeft, ymin, ymax);
  replot();
  // set zoomer to new axes limits
  emit setNewBase(axisScaleDiv(QwtPlot::xBottom), axisScaleDiv(QwtPlot::yLeft));
}

QSize
BasicPlot::sizeHint() const
{
  return QSize(540, 400);
}

