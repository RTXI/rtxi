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

#include <mimic.h>
#include <qwhatsthis.h>

extern "C" Plugin::Object *createRTXIPlugin(void) {
    return new Mimic();
}

static DefaultGUIModel::variable_t vars[] = {
    {
        "Vin",
        "The signal to be mimicked",
        DefaultGUIModel::INPUT,
    },
    {
        "Vout",
        "The scaled and offset copy of the input",
        DefaultGUIModel::OUTPUT,
    },
    {
        "Gain",
        "Scaling of the copied signal",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,
    },
    {
        "Offset",
        "Offset of the copied signal",
        DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,
    },
};

static size_t num_vars = sizeof(vars)/sizeof(DefaultGUIModel::variable_t);

Mimic::Mimic(void)
    : DefaultGUIModel("Mimic",::vars,::num_vars) {
  QWhatsThis::add(this,
      "<p><b>Mimic:</b><br>This module outputs the signal that is used as input with an optional gain and offset applied..</p>");
    createGUI(vars, num_vars);
    /*
     * Initialize Parameters & Variables
     */
    gain   = 1.0;
    offset = 0.0;

    /*
     * Initialize the GUI
     */
    setParameter("Gain",gain);
    setParameter("Offset",offset);
    refresh();
}

Mimic::~Mimic(void) {}

void Mimic::execute(void) {
    output(0) = gain*input(0)+offset;
}

void Mimic::update(DefaultGUIModel::update_flags_t flag) {
    if(flag == MODIFY) {
        gain   = getParameter("Gain").toDouble();
        offset = getParameter("Offset").toDouble();
    }
}
