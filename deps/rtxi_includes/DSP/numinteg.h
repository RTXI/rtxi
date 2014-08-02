//
// File = numinteg.h
//
#ifndef _NUMINTEG_H_
#define _NUMINTEG_H_  


class NumericInteg
{
public: 

  //  constructor
  NumericInteg( double delta_t );
  
  double Integrate( double input );

private:

  double Integ_Mem;
  double Delta_T; 
}; 
#endif 