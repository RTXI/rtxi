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

/*
 * Reads values from ACSII file and generates that signal in real-time
 */

#include <default_gui_model.h>
#include "../include/plotdialog.h"
#include "../include/basicplot.h"
#include <qfiledialog.h>

class WaveMaker : public DefaultGUIModel
{

Q_OBJECT

public:

  WaveMaker(void);
  virtual
  ~WaveMaker(void);

  void
  execute(void);
  void
  createGUI(DefaultGUIModel::variable_t *, int);

protected:

  virtual void
  update(DefaultGUIModel::update_flags_t);

private:
  // inputs, states

  double dt;
  QString filename;
  size_t idx;
  size_t loop;
  size_t nloops;
  std::vector<double> wave;
  double length;
  double gain;

  // WaveMaker functions
  void
  initParameters();

private slots:
  // all custom slots
  void
  loadFile();
  void
  loadFile(QString);
  void
  previewFile();

};

