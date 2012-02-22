#include "basicplot.h"
#include <qdialog.h>

class PlotDialog : public QDialog
{

Q_OBJECT

public:

  PlotDialog(QWidget *parent, QString name, double* xData, double* yData,
      int size);

signals:

  void
  setPlotRange(double newminx, double newmaxx, double newminy, double newmaxy);

};

