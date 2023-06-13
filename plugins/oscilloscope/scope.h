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

#include <qnamespace.h>
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

namespace Oscilloscope
{

// values meant to be used with qt timer for redrawing the screen
// values are in milliseconds
namespace FrameRates
{
constexpr size_t HZ60 = 17;
constexpr size_t HZ120 = 8;
constexpr size_t HZ240 = 4;
};  // namespace FrameRates

constexpr size_t DEFAULT_BUFFER_SIZE = 10000;
typedef struct sample
{
  double value;
  int64_t time;
} sample;

typedef struct scope_channel
{
  QString label;
  double scale = 1;
  double offset = 0;
  std::vector<double> xbuffer;
  std::vector<double> ybuffer;
  size_t data_indx = 0;
  QwtPlotCurve* curve = nullptr;
  QPen* pen = nullptr;
  IO::Block* block = nullptr;
  size_t port = 0;
  IO::flags_t direction;
} scope_channel;

typedef struct probe
{
  IO::Block* block = nullptr;
  size_t port = 0;
  IO::flags_t direction = IO::UNKNOWN;
} probe;

constexpr std::array<QColor, 7> penColors = {QColor(255, 0, 16, 255),
                                             QColor(255, 164, 5, 255),
                                             QColor(43, 206, 72, 255),
                                             QColor(0, 117, 220, 255),
                                             QColor(178, 102, 255, 255),
                                             QColor(0, 153, 143, 255),
                                             QColor(83, 81, 84, 255)};

constexpr std::array<Qt::PenStyle, 5> penStyles = {Qt::SolidLine,
                                                   Qt::DashLine,
                                                   Qt::DotLine,
                                                   Qt::DashDotLine,
                                                   Qt::DashDotDotLine};

class LegendItem : public QwtPlotLegendItem
{
public:
  LegendItem();
};  // LegendItem

class Canvas : public QwtPlotCanvas
{
public:
  explicit Canvas(QwtPlot* plot);

private:
  void setupPalette();
};  // Canvas

class Scope : public QwtPlot
{
  Q_OBJECT

public:
  Scope(const Scope&) = delete;
  Scope(Scope&&) = delete;
  Scope& operator=(const Scope&) = delete;
  Scope& operator=(Scope&&) = delete;
  explicit Scope(QWidget* parent);
  ~Scope() override;

  bool paused() const;
  void createChannel(Oscilloscope::probe probeInfo);
  void removeChannel(Oscilloscope::probe probeInfo);
  size_t getChannelCount() const;
  // scope_channel getChannel(IO::Block* block, size_t port);

  void clearData();
  void setData(probe channel, std::vector<sample> probe_data);
  size_t getDataSize() const;
  void setDataSize(size_t);

  // Trigger::trig_t getTriggerDirection();
  // double getTriggerThreshold();

  double getDivT() const;
  void setDivT(double);

  void setPeriod(double);
  size_t getDivX() const;
  size_t getDivY() const;

  size_t getRefresh() const;
  void setRefresh(size_t);

  void setChannelScale(probe channel, double scale);
  double getChannelScale(probe channel);
  void setChannelOffset(probe channel, double offset);
  double getChannelOffset(probe channel);
  void setChannelPen(probe channel, QPen* pen);
  QPen* getChannelPen(probe channel);
  void setChannelLabel(probe channel, const QString& label);
  // Trigger::Info capture_trigger;

  void drawCurves();

protected:
  void resizeEvent(QResizeEvent* event) override;

private:
  size_t buffer_size = DEFAULT_BUFFER_SIZE;

  bool isPaused = false;
  size_t divX = 10;
  size_t divY = 10;
  size_t refresh = Oscilloscope::FrameRates::HZ60;
  double hScl = 1.0;  // horizontal scale for time (ms)
  bool triggering = false;

  // Scope primary paint element
  QwtPlotDirectPainter* d_directPainter = nullptr;

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
  std::list<scope_channel> channels;
};  // Scope

};  // namespace Oscilloscope

#endif  // SCOPE_H
