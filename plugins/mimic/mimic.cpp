/*
 * Copyright (C) 2004 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <mimic.h>

extern "C" Plugin::Object *createRTXIPlugin(void) {
    return new Mimic();
}

static DefaultGUIModel::variable_t vars[] = {
    {
        "Vin",
        "The signal to be mimiced",
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
