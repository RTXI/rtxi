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

#include <stdlib.h>
#include <cmath>
#include <algorithm>

#include <qwt_abstract_scale_draw.h>
#include <qwt_scale_map.h>

#include "rt.hpp"
#include "scope.h"

// Scope constructor; inherits from QwtPlot
Oscilloscope::Scope::Scope(QWidget* parent): QwtPlot(parent), 
  d_directPainter(new QwtPlotDirectPainter()),
  grid(new QwtPlotGrid()),
  origin(new QwtPlotMarker()),
  scaleMapY(new QwtScaleMap()),
  scaleMapX(new QwtScaleMap()),
  legendItem(new LegendItem()),
  timer(new QTimer())
{
  // Initialize director
  plotLayout()->setAlignCanvasToScales(true);
  setAutoReplot(false);

  // Set scope canvas
  setCanvas(new Oscilloscope::Canvas());

  // Setup grid
  this->grid->setPen(Qt::gray, 0, Qt::DotLine);
  this->grid->attach(this);

  // Set division limits on the scope
  setAxisMaxMajor(QwtPlot::xBottom, divX);
  setAxisMaxMajor(QwtPlot::yLeft, divY);

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
  this->legendItem->setAlignmentInCanvas(Qt::Alignment(Qt::AlignTop | Qt::AlignRight));
  this->legendItem->setBorderRadius(8);
  this->legendItem->setMargin(4);
  this->legendItem->setSpacing(2);
  this->legendItem->setItemMargin(0);
  this->legendItem->setBackgroundBrush(QBrush(QColor(225, 225, 225)));

  // Update scope background/scales/axes
  replot();

  // Timer controls refresh rate of scope
  this->timer->setTimerType(Qt::PreciseTimer);
  QObject::connect(
      timer, SIGNAL(timeout()), this, SLOT(timeoutEvent()));
  this->timer->start(this->refresh);
  resize(sizeHint());
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

// Timeout event slot
void Oscilloscope::Scope::timeoutEvent()
{
  if (!triggering){
    drawCurves();
  }
}

void Oscilloscope::Scope::insertChannel(const Oscilloscope::scope_channel& channel)
{
  channels.push_back(channel);
}

void Oscilloscope::Scope::removeChannel(IO::Block* block, size_t port, IO::flags_t type)
{
  auto iter = std::find_if(this->channels.begin(),
                           this->channels.end(),
                           [&](const Oscilloscope::scope_channel& channel_info){
                             return block == channel_info.block && 
                                    port == channel_info.port &&
                                    type == channel_info.direction;
                           });
  if(iter == this->channels.end()) {
    return;
  }
  // Make sure we aren't triggering a non-existent oscilloscope channel
  //this->capture_trigger.direction = Oscilloscope::Trigger::NONE;
  iter->curve->detach();
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

//Oscilloscope::scope_channel Oscilloscope::Scope::getChannel(IO::Block* block, size_t port)
//{
//  Oscilloscope::scope_channel result;
//  auto iter = std::find_if(this->channels.begin(),
//                           this->channels.end(),
//                           [&](const Oscilloscope::scope_channel& chan){
//                             return chan.block == block && chan.port == port;
//                           });
//  if(iter != this->channels.end()) { result = *iter; }
//  return result;
//}

void Oscilloscope::Scope::clearData()
{
  size_t data_size = 0;
  for (auto& chan : this->channels)
  {
    data_size = chan.xbuffer.size();
    chan.xbuffer.assign(data_size, 0);
    chan.ybuffer.assign(data_size, 0);
    chan.data_indx = 0;
  }
}

void Oscilloscope::Scope::setData(IO::Block* block, size_t port, std::vector<sample> data)
{
  if (isPaused){
    return;
  }

  auto iter = std::find_if(this->channels.begin(),
                           this->channels.end(),
                           [&](const Oscilloscope::scope_channel& chan){
                             return chan.block == block && chan.port == port;
                           });
  if(iter == this->channels.end()) { return; }

  if (data.size() != iter->xbuffer.size()) {
    iter->xbuffer.assign(data.size(), 0);
    iter->ybuffer.assign(data.size(), 0);
  }
  for(size_t i=iter->data_indx; i<data.size(); i++) {
    if(i >= iter->xbuffer.size()) { i = 0; }
    iter->xbuffer[i] = static_cast<double>(data[i].time);
    iter->ybuffer[i] = data[i].value;
  }
}

//void Oscilloscope::Scope::setTrigger(Oscilloscope::Trigger::Info trigger_info)
//{
//  auto chan_loc = std::find_if(this->channels.begin(),
//                               this->channels.end(),
//                               [&](const Oscilloscope::scope_channel& chann){
//                                 return chann.block == trigger_info.block && 
//                                        chann.port == trigger_info.port;
//                               });
//  if(chan_loc == this->channels.end()) { 
//    ERROR_MSG("Oscilloscope::Scope::setTrigger : The given channel was not found!");
//    return; 
//  }
//
//  this->capture_trigger = trigger_info;
//  // Update if direction has changed
//  if (trigger_info.direction == Oscilloscope::Trigger::NONE) {
//    triggering = false;
//    timer->start(refresh);
//  } else {
//    triggering = true;
//    timer->stop();
//  }
//}

double Oscilloscope::Scope::getDivT() const
{
  return hScl;
}

void Oscilloscope::Scope::setDivT(double divT)
{
  hScl = divT;
  if (divT >= 1000.)
    dtLabel = QString::number(divT * 1e-3) + "s";
  else if (divT >= 1.)
    dtLabel = QString::number(divT) + "ms";
  else if (divT >= 1e-3)
    dtLabel = QString::number(divT * 1e3) + "µs";
  else
    dtLabel = QString::number(divT * 1e6) + "ns";
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

void Oscilloscope::Scope::setChannelScale(probe channel, double scale)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann){
                                 return chann.block == channel.block &&
                                        chann.direction == channel.direction &&
                                        chann.port == channel.port;
                               });
  if(chan_loc == channels.end()) { return; }
  chan_loc->scale = scale;
}

double Oscilloscope::Scope::getChannelScale(probe channel)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann){
                                 return chann.block == channel.block && 
                                        chann.direction == channel.direction &&
                                        chann.port == channel.port;
                               });
  if(chan_loc == channels.end()) { return 0.0; }
  return chan_loc->scale;
}

void Oscilloscope::Scope::setChannelOffset(probe channel, double offset)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann){
                                 return chann.block == channel.block &&
                                        chann.direction == channel.direction &&
                                        chann.port == channel.port;
                               });
  if(chan_loc == channels.end()) { return; }
  chan_loc->offset = offset;
}

double Oscilloscope::Scope::getChannelOffset(probe channel)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann){
                                 return chann.block == channel.block && 
                                        chann.direction == channel.direction &&
                                        chann.port == channel.port;
                               });
  if(chan_loc == channels.end()) { return 0.0; }
  return chan_loc->offset;
}

void Oscilloscope::Scope::setChannelPen(probe channel, const QPen& pen)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann){
                                 return chann.block == channel.block && 
                                        chann.direction == channel.direction &&
                                        chann.port == channel.port;
                               });
  if(chan_loc == channels.end()) { return; }
  chan_loc->curve->setPen(pen);
}

void Oscilloscope::Scope::setChannelLabel(probe channel, const QString& label)
{
  auto chan_loc = std::find_if(this->channels.begin(),
                               this->channels.end(),
                               [&](const Oscilloscope::scope_channel& chann){
                                 return chann.block == channel.block && 
                                        chann.direction == channel.direction &&
                                        chann.port == channel.port;
                               });
  if(chan_loc == channels.end()) { return; }
  chan_loc->curve->setTitle(label);
}

// Draw data on the scope
void Oscilloscope::Scope::drawCurves()
{
  if (isPaused) {
    return;
  }
  double max_val = 0;
  double local_max_val = 0;
  for(auto chan : this->channels){
    if(chan.xbuffer.size() == 0) { continue; }
    local_max_val = *std::max_element(chan.xbuffer.begin(),
                                      chan.xbuffer.end());
    if(local_max_val > max_val) { max_val = local_max_val; }
  }
  // Set X scale map is same for all channels
  double max_window_time = local_max_val;
  double min_window_time = max_window_time - hScl * divX;
  scaleMapX->setScaleInterval(min_window_time , max_window_time);
  for (auto& channel : this->channels)
  {
    // Set data for channel
    //std::vector<double> x(channel.data.size());
    //std::vector<double> y(channel.data.size());
    //double* x_loc = x.data();
    //double* y_loc = y.data();

    // Set Y scale map for channel
    // TODO this should not happen each iteration, instead build into channel
    // struct
    scaleMapY->setScaleInterval(-channel.scale * divY / 2, channel.scale * divY / 2);
    // Scale data to pixel coordinates
    //for (size_t j = 0; j < channel.data.size(); ++j) {
    //  *x_loc = scaleMapX->transform(j * period);
    //  *y_loc = scaleMapY->transform(channel.data[(data_idx + j) % channel.data.size()]
    //                                + channel.offset);
    //  ++x_loc;
    //  ++y_loc;
    //}

    // Append data to curve
    // Makes deep copy - which is not optimal
    // TODO: change to pointer based method
    channel.curve->setSamples(channel.xbuffer.data(), channel.ybuffer.data(), channel.xbuffer.size());
  }

  // Update plot
  replot();
}
