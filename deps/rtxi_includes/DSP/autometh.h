//
//  File = autometh.h
//

#ifndef _AUTOMETH_H_
#define _AUTOMETH_H_

#include "toeplitz.h"
#include "complex.h"
#include "typedefs.h"

template <class T>
class AutocorrMethCorrMtx : public ToeplitzMatrix<T>
{
public:
  AutocorrMethCorrMtx( T *signal,
                       int seq_len,
                       int max_lag);
  ~AutocorrMethCorrMtx();

private:

};

#endif
