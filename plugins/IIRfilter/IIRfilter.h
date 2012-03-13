/*
 Copyright (C) 2011 Georgia Institute of Technology

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

/*
 * Creates IIR filters
 * Butterworth: passband_edge
 * Chebyshev: passband_ripple, passband_edge, ripple_bw_norm
 * Elliptical: passband_ripple, stopband_ripple, passband_edge, stopband_edge
 */

#include <default_gui_model.h>
#include "../include/DSP/iir_dsgn.h"
#include "../include/DSP/dir1_iir.h"
#include "../include/DSP/unq_iir.h"
#include "../include/DSP/buttfunc.h"
#include "../include/DSP/chebfunc.h"
#include "../include/DSP/elipfunc.h"
#include "../include/DSP/bilinear.h"
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtextstream.h>
#include <qvalidator.h>
#include <qvbox.h>

#define TWO_PI 6.28318531

class IIRfilter : public DefaultGUIModel
{

Q_OBJECT

public:

  IIRfilter(void);
  virtual
  ~IIRfilter(void);

  void
  execute(void);
  void
  createGUI(DefaultGUIModel::variable_t *, int);

  enum filter_t
  {
    BUTTER, CHEBY, ELLIP,
  };

protected:

  virtual void
  update(DefaultGUIModel::update_flags_t);

private:

  // filter parameters
  FilterTransFunc *analog_filter;
  IirFilterDesign *filter_design;
  FilterImplementation *filter_implem;

  double* h3; // filter coefficients
  filter_t filter_type; // type of filter
  double passband_ripple; // dB?
  double stopband_ripple; // dB?
  double passband_edge; // Hz
  double stopband_edge; // Hz
  int filter_order; // filter order
  int ripple_bw_norm; // type of normalization for Chebyshev filter

  bool quant_enabled; // quantize input signal and coefficients
  bool predistort_enabled; // predistort frequencies for bilinear transform
  int input_quan_factor; // quantization factor 2^bits for input signal
  int coeff_quan_factor; // quantization factor 2^bits for filter coefficients

  // bookkeeping
  double out; // bookkeeping for computing convolution
  int n; // bookkeeping for computing convolution
  double dt; // real-time period of system (s)
  long long count; // keep track of time
  double systime; // time that module has been running

  // IIRfilter functions
  void
  initParameters();
  void
  bookkeep();
  void
  makeFilter();
  QComboBox *filterType;
  QComboBox *normType;

  // Saving FIR filter data to file without Data Recorder
  bool
  OpenFile(QString);
  QFile dataFile;
  QTextStream stream;

private slots:

  // all custom slots
  void saveIIRData(); // write filter parameters to a file
  void
  updateFilterType(int);
  void
  updateNormType(int);
  void
  togglePredistort(bool);
  void
  toggleQuantize(bool);

};
