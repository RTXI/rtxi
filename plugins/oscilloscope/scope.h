#ifndef SCOPE_H
#define SCOPE_H

#include <qevent.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qwidget.h>

#include <list>
#include <vector>

class QPainter;
class QTimer;

class Scope : public QWidget {

    Q_OBJECT

public:

    class Channel {

        friend class Scope;

    public:

        Channel(void);
        virtual ~Channel(void);

        void *getInfo(void);
        const void *getInfo(void) const;

        double getScale(void) const;
        double getOffset(void) const;
        QPen getPen(void) const;
        QString getLabel(void) const;

    private:

        QPen pen;
        QString label;
        double scale;
        double offset;
        std::vector<double> data;
        void *info;

    }; // class Channel

    enum trig_t{
        NONE,
        POS,
        NEG,
    };

    Scope(QWidget *,Qt::WFlags =0);
    virtual ~Scope(void);

    bool paused(void) const;

    std::list<Channel>::iterator insertChannel(QString,double,double,const QPen &,void *);
    void *removeChannel(std::list<Channel>::iterator);
    size_t getChannelCount(void) const;
    std::list<Channel>::iterator getChannelsBegin(void);
    std::list<Channel>::iterator getChannelsEnd(void);

    std::list<Channel>::const_iterator getChannelsBegin(void) const;
    std::list<Channel>::const_iterator getChannelsEnd(void) const;

    void clearData(void);
    void setData(double *,size_t);
    size_t getDataSize(void) const;
    void setDataSize(size_t);

    trig_t getTriggerDirection(void);
    double getTriggerThreshold(void);
    std::list<Channel>::iterator getTriggerChannel(void);
    bool getTriggerHolding(void);
    double getTriggerHoldoff(void);
    void setTrigger(trig_t,double,std::list<Channel>::iterator,bool,double);

    double getDivT(void) const;
    void setDivT(double);

    void setPeriod(double);

    size_t getDivX(void) const;
    size_t getDivY(void) const;
    void setDivXY(size_t,size_t);

    size_t getRefresh(void) const;
    void setRefresh(size_t);

    void setChannelScale(std::list<Channel>::iterator,double);
    void setChannelOffset(std::list<Channel>::iterator,double);
    void setChannelPen(std::list<Channel>::iterator,const QPen &);
    void setChannelLabel(std::list<Channel>::iterator,const QString &);

public slots:

    void timeoutEvent(void);
    void togglePause(void);

protected:

    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);

private:

    void drawBackground(void);
    QRect drawForeground(void);

    void positionLabels(QPainter &);

    void refreshBackground(void);

    bool drawZero;
    size_t divX;
    size_t divY;

    size_t data_idx;
    size_t data_size;

    double hScl;
    double period;

    size_t refresh;

    bool triggering;
    bool triggerHolding;
    trig_t triggerDirection;
    double triggerThreshold;
    double triggerHoldoff;
    std::list<size_t> triggerQueue;
    std::list<Channel>::iterator triggerChannel;
    size_t triggerLast;

    bool isPaused;
    QPixmap background;
    QPixmap foreground;
    QRect drawRect;
    QTimer *timer;
    QString dtLabel;
    std::list<Channel> channels;

}; // class Scope

#endif // SCOPE_H
