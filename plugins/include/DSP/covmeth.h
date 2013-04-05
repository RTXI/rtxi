//
//  File = covmeth.h
//

#ifndef _COVMETH_H_
#define _COVMETH_H_

//#include "cmpx_mat.h"
#include "matrix_T.h"
#include "complex.h"
#include "typedefs.h"

//class CovarMethCorrMtx : public complex_matrix
class CovarMethCorrMtx : public matrix<complex>
{
public:
  CovarMethCorrMtx( complex *signal,
                    int seq_len,
                    int max_lag);

private:

};
complex* CovarMethRightHandVect( complex *x,
                                 int seq_len,
                                 int max_lag);

#endif
