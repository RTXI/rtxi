#include "basicplot.h"
#include <stdlib.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>

BasicPlot::BasicPlot(QWidget *parent) :
  QwtPlot(parent)
{
  setFrameStyle(QFrame::NoFrame);
  setLineWidth(0);
  QFrame(canvas()).setLineWidth(2);
  plotLayout()->setAlignCanvasToScales(true);
  QwtPlotGrid *grid = new QwtPlotGrid;
  grid->setMajorPen(QPen(Qt::gray, 0, Qt::DotLine));
  grid->attach(this);
  setCanvasBackground(QColor(29, 100, 141)); // nice blue

  // enable zooming
  Zoomer *zoomer = new Zoomer(qobject_cast<QwtPlotCanvas *>(canvas()));
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
  QwtScaleDiv * bottom = new QwtScaleDiv;
  *bottom = axisScaleDiv(QwtPlot::xBottom);
  QwtScaleDiv * left = new QwtScaleDiv;
  *left = axisScaleDiv(QwtPlot::yLeft);
//  emit setNewBase(&axisScaleDiv(QwtPlot::xBottom), &axisScaleDiv(QwtPlot::yLeft));
  setNewBase(bottom, left);
}

QSize
BasicPlot::sizeHint() const
{
  return QSize(540, 400);
}

