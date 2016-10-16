/*
	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <rt.h>
#include <debug.h>
#include <cmath>
#include <stdlib.h>
#include <main_window.h>

#include <qwt_abstract_scale_draw.h>

#include "scope.h"

#define fs 1/(period*1e-3)

// Constructor for a channel
Scope::Channel::Channel(void) {}

// Destructor for a channel
Scope::Channel::~Channel(void) {}

// Returns channel information
void *Scope::Channel::getInfo(void)
{
    return info;
}

// Return read-only version of channel information
const void *Scope::Channel::getInfo(void) const
{
    return info;
}

// Returns channel pen
QPen Scope::Channel::getPen(void) const
{
    return this->curve->pen();
}

// Returns channel scale
double Scope::Channel::getScale(void) const
{
    return scale;
}

// Returns channel offset
double Scope::Channel::getOffset(void) const
{
    return offset;
}

// Returns channel label
QString Scope::Channel::getLabel(void) const
{
    return label;
}

// Scope constructor; inherits from QwtPlot
Scope::Scope(QWidget *parent) :	QwtPlot(parent), legendItem(NULL)
{

    // Initialize vars
    isPaused = false;
    divX = 10;
    divY = 10;
    data_idx = 0;
    data_size = 100;
    hScl = 1.0;
    period = 1.0;
    dtLabel = "1ms";
    refresh = 250;
    triggering = false;
    triggerDirection = Scope::NONE;
    triggerThreshold = 0.0;
    triggerChannel = channels.end();
    triggerWindow = 10.0;

    // Initialize director
    d_directPainter = new QwtPlotDirectPainter();
    plotLayout()->setAlignCanvasToScales(true);
    setAutoReplot(false);

    // Set scope canvas
    setCanvas(new Canvas());

    // Setup grid
    grid = new QwtPlotGrid();
    grid->setPen(Qt::gray, 0, Qt::DotLine);
    grid->attach(this);

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
    origin = new QwtPlotMarker();
    origin->setLineStyle(QwtPlotMarker::Cross);
    origin->setValue(500.0, 0.0);
    origin->setLinePen(Qt::gray, 2.0, Qt::DashLine);
    origin->attach(this);

    // Setup scaling map
    scaleMapY = new QwtScaleMap();
    scaleMapY->setPaintInterval(-1.0, 1.0);
    scaleMapX = new QwtScaleMap();
    scaleMapX->setPaintInterval(0.0, 1000.0);

    // Create and attach legend
    legendItem = new LegendItem();
    legendItem->attach(this);
    legendItem->setMaxColumns(1);
    legendItem->setAlignment(Qt::Alignment(Qt::AlignTop | Qt::AlignRight));
    legendItem->setBorderRadius(8);
    legendItem->setMargin(4);
    legendItem->setSpacing(2);
    legendItem->setItemMargin(0);
    legendItem->setBackgroundBrush(QBrush(QColor(225,225,225)));

    // Update scope background/scales/axes
    replot();

    // Timer controls refresh rate of scope
    timer = new QTimer;
    timer->setTimerType(Qt::PreciseTimer);
    QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(timeoutEvent(void)));
    timer->start(refresh);
    resize(sizeHint());
}

// Kill me
Scope::~Scope(void)
{
    delete d_directPainter;
}

// Returns pause status of scope
bool Scope::paused(void) const
{
    return isPaused;
}

// Timeout event slot
void Scope::timeoutEvent(void)
{
    if(!triggering)
        drawCurves();
}

// Insert user specified channel into active list of channels with specified settings
std::list<Scope::Channel>::iterator Scope::insertChannel(QString label, double scale, double offset, const QPen &pen, QwtPlotCurve *curve, void *info)
{
    struct Channel channel;
    channel.label = label;
    channel.scale = scale;
    channel.offset = offset;
    channel.info = info;
    channel.data.resize(data_size,0.0);
    channel.curve = curve;
    channel.curve->setPen(pen);
    channel.curve->setStyle(QwtPlotCurve::Lines);
    channel.curve->setRenderHint(QwtPlotItem::RenderAntialiased, false);
    channel.curve->attach(this);
    channels.push_back(channel);
    return --channels.end();
}

// Remove user specified channel from active channels list
void *Scope::removeChannel(std::list<Scope::Channel>::iterator channel)
{
    channel->curve->detach();
    replot();
    void *info = channel->info;
    channels.erase(channel);
    return info;
}

// Resize event for scope
void Scope::resizeEvent(QResizeEvent *event)
{
    d_directPainter->reset();
    QwtPlot::resizeEvent(event);
}

// Returns count of number of active channels
size_t Scope::getChannelCount(void) const
{
    return channels.size();
}

// Returns beginning of channels list
std::list<Scope::Channel>::iterator Scope::getChannelsBegin(void)
{
    return channels.begin();
}

// Returns end of channels list
std::list<Scope::Channel>::iterator Scope::getChannelsEnd(void)
{
    return channels.end();
}

// Returns read-only pointer to beginning of channel list
std::list<Scope::Channel>::const_iterator Scope::getChannelsBegin(void) const
{
    return channels.begin();
}

// Returns read-only pointer to end of channels list
std::list<Scope::Channel>::const_iterator Scope::getChannelsEnd(void) const
{
    return channels.end();
}

// Zeros data
void Scope::clearData(void)
{
    for(std::list<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
        {
            i->data.assign(data_size, 0);
        }
}

// Scales data based upon desired settings for the channel
void Scope::setData(double data[],size_t size)
{
    if(isPaused)
        return;

    if(size < getChannelCount())
        {
            ERROR_MSG("Scope::setData() : data size mismatch detected\n");
            return;
        }

    size_t index = 0;
    for(std::list<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
        {
            i->data[data_idx] = data[index++];

            if(triggering && i == triggerChannel &&
                    ((triggerDirection == POS && i->data[data_idx-1] < triggerThreshold && i->data[data_idx] > triggerThreshold) ||
                     (triggerDirection == NEG && i->data[data_idx-1] > triggerThreshold && i->data[data_idx] < triggerThreshold)))
                {
                    if(data_idx > triggerWindow*fs)
                        triggerQueue.push_back(data_idx-(triggerWindow*fs));
                }
        }

    ++data_idx %= data_size;

    if(triggering && !triggerQueue.empty() && (data_idx+2)%data_size == triggerQueue.front())
        {
            triggerQueue.pop_front();
            drawCurves();
        }
}

// Returns the data size
size_t Scope::getDataSize(void) const
{
    return data_size;
}

void Scope::setDataSize(size_t size)
{
    for(std::list<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
        i->data.resize(size,0.0);
    data_idx = 0;
    data_size = size;
    triggerQueue.clear();
}

Scope::trig_t Scope::getTriggerDirection(void)
{
    return triggerDirection;
}

double Scope::getTriggerThreshold(void)
{
    return triggerThreshold;
}

double Scope::getTriggerWindow(void)
{
    return triggerWindow;
}

std::list<Scope::Channel>::iterator Scope::getTriggerChannel(void)
{
    return triggerChannel;
}

void Scope::setTrigger(trig_t direction,double threshold,std::list<Channel>::iterator channel, double window)
{
    if(triggerChannel != channel || triggerThreshold != threshold)
        {
            triggerChannel = channel;
            triggerThreshold = threshold;
        }

    // Update if direction has changed
    if(triggerDirection != direction)
        {
            if(direction == Scope::NONE)
                {
                    triggering = false;
                    timer->start(refresh);
                    triggerQueue.clear();
                }
            else
                {
                    triggering = true;
                    timer->stop();
                }
            triggerDirection = direction;
        }
    triggerWindow = window;
}

double Scope::getDivT(void) const
{
    return hScl;
}

// Set x divisions
void Scope::setDivT(double divT)
{
    hScl = divT;
    if(divT >= 1000.)
        dtLabel = QString::number(divT*1e-3)+"s";
    else if(divT >= 1.)
        dtLabel = QString::number(divT)+"ms";
    else if(divT >= 1e-3)
        dtLabel = QString::number(divT*1e3)+"µs";
    else
        dtLabel = QString::number(divT*1e6)+"ns";
}

// Set period
void Scope::setPeriod(double p)
{
    period = p;
}

// Get number of x divisions on scope
size_t Scope::getDivX(void) const
{
    return divX;
}

// Get number of y divisions on scope
size_t Scope::getDivY(void) const
{
    return divY;
}

// Get current refresh rate
size_t Scope::getRefresh(void) const
{
    return refresh;
}

// Set new refresh rate
void Scope::setRefresh(size_t r)
{
    refresh = r;
    timer->setInterval(refresh);
}

// Set channel scale
void Scope::setChannelScale(std::list<Channel>::iterator channel,double scale)
{
    channel->scale = scale;
}

// Set channel offset
void Scope::setChannelOffset(std::list<Channel>::iterator channel,double offset)
{
    channel->offset = offset;
}

// Set pen for channel specified by user
void Scope::setChannelPen(std::list<Channel>::iterator channel,const QPen &pen)
{
    channel->curve->setPen(pen);
}

// Set channel label
void Scope::setChannelLabel(std::list<Channel>::iterator channel,const QString &label)
{
    channel->curve->setTitle(label);
}

// Draw data on the scope
void Scope::drawCurves(void)
{
    if(isPaused)
        return;

    // Set X scale map is same for all channels
    scaleMapX->setScaleInterval(0, hScl*divX);
    for(std::list<Channel>::iterator i = channels.begin(), iend = channels.end(); i != iend; ++i)
        {
            // Set data for channel
            std::vector<double> x (i->data.size());
            std::vector<double> y (i->data.size());
            double *x_loc = x.data();
            double *y_loc = y.data();

            // Set Y scale map for channel
            // TODO this should not happen each iteration, instead build into channel struct
            scaleMapY->setScaleInterval(-i->scale*divY/2, i->scale*divY/2);

            // Scale data to pixel coordinates
            for(size_t j = 0; j < i->data.size(); ++j)
                {
                    *x_loc = scaleMapX->transform(j*period);
                    *y_loc = scaleMapY->transform(i->data[(data_idx+j)%i->data.size()]+i->offset);
                    ++x_loc;
                    ++y_loc;
                }

            // Append data to curve
            // Makes deep copy - which is not optimal
            // TODO: change to pointer based method
            i->curve->setSamples(x.data(), y.data(), i->data.size());
        }

    // Update plot
    replot();
}
