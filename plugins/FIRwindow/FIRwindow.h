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
 * Computes FIR filter coefficients using the window method.
 */

#include <boost/circular_buffer.hpp>
#include "../include/DSP/gen_win.h"
#include "../include/DSP/rectnglr.h"
#include "../include/DSP/trianglr.h"
#include "../include/DSP/hamming.h"
#include "../include/DSP/hann.h"
#include "../include/DSP/dolph.h"
#include "../include/DSP/kaiser.h"
#include "../include/DSP/fir_dsgn.h"
#include "../include/DSP/lin_dsgn.h"
#include "../include/DSP/firideal.h"

#include <qobject.h>
#include <qwidget.h>
#include <event.h>
#include <map>
#include <mutex.h>
#include <plugin.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <rt.h>
#include <workspace.h>
#include <default_gui_model.h>
#include <qstring.h>
#include <cstdlib>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qtextstream.h>
#include <qcombobox.h>

class FIRwindow : public DefaultGUIModel
{

Q_OBJECT

public:

  FIRwindow(void);
  virtual
  ~FIRwindow(void);

  void
  execute(void);
  void
  createGUI(DefaultGUIModel::variable_t *, int);

  enum window_t
  {
    RECT, TRI, HAMM, HANN, CHEBY, KAISER,
  };

  enum filter_t
  {
    LOWPASS, HIGHPASS, BANDPASS, BANDSTOP,
  };

protected:

  virtual void
  update(DefaultGUIModel::update_flags_t);

private:
  // inputs, states
  boost::circular_buffer<double> signalin;
  double* convolution;
  double out;
  double dt;
  long long count; // keep track of time
  double systime;
  int n;

  // filter parameters
  double* h1; // left for debugging purposes
  //	double* h2;
  double* h3; // filter coefficients
  window_t window_shape;
  filter_t filter_type;
  int num_taps;
  double lambda1; // cutoff frequencies
  double lambda2;
  double Kalpha; // Kaiser window sidelobe attenuation parameter
  double Calpha; // Chebyshev window sidelobe attenuation parameter

  // FIRwindow functions
  void
  initParameters();
  void
  bookkeep();
  void
  makeFilter();
  GenericWindow *disc_window;
  FirIdealFilter *filter_design;
  QComboBox *windowShape;
  QComboBox *filterType;

  // Saving FIR filter data to file without Data Recorder
  bool
  OpenFile(QString);
  QFile dataFile;
  QTextStream stream;

private slots:
// all custom slots
void saveFIRData(); // write filter parameters to a file
void updateWindow(int);
void updateFilterType(int);

};
