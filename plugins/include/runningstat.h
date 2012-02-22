/*
 RunningStat computes running statistics (mean, standard deviation, and variance) 
 using accurate numerical methods without storing all the numbers.
 */

#include <math.h>

class RunningStat
{
public:
  RunningStat();
  ~RunningStat();
  void
  clear();
  void
  push(double x);
  int
  numValues() const;
  double
  mean() const;
  double
  var() const;
  double
  std() const;

private:
  int m_n;
  double m_oldM, m_newM, m_oldS, m_newS;

};

