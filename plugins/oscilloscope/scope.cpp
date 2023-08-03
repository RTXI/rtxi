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

#include <algorithm>
#include <cmath>

#include "scope.h"

#include <qwt_abstract_scale_draw.h>
#include <qwt_plot_legenditem.h>
#include <qwt_scale_map.h>
#include <stdlib.h>

#include "rt.hpp"

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
    , timer(new QTimer())
{
  // Initialize director
  plotLayout()->setAlignCanvasToScales(true);
  setAutoReplot(false);

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
  this->legendItem->setAlignment(
      static_cast<Qt::Alignment>(Qt::AlignTop | Qt::AlignRight));
  this->legendItem->setBorderRadius(8);
  this->legendItem->setMargin(4);
  this->legendItem->setSpacing(2);
  this->legendItem->setItemMargin(0);
  this->legendItem->setBackgroundBrush(QBrush(QColor(225, 225, 225)));

  // Update scope background/scales/axes
  replot();

  resize(sizeHint());
  // Timer controls refresh rate of scope
  this->timer->setTimerType(Qt::PreciseTimer);
  QObject::connect(timer, SIGNAL(timeout()), this, SLOT(process_data()));
  this->timer->start(static_cast<int>(this->refresh));
}

// Kill me
Oscilloscope::Scope::~Scope()
{
  delete d_directPainter;
}

// Returns pause status of scope
bool Oscilloscope::Scope::paused() const
{
  return isPaused;
}

// TODO: make this thread-safe
void Oscilloscope::Scope::createChannel(IO::endpoint probeInfo,
                                        RT::OS::Fifo* fifo)
{
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
  chan.scale = 1;
  chan.offset = 0;
  chan.data_indx = 0;
  chan.pen = new QPen();
  chan.pen->setColor(Oscilloscope::penColors[0]);
  chan.pen->setStyle(Oscilloscope::penStyles[0]);
  chan.fifo = fifo;
  this->channels.push_back(chan);
}

// TODO: make this thread-safe
void Oscilloscope::Scope::removeChannel(IO::endpoint probeInfo)
{
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

void Oscilloscope::Scope::resizeEvent(QResizeEvent* event)
{
  this->d_directPainter->reset();
  QwtPlot::resizeEvent(event);
}

size_t Oscilloscope::Scope::getChannelCount() const
{
  return channels.size();
}

// TODO: make this thread-safe
void Oscilloscope::Scope::clearData()
{
  for (auto& chan : this->channels) {
    chan.xbuffer.assign(this->buffer_size, 0);
    chan.ybuffer.assign(this->buffer_size, 0);
    chan.data_indx = 0;
  }
}

// TODO: make this thread-safe
void Oscilloscope::Scope::setDataSize(size_t size)
{
  this->buffer_size = size;
}

// TODO: make this thread-safe
size_t Oscilloscope::Scope::getDataSize() const
{
  return this->buffer_size;
}

double Oscilloscope::Scope::getDivT() const
{
  return horizontal_scale_ms;
}

void Oscilloscope::Scope::setDivT(double divT)
{
  horizontal_scale_ms = divT;
  if (divT >= 1000.) {
    dtLabel = QString::number(divT * 1e-3) + "s";
  } else if (divT >= 1.) {
    dtLabel = QString::number(divT) + "ms";
  } else if (divT >= 1e-3) {
    dtLabel = QString::number(divT * 1e3) + "µs";
  } else {
    dtLabel = QString::number(divT * 1e6) + "ns";
  }
}

size_t Oscilloscope::Scope::getDivX() const
{
  return divX;
}

size_t Oscilloscope::Scope::getDivY() const
{
  return divY;
}

size_t Oscilloscope::Scope::getRefresh() const
{
  return refresh;
}

void Oscilloscope::Scope::setRefresh(size_t r)
{
  refresh = r;
  timer->setInterval(static_cast<int>(refresh));
}

void Oscilloscope::Scope::setChannelScale(IO::endpoint endpoint, double scale)
{
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
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return 0.0;
  }
  return chan_loc->scale;
}

void Oscilloscope::Scope::setChannelOffset(IO::endpoint endpoint, double offset)
{
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
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return 0.0;
  }
  return chan_loc->offset;
}

void Oscilloscope::Scope::setChannelPen(IO::endpoint endpoint, QPen* pen)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return;
  }
  chan_loc->curve->setPen(*pen);
  chan_loc->pen = pen;
}

QPen* Oscilloscope::Scope::getChannelPen(IO::endpoint endpoint)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return nullptr;
  }
  return chan_loc->pen;
}

void Oscilloscope::Scope::setChannelLabel(IO::endpoint endpoint,
                                          const QString& label)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann)
                               { return chann.endpoint == endpoint; });
  if (chan_loc == channels.end()) {
    return;
  }
  chan_loc->curve->setTitle(label);
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
  if (isPaused) {
    return;
  }
  int64_t max_time = 0;
  int64_t local_max_time = 0;
  int64_t min_time = std::numeric_limits<int64_t>::max();
  int64_t local_min_time = min_time;
  for (auto chan : this->channels) {
    if (chan.xbuffer.empty()) {
      continue;
    }
    local_max_time = *std::max_element(chan.timebuffer.begin(), chan.timebuffer.end());
    local_min_time = *std::min_element(chan.timebuffer.begin(), chan.timebuffer.end());
    if (local_max_time > max_time) {
      max_time = local_max_time;
    }
    if (local_min_time < min_time) {
      min_time = local_min_time;
    }
  }
  // Set X scale map is same for all channels
  const auto max_window_time = static_cast<double>(max_time - min_time);
  const double min_window_time = 
      max_window_time - horizontal_scale_ms * static_cast<double>(divX);
  scaleMapX->setScaleInterval(min_window_time, max_window_time);
  for (auto& channel : this->channels) {
    for(size_t i=0; i < channel.xbuffer.size(); i++){
      channel.xbuffer[i] = static_cast<double>(channel.timebuffer[i] - min_time);
    }
    // TODO this should not happen each iteration, instead build into channel
    scaleMapY->setScaleInterval(-channel.scale * static_cast<double>(divY) / 2,
                                channel.scale * static_cast<double>(divY) / 2);
    // Append data to curve
    // Makes deep copy - which is not optimal
    // TODO: change to pointer based method
    channel.curve->setSamples(channel.xbuffer.data(),
                              channel.ybuffer.data(),
                              static_cast<int>(channel.xbuffer.size()));
  }
  // Update plot
  replot();
}

// TODO: look into SIMD for optimize move and calculation of data
void Oscilloscope::Scope::process_data()
{
  std::vector<Oscilloscope::sample> sample_buffer(this->buffer_size);
  ssize_t bytes = 0;
  size_t sample_count = 0;
  size_t array_indx = 0;
  const size_t sample_capacity_bytes =
      sample_buffer.size() * sizeof(Oscilloscope::sample);
  for (auto& channel : this->channels) {
    // Read as many samples as possible in chunks of buffer size or less.
    // overwrite old samples from previous write if available
    while (bytes = channel.fifo->read(sample_buffer.data(), sample_capacity_bytes),
           bytes > 0)
    {
      sample_count = static_cast<size_t>(bytes)/sizeof(Oscilloscope::sample);
      for (size_t i = 0; i < sample_count; i++) {
        array_indx = (i + channel.data_indx) % this->buffer_size;
        channel.timebuffer[array_indx] = sample_buffer[i].time;
        channel.ybuffer[array_indx] = sample_buffer[i].value;
      }
      channel.data_indx =
          (channel.data_indx + sample_count) % this->buffer_size;
    };

    // zero out so the buffer so it doesn't spill over to the next channel
    sample_buffer.assign(this->buffer_size, {0, 0.0});
  }
  this->drawCurves();
}
