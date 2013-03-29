//
//  File = sig_src.h
//

#ifndef _SIG_SRC_H_
#define _SIG_SRC_H_ 

 #include "complex.h"
 #include "sig_src.h"

 class SignalSource
 { 
 public:

 SignalSource( ){};

  ~SignalSource(){}; 
  
//  float_complex* GetNextSegment(void);
  virtual void GetNextSegment(complex*,int){};

  virtual void GetNextSegment(double*,int){};

  virtual void ResetSource(void){};
  
 };
#endif // _SIG_SRC_H_