#include <qwt-qt3/qwt_plot_printfilter.h>

class RTXIPrintFilter : public QwtPlotPrintFilter
{

public:

  QColor
  color(const QColor &c, Item item) const
  {
    switch (item)
      {
    case Marker:
      return Qt::black;
    case MarkerSymbol:
      return Qt::black;
    case CurveSymbol:
      return Qt::black;
    case CanvasBackground:
      return Qt::white;
    default:
      ;
      }
    return c;
  }
};
