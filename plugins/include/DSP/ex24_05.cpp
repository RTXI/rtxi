//
//  File = ex24_05.cpp
//
#include <iostream.h> 
#include <fstream.h>
#include "sd_theor.h"

#ifdef _DEBUG
 ofstream DebugFile("ex24_05.bug", ios::out);
#endif

main()
{
  int seq_len;
  double mu;  

  cout << "Program for Example 24.5" << endl;
  cout << "========================\n" << endl;


  cout << "length of signal seq ?" << endl;
  cin >> seq_len;

  double min_trajec_dist;
  cout << "min pt-pt distance for trajectory plot" << endl;
  cin >> min_trajec_dist;

  double start_0, start_1;
  cout << "start value for w0 ?" << endl;
  cin >> start_0;
  cout << "start value for w1 ?" << endl;
  cin >> start_1;
  cout << "adaptation gain (mu) ?" << endl;
  cin >> mu;
  SteepestDescentTheoretic( start_0, start_1,
                            mu, seq_len, 
                            min_trajec_dist);
  return 0;
}  
