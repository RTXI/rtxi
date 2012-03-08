//
//  File = filt_imp.h
//

#ifndef _FILT_IMP_H_
#define _FILT_IMP_H_

class FilterImplementation
{
public:
  FilterImplementation( ){};

  virtual int GetNumTaps(void)=0;
  virtual double ProcessSample( double input_val )=0;
  virtual long ProcessSample( long input_val )=0;

};

#endif
