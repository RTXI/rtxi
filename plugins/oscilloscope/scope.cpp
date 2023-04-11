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
  if (!triggering)
    drawCurves();
}

// Insert user specified channel into active list of channels with specified
// settings
void Oscilloscope::Scope::insertChannel(Oscilloscope::scope_channel channel)
{
  //struct scope_channel channel;
  //channel.label = label;
  //channel.scale = scale;
  //channel.offset = offset;
  //channel.info = info;
  //channel.data.resize(data_size, 0.0);
  //channel.curve = curve;
  //channel.curve->setPen(pen);
  //channel.curve->setStyle(QwtPlotCurve::Lines);
  //channel.curve->setRenderHint(QwtPlotItem::RenderAntialiased, false);
  //channel.curve->attach(this);
  channels.push_back(channel);
  //return --(this->channels.end());
}

// Remove user specified channel from active channels list
void Oscilloscope::Scope::removeChannel(IO::Block* block, int port)
{
  auto iter = std::find_if(this->channels.begin(),
                           this->channels.end(),
                           [&](Oscilloscope::scope_channel channel_info){
                             return block == channel_info.block && 
                                    port == channel_info.port;
                           });
  if(iter == this->channels.end()) {
    return;
  }
  iter->curve->detach();
  channels.erase(iter);
  replot();
}

// Resize event for scope
void Oscilloscope::Scope::resizeEvent(QResizeEvent* event)
{
  this->d_directPainter->reset();
  QwtPlot::resizeEvent(event);
}

// Returns count of number of active channels
size_t Oscilloscope::Scope::getChannelCount() const
{
  return channels.size();
}

void Oscilloscope::Scope::clearData()
{
  for (auto& chan : this->channels)
  {
    chan.data.assign(data_size, 0);
  }
}

// Scales data based upon desired settings for the channel
void Oscilloscope::Scope::setData(double data[], size_t size)
{
  if (isPaused)
    return;
  double fs = 1/(this->period * 1e-3);
  if (size < getChannelCount()) {
    ERROR_MSG("Scope::setData() : data size mismatch detected\n");
    return;
  }

  size_t index = 0;
  for (std::list<Oscilloscope::scope_channel>::iterator i = channels.begin(), end = channels.end();
       i != end;
       ++i)
  {
    i->data[data_idx] = data[index++];

    if (triggering && i == triggerChannel
        && ((triggerDirection == Oscilloscope::Trigger::POS && i->data[data_idx - 1] < triggerThreshold
             && i->data[data_idx] > triggerThreshold)
            || (triggerDirection == Oscilloscope::Trigger::NEG
                && i->data[data_idx - 1] > triggerThreshold
                && i->data[data_idx] < triggerThreshold)))
    {
      if (data_idx > triggerWindow * fs)
        triggerQueue.push_back(data_idx - (triggerWindow * fs));
    }
  }

  ++data_idx %= data_size;

  if (triggering && !triggerQueue.empty()
      && (data_idx + 2) % data_size == triggerQueue.front())
  {
    triggerQueue.pop_front();
    drawCurves();
  }
}

// Returns the data size
size_t Oscilloscope::Scope::getDataSize() const
{
  return data_size;
}

void Oscilloscope::Scope::setDataSize(size_t size)
{
  for (std::list<Oscilloscope::scope_channel>::iterator i = channels.begin(), end = channels.end();
       i != end;
       ++i)
    i->data.resize(size, 0.0);
  data_idx = 0;
  data_size = size;
  triggerQueue.clear();
}

Oscilloscope::Trigger::trig_t Oscilloscope::Scope::getTriggerDirection()
{
  return triggerDirection;
}

double Oscilloscope::Scope::getTriggerThreshold()
{
  return triggerThreshold;
}

double Oscilloscope::Scope::getTriggerWindow()
{
  return triggerWindow;
}

std::list<Oscilloscope::scope_channel>::iterator Oscilloscope::Scope::getTriggerChannel()
{
  return triggerChannel;
}

void Oscilloscope::Scope::setTrigger(Oscilloscope::Trigger::trig_t direction,
                                     double threshold,
                                     std::list<Oscilloscope::scope_channel>::iterator channel,
                                     double window)
{
  triggerChannel = channel;
  triggerThreshold = threshold;

  // Update if direction has changed
  if (triggerDirection != direction) {
    if (direction == Oscilloscope::Trigger::NONE) {
      triggering = false;
      timer->start(refresh);
      triggerQueue.clear();
    } else {
      triggering = true;
      timer->stop();
    }
    triggerDirection = direction;
  }
  triggerWindow = window;
}

double Oscilloscope::Scope::getDivT() const
{
  return hScl;
}

// Set x divisions
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

// Set period
void Oscilloscope::Scope::setPeriod(double p)
{
  period = p;
}

// Get number of x divisions on scope
size_t Oscilloscope::Scope::getDivX() const
{
  return divX;
}

// Get number of y divisions on scope
size_t Oscilloscope::Scope::getDivY() const
{
  return divY;
}

// Get current refresh rate
size_t Oscilloscope::Scope::getRefresh() const
{
  return refresh;
}

// Set new refresh rate
void Oscilloscope::Scope::setRefresh(size_t r)
{
  refresh = r;
  timer->setInterval(refresh);
}

// Set channel scale
void Oscilloscope::Scope::setChannelScale(std::list<Oscilloscope::scope_channel>::iterator channel, double scale)
{
  channel->scale = scale;
}

// Set channel offset
void Oscilloscope::Scope::setChannelOffset(std::list<Oscilloscope::scope_channel>::iterator channel,
                             double offset)
{
  channel->offset = offset;
}

// Set pen for channel specified by user
void Oscilloscope::Scope::setChannelPen(std::list<Oscilloscope::scope_channel>::iterator channel, const QPen& pen)
{
  channel->curve->setPen(pen);
}

// Set channel label
void Oscilloscope::Scope::setChannelLabel(std::list<Oscilloscope::scope_channel>::iterator channel,
                            const QString& label)
{
  channel->curve->setTitle(label);
}

// Draw data on the scope
void Oscilloscope::Scope::drawCurves()
{
  if (isPaused) {
    return;
  }

  // Set X scale map is same for all channels
  scaleMapX->setScaleInterval(0, hScl * divX);
  for (auto& channel : this->channels)
  {
    // Set data for channel
    std::vector<double> x(channel.data.size());
    std::vector<double> y(channel.data.size());
    double* x_loc = x.data();
    double* y_loc = y.data();

    // Set Y scale map for channel
    // TODO this should not happen each iteration, instead build into channel
    // struct
    scaleMapY->setScaleInterval(-channel.scale * divY / 2, channel.scale * divY / 2);

    // Scale data to pixel coordinates
    for (size_t j = 0; j < channel.data.size(); ++j) {
      *x_loc = scaleMapX->transform(j * period);
      *y_loc = scaleMapY->transform(channel.data[(data_idx + j) % channel.data.size()]
                                    + channel.offset);
      ++x_loc;
      ++y_loc;
    }

    // Append data to curve
    // Makes deep copy - which is not optimal
    // TODO: change to pointer based method
    channel.curve->setSamples(x.data(), y.data(), channel.data.size());
  }

  // Update plot
  replot();
}
