/*
 * Copyright (C) 2006 Weill Medical College of Cornell University
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

#include <default_gui_model.h>

class Neuron : public DefaultGUIModel
{

public:

    Neuron(void);
    virtual ~Neuron(void);

    void execute(void);

protected:

    void update(DefaultGUIModel::update_flags_t);

private:

    void derivs(double *,double *);
    void solve(double,double *);

    double y[4];
    double period;
    int steps;

    double V0;
    double Cm;
    double G_Na_max;
    double E_Na;
    double G_K_max;
    double E_K;
    double G_L;
    double E_L;
    double Iapp_offset;
    double rate;

};
