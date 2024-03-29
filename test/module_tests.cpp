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

#include "module_tests.hpp"

std::vector<Widgets::Variable::Info> generateDefaultComponentVariables()
{
  std::vector<Widgets::Variable::Info> default_variable_list;
  Widgets::Variable::Info tempvarinfo;
  tempvarinfo.id = TEST_PARAMETER_ID::TESTINT;
  tempvarinfo.name = std::string("testint");
  tempvarinfo.description = std::string("TEST INTEGER");
  tempvarinfo.vartype = Widgets::Variable::INT_PARAMETER;
  tempvarinfo.value = static_cast<int64_t>(0);
  default_variable_list.push_back(tempvarinfo);

  tempvarinfo.id = TEST_PARAMETER_ID::TESTDOUBLE;
  tempvarinfo.name = std::string("testdouble");
  tempvarinfo.description = std::string("TEST DOUBLE");
  tempvarinfo.vartype = Widgets::Variable::DOUBLE_PARAMETER;
  tempvarinfo.value = 0.0;
  default_variable_list.push_back(tempvarinfo);

  tempvarinfo.id = TEST_PARAMETER_ID::TESTUINT;
  tempvarinfo.name = std::string("testuint");
  tempvarinfo.description = std::string("TEST UNSIGNED INTEGER");
  tempvarinfo.vartype = Widgets::Variable::UINT_PARAMETER;
  tempvarinfo.value = static_cast<uint64_t>(0);
  default_variable_list.push_back(tempvarinfo);

  tempvarinfo.id = TEST_PARAMETER_ID::TESTCOMMENT;
  tempvarinfo.name = std::string("teststring");
  tempvarinfo.description = std::string("TEST STRING");
  tempvarinfo.vartype = Widgets::Variable::COMMENT;
  tempvarinfo.value = std::string("");
  default_variable_list.push_back(tempvarinfo);

  // tempvarinfo.id = TEST_PARAMETER_ID::TESTSTATE;
  // tempvarinfo.name = std::string("teststate");
  // tempvarinfo.description = std::string("TEST STATE");
  // tempvarinfo.vartype = Widgets::Variable::STATE;
  // tempvarinfo.value = Widgets::Variable::INIT;
  // default_variable_list.push_back(tempvarinfo);

  return default_variable_list;
}

TEST_F(ModulePluginTests, attachComponent)
{
  // there is a component already registered automatically during
  // initialization of test. We just have to check whether the
  // real-time thread executed the thread object.
  EXPECT_TRUE(this->component_ptr->wasExecuted());
}

TEST_F(ModulePluginTests, ComponentParameters)
{
  // We will check whether plugin actually retrieves component values
  std::vector<Widgets::Variable::Info> variable_list =
      generateDefaultComponentVariables();
  ASSERT_EQ(std::get<int64_t>(variable_list[0].value),
            this->plugin->getComponentIntParameter(variable_list[0].id));
  ASSERT_DOUBLE_EQ(
      std::get<double>(variable_list[1].value),
      this->plugin->getComponentDoubleParameter(variable_list[1].id));
  ASSERT_EQ(std::get<uint64_t>(variable_list[2].value),
            this->plugin->getComponentUIntParameter(variable_list[2].id));
}

TEST_F(ModulePluginTests, getName)
{
  ASSERT_EQ(this->plugin->getName(), this->component_ptr->getName());
}

TEST_F(ModulePluginTests, activity)
{
  ASSERT_EQ(this->plugin->getActive(), this->component_ptr->getActive());
  int result = this->plugin->setActive(/*state=*/false);
  EXPECT_EQ(result, 0);
  ASSERT_FALSE(this->component_ptr->getActive());
  result = this->plugin->setActive(/*state=*/true);
  EXPECT_EQ(result, 0);
  ASSERT_TRUE(this->component_ptr->getActive());
}

TEST_F(ModulePluginTests, receiveEvent) {}

TEST_F(ModulePanelTests, getLayout) {}

TEST_F(ModulePanelTests, update) {}

TEST_F(ModulePanelTests, createGUI) {}

TEST_F(ModuleManagerTests, loadingPlugin) {}

TEST_F(ModuleManagerTests, moduleRegistration) {}

TEST_F(ModuleManagerTests, receiveEvent) {}

TEST_F(ModuleComponetTests, integration) {}

TEST_F(ModulePanelTests, integration) {}

TEST_F(ModulePluginTests, integration) {}

TEST_F(ModuleManagerTests, integration) {}
