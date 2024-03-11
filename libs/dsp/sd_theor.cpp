//
//  File = sd_filt.cpp
//

#include "sd_theor.h"

void SteepestDescentTheoretic(double start_pt_0,
                              double start_pt_1,
                              double mu,
                              int num_pts,
                              double min_dist)
{
  int n;
  double w0, w1, old_w0, old_w1, grad_0, grad_1;
  double dist;
  std::cout << "in SteepestDescentTheoretic" << std::endl;
  ofstream out_file("sd_traj.txt", ios::out);

  w0 = start_pt_0;
  w1 = start_pt_1;
  old_w0 = w0;
  old_w1 = w1;
  out_file << "0, " << old_w0 << ", " << old_w1 << std::endl;

  for (n = 1; n < num_pts; n++) {
    grad_0 = 2.0 * (w0 + (w1 - 1.0) / 3.0);
    grad_1 = 2.0 * w1 + (2.0 * w0 / 3.0) + 1.2;
    w0 -= mu * grad_0;
    w1 -= mu * grad_1;
    dist = sqrt((w0 - old_w0) * (w0 - old_w0) + (w1 - old_w1) * (w1 - old_w1));
    if (dist < min_dist)
      continue;
    out_file << n << ", " << w0 << ", " << w1 << std::endl;
    old_w0 = w0;
    old_w1 = w1;
  }
  out_file.close();
}
