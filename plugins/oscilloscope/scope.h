/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

         This program is free software: you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation, either version 3 of the License, or
         (at your option) any later version.

         This program is distributed in the hope that it will be useful,
         but WITHOUT ANY WARRANTY; without even the implied warranty of
         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
         GNU General Public License for more details.

         You should have received a copy of the GNU General Public License
         along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

/*
         This class instantiates a single scope instance.
         This includes the painting director, the canvass,
         and the functions necessary for modifying those
         scopes. Multiple scope objects can be instantiated,
         and each instance will have it's own set of settings
         that can be edited with the Panel class.
         */

#ifndef SCOPE_H
#define SCOPE_H

#include <QtWidgets>
#include <vector>

#include <qwt.h>
#include <qwt_curve_fitter.h>
#include <qwt_interval.h>
#include <qwt_painter.h>
#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_engine.h>
#include <qwt_system_clock.h>

#include "io.hpp"

namespace Oscilloscope {

// values meant to be used with qt timer for redrawing the screen
// values are in milliseconds 
namespace FrameRates {
constexpr int HZ60 = 17;
constexpr int HZ120 = 8;
constexpr int HZ240 = 4;
}; // namespace FrameRates 

namespace Trigger{
enum trig_t
{
  NONE,
  POS,
  NEG,
};
}; // namespace Trigger

typedef struct sample {
  double value;
  int64_t time;
} sample;

struct scope_channel
{
  QString label;
  double scale;
  double offset;
  std::vector<int64_t> xbuffer;
  std::vector<double> ybuffer;
  size_t end_data_indx;
  QwtPlotCurve* curve;
  IO::Block* block;
  int port;
  IO::channel_t info;
};

class LegendItem : public QwtPlotLegendItem
{
public:
  LegendItem()
  {
    setRenderHint(QwtPlotItem::RenderAntialiased);
    QColor color(Qt::black);
    setTextPen(color);
  }
};  // LegendItem

class Canvas : public QwtPlotCanvas
{
public:
  Canvas(QwtPlot* plot = nullptr) : QwtPlotCanvas(plot)
  {
    setPaintAttribute(QwtPlotCanvas::BackingStore, false);
    if (QwtPainter::isX11GraphicsSystem()) {
      if (testPaintAttribute(QwtPlotCanvas::BackingStore)) {
        setAttribute(Qt::WA_PaintOnScreen, true);
        setAttribute(Qt::WA_NoSystemBackground, true);
      }
    }
    setupPalette();
  }

private:
  void setupPalette()
  {
    QPalette pal = palette();
    QLinearGradient gradient;
    gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
    gradient.setColorAt(1.0, QColor(Qt::white));
    pal.setBrush(QPalette::Window, QBrush(gradient));
    pal.setColor(QPalette::WindowText, Qt::green);
    setPalette(pal);
  }
};  // Canvas


class Scope : public QwtPlot
{
public:
  explicit Scope(QWidget* = nullptr);
  ~Scope();

  bool paused() const;
  void insertChannel(const scope_channel& channel);
  void removeChannel(IO::Block* block, int port);
  size_t getChannelCount() const;

  void clearData();
  void setData(IO::Block* block, int port, std::vector<sample> data);
  size_t getDataSize() const;
  void setDataSize(size_t);

  Trigger::trig_t getTriggerDirection();
  double getTriggerThreshold();
  double getTriggerWindow();
  //std::list<scope_channel>::iterator getTriggerChannel();
  void setTrigger(Trigger::trig_t direction, double threshold, IO::Block* block, int port);

  double getDivT() const;
  void setDivT(double);

  void setPeriod(double);
  size_t getDivX() const;
  size_t getDivY() const;

  size_t getRefresh() const;
  void setRefresh(size_t);


  void setChannelScale(IO::Block* block, int port, double chan_scale);
  void setChannelOffset(IO::Block* block, int port, double chan_offset);
  void setChannelPen(IO::Block* block, int port, const QPen& pen);
  void setChannelLabel(IO::Block* block, int port, const QString& pen);

protected:
  void resizeEvent(QResizeEvent* event);

private slots:
  void timeoutEvent();

private:
  void drawCurves();

  bool isPaused = false;
  int divX=10;
  int divY=10;
  int data_idx=0;
  size_t data_size=100;
  int refresh=250;
  double hScl=1.0;  // horizontal scale for time (ms)
  int64_t period;  // real-time period of system (ms)
  bool triggering=false;
  Trigger::trig_t triggerDirection=Oscilloscope::Trigger::NONE;
  double triggerThreshold=0.0;
  double triggerWindow=10.0;
  std::list<size_t> triggerQueue;
  scope_channel* triggerChannel=nullptr;

  // Scope primary paint element
  QwtPlotDirectPainter* d_directPainter=nullptr;

  // Scope painter elements
  QwtPlotGrid* grid;
  QwtPlotMarker* origin;

  // Scaling engine
  QwtScaleMap* scaleMapY;
  QwtScaleMap* scaleMapX;

  // Legend
  LegendItem* legendItem;

  QTimer* timer;
  QString dtLabel;
  std::vector<scope_channel> channels;
};  // Scope

}; // namespace Oscilloscope

#endif  // SCOPE_H
