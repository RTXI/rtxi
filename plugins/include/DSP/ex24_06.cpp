//
// file = ex24_06.cpp
//
#include <stdlib.h> 
#include <iostream.h> 
#include <fstream.h>
main()
{
  double mse, mse_min, mu;
  int n, num_taps;
  int k, k_max;
  cout << "enter number of taps in SD filter" << endl;
  cin >> num_taps;
  double* lambda = new double[num_taps];
  double* factor = new double[num_taps];
  double* fact_prod = new double[num_taps];
  double* tap_init = new double[num_taps];
  ofstream out_file("sd_learn.txt", ios::out);

  for(n=0; n<num_taps; n++)
    {
    cout << "enter lambda[" << n << "] " << endl;
    cin >> (lambda[n]);
    }
  for(n=0; n<num_taps; n++)
    {
    cout << "enter initial value for tap " << n << endl;
    cin >> (tap_init[n]);
    }
  cout << "enter minimum MSE" << endl;
  cin >> mse_min;
  cout << "enter mu" << endl;
  cin >> mu;
  for(n=0; n<num_taps; n++)
    {
    factor[n] = 1.0 - 2.0 * mu * lambda[n];
    fact_prod[n] = lambda[n] * tap_init[n] * tap_init[n];
    }
  cout << "how many iterations?" << endl;
  cin >> k_max;
  for(k=0; k<k_max; k++)
    {
    mse = mse_min;
    for(n=0; n<num_taps; n++)
      {
      mse += fact_prod[n];
      fact_prod[n] *= factor[n];
      }
    out_file << k << ", " << mse << ", " << mse_min << endl;
    } // end of loop over k
  out_file.close();
return 0;
}