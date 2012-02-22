/*
 BasicPlot, derived from QwtPlot, is the base plot that sets color, grid lines, and zoom functionality.
 */

#ifndef _BASICPLOT_H_
#define _BASICPLOT_H_ 1

#include "scrollzoomer.h"
#include <qwt-qt3/qwt_scale_widget.h>
#include <qwt-qt3/qwt_scale_draw.h>
#include <qwt-qt3/qwt_plot.h>

class Zoomer : public ScrollZoomer
{

Q_OBJECT

public:
  Zoomer(QwtPlotCanvas *canvas) :
    ScrollZoomer(canvas)
  {
  }
  ;

  virtual void
  rescale()
  {
    QwtScaleWidget *scaleWidget = plot()->axisWidget(yAxis());
    QwtScaleDraw *sd = scaleWidget->scaleDraw();
    int minExtent = 0;
    if (zoomRectIndex() > 0)
      {
        minExtent = sd->spacing() + sd->majTickLength() + 1;
        minExtent += sd->labelSize(scaleWidget->font(), 1000).width();
      }
    sd->setMinimumExtent(minExtent);
    ScrollZoomer::rescale();
  }
  ;

public slots:

  void
  setNewBase(QwtScaleDiv * xscalediv, QwtScaleDiv * yscalediv)
  {
    setMaxStackDepth(-1);
    double rleft = xscalediv->lowerBound();
    double rtop = yscalediv->upperBound();
    double rheight = yscalediv->lowerBound() - yscalediv->upperBound();
    double rwidth = xscalediv->upperBound() - xscalediv->lowerBound();
    QwtDoubleRect boundRect(rleft, rtop, rwidth, rheight);
    setZoomBase(boundRect);
  }
  ;

};

class BasicPlot : public QwtPlot
{
Q_OBJECT

public:

  BasicPlot(QWidget *parent);
  virtual
  ~BasicPlot()
  {
  }
  ;
  virtual QSize
  sizeHint() const;

signals:

  void
  setNewBase(QwtScaleDiv * xscalediv, QwtScaleDiv * yscalediv);

public slots:

  void
  setAxes(double xmin, double xmax, double ymin, double ymax);
  Zoomer *zoomer;

private:

};

#endif // _BasicPlot_H_
