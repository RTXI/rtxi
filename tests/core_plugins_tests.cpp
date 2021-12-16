/*
 	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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

#include <typeinfo>
#include <numeric>
#include <cmath>
#include <core_plugins_tests.h>

TEST_F(PerformanceMeasurementPluginTests, pluginLoad)
{
    // TODO: decouple PerformanceMeasurement::Plugin loading from QT gui framework

    Plugin::Object *testobj = new Plugin::Object();
    ASSERT_EQ(typeid(plugin).name(), typeid(testobj).name());
    delete testobj;
}

TEST_F(PerformanceMeasurementPluginTests, getInstance)
{
    // TODO: decouple PerformanceMeasurement::Plugin class from qt gui framework.
    // TODO: change PerformanceMeasurement::Plugin::getInstance function to virutal for late linkage 
    //auto testplugin = reinterpret_cast<PerformanceMeasurement::Plugin *>(plugin); 
    //ASSERT_EQ(testplugin, PerformanceMeasurement::Plugin::getInstance());
    //ASSERT_EQ(testplugin, testplugin->getInstance());
}

TEST_F(RunningStatTests, numValues)
{
    ASSERT_EQ(statobj->numValues(), 0);
}

TEST_F(RunningStatTests, push)
{
    for(int i=0; i<5; ++i)
    {
        statobj->push(i);
        ASSERT_EQ(statobj->numValues(), i+1);
    }
}

TEST_F(RunningStatTests, mean)
{
    for(auto iter=randnums.begin(); iter!=randnums.end(); ++iter)
        statobj->push(*iter);
    double init=0.0;
    ASSERT_DOUBLE_EQ(std::accumulate(randnums.begin(), randnums.end(), init)/100.0, statobj->mean());
}

TEST_F(RunningStatTests, var)
{
    double squares = 0.0;
    double init = 0.0;
    double mean = std::accumulate(randnums.begin(), randnums.end(), init)/100.0;
    for(auto iter=randnums.begin(); iter!=randnums.end(); ++iter)
    {
        statobj->push(*iter);
        squares += pow(*iter - mean, 2);
    }
    ASSERT_DOUBLE_EQ(squares/(100.0), statobj->var());
}

TEST_F(RunningStatTests, std)
{
    double squares = 0.0;
    double init = 0.0;
    double mean = std::accumulate(randnums.begin(), randnums.end(), init)/100.0;
    for(auto iter=randnums.begin(); iter!=randnums.end(); ++iter)
    {
        statobj->push(*iter);
        squares += pow(*iter - mean, 2);
    }
    ASSERT_DOUBLE_EQ(sqrt(squares/100.0), statobj->std());
}

TEST_F(RunningStatTests, clear)
{ 
    for(auto iter=randnums.begin(); iter!=randnums.end(); ++iter)
    {
        statobj->push(*iter);
    }
    ASSERT_EQ(100, statobj->numValues());
    statobj->clear();
    ASSERT_EQ(0, statobj->numValues());
}

TEST_F(ConnectorPluginTests, loadPlugin)
{
    // TODO: decouple plugin loading from QT gui framework for unit testing

    Plugin::Object *testobj = new Plugin::Object();
    ASSERT_EQ(typeid(plugin).name(), typeid(testobj).name());
    delete testobj;
}

TEST_F(ConnectorPluginTests, getInstance)
{
    // TODO: decouple getInstance function from QT gui framework for unit testing
    // TODO: change Connector::Plugin::getInstance to virtual for late linkage
}
