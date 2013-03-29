/*
 ScatterPlot, derived from IncrementalPlot. Includes a plot zoomer.
 */

#ifndef _SCATTERPLOT_H_
#define _SCATTERPLOT_H_ 1

#include "incrementalplot.h"
#include <qwt-qt3/qwt_scale_widget.h>
#include <qwt-qt3/qwt_scale_draw.h>

class QTimer;
/*
 class Zoomer: public ScrollZoomer
 {

 Q_OBJECT

 public:
 Zoomer(QwtPlotCanvas *canvas):
 ScrollZoomer(canvas)
 {
 };

 virtual void rescale() {
 QwtScaleWidget *scaleWidget = plot()->axisWidget(yAxis());
 QwtScaleDraw *sd = scaleWidget->scaleDraw();
 int minExtent = 0;
 if ( zoomRectIndex() > 0 )
 {
 minExtent = sd->spacing() + sd->majTickLength() + 1;
 minExtent += sd->labelSize(scaleWidget->font(), 1000).width();
 }
 sd->setMinimumExtent(minExtent);
 ScrollZoomer::rescale();
 };



 public slots:

 void setNewBase(QwtScaleDiv * xscalediv, QwtScaleDiv * yscalediv) {
 setMaxStackDepth(-1);
 double rleft = xscalediv->lowerBound();
 double rtop = yscalediv->upperBound();
 double rheight = yscalediv->lowerBound() - yscalediv->upperBound();
 double rwidth = xscalediv->upperBound() - xscalediv->lowerBound();
 QwtDoubleRect boundRect(rleft,rtop,rwidth,rheight);
 setZoomBase(boundRect);
 };
 };
 */

class ScatterPlot : public IncrementalPlot
{
Q_OBJECT

public:
  ScatterPlot(QWidget *parent);
  virtual
  ~ScatterPlot()
  {
  }
  ;
  //    virtual QSize sizeHint() const;

signals:
  //	void setNewBase(QwtScaleDiv * xscalediv, QwtScaleDiv * yscalediv);

public slots:
  void
  clear();
  //		void setAxes(double xmin, double xmax, double ymin, double ymax);
  //Zoomer *zoomer;

private slots:
  void
  appendPoint(double x, double y);
  void
  appendPoint(double x, double y, QwtSymbol::Style s);

};

#endif // _SCATTERPLOT_H_
