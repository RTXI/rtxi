/*
 	 The Real-Time eXperiment Interface (RTXI)
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Will Cornell Medical College

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

class DataRecorderPluginTests : public ::testing::Test
{
protected:
    DataRecorderPluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/data_recorder/.libs/data_recorder.so";
        plugin = manager->load(libraryPath);
    }
    ~DataRecorderPluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;

};

class ModelLoaderPluginTests : public ::testing::Test
{
protected:
    ModelLoaderPluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/model_loader/.libs/model_loader.so";
        plugin = manager->load(libraryPath);
    }
    ~ModelLoaderPluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;

};


class ModuleInstallerPluginTests : public ::testing::Test
{
protected:
    ModuleInstallerPluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/module_installer/.libs/module_installer.so";
        plugin = manager->load(libraryPath);
    }
    ~ModuleInstallerPluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;

};

class OscilloscopePluginTests : public ::testing::Test
{
protected:
    OscilloscopePluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/oscilloscope/.libs/oscilloscope.so";
        plugin = manager->load(libraryPath);
    }
    ~OscilloscopePluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;

};

class SystemControlPluginTests : public ::testing::Test
{
protected:
    SystemControlPluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/system_control/.libs/system_control.so";
        plugin = manager->load(libraryPath);
    }
    ~SystemControlPluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;

};

class UserPreferencesPluginTests : public ::testing::Test
{
protected:
    UserPreferencesPluginTests() { 
        manager = Plugin::Manager::getInstance();
        QString libraryPath(std::filesystem::current_path().string().c_str());
        libraryPath += "/../plugins/userprefs/.libs/userprefs.so";
        plugin = manager->load(libraryPath);
    }
    ~UserPreferencesPluginTests() { }

    Plugin::Object *plugin;
    Plugin::Manager *manager;

};


#endif
