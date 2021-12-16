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

#ifndef CORE_PLUGINS_TESTS_H
#define CORE_PLUGINS_TESTS_H

#include <filesystem>
#include <random>
#include <vector>
#include <plugin.h>
#include <gtest/gtest.h>
#include <math/runningstat.h>
#include <connector/connector.h>
#include <performance_measurement/performance_measurement.h>

class PerformanceMeasurementPluginTests : public ::testing::Test
{
protected:
    PerformanceMeasurementPluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/performance_measurement/.libs/performance_measurement.so";
        plugin = manager->load(libraryPath);
    }
    ~PerformanceMeasurementPluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;
};

class RunningStatTests : public ::testing::Test
{
protected:
    RunningStatTests() { 
        statobj = new RunningStat();
        // generate 100 random numbers between 0 and 100
        std::default_random_engine randgen;
        std::uniform_real_distribution<double> dist(0.0, 100.0);
        for(int i=0; i<100; ++i)
        {
            randnums.push_back(dist(randgen));
        }
    }
    ~RunningStatTests() { delete statobj;}

    RunningStat *statobj;
    std::vector<double> randnums;
};



class ConnectorPluginTests : public ::testing::Test
{
protected:
    ConnectorPluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/connector/.libs/connector.so";
        plugin = manager->load(libraryPath);
    }
    ~ConnectorPluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;
};

#endif
