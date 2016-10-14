#include "plotdialog.h"
#include <qlayout.h>
#include <qpushbutton.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>

PlotDialog::PlotDialog(QWidget* parent, QString name, double* xData,
                       double* yData, int size)
  : QDialog(parent)
{
  setWindowTitle(name);
  setModal(true);
  QVBoxLayout* layout = new QVBoxLayout(this); // overall GUI layout

  BasicPlot* gPlot = new BasicPlot(this);
  QPushButton* closeBttn = new QPushButton("Close", this);
  closeBttn->setMaximumWidth(100);

  layout->addWidget(gPlot);
  layout->addWidget(closeBttn);

  QObject::connect(closeBttn, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(this, SIGNAL(setPlotRange(double, double, double, double)),
                   gPlot, SLOT(setAxes(double, double, double, double)));

  QwtPlotCurve* gCurve = new QwtPlotCurve("Curve 1");
  gCurve->setSamples(xData, yData, size);
  gCurve->attach(gPlot);
  gCurve->setPen(QColor(Qt::white));
  gPlot->replot();

  emit setPlotRange(gPlot->axisScaleDiv(QwtPlot::xBottom).lowerBound(),
                    gPlot->axisScaleDiv(QwtPlot::xBottom).upperBound(),
                    gPlot->axisScaleDiv(QwtPlot::yLeft).lowerBound(),
                    gPlot->axisScaleDiv(QwtPlot::yLeft).upperBound());
};
