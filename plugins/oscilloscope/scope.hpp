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

#include <cstddef>
#include <shared_mutex>
#include <vector>

#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot.h>
#include "fifo.hpp"
#include "io.hpp"

class QwtPlotCurve;
class QwtPlotDirectPainter;
class QwtPlotGrid;
class QwtPlotMarker;

namespace Oscilloscope
{

namespace Trigger
{
enum trig_t : int
{
  NONE = 0,
  POS,
  NEG,
};

typedef struct Info
{
  IO::endpoint endpoint;
  Trigger::trig_t trigger_direction = NONE;
  double threshold = 0.0;
} Info;
}  // namespace Trigger

// values meant to be used with qt timer for redrawing the screen
// values are in milliseconds
namespace FrameRates
{
constexpr size_t HZ60 = 17;
constexpr size_t HZ120 = 8;
constexpr size_t HZ240 = 4;
}  // namespace FrameRates

constexpr size_t DEFAULT_BUFFER_SIZE = 100000;
typedef struct sample
{
  int64_t time;
  double value;
} sample;

typedef struct scope_channel
{
  QString label;
  IO::endpoint endpoint;
  RT::OS::Fifo* fifo = nullptr;
  double scale = 1.0;
  double offset = 0.0;
  std::vector<int64_t> timebuffer;
  std::vector<double> xbuffer;
  std::vector<double> ybuffer;
  std::vector<double> xtransformed;
  std::vector<double> ytransformed;
  size_t data_indx = 0;
  QwtPlotCurve* curve = nullptr;
} scope_channel;

namespace ColorID
{
enum color_id : size_t
{
  Red = 0,
  Orange,
  Green,
  Blue,
  Purple,
  Teal,
  Black
};
}  // namespace ColorID

const std::array<QColor, 7> penColors = {QColor(255, 0, 16, 255),
                                         QColor(255, 164, 5, 255),
                                         QColor(43, 206, 72, 255),
                                         QColor(0, 117, 220, 255),
                                         QColor(178, 102, 255, 255),
                                         QColor(0, 153, 143, 255),
                                         QColor(83, 81, 84, 255)};

constexpr std::array<std::string_view, 7> color2string {
    "Red", "Orange", "Green", "Blue", "Purple", "Teal", "Black"};

constexpr std::array<Qt::PenStyle, 5> penStyles = {Qt::SolidLine,
                                                   Qt::DashLine,
                                                   Qt::DotLine,
                                                   Qt::DashDotLine,
                                                   Qt::DashDotDotLine};

constexpr std::array<std::string_view, 5> penstyles2string {
    "Solid", "Dash", "Dot", "Dash Dot", "Dash Dot Dot"};

namespace PenStyleID
{
enum penstyle_id : size_t
{
  SolidLine = 0,
  DashLine,
  DotLine,
  DashDotLine,
  DashDotDotLine
};
}  // namespace PenStyleID

constexpr std::array<QwtPlotCurve::CurveStyle, 2> curveStyles = {QwtPlotCurve::Lines, QwtPlotCurve::Dots};

constexpr std::array<std::string_view, 2> curvestyles2string {
    "Lines", "Dots"};

namespace CurveStyleID
{
enum curvestyle_id : size_t
{
  Line = 0,
  Dot,
};
}  // namespace CurveStyleID


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
  void setPause(bool value);
  void createChannel(IO::endpoint probeInfo, RT::OS::Fifo* fifo);
  bool channelRegistered(IO::endpoint probeInfo);
  void removeChannel(IO::endpoint probeInfo);
  void removeBlockChannels(IO::Block* block);
  size_t getChannelCount();

  void clearData();
  size_t getDataSize() const;
  void setDataSize(size_t size);

  int64_t getDivT();
  void setDivT(int64_t value);

  size_t getDivX() const;
  size_t getDivY() const;

  size_t getRefresh() const;
  void setRefresh(size_t r);

  void setChannelScale(IO::endpoint endpoint, double scale);
  double getChannelScale(IO::endpoint endpoint);
  void setChannelOffset(IO::endpoint endpoint, double offset);
  double getChannelOffset(IO::endpoint endpoint);
  void setChannelLabel(IO::endpoint endpoint, const QString& label);
  QColor getChannelColor(IO::endpoint endpoint);
  Qt::PenStyle getChannelStyle(IO::endpoint endpoint);
  QwtPlotCurve::CurveStyle getChannelCurveStyle(IO::endpoint endpoint);
  void setChannelPen(IO::endpoint endpoint, const QPen& pen);
  void setChannelCurveStyle(IO::endpoint endpoint, const QwtPlotCurve::CurveStyle& curveStyle);
  int getChannelWidth(IO::endpoint endpoint);
  double getTriggerThreshold() const;
  void setTriggerThreshold(double threshold);
  Trigger::trig_t getTriggerDirection();
  void setTriggerDirection(Trigger::trig_t direction);

  void drawCurves();
  IO::endpoint getTriggerEndpoint() const
  {
    return this->m_trigger_info.endpoint;
  }
public slots:
  void process_data();

protected:
  void resizeEvent(QResizeEvent* event) override;

private:
  Oscilloscope::Trigger::Info m_trigger_info;
  std::atomic<size_t> buffer_size = DEFAULT_BUFFER_SIZE;

  std::atomic<bool> isPaused = false;
  int64_t divX = 10;
  int64_t divY = 10;
  size_t refresh = Oscilloscope::FrameRates::HZ60;
  int64_t horizontal_scale_ns = 1000000;  // horizontal scale for time (ns)
  bool triggering = false;
  std::vector<sample> sample_buffer;

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
  std::vector<scope_channel> channels;

  // synchronization
  std::shared_mutex m_channel_mutex;
};  // Scope

}  // namespace Oscilloscope

Q_DECLARE_METATYPE(QwtPlotCurve::CurveStyle)
#endif  // SCOPE_H
