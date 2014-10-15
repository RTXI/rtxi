#include "runningstat.h"
#include <rt.h>

// class constructor
RunningStat::RunningStat() : m_n(0) {}

RunningStat::~RunningStat() {}

void RunningStat::clear() {
  m_n = 0;
}

void RunningStat::push(double x) {
<<<<<<< HEAD
  double p = RT::System::getInstance()->getPeriod();
  m_n++;

=======
  m_n++;
>>>>>>> df956023e03a0f8748eba28d4cd28a6143e24cb5
  if (m_n == 1) {
      m_oldM = x;
      m_oldS = 0.0;
  } else {
      m_newM = m_oldM + (x - m_oldM) / m_n;
//		m_newS = (m_oldS*(m_n-1)/m_n) + (x-m_newM)*(x-m_newM)/(m_n-1); 
<<<<<<< HEAD
		m_newS = (m_n-1.0)/m_n*m_oldS + (x-p)*(x-p)/(m_n);
=======
		m_newS = (m_n-1.0)/m_n*m_oldS + (x-p)*(x-p)/(m_n)
>>>>>>> df956023e03a0f8748eba28d4cd28a6143e24cb5
      m_oldM = m_newM;
      m_oldS = m_newS;
    }
}

int RunningStat::numValues() const {
  return m_n;
}

double RunningStat::mean() const {
  return (m_n > 0) ? m_newM : 0.0;
}

double RunningStat::var() const {
  return ((m_n > 1) ? m_newS / (m_n) : 0.0);
}

double RunningStat::std() const {
  return sqrt(var());
}
