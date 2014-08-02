#include "runningstat.h"

// class constructor
RunningStat::RunningStat() :
  m_n(0)
{
}

RunningStat::~RunningStat()
{
}

void
RunningStat::clear()
{
  m_n = 0;
}

void
RunningStat::push(double x)
{
  m_n++;
  if (m_n == 1)
    {
      m_oldM = x;
      m_oldS = 0.0;
    }
  else
    {
      m_newM = m_oldM + (x - m_oldM) / m_n;
      //m_newS = m_oldS + (x-m_oldM)*(x-m_newM);
      m_newS = m_oldS + (m_n - 1) * (x - m_oldM) * (x - m_oldM) / m_n;
      m_oldM = m_newM;
      m_oldS = m_newS;
    }
}

int
RunningStat::numValues() const
{
  return m_n;
}

double
RunningStat::mean() const
{
  return (m_n > 0) ? m_newM : 0.0;
}

double
RunningStat::var() const
{
  return ((m_n > 1) ? m_newS / (m_n) : 0.0);
}

double
RunningStat::std() const
{
  return sqrt(var());
}
