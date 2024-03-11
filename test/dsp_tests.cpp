/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

         This program is free software: you can redistribute it and/or modify
         it under the terms of the GNU General Public License as published by
         the Free Software Foundation, either version 3 of the License, or
         (at your option) any later version.

         This program is distributed in the hope that it will be useful,
         but WITHOUT ANY WARRANTY; without even the implied warranty of
         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
         GNU General Public License for more details.

         You should have received a copy of the GNU General Public License
         along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <random>

#include "dsp_tests.hpp"

#include "dsp/generic_window.hpp"

TEST_F(GenericWindowTests, initialize)
{
  std::vector<double> half_lag_window;
  half_lag_window.reserve(even_window_size);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distribution(0, 50);
  for (size_t n = 0; n < even_window_size; n++) {
    half_lag_window.push_back(distribution(gen));
  }
  rtxi::dsp::GenericWindow window(half_lag_window);
  ASSERT_EQ(window.GetDataWindow().size(), half_lag_window.size() * 2);
  for (size_t i = 0; i < even_window_size / 2; i++) {
    ASSERT_DOUBLE_EQ(window.GetDataWindow().at(i),
                     window.GetDataWindow().at(even_window_size - i - 1));
  }
  half_lag_window.push_back(distribution(gen));
  window.Initialize(half_lag_window);
  ASSERT_EQ(window.GetDataWindow().size(), half_lag_window.size()*2-1);
  for (size_t i = 0; i < even_window_size / 2 + 1; i++) {
    ASSERT_DOUBLE_EQ(window.GetDataWindow().at(i),
                     window.GetDataWindow().at(even_window_size - i - 1));
  }
}

TEST_F(GenericWindowTests, normalize)
{
  std::vector<double> half_lag_window;
  half_lag_window.reserve(even_window_size);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distribution(0, 50);
  for (size_t n = 0; n < even_window_size; n++) {
    half_lag_window.push_back(distribution(gen));
  }
  rtxi::dsp::GenericWindow window(half_lag_window);
  window.NormalizeWindow();
  const double max_val = *std::max_element(window.GetDataWindow().begin(),
                                           window.GetDataWindow().end());
  ASSERT_TRUE(max_val <= 1.0);
}

TEST_F(GenericWindowTests, GetNumTaps)
{
  std::vector<double> half_lag_window;
  half_lag_window.reserve(even_window_size);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> distribution(0, 50);
  for (size_t n = 0; n < even_window_size; n++) {
    half_lag_window.push_back(distribution(gen));
  }
  rtxi::dsp::GenericWindow window(half_lag_window);
  ASSERT_EQ(window.GetDataWindow().size(), window.GetNumTaps());
}
