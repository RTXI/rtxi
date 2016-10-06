#include "runningstat.h"

// class constructor <- of all the comments this code could use, this is the one
// we chose...
RunningStat::RunningStat()
  : m_n(0)
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
  // double p = RT::System::getInstance()->getPeriod(); // gets period in
  // nanoseconds
  m_n++;

  if (m_n == 1) {
    m_oldM = x;
    m_oldS = 0.0;
  } else {
    m_newM = m_oldM + (x - m_oldM) / m_n;
    // m_newS = (m_oldS*(m_n-1)/m_n) + (x-m_newM)*(x-m_newM)/(m_n-1); // Sample
    // variance
    m_newS = (m_n - 1.0) / m_n * m_oldS + (1.0 / m_n) * (x) * (x); // Variance
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
  return ((m_n > 1) ? m_newS : 0.0);
}

double
RunningStat::std() const
{
  return sqrt(var());
}
