#include "incrementalplot.h"
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_map.h>
#if QT_VERSION >= 0x040000
#include <QPaintEngine>
#endif

CurveData::CurveData()
  : d_count(0)
{
}

void
CurveData::append(double* x, double* y, int count)
{
  int newSize = ((d_count + count) / 1000 + 1) * 1000;
  if (newSize > size()) {
    d_x.resize(newSize);
    d_y.resize(newSize);
  }

  for (int i = 0; i < count; i++) {
    d_x[d_count + i] = x[i];
    d_y[d_count + i] = y[i];
  }
  d_count += count;
}

int
CurveData::count() const
{
  return d_count;
}

int
CurveData::size() const
{
  return d_x.size();
}

const double*
CurveData::x() const
{
  return d_x.data();
}

const double*
CurveData::y() const
{
  return d_y.data();
}

IncrementalPlot::IncrementalPlot(QWidget* parent)
  : BasicPlot(parent)
  , d_data(NULL)
  , d_curve(NULL)
  , l_data(NULL)
  , l_curve(NULL)
{
  setAutoReplot(false);
}

IncrementalPlot::~IncrementalPlot()
{
  delete d_data;
}

const double*
IncrementalPlot::xData()
{
  return d_data->x();
}

const double*
IncrementalPlot::yData()
{
  return d_data->y();
}

uint
IncrementalPlot::dataSize()
{
  return d_data->count();
}

bool
IncrementalPlot::dataExists()
{
  if (d_data == NULL)
    return false;
  else
    return true;
}

void
IncrementalPlot::appendData(double x, double y)
{
  appendData(&x, &y, 1);
}

void
IncrementalPlot::appendData(double x, double y, QwtSymbol::Style s)
{
  appendData(&x, &y, 1, s);
}

void
IncrementalPlot::appendData(double* x, double* y, int size)
{
  if (d_data == NULL)
    d_data = new CurveData;

  if (d_curve == NULL) {
    d_curve = new QwtPlotCurve("Data");
    d_curve->setStyle(QwtPlotCurve::NoCurve);
    d_curve->setPaintAttribute(QwtPlotCurve::FilterPoints, true);
    const QColor& c = Qt::white;
    QwtSymbol* symbol = new QwtSymbol(QwtSymbol::Ellipse);
    symbol->setBrush(QBrush(c));
    symbol->setPen(QPen(c));
    symbol->setSize(QSize(6, 6));
    d_curve->setSymbol(symbol);
    d_curve->attach(this);
  }

  d_data->append(x, y, size);
  d_curve->setRawSamples(d_data->x(), d_data->y(), d_data->count());
#ifdef __GNUC__
#warning better use QwtData
#endif

  const bool cacheMode =
    qobject_cast<QwtPlotCanvas*>(canvas())->testPaintAttribute(
      QwtPlotCanvas::BackingStore);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
  // works on X11. This has an tremendous effect on the performance..

  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

  QwtPlotCanvas* plotCanvas = qobject_cast<QwtPlotCanvas*>(canvas());
  plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, false);
  QPainter painter;
  QwtScaleMap xMap = plotCanvas->plot()->canvasMap(d_curve->xAxis());
  QwtScaleMap yMap = plotCanvas->plot()->canvasMap(d_curve->yAxis());
  d_curve->drawSeries(&painter, xMap, yMap, (d_curve->paintRect(xMap, yMap)),
                      d_curve->dataSize() - size, d_curve->dataSize() - 1);
  plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, cacheMode);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, false);
#endif
}

void
IncrementalPlot::appendData(double* x, double* y, int size, QwtSymbol::Style s)
{
  if (d_data == NULL)
    d_data = new CurveData;

  if (d_curve == NULL) {
    d_curve = new QwtPlotCurve("Data");
    d_curve->setStyle(QwtPlotCurve::NoCurve);
    d_curve->setPaintAttribute(QwtPlotCurve::FilterPoints);
    const QColor& c = Qt::white;
    QwtSymbol* symbol = new QwtSymbol(s, QBrush(c), QPen(c), QSize(6, 6));
    d_curve->setSymbol(symbol);
    d_curve->attach(this);
  }

  d_data->append(x, y, size);
  d_curve->setRawSamples(d_data->x(), d_data->y(), d_data->count());
#ifdef __GNUC__
#warning better use QwtData
#endif

  const bool cacheMode =
    qobject_cast<QwtPlotCanvas*>(canvas())->testPaintAttribute(
      QwtPlotCanvas::BackingStore);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
  // works on X11. This has an tremendous effect on the performance..

  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

  QwtPlotCanvas* plotCanvas = qobject_cast<QwtPlotCanvas*>(canvas());
  plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, false);
  QPainter painter;
  QwtScaleMap xMap = plotCanvas->plot()->canvasMap(d_curve->xAxis());
  QwtScaleMap yMap = plotCanvas->plot()->canvasMap(d_curve->yAxis());
  d_curve->drawSeries(&painter, xMap, yMap, (d_curve->paintRect(xMap, yMap)),
                      d_curve->dataSize() - size, d_curve->dataSize() - 1);
  plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, cacheMode);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, false);
#endif
}

void
IncrementalPlot::appendLine(double* x, double* y, int size)
{
  if (l_data == NULL)
    l_data = new CurveData;

  if (l_curve == NULL) {
    l_curve = new QwtPlotCurve("Line");
    l_curve->setStyle(QwtPlotCurve::Lines);
    l_curve->setPaintAttribute(QwtPlotCurve::FilterPoints);
    l_curve->setPen(QColor(Qt::white));
    const QColor& c = Qt::white;
    QwtSymbol* symbol =
      new QwtSymbol(QwtSymbol::NoSymbol, QBrush(c), QPen(c), QSize(6, 6));
    l_curve->setSymbol(symbol);
    l_curve->attach(this);
  }

  l_data->append(x, y, size);
  l_curve->setRawSamples(l_data->x(), l_data->y(), l_data->count());
#ifdef __GNUC__
#warning better use QwtData
#endif

  const bool cacheMode =
    qobject_cast<QwtPlotCanvas*>(canvas())->testPaintAttribute(
      QwtPlotCanvas::BackingStore);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
  // works on X11. This has an tremendous effect on the performance..

  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

  QwtPlotCanvas* plotCanvas = qobject_cast<QwtPlotCanvas*>(canvas());
  plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, false);
  QPainter painter;
  QwtScaleMap xMap = plotCanvas->plot()->canvasMap(l_curve->xAxis());
  QwtScaleMap yMap = plotCanvas->plot()->canvasMap(l_curve->yAxis());
  l_curve->drawSeries(&painter, xMap, yMap, (l_curve->paintRect(xMap, yMap)),
                      l_curve->dataSize() - size, l_curve->dataSize() - 1);
  plotCanvas->setPaintAttribute(QwtPlotCanvas::BackingStore, cacheMode);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, false);
#endif
}

void
IncrementalPlot::removeData()
{
  delete d_curve;
  d_curve = NULL;
  delete l_curve;
  l_curve = NULL;

  delete d_data;
  d_data = NULL;
  delete l_data;
  l_data = NULL;
  replot();
}

void
IncrementalPlot::nextSymbol()
{
  int n = d_curve->symbol()->style() + 1;
  if (n > 14)
    n = 0;

  d_curve->setSymbol(new QwtSymbol((QwtSymbol::Style)n, QBrush(Qt::white),
                                   QPen(Qt::white), QSize(6, 6)));
}
