/*
 * Generic window for filter
 */

#ifndef GENERIC_WINDOW_HPP
#define GENERIC_WINDOW_HPP

#include <vector>

class GenericWindow
{
public:
  // constructors

  explicit GenericWindow(std::vector<double> half_lag_window);
  void Initialize(std::vector<double> half_lag_window);

  void NormalizeWindow();

  const std::vector<double>& GetDataWindow();

  std::size_t GetNumTaps() { return window.size(); }

private:
  std::vector<double> window;
};

#endif
