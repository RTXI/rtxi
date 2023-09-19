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

/*
         Oscilloscope namespace. The namespace works with
         both Scope and Panel classes to instantiate an oscilloscope
         plugin.
 */

#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QComboBox>

#include "event.hpp"
#include "fifo.hpp"
#include "io.hpp"
#include "module.hpp"
#include "rt.hpp"
#include "scope.h"

namespace Oscilloscope
{

constexpr std::string_view MODULE_NAME = "Oscilloscope";

enum PARAMETER : size_t
{
  TRIGGERING = 0,
};

inline std::vector<Modules::Variable::Info> get_default_vars()
{
  return {{PARAMETER::TRIGGERING,
           "Trigger State",
           "Trigger activity for the oscilloscope",
           Modules::Variable::STATE,
           RT::State::INIT}};
}

inline std::vector<IO::channel_t> get_default_channels()
{
  return {{"Probing Channel",
           "This is the channel used by the osciloscope to probe on other "
           "inputs and "
           "output ports",
           IO::INPUT}};
}

class Component : public Modules::Component
{
public:
  // We are forced to have a default constructor for Oscilloscope probes
  // if we wish to be able to have them as values in a c++ standard map
  Component(Modules::Plugin* hplugin, const std::string& probe_name);
  void flushFifo();
  RT::OS::Fifo* getFifoPtr() { return this->fifo.get(); }
  void execute() override;

private:
  std::unique_ptr<RT::OS::Fifo> fifo;
};

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(QMainWindow* mw, Event::Manager* ev_manager);

  void setActivity(IO::endpoint endpoint, bool activity);
  void adjustDataSize();
  void updateTrigger();

signals:
  void updateBlockInfo();
  void updateBlockChannels(IO::Block* block);

public slots:
  void togglePause();

private slots:
  void syncBlockInfo();
  void showChannelTab();
  void showDisplayTab();
  void buildChannelList();
  void screenshot();
  void apply();
  void showTab(int);
  void activateChannel(bool);
  void removeBlockChannels(IO::Block* block);

private:
  void buildBlockList();
  void enableChannel();
  void disableChannel();

  // some utility functions
  void updateChannelLabel(IO::endpoint probe_info);
  void updateChannelScale(IO::endpoint probe_info);
  void updateChannelOffset(IO::endpoint probe_info);
  void updateChannelPen(IO::endpoint endpoint);
  void updateWindowTimeDiv();

  // Tab Widget
  QTabWidget* tabWidget = nullptr;

  // Create scope
  Scope* scopeWindow = nullptr;

  // Create curve element
  QwtPlotCurve* curve = nullptr;

  // Functions to initialize and
  // apply changes made in tabs
  void applyChannelTab();
  void applyDisplayTab();
  QWidget* createChannelTab(QWidget* parent);
  QWidget* createDisplayTab(QWidget* parent);

  // Group and layout information
  QVBoxLayout* layout = nullptr;
  QWidget* scopeGroup = nullptr;
  QGroupBox* setBttnGroup = nullptr;

  // Properties
  // QSpinBox* ratesSpin=nullptr;
  QLineEdit* sizesEdit = nullptr;
  QButtonGroup* trigsGroup = nullptr;
  QComboBox* timesList = nullptr;
  QComboBox* trigsChanList = nullptr;
  QComboBox* trigsThreshList = nullptr;
  QComboBox* refreshDropdown = nullptr;
  QLineEdit* trigsThreshEdit = nullptr;
  QLineEdit* trigWindowEdit = nullptr;
  QComboBox* trigWindowList = nullptr;

  // Lists
  QComboBox* blocksListDropdown = nullptr;
  QComboBox* typesList = nullptr;
  QComboBox* channelsList = nullptr;
  QComboBox* colorsList = nullptr;
  QComboBox* offsetsList = nullptr;
  QComboBox* scalesList = nullptr;
  QComboBox* stylesList = nullptr;
  QComboBox* widthsList = nullptr;
  QLineEdit* offsetsEdit = nullptr;

  // Buttons
  QPushButton* pauseButton = nullptr;
  QPushButton* settingsButton = nullptr;
  QPushButton* applyButton = nullptr;
  QPushButton* activateButton = nullptr;

};  // Panel

class Plugin : public Modules::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager);
  Plugin(const Plugin&) = delete;
  Plugin(Plugin&&) = delete;
  Plugin& operator=(const Plugin&) = delete;
  Plugin& operator=(Plugin&&) = delete;
  ~Plugin() override;

  void receiveEvent(Event::Object* event) override;
  RT::OS::Fifo* createProbe(IO::endpoint probe_info);
  void deleteProbe(IO::endpoint probe_info);
  void deleteAllProbes(IO::Block* block);
  Oscilloscope::Trigger::Info getTriggerInfo() { return this->trigger_info; }
  void setProbeActivity(IO::endpoint endpoint, bool activity);
  std::vector<IO::endpoint> getTrackedEndpoints();
  void setAllProbesActivity(bool activity);
  Oscilloscope::Component* getProbeComponentPtr(IO::endpoint endpoint);

private:
  struct registry_entry_t
  {
    IO::endpoint endpoint;
    std::unique_ptr<Oscilloscope::Component> component;
  };
  // List to maintain multiple scopes
  std::vector<registry_entry_t> m_component_registry;
  Trigger::Info trigger_info;
};  // Plugin

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Modules::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(
    Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();
} // namespace Oscilloscope

#endif  // OSCILLOSCOPE_H
