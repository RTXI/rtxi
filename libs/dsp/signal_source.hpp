//
//  File = sig_src.h
//

#ifndef SIGNAL_SOURCE_HPP
#define SIGNAL_SOURCE_HPP

#include <complex>

class SignalSource
{
public:
  SignalSource(const SignalSource&) = default;
  SignalSource(SignalSource&&) = delete;
  SignalSource& operator=(const SignalSource&) = default;
  SignalSource& operator=(SignalSource&&) = delete;
  virtual ~SignalSource() = default;

  virtual void GetNextSegment(std::complex<double>, int);

  virtual void GetNextSegment(double*, int);

  virtual void ResetSource();
};
#endif  // _SIG_SRC_H_
