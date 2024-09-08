/*
 * Generic window for filter
 */

#include <algorithm>
#include <utility>

#include "generic_window.hpp"

GenericWindow::GenericWindow(std::vector<double> half_lag_window)
{
  Initialize(std::move(half_lag_window));
}

void GenericWindow::Initialize(std::vector<double> half_lag_window)
{
  window.clear();
  const int even = static_cast<int>(half_lag_window.size() % 2);
  window.insert(
      window.end(), half_lag_window.rbegin(), half_lag_window.rend() - even);
  window.insert(window.end(), half_lag_window.begin(), half_lag_window.end());
}

void GenericWindow::NormalizeWindow()
{
  const double peak = *std::max_element(window.begin(), window.end());
  for (auto& val : window) {
    val /= peak;
  }
}

const std::vector<double>& GenericWindow::GetDataWindow()
{
  return window;
}

void GenericWindow::SetDataWindow(const std::vector<double>& win)
{
  this->window = win;
}
