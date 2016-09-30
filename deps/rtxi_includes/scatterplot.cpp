#include "scatterplot.h"

ScatterPlot::ScatterPlot(QWidget *parent) : IncrementalPlot(parent) {
	// setAxes(0, 1000, 0, 100); // initial view
}

void ScatterPlot::appendPoint(double x, double y) {
	// appendData(x, y);
	IncrementalPlot::appendPoint(QPointF(x,y));
}

void ScatterPlot::appendPoint(double x, double y, QwtSymbol::Style s) {
	// appendData(x, y, s);
	IncrementalPlot::appendPoint(QPointF(x,y));
}

void ScatterPlot::clear() {
	clearPoints(); 
	// removeData();
	// replot();
}
