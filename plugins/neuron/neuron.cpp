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

#include <neuron.h>
#include <math.h>
#include <qwhatsthis.h>

/*
 * Model Functions
 */

static inline double
alpha_m(double V)
{
  double x = -(V + 40.0);
  double y = 10.0;

  if (fabs(x / y) < 1e-6)
    return 0.1 * y * (1 - x / y / 2.0);
  else
    return 0.1 * x / (exp(x / y) - 1.0);
}

static inline double
beta_m(double V)
{
  return 4.0 * exp(-(V + 65.0) / 18.0);
}

static inline double
m_inf(double V)
{
  return alpha_m(V) / (alpha_m(V) + beta_m(V));
}

static inline double
tau_m(double V)
{
  return 1.0 / (alpha_m(V) + beta_m(V));
}

static inline double
alpha_h(double V)
{
  return 0.07 * exp(-(V + 65.0) / 20.0);
}

static inline double
beta_h(double V)
{
  return 1.0 / (1.0 + exp(-(V + 35.0) / 10.0));
}

static inline double
h_inf(double V)
{
  return alpha_h(V) / (alpha_h(V) + beta_h(V));
}

static inline double
tau_h(double V)
{
  return 1.0 / (alpha_h(V) + beta_h(V));
}

static inline double
alpha_n(double V)
{
  double x = -(V + 55.0);
  double y = 10.0;

  if (fabs(x / y) < 1e-6)
    return 0.01 * y * (1 - x / y / 2.0);
  else
    return 0.01 * x / (exp(x / y) - 1.0);
}

static inline double
beta_n(double V)
{
  return 0.125 * exp(-(V + 65.0) / 80.0);
}

static inline double
n_inf(double V)
{
  return alpha_n(V) / (alpha_n(V) + beta_n(V));
}

static inline double
tau_n(double V)
{
  return 1.0 / (alpha_n(V) + beta_n(V));
}

extern "C" Plugin::Object *
createRTXIPlugin(void)
{
  return new Neuron();
}

static DefaultGUIModel::variable_t vars[] =
  {
    { "Iapp", "A", DefaultGUIModel::INPUT, },
    { "Vm", "V", DefaultGUIModel::OUTPUT, },
    { "V0", "mV", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "Cm", "uF/cm^2", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "G_Na_max", "mS/cm^2", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "E_Na", "mV", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
    { "G_K_max", "mS/cm^2", DefaultGUIModel::PARAMETER
        | DefaultGUIModel::DOUBLE, },
    { "E_K", "mV", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
        { "G_L", "mS/cm^2", DefaultGUIModel::PARAMETER
            | DefaultGUIModel::DOUBLE, },
        { "E_L", "mV", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
        { "Iapp_offset", "uA/cm^2 - Current added to the input.",
            DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE, },
        { "rate", "Hz - The rate of integration.", DefaultGUIModel::PARAMETER
            | DefaultGUIModel::UINTEGER, },
        { "m", "Sodium Activation", DefaultGUIModel::STATE, },
        { "h", "Sodium Inactivation", DefaultGUIModel::STATE, },
        { "n", "Potassium Activation", DefaultGUIModel::STATE, }, };

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

/*
 * Macros for making the code below a little bit cleaner.
 */

#define V (y[0])
#define m (y[1])
#define h (y[2])
#define n (y[3])
#define dV (dydt[0])
#define dm (dydt[1])
#define dh (dydt[2])
#define dn (dydt[3])
#define G_Na (G_Na_max*m*m*m*h)
#define G_K  (G_K_max*n*n*n*n)
#define Iapp (input(0)*1e6+Iapp_offset)

Neuron::Neuron(void) :
  DefaultGUIModel("Neuron", ::vars, ::num_vars)
{
  QWhatsThis::add(this,
      "<p><b>Hodgkin-Huxley Neuron:</b><br>This module simulates a Hodgkin-Huxley neuron "
      "in real-time. It can be used in place of a biological cell to test algorithms.</p>");
  createGUI(vars, num_vars);
  /*
   * Initialize Parameters
   */
  V0 = -65.0;
  Cm = 1.0;
  G_Na_max = 120.0;
  E_Na = 50.0;
  G_K_max = 36.0;
  E_K = -77.0;
  G_L = 0.3;
  E_L = -54.4;
  Iapp_offset = 0.0;
  rate = 40000;

  /*
   * Initialize Variables
   */
  V = V0;
  m = m_inf(V0);
  h = h_inf(V0);
  n = n_inf(V0);
  period = RT::System::getInstance()->getPeriod() * 1e-6;
  steps = static_cast<int> (ceil(period / rate / 1000.0));

  /*
   * Initialize States
   */
  setState("m", m);
  setState("h", h);
  setState("n", n);

  /*
   * Initialize GUI
   */
  setParameter("V0", V0);
  setParameter("Cm", Cm);
  setParameter("G_Na_max", G_Na_max);
  setParameter("E_Na", E_Na);
  setParameter("G_K_max", G_K_max);
  setParameter("E_K", E_K);
  setParameter("G_L", G_L);
  setParameter("E_L", E_L);
  setParameter("Iapp_offset", Iapp_offset);
  setParameter("rate", rate);
  refresh();
}

Neuron::~Neuron(void)
{
}

/*
 * Simple Euler solver.
 */

void
Neuron::solve(double dt, double *y)
{
  double dydt[4];

  derivs(y, dydt);

  for (size_t i = 0; i < 4; ++i)
    y[i] += dt * dydt[i];
}

void
Neuron::derivs(double *y, double *dydt)
{
  dV = (Iapp - G_Na * (V - E_Na) - G_K * (V - E_K) - G_L * (V - E_L)) / Cm;
  dm = (m_inf(V) - m) / tau_m(V);
  dh = (h_inf(V) - h) / tau_h(V);
  dn = (n_inf(V) - n) / tau_n(V);
}

void
Neuron::execute(void)
{

  /*
   * Because the real-time thread may run much slower than we want to
   *   integrate we need to run multiple interations of the solver.
   */

  for (int i = 0; i < steps; ++i)
    solve(period / steps, y);

  output(0) = V / 1000; //convert to mV
}

void
Neuron::update(DefaultGUIModel::update_flags_t flag)
{
  if (flag == MODIFY)
    {
      V0 = getParameter("V0").toDouble();
      Cm = getParameter("Cm").toDouble();
      G_Na_max = getParameter("G_Na_max").toDouble();
      E_Na = getParameter("E_Na").toDouble();
      G_K_max = getParameter("G_K_max").toDouble();
      E_K = getParameter("E_K").toDouble();
      G_L = getParameter("G_L").toDouble();
      E_L = getParameter("E_L").toDouble();
      Iapp_offset = getParameter("Iapp_offset").toDouble();
      rate = getParameter("rate").toDouble();
      steps = static_cast<int> (ceil(period * rate / 1000.0));

      V = V0;
      m = m_inf(V0);
      h = h_inf(V0);
      n = n_inf(V0);
    }
  else if (flag == PERIOD)
    {
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      steps = static_cast<int> (ceil(period * rate / 1000.0));
    }
}
