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

#ifndef PERFORMANCE_MEASUREMENT_PLUGIN_TESTS_H
#define PERFORMANCE_MEASUREMENT_PLUGIN_TESTS_H

#include <filesystem>
#include <random>
#include <vector>
#include <gtest/gtest.h>
#include <plugin.h>
#include <math/runningstat.h>
#include <performance_measurement/performance_measurement.h>

class PerformanceMeasurementPanelTests : public ::testing::Test
{
protected:
    PerformanceMeasurementPanelTests() { }
    ~PerformanceMeasurementPanelTests() { }
    
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

#endif
