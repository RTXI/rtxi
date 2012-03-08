//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//
//  File = abstmath.cpp
//
//  Mathematical Operations for Abstract Algebra
//

#include <stdlib.h> 
#include <iostream> 
#include <fstream>
#include <math.h>
#include "misdefs.h"
#include "abstmath.h"

//======================================================
//  constructor

PrimeFactorSet::PrimeFactorSet( int num_factors, int* factors )
{
  int n1, n2, match;
  int distinct_factor_tally;
  Num_Factors = num_factors;
  Num_Distinct_Factors = 1;
  for(n1=1; n1<num_factors; n1++)
  {
    match=0;
    for(n2=0; n2<n1; n2++)
    {
      // compare factor n1 to all other factors that
      // come before it in the vector factors[].
      // if no match, increment count of distinct factors
      if(factors[n1] == factors[n2]) match=1;
    }
    if(match==1) continue;
      Num_Distinct_Factors++;
  }
  Factor_Vector = new int[Num_Distinct_Factors];
  Factor_Multiplicity = new int[Num_Distinct_Factors];
  Factor_Vector[0] = factors[0];
  Factor_Multiplicity[0] = 1;
  distinct_factor_tally = 1;
  for(n1=1; n1<num_factors; n1++)
  {
    match=0;
    for(n2=0; n2<distinct_factor_tally; n2++)
    {
      if(factors[n1] != Factor_Vector[n2]) continue;
        Factor_Multiplicity[n2]++;
        match=1;
        break;
    }
    if(match) continue;
      Factor_Vector[distinct_factor_tally] = factors[n1];
      Factor_Multiplicity[distinct_factor_tally] = 1;
      distinct_factor_tally++;
  }
  for(n1=0; n1<Num_Distinct_Factors; n1++)
  {
    std::cout << Factor_Vector[n1] << " ** " 
         << Factor_Multiplicity[n1] << std::endl;
  }
  return;
};

//======================================================
//  constructor

OrderedFactorSet::OrderedFactorSet( int num_factors, 
                                    int* factors )
{
  int n;
  Num_Factors = num_factors;

  Factor_Vector = new int[Num_Factors];

  for(n=0; n<num_factors; n++)
  {
    Factor_Vector[n] = factors[n];
  }
  return;
};

PrimeFactorSet* PrimeFactorization( int number )
{
  int num_factors;
  int first_n,geom_middle;
  int residue;
  int factor_not_found;
  int* temp_factors = new int[20];

  residue = number;

  geom_middle = (int) ceil(sqrt(double(residue)));
  factor_not_found = 1;
  num_factors = 0;
  first_n = 2;

  for(;;)
  {
    for(int n=first_n; n<=geom_middle; n++)
    {
      if(residue%n != 0) continue;
        factor_not_found = 0;
        temp_factors[num_factors] = n;
        num_factors++;
        residue /= n;
        geom_middle = (int) ceil(sqrt(double(residue)));
        first_n = n;
        std::cout << "found factor " << n << std::endl;
        break;
    }
    if(residue == 1) break;
    if(factor_not_found) break;
    factor_not_found = 1;
  }
  if(residue != 1) {
    temp_factors[num_factors] = residue;
    num_factors++;
  }
  std::cout << "final residue is " << residue << std::endl;
  PrimeFactorSet* factor_set = new PrimeFactorSet( num_factors, 
                                         temp_factors);

  delete[] temp_factors;
  return factor_set;
}
