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

#include <QTimer>
#include <algorithm>
#include <mutex>

#include "scope.hpp"

#include <qwt_abstract_scale_draw.h>
#include <qwt_global.h>
#include <qwt_painter.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_directpainter.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_legenditem.h>
#include <qwt_plot_marker.h>
#include <qwt_scale_map.h>

Oscilloscope::LegendItem::LegendItem()
{
  setRenderHint(QwtPlotItem::RenderAntialiased);
  const QColor color(Qt::black);
  setTextPen(color);
}

Oscilloscope::Canvas::Canvas(QwtPlot* plot)
    : QwtPlotCanvas(plot)
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

void Oscilloscope::Canvas::setupPalette()
{
  QPalette pal = palette();
  QLinearGradient gradient;
  gradient.setCoordinateMode(QGradient::StretchToDeviceMode);
  gradient.setColorAt(1.0, QColor(Qt::white));
  pal.setBrush(QPalette::Window, QBrush(gradient));
  pal.setColor(QPalette::WindowText, Qt::green);
  setPalette(pal);
}

// Scope constructor; inherits from QwtPlot
Oscilloscope::Scope::Scope(QWidget* parent)
    : QwtPlot(parent)
    , d_directPainter(new QwtPlotDirectPainter())
    , grid(new QwtPlotGrid())
    , origin(new QwtPlotMarker())
    , scaleMapY(new QwtScaleMap())
    , scaleMapX(new QwtScaleMap())
    , legendItem(new LegendItem())
    , timer(new QTimer(this))
{
  // Initialize director
  plotLayout()->setAlignCanvasToScales(true);
  setAutoReplot(false);
  setAutoDelete(true);

  // Set scope canvas
  setCanvas(new Oscilloscope::Canvas(nullptr));

  // Setup grid
  this->grid->setPen(Qt::gray, 0, Qt::DotLine);
  this->grid->attach(this);

  // Set division limits on the scope
  setAxisMaxMajor(QwtPlot::xBottom, static_cast<int>(divX));
  setAxisMaxMajor(QwtPlot::yLeft, static_cast<int>(divY));

  // Disable axes
  enableAxis(QwtPlot::yLeft, false);
  enableAxis(QwtPlot::xBottom, false);

  // Statically set y interval
  setAxisScale(QwtPlot::yLeft, -1.0, 1.0);
  setAxisScale(QwtPlot::xBottom, 0.0, 1000.0);

  // Disable autoscaling
  setAxisAutoScale(QwtPlot::yLeft, false);
  setAxisAutoScale(QwtPlot::xBottom, false);

  // Set origin markers
  this->origin->setLineStyle(QwtPlotMarker::Cross);
  this->origin->setValue(500.0, 0.0);
  this->origin->setLinePen(Qt::gray, 2.0, Qt::DashLine);
  this->origin->attach(this);

  // Setup scaling map
  this->scaleMapY->setPaintInterval(-1.0, 1.0);
  this->scaleMapX->setPaintInterval(0.0, 1000.0);

  // Create and attach legend
  this->legendItem->attach(this);
  this->legendItem->setMaxColumns(1);

// Here we have a problem. Plot legends in QWT are
// aligned using setAlignment in pre 6.2, but the
// function does not exist >= 6.2 and is instead
// replaced by setAlignmentInCanvas. This needs a bit
// of preprocessor to fix.
#if QWT_VERSION >= 0X060200
  this->legendItem->setAlignmentInCanvas(
      static_cast<Qt::Alignment>(Qt::AlignTop | Qt::AlignRight));
#else
  this->legendItem->setAlignment(
      static_cast<Qt::Alignment>(Qt::AlignTop | Qt::AlignRight));
#endif

  this->legendItem->setBorderRadius(8);
  this->legendItem->setMargin(4);
  this->legendItem->setSpacing(2);
  this->legendItem->setItemMargin(0);
  this->legendItem->setBackgroundBrush(QBrush(QColor(225, 225, 225)));
  this->legendItem->attach(this);

  // Update scope background/scales/axes
  replot();

  resize(sizeHint());
  // Timer controls refresh rate of scope
  this->timer->setTimerType(Qt::PreciseTimer);
  QObject::connect(
      timer, &QTimer::timeout, this, &Oscilloscope::Scope::process_data);
  this->timer->start(static_cast<int>(this->refresh));
}

Oscilloscope::Scope::~Scope()
{
  delete d_directPainter;
  delete scaleMapX;
  delete scaleMapY;
}

// Returns pause status of scope
bool Oscilloscope::Scope::paused() const
{
  return isPaused;
}

void Oscilloscope::Scope::setPause(bool value)
{
  this->isPaused.store(value);
}

void Oscilloscope::Scope::createChannel(IO::endpoint probeInfo,
                                        RT::OS::Fifo* fifo)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto iter = std::find_if(this->channels.begin(),
                           this->channels.end(),
                           [&](const scope_channel& chan)
                           { return chan.endpoint == probeInfo; });
  if (iter != this->channels.end()) {
    return;
  }
  Oscilloscope::scope_channel chan;
  chan.curve = new QwtPlotCurve;
  chan.endpoint = probeInfo;
  chan.timebuffer.assign(this->buffer_size, 0);
  chan.xbuffer.assign(this->buffer_size, 0.0);
  chan.ybuffer.assign(this->buffer_size, 0.0);
  chan.xtransformed.assign(this->buffer_size, 0.0);
  chan.ytransformed.assign(this->buffer_size, 0.0);
  chan.scale = 1;
  chan.offset = 0;
  chan.data_indx = 0;
  auto pen = QPen();
  pen.setColor(Oscilloscope::penColors[0]);
  pen.setStyle(Oscilloscope::penStyles[0]);
  chan.fifo = fifo;
  chan.curve->setPen(pen);
  chan.curve->setStyle(Oscilloscope::curveStyles[0]);
  chan.curve->attach(this);
  this->channels.push_back(chan);
}

bool Oscilloscope::Scope::channelRegistered(IO::endpoint probeInfo)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto iter = std::find_if(this->channels.begin(),
                           this->channels.end(),
                           [&](const scope_channel& chan)
                           { return chan.endpoint == probeInfo; });
  return iter != this->channels.end();
}

void Oscilloscope::Scope::removeChannel(IO::endpoint probeInfo)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto iter = std::find_if(this->channels.begin(),
                           this->channels.end(),
                           [&](const scope_channel& chan)
                           { return chan.endpoint == probeInfo; });
  if (iter == this->channels.end()) {
    return;
  }
  iter->curve->detach();
  delete iter->curve;
  iter->curve = nullptr;
  channels.erase(iter);
  replot();
}

void Oscilloscope::Scope::removeBlockChannels(IO::Block* block)
{
  std::vector<IO::endpoint> all_block_endpoints;
  std::shared_lock<std::shared_mutex> read_lock(this->m_channel_mutex);
  for (auto& channel : channels) {
    if (channel.endpoint.block == block) {
      all_block_endpoints.push_back(channel.endpoint);
    }
  }
  read_lock.unlock();
  for (const auto& endpoint : all_block_endpoints) {
    this->removeChannel(endpoint);
  }
}

void Oscilloscope::Scope::resizeEvent(QResizeEvent* event)
{
  this->d_directPainter->reset();
  QwtPlot::resizeEvent(event);
}

size_t Oscilloscope::Scope::getChannelCount()
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  return channels.size();
}

void Oscilloscope::Scope::clearData()
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  for (auto& chan : this->channels) {
    chan.timebuffer.assign(this->buffer_size, 0);
    chan.xbuffer.assign(this->buffer_size, 0);
    chan.ybuffer.assign(this->buffer_size, 0);
    chan.xtransformed.assign(this->buffer_size, 0);
    chan.ytransformed.assign(this->buffer_size, 0);
    chan.data_indx = 0;
  }
}

void Oscilloscope::Scope::setDataSize(size_t size)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  this->buffer_size.store(size);
  this->sample_buffer.assign(buffer_size, {});
  for (auto& chan : this->channels) {
    chan.timebuffer.assign(this->buffer_size, 0);
    chan.xbuffer.assign(this->buffer_size, 0);
    chan.ybuffer.assign(this->buffer_size, 0);
    chan.xtransformed.assign(this->buffer_size, 0);
    chan.ytransformed.assign(this->buffer_size, 0);
    chan.data_indx = 0;
  }
}

size_t Oscilloscope::Scope::getDataSize() const
{
  return this->buffer_size.load();
}

int64_t Oscilloscope::Scope::getDivT()
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  return horizontal_scale_ns;
}

void Oscilloscope::Scope::setDivT(int64_t value)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  horizontal_scale_ns = value;
  if (value >= 1000000000) {
    dtLabel = "s";
  } else if (value >= 1000000) {
    dtLabel = "ms";
  } else if (value >= 1000) {
    dtLabel = "µs";
  } else {
    dtLabel = "ns";
  }
}

size_t Oscilloscope::Scope::getDivX() const
{
  return static_cast<size_t>(divX);
}

size_t Oscilloscope::Scope::getDivY() const
{
  return static_cast<size_t>(divY);
}

size_t Oscilloscope::Scope::getRefresh() const
{
  return refresh;
}

void Oscilloscope::Scope::setRefresh(size_t r)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  refresh = r;
  timer->setInterval(static_cast<int>(refresh));
}

void Oscilloscope::Scope::setChannelScale(IO::endpoint endpoint, double scale)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return;
  }
  chan_loc->scale = scale;
}

double Oscilloscope::Scope::getChannelScale(IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return 1.0;
  }
  return chan_loc->scale;
}

void Oscilloscope::Scope::setChannelOffset(IO::endpoint endpoint, double offset)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return;
  }
  chan_loc->offset = offset;
}

double Oscilloscope::Scope::getChannelOffset(IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return 0.0;
  }
  return chan_loc->offset;
}

void Oscilloscope::Scope::setChannelLabel(IO::endpoint endpoint,
                                          const QString& label)
{
  const std::unique_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return;
  }
  chan_loc->curve->setTitle(label);
}

QColor Oscilloscope::Scope::getChannelColor(IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return penColors[Oscilloscope::ColorID::Black];
  }
  return chan_loc->curve->pen().color();
}

Qt::PenStyle Oscilloscope::Scope::getChannelStyle(IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return penStyles[Oscilloscope::PenStyleID::SolidLine];
  }
  return chan_loc->curve->pen().style();
}

QwtPlotCurve::CurveStyle Oscilloscope::Scope::getChannelCurveStyle(IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return curveStyles[Oscilloscope::CurveStyleID::Line];
  }
  return chan_loc->curve->style();
}

int Oscilloscope::Scope::getChannelWidth(IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return 1;
  }
  return chan_loc->curve->pen().width();
}

void Oscilloscope::Scope::setChannelPen(IO::endpoint endpoint, const QPen& pen)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc != channels.end()) {
    chan_loc->curve->setPen(pen);
  }
}

void Oscilloscope::Scope::setChannelCurveStyle(IO::endpoint endpoint, const QwtPlotCurve::CurveStyle& curveStyle)
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc != channels.end()) {
    chan_loc->curve->setStyle(curveStyle);
  }
}

void Oscilloscope::Scope::setTriggerThreshold(double threshold)
{
  this->m_trigger_info.threshold = threshold;
}

double Oscilloscope::Scope::getTriggerThreshold() const
{
  return this->m_trigger_info.threshold;
}

// Draw data on the scope
void Oscilloscope::Scope::drawCurves()
{
  if (isPaused.load() || this->channels.empty()) {
    return;
  }
  int64_t max_time = 0;
  int64_t local_max_time = 0;
  for (const auto& chan : this->channels) {
    local_max_time =
        *std::max_element(chan.timebuffer.begin(), chan.timebuffer.end());
    if (local_max_time > max_time) {
      max_time = local_max_time;
    }
  }
  const int64_t min_time = (max_time - horizontal_scale_ns * divX);
  // Set X scale map is same for all channels
  const auto max_window_time = static_cast<double>(max_time - min_time);
  const double min_window_time = 0.0;
  scaleMapX->setScaleInterval(min_window_time, max_window_time);
  size_t ringbuffer_index = 0;
  for (auto& channel : this->channels) {
    channel.xtransformed.assign(this->buffer_size, 0);
    channel.ytransformed.assign(this->buffer_size, 0);
    scaleMapY->setScaleInterval(-channel.scale * static_cast<double>(divY) / 2,
                                channel.scale * static_cast<double>(divY) / 2);
    for (size_t i = 0; i < channel.xbuffer.size(); i++) {
      ringbuffer_index = (i + channel.data_indx) % this->buffer_size;
      channel.xbuffer[i] =
          static_cast<double>(channel.timebuffer[i] - min_time);
      // Thanks to qwt's interface we need the x axis points to be sorted
      // and scaled. Shenanigans alert
      channel.xtransformed[i] =
          scaleMapX->transform(channel.xbuffer[ringbuffer_index]);
      channel.ytransformed[i] =
          scaleMapY->transform(channel.ybuffer[ringbuffer_index]);
    }
    // TODO this should not happen each iteration, instead build into channel
    // Append data to curve
    // Makes deep copy - which is not optimal
    // TODO: change to pointer based method
    channel.curve->setSamples(channel.xtransformed.data(),
                              channel.ytransformed.data(),
                              static_cast<int>(channel.ytransformed.size()));
  }
  // Update plot
  replot();
}

// TODO: look into SIMD for optimize move and calculation of data
void Oscilloscope::Scope::process_data()
{
  const std::shared_lock<std::shared_mutex> lock(this->m_channel_mutex);
  int64_t bytes = 0;
  size_t sample_count = 0;
  size_t array_indx = 0;
  const size_t sample_capacity_bytes =
      sample_buffer.size() * sizeof(Oscilloscope::sample);
  for (auto& channel : this->channels) {
    // Read as many samples as possible in chunks of buffer size or less.
    // overwrite old samples from previous write if available
    while (
        bytes = channel.fifo->read(sample_buffer.data(), sample_capacity_bytes),
        bytes > 0)
    {
      sample_count = static_cast<size_t>(bytes) / sizeof(Oscilloscope::sample);
      for (size_t i = 0; i < sample_count; i++) {
        array_indx = (i + channel.data_indx) % this->buffer_size;
        channel.timebuffer[array_indx] = sample_buffer[i].time;
        channel.ybuffer[array_indx] = sample_buffer[i].value + channel.offset;
      }
      channel.data_indx =
          (channel.data_indx + sample_count) % this->buffer_size;
    };

    // zero out so the buffer so it doesn't spill over to the next channel
    sample_buffer.assign(this->buffer_size, {0, 0.0});
  }
  this->drawCurves();
}
