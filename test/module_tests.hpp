/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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
#ifndef MODULE_TESTS
#define MODULE_TESTS

#include <gtest/gtest.h>

#include "module.hpp"

class ModuleComponetTests : public ::testing::Test
{
protected:
  ModuleComponetTests() = default;
  ~ModuleComponetTests() = default;
};

class ModulePanelTests : public ::testing::Test
{
protected:
  ModulePanelTests() = default;
  ~ModulePanelTests() = default;
};

class ModulePluginTests : public ::testing::Test
{
  ModulePluginTests() = default;
  ~ModulePluginTests() = default;
};

class ModuleManagerTests : public ::testing::Test
{
  ModuleManagerTests() = default;
  ~ModuleManagerTests() = default;
};

#endif