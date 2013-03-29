#include <qwt-qt3/qwt_plot.h>
#include <qwt-qt3/qwt_plot_canvas.h>
#include <qwt-qt3/qwt_plot_curve.h>
#include "incrementalplot.h"
#if QT_VERSION >= 0x040000
#include <qpaintengine.h>
#endif

CurveData::CurveData() :
  d_count(0)
{
}

void
CurveData::append(double *x, double *y, int count)
{
  int newSize = ((d_count + count) / 1000 + 1) * 1000;
  if (newSize > size())
    {
      d_x.resize(newSize);
      d_y.resize(newSize);
    }

  for (register int i = 0; i < count; i++)
    {
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

const double *
CurveData::x() const
{
  return d_x.data();
}

const double *
CurveData::y() const
{
  return d_y.data();
}

IncrementalPlot::IncrementalPlot(QWidget *parent) :
  BasicPlot(parent), d_data(NULL), d_curve(NULL), l_data(NULL), l_curve(NULL)
{
  setAutoReplot(false);
}

IncrementalPlot::~IncrementalPlot()
{
  delete d_data;
}

const double *
IncrementalPlot::xData()
{
  return d_data->x();
}

const double *
IncrementalPlot::yData()
{
  return d_data->y();
}

const uint
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
IncrementalPlot::appendData(double *x, double *y, int size)
{
  if (d_data == NULL)
    d_data = new CurveData;

  if (d_curve == NULL)
    {
      d_curve = new QwtPlotCurve("Data");
      d_curve->setStyle(QwtPlotCurve::NoCurve);
      d_curve->setPaintAttribute(QwtPlotCurve::PaintFiltered);
      const QColor &c = Qt::white;
      d_curve->setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(c), QPen(c),
          QSize(6, 6)));

      d_curve->attach(this);
    }

  d_data->append(x, y, size);
  d_curve->setRawData(d_data->x(), d_data->y(), d_data->count());
#ifdef __GNUC__
#warning better use QwtData
#endif

  const bool cacheMode = canvas()->testPaintAttribute(
      QwtPlotCanvas::PaintCached);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
  // works on X11. This has an tremendous effect on the performance..

  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
  d_curve->draw(d_curve->dataSize() - size, d_curve->dataSize() - 1);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, cacheMode);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, false);
#endif
}

void
IncrementalPlot::appendData(double *x, double *y, int size, QwtSymbol::Style s)
{
  if (d_data == NULL)
    d_data = new CurveData;

  if (d_curve == NULL)
    {
      d_curve = new QwtPlotCurve("Data");
      d_curve->setStyle(QwtPlotCurve::NoCurve);
      d_curve->setPaintAttribute(QwtPlotCurve::PaintFiltered);
      const QColor &c = Qt::white;
      d_curve->setSymbol(QwtSymbol(s, QBrush(c), QPen(c), QSize(6, 6)));
      d_curve->attach(this);
    }

  d_data->append(x, y, size);
  d_curve->setRawData(d_data->x(), d_data->y(), d_data->count());
#ifdef __GNUC__
#warning better use QwtData
#endif

  const bool cacheMode = canvas()->testPaintAttribute(
      QwtPlotCanvas::PaintCached);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
  // works on X11. This has an tremendous effect on the performance..

  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
  d_curve->draw(d_curve->dataSize() - size, d_curve->dataSize() - 1);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, cacheMode);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, false);
#endif
}

void
IncrementalPlot::appendLine(double *x, double *y, int size)
{
  if (l_data == NULL)
    l_data = new CurveData;

  if (l_curve == NULL)
    {
      l_curve = new QwtPlotCurve("Line");
      l_curve->setStyle(QwtPlotCurve::Lines);
      l_curve->setPaintAttribute(QwtPlotCurve::PaintFiltered);
      l_curve->setPen(QColor(Qt::white));
      const QColor &c = Qt::white;
      l_curve->setSymbol(QwtSymbol(QwtSymbol::NoSymbol, QBrush(c), QPen(c),
          QSize(6, 6)));

      l_curve->attach(this);
    }

  l_data->append(x, y, size);
  l_curve->setRawData(l_data->x(), l_data->y(), l_data->count());
#ifdef __GNUC__
#warning better use QwtData
#endif

  const bool cacheMode = canvas()->testPaintAttribute(
      QwtPlotCanvas::PaintCached);

#if QT_VERSION >= 0x040000 && defined(Q_WS_X11)
  // Even if not recommended by TrollTech, Qt::WA_PaintOutsidePaintEvent
  // works on X11. This has an tremendous effect on the performance..

  canvas()->setAttribute(Qt::WA_PaintOutsidePaintEvent, true);
#endif

  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, false);
  l_curve->draw(l_curve->dataSize() - size, l_curve->dataSize() - 1);
  canvas()->setPaintAttribute(QwtPlotCanvas::PaintCached, cacheMode);

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
  int n = d_curve->symbol().style() + 1;
  if (n > 14)
    n = 0;
  d_curve->setSymbol(QwtSymbol((QwtSymbol::Style) n, QBrush(Qt::white), QPen(
      Qt::white), QSize(6, 6)));
}

