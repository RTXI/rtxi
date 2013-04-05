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

#include <default_gui_model.h>
#include <../include/gen_whitenoise.h>
#include <math.h>
#include <string>

class NoiseGen : public DefaultGUIModel
{

Q_OBJECT

public:

  NoiseGen(void);
  virtual
  ~NoiseGen(void);

  void
  execute(void);
  void
  createGUI(DefaultGUIModel::variable_t *, int);

  enum mode_t
  {
    WHITEBM, OU,
  };

public slots:

signals: // custom signals

protected:

virtual void
update(DefaultGUIModel::update_flags_t);

private:

  void
  initParameters();
  void
  initStimulus(); // creates AP stim and NoiseGen stimuli

  double mean;
  double variance;
  mode_t mode;

  GeneratorWNoise whitenoisewave;

  long long count;
  double systime;
  double dt;

  // QT components
  QPushButton *wnoiseButton;

private slots:

  void
  updateMode(int);
};
