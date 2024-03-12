//
//  File = sig_src.h
//

#ifndef _SIG_SRC_H_
#define _SIG_SRC_H_

#include "complex.h" 

/*!
* Signal Source Interface
*
* Classes that hope to simulate a signal derive this class
*/
class SignalSource
{
public:
  SignalSource(const SignalSource&) = default;
  SignalSource(SignalSource&&) = delete;
  SignalSource& operator=(const SignalSource&) = default;
  SignalSource& operator=(SignalSource&&) = delete;
  virtual ~SignalSource() = default;

  /*!
    * Obtain the following segment.
    *
    * \param data Pointer to the segment data requested in complex form
    * \param size The size of the segment data buffer
    */
  virtual void GetNextSegment(complex* data, int size) = 0;

  /*!
    * Obtain the following segment.
    *
    * \param data Pointer to the segment data requested in real form
    * \param size The size of the segment data buffer
    */
  virtual void GetNextSegment(double* data, int size) = 0;

  /*!
    * Reset the internal buffer.
    */
  virtual void ResetSource() = 0;
};
#endif // _SIG_SRC_H_
