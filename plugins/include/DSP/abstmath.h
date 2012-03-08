//
// File = abstmath.h
//
#ifndef _ABSTMATH_H_
#define _ABSTMATH_H_  

class PrimeFactorSet
{
public: 
  // constructor  
  PrimeFactorSet( int num_factors,
                  int *factors);
private:
  int Num_Factors;
  int Num_Distinct_Factors;
  int *Factor_Vector;
  int *Factor_Multiplicity;
};

class OrderedFactorSet
{
public: 
  // constructor  
  OrderedFactorSet( int num_factors,
                    int *factors);
private:
  int Num_Factors;
  int *Factor_Vector;
};

PrimeFactorSet* PrimeFactorization( int number ); 
#endif //_ABSTMATH_H_