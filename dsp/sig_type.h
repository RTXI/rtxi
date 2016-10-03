//
// file = sig_type.h
//

#ifndef _SIG_TYPE_H_
#define _SIG_TYPE_H_ 

//#define _CMPX_SIGS 1

#ifdef _CMPX_SIGS
  #include "complex.h"
  typedef complex type_of_sig_vals_T;
#else
  typedef double type_of_sig_vals_T;
#endif  // _CMPX_SIGS

#endif  // _SIG_TYPE_H_
