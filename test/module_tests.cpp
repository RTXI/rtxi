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

#include "module_tests.hpp"

std::vector<Modules::Variable::Info> generateDefaultComponentVariables()
{
    std::vector<Modules::Variable::Info> default_variable_list;
    Modules::Variable::Info tempvarinfo;
    tempvarinfo.name = std::string("testint");
    tempvarinfo.description = std::string("TEST INTEGER");
    tempvarinfo.vartype = Modules::Variable::INT_PARAMETER;
    tempvarinfo.value = 0;
    default_variable_list.push_back(tempvarinfo);

    tempvarinfo.name = std::string("testdouble");
    tempvarinfo.description = std::string("TEST DOUBLE");
    tempvarinfo.vartype = Modules::Variable::DOUBLE_PARAMETER;
    tempvarinfo.value = 0.0;
    default_variable_list.push_back(tempvarinfo);

    tempvarinfo.name = std::string("testuint");
    tempvarinfo.description = std::string("TEST UNSIGNED INTEGER");
    tempvarinfo.vartype = Modules::Variable::UINT_PARAMETER;
    tempvarinfo.value = 0ul;
    default_variable_list.push_back(tempvarinfo);

    tempvarinfo.name = std::string("teststring");
    tempvarinfo.description = std::string("TEST STRING");
    tempvarinfo.vartype = Modules::Variable::COMMENT;
    tempvarinfo.value = std::string("");
    default_variable_list.push_back(tempvarinfo);

    tempvarinfo.name = std::string("teststate");
    tempvarinfo.description = std::string("TEST STATE");
    tempvarinfo.vartype = Modules::Variable::STATE;
    tempvarinfo.value = Modules::Variable::INIT;
    default_variable_list.push_back(tempvarinfo);

    return default_variable_list;
}

TEST_F(ModulePluginTests, attachComponent)
{
  // mocking unique pointers can be tricky
  auto component = std::make_unique<mockModuleComponent>();
  // Unfortunately gmock is not capable of capturing calls to 
  // mocked functions from another thread. Therefore we are not able
  // to use EXPECT_CALL on execute as it would not be caputured
  // in the test thread even though it is definitely called by
  // the rt system thread.
  // TODO: Test execute is being called at all!
  //EXPECT_CALL(*component, execute());
  this->plugin->attachComponent(std::move(component));

}

TEST_F(ModulePluginTests, exit)
{

}

TEST_F(ModulePluginTests, ComponentParameters)
{

}

TEST_F(ModulePluginTests, getName)
{

}

TEST_F(ModulePluginTests, activity)
{

}

TEST_F(ModulePluginTests, receiveEvent)
{

}

TEST_F(ModuleComponetTests, value)
{

}


TEST_F(ModuleComponetTests, getDescription)
{

}

TEST_F(ModuleComponetTests, getValueString)
{

}

TEST_F(ModuleComponetTests, execute)
{

}

TEST_F(ModuleComponetTests, active)
{

}

TEST_F(ModulePanelTests, getLayout)
{

}

TEST_F(ModulePanelTests, update)
{

}

TEST_F(ModulePanelTests, createGUI)
{

}

TEST_F(ModuleManagerTests, loadingPlugin)
{

}

TEST_F(ModuleManagerTests, moduleRegistration)
{

}

TEST_F(ModuleManagerTests, receiveEvent)
{

}

TEST_F(ModuleComponetTests, integration)
{

}

TEST_F(ModulePanelTests, integration)
{

}

TEST_F(ModulePluginTests, integration)
{

}

TEST_F(ModuleManagerTests, integration)
{
  
}