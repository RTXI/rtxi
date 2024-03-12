//
//  File = cbitrev.cpp
//

#include "complex.h"
#include "cbitrev.h"

void ComplexBitReverse(complex* array, int size)
{
  complex tt;
  int nv2 = 0;
  int nm1 = 0;
  int i = 0;
  int j = 0;
  int k = 0;

  nv2 = size / 2;
  nm1 = size - 1;

  j = 0;
  for (i = 0; i < nm1; i++) {
    if (i < j) {
      tt = array[j];
      array[j] = array[i];
      array[i] = tt;
    }
    k = nv2;
    while (k <= j) {
      j -= k;
      k /= 2;
    }
    j += k;
  }
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
