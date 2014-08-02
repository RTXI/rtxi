//
//  File = thy_spec.h
//                   
#ifndef _THY_SPEC_H_
#define _THY_SPEC_H_

class TheoreticalSpectrum{
public:
  TheoreticalSpectrum( ){}; 
  virtual double GetPsdValue(double freq)=0;
  //double GetPsdValue( double freq ); 
};
#endif
 