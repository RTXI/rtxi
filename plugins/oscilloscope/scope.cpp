/*
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

#include "scope.h"

#include <debug.h>

#include <qpainter.h>
#include <qtimer.h>

#include <cmath>
#include <stdlib.h>

Scope::Channel::Channel(void) {}

Scope::Channel::~Channel(void) {}

void *Scope::Channel::getInfo(void) {
    return info;
}

const void *Scope::Channel::getInfo(void) const {
    return info;
}

QPen Scope::Channel::getPen(void) const {
    return pen;
}

double Scope::Channel::getScale(void) const {
    return scale;
}

double Scope::Channel::getOffset(void) const {
    return offset;
}

QString Scope::Channel::getLabel(void) const {
    return label;
}

Scope::Scope(QWidget *parent,Qt::WFlags flags)
    : QWidget(parent,NULL,flags) {
    setBackgroundMode(Qt::NoBackground);

    setMinimumSize(16,9);

    background.setOptimization(QPixmap::BestOptim);
    foreground.setOptimization(QPixmap::BestOptim);

    isPaused = false;

    drawZero = true;
    divX = 16;
    divY = 10;

    data_idx = 0;
    data_size = 100;

    hScl = 10.0;
    period = 1.0;

    dtLabel = "10ms";

    refresh = 250;

    triggering = false;
    triggerHolding = false;
    triggerDirection = NONE;
    triggerThreshold = 0.0;
    triggerHoldoff = 5.0;
    triggerLast = (size_t)(-1);
    triggerChannel = channels.end();

    timer = new QTimer(this);
    QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(timeoutEvent(void)));
    timer->start(refresh);
}

Scope::~Scope(void) {}

bool Scope::paused(void) const {
    return isPaused;
}

#include <rt.h>

void Scope::timeoutEvent(void) {
    if(!triggering)
        update(drawForeground());
}

void Scope::togglePause(void) {
    isPaused = !isPaused;
}

std::list<Scope::Channel>::iterator Scope::insertChannel(QString label,double scale,double offset,const QPen &pen,void *info) {
    struct Channel channel;

    channel.label = label;
    channel.scale = scale;
    channel.offset = offset;
    channel.pen = pen;
    channel.info = info;
    channel.data.resize(data_size,0.0);

    channels.push_back(channel);

    refreshBackground();

    return --channels.end();
}

void *Scope::removeChannel(std::list<Scope::Channel>::iterator channel) {
    void *info = channel->info;
    channels.erase(channel);

    refreshBackground();

    return info;
}

size_t Scope::getChannelCount(void) const {
    return channels.size();
}

std::list<Scope::Channel>::iterator Scope::getChannelsBegin(void) {
    return channels.begin();
}

std::list<Scope::Channel>::iterator Scope::getChannelsEnd(void) {
    return channels.end();
}

std::list<Scope::Channel>::const_iterator Scope::getChannelsBegin(void) const {
    return channels.begin();
}

std::list<Scope::Channel>::const_iterator Scope::getChannelsEnd(void) const {
    return channels.end();
}

void Scope::clearData(void) {
    for(std::list<Channel>::iterator i = channels.begin(), end = channels.end();i != end;++i)
        for(size_t j = 0;j < data_size;++j)
            i->data[j] = 0.0;
}

void Scope::setData(double data[],size_t size) {
    if(isPaused)
        return;

    if(size < getChannelCount()) {
        ERROR_MSG("Scope::setData() : data size mismatch detected\n");
        return;
    }

    size_t index = 0;
    for(std::list<Channel>::iterator i = channels.begin(), end = channels.end();i != end;++i) {
        i->data[data_idx] = data[index++];

        if(triggering && i == triggerChannel &&
           ((triggerDirection == POS && i->data[data_idx-1] < triggerThreshold && i->data[data_idx] > triggerThreshold) ||
            (triggerDirection == NEG && i->data[data_idx-1] > triggerThreshold && i->data[data_idx] < triggerThreshold))) {
            triggerQueue.push_back(data_idx);
        }
    }

    ++data_idx %= data_size;

    if(triggering && !triggerQueue.empty() && (data_idx+2)%data_size == triggerQueue.front()) {
        if(triggerLast != (size_t)(-1) && (triggerQueue.front()+data_size-triggerLast)%data_size*period < triggerHoldoff)
            triggerQueue.pop_front();
        else {
            triggerLast = triggerQueue.front();
            triggerQueue.pop_front();

            if(!triggerHolding)
                foreground = background;

            QPainter painter(&foreground);

            size_t x, y;
            double scale;
            for(std::list<Channel>::iterator i = channels.begin(), iend = channels.end();i != iend;++i) {
                scale = height()/(i->scale*divY);
                painter.setPen(i->getPen());
                x = 0;
                y = round(height()/2-scale*(i->data[data_idx]+i->offset));
                painter.moveTo(x,y);
                for(size_t j = 1;j<i->data.size();++j) {
                    x = round(((j*period)*width())/(hScl*divX));
                    y = round(height()/2-scale*(i->data[(data_idx+j)%data_size]+i->offset));
                    painter.lineTo(x,y);
                    if(x >= width()) break;
                }
            }
            update();
        }
    }
}

size_t Scope::getDataSize(void) const {
    return data_size;
}

void Scope::setDataSize(size_t size) {
    for(std::list<Channel>::iterator i = channels.begin(), end = channels.end();i != end;++i) {
        i->data.clear();
        i->data.resize(size,0.0);
    }
    data_idx = 0;
    data_size = size;
    triggerQueue.clear();
}

Scope::trig_t Scope::getTriggerDirection(void) {
    return triggerDirection;
}

double Scope::getTriggerThreshold(void) {
    return triggerThreshold;
}

std::list<Scope::Channel>::iterator Scope::getTriggerChannel(void) {
    return triggerChannel;
}

bool Scope::getTriggerHolding(void) {
    return triggerHolding;
}

double Scope::getTriggerHoldoff(void) {
    return triggerHoldoff;
}

void Scope::setTrigger(trig_t direction,double threshold,std::list<Channel>::iterator channel,bool holding,double holdoff) {
    triggerHolding = holding;
    triggerHoldoff = holdoff;
    triggerLast = (size_t)(-1);

    if(triggerChannel != channel || triggerThreshold != threshold) {
        triggerChannel = channel;
        triggerThreshold = threshold;

        refreshBackground();
    }

    if(triggerDirection != direction) {
        if(direction == NONE) {
            triggering = false;
            timer->start(refresh);
            triggerQueue.clear();
        } else {
            triggering = true;
            timer->stop();

            foreground = background;
            update();
        }
        triggerDirection = direction;
    }
}

double Scope::getDivT(void) const {
    return hScl;
}

void Scope::setDivT(double divT) {
    hScl = divT;
    QChar mu = QChar(0x3BC);
    if(divT >= 1000.)
        dtLabel = QString::number(divT*1e-3)+"s";
    else if(divT >= 1.)
        dtLabel = QString::number(divT)+"ms";
    else if(divT >= 1e-3)
        dtLabel = QString::number(divT*1e3)+mu+"s";
    else
        dtLabel = QString::number(divT*1e6)+"ns";

    refreshBackground();
}

void Scope::setPeriod(double p) {
    period = p;
}

size_t Scope::getDivX(void) const {
    return divX;
}

size_t Scope::getDivY(void) const {
    return divY;
}

void Scope::setDivXY(size_t dx,size_t dy) {
    divX = dx;
    divY = dy;

    drawBackground();
    if(triggering)
        foreground = background;
    else
        drawForeground();
    update();
}

size_t Scope::getRefresh(void) const {
    return refresh;
}

void Scope::setRefresh(size_t r) {
    refresh = r;
    timer->changeInterval(refresh);
}

void Scope::setChannelScale(std::list<Channel>::iterator channel,double scale) {
    channel->scale = scale;
}

void Scope::setChannelOffset(std::list<Channel>::iterator channel,double offset) {
    channel->offset = offset;
}

void Scope::setChannelPen(std::list<Channel>::iterator channel,const QPen &pen) {
    channel->pen = pen;
    refreshBackground();
}

void Scope::setChannelLabel(std::list<Channel>::iterator channel,const QString &label) {
    channel->label = label;
    refreshBackground();
}

void Scope::paintEvent(QPaintEvent *e) {
    bitBlt(this,e->rect().topLeft(),&foreground,e->rect(),Qt::CopyROP);
}

void Scope::resizeEvent(QResizeEvent *) {
    refreshBackground();
}

void Scope::drawBackground(void) {
    int xDiv = static_cast<int>(round(1.0*width()/divX));
    int yDiv = static_cast<int>(round(1.0*height()/divY));
    int zero = static_cast<int>(round(height()/2.0));

    background.resize(width(),height());
    background.fill(Qt::white);

    QPainter painter(&background);

    if(drawZero) {
        painter.setPen(QPen(Qt::black,3,Qt::DashDotLine));
        painter.drawLine(0,zero,width(),zero);
    }

    painter.setPen(QPen(Qt::black,1,Qt::DotLine));
    for(int i=yDiv;i<height()-yDiv/2;i+=yDiv)
        if(!drawZero || abs(i-zero) >= yDiv/3)
            painter.drawLine(0,i,width(),i);
    for(int i=xDiv;i<width()-xDiv/2;i+=xDiv)
        painter.drawLine(i,0,i,height());

    positionLabels(painter);

    if(triggerChannel != channels.end()) {
        painter.setPen(QPen(Qt::yellow,2,Qt::DashLine));
        double scale = height()/(triggerChannel->scale*divY);
        double offset = triggerChannel->offset;
        int thresh = round(height()/2-scale*(triggerThreshold+offset));
        painter.drawLine(0,thresh,width(),thresh);
    }
}

QRect Scope::drawForeground(void) {
    foreground = background;
    QPainter painter(&foreground);

    int x, y;
    int miny = height(), maxy = 0;
    double scale;
    for(std::list<Channel>::iterator i = channels.begin(), iend = channels.end();i != iend;++i) {
        scale = height()/(i->scale*divY);
        painter.setPen(i->getPen());
        x = 0;
        y = round(height()/2-scale*(i->data[(data_idx)%i->data.size()]+i->offset));
        if(y < miny) miny = y;
        if(y > maxy) maxy = y;
        painter.moveTo(x,y);
        for(size_t j = 1;j<i->data.size();++j) {
            x = round(((j*period)*width())/(hScl*divX));
            y = round(height()/2-scale*(i->data[(data_idx+j)%i->data.size()]+i->offset));
            if(y < miny) miny = y;
            if(y > maxy) maxy = y;
            painter.lineTo(x,y);
            if(x >= width()) break;
        }
    }

    QRect newDrawRect;
    if(miny <= maxy)
        newDrawRect.setRect(0,miny-1,width(),maxy-miny+2);
    QRect redrawRect = newDrawRect.unite(drawRect);
    drawRect = newDrawRect;

    return redrawRect;
}

void Scope::positionLabels(QPainter &painter) {
    QRect bound;

    if(getChannelCount()) {
        int maxh = 1, maxw = 1;
        for(std::list<Channel>::iterator i = channels.begin(),end = channels.end();i != end;++i) {
            bound = painter.boundingRect(rect(),0,i->label);
            if(maxw < bound.width()+25) maxw = bound.width()+25;
            if(maxh < bound.height()) maxh = bound.height();
        }

        size_t cols = (width()-painter.boundingRect(rect(),0,dtLabel).width()-25)/maxw;

        size_t col = 0, row = 0;
        for(std::list<Channel>::iterator i = channels.begin(),end = channels.end();i != end;++i) {
            painter.setPen(i->getPen());
            painter.drawText(col*maxw,static_cast<int>(floor(height()-(row+1)*1.5*maxh)),i->label);
            if(++col >= cols) {
                ++row;
                col = 0;
            }
        }
    }

    bound = painter.boundingRect(rect(),0,dtLabel);
    painter.setPen(QPen(Qt::black,1,Qt::SolidLine));
    painter.drawText(static_cast<int>(floor(width()-bound.width()-10)),static_cast<int>(floor(height()-1.5*bound.height())),dtLabel);
}

void Scope::refreshBackground(void) {
    drawBackground();
    if(triggering)
        foreground = background;
    else
        drawForeground();
    update();    
}
