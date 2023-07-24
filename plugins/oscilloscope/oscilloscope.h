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

#include <QtWidgets>

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
  STATE = 0,
  TRIGGERING,
};

inline std::vector<Modules::Variable::Info> get_default_vars()
{
  return {{PARAMETER::STATE,
           "Oscilloscope Probe State",
           "State of the probing component within the oscilloscope",
           Modules::Variable::STATE,
           Modules::Variable::INIT},
          {PARAMETER::TRIGGERING,
           "Trigger State",
           "Trigger activity for the oscilloscope",
           Modules::Variable::STATE,
           Modules::Variable::INIT}};
}

inline std::vector<IO::channel_t> get_default_channels()
{
  return {{"Probing Channel",
           "This is the channel used by the osciloscope to probe on other "
           "inputs and "
           "output ports",
           IO::INPUT,
           1}};
}

namespace Trigger
{
enum trig_t : int
{
  NONE = 0,
  POS,
  NEG,
};

typedef struct Info
{
  IO::Block* block = nullptr;
  size_t port = 0;
  IO::flags_t io_direction = IO::UNKNOWN;
  Trigger::trig_t trigger_direction = NONE;
  double threshold = 0.0;
} Info;
};  // namespace Trigger

class Component;

typedef struct channel_info
{
  QString name;
  IO::endpoint probe;
  Oscilloscope::Component* measuring_component;
  RT::OS::Fifo* fifo;
} channel_info;  // channel_info

class Component : public Modules::Component
{
public:
  Component(Modules::Plugin* hplugin, const std::string& probe_name);
  void flushFifo();
  RT::OS::Fifo* getFifoPtr() { return this->fifo.get(); }

private:
  void execute() override;
  std::unique_ptr<RT::OS::Fifo> fifo;
};

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(QMainWindow* mw, Event::Manager* ev_manager);

  void setActivity(Oscilloscope::Component* comp, bool activity);
  void adjustDataSize();
  void updateTrigger();
  // void doDeferred(const Settings::Object::State&);
  // void doLoad(const Settings::Object::State&);
  // void doSave(Settings::Object::State&) const;

signals:
  void updateBlockInfo();

public slots:
  void timeoutEvent();
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

private:
  void buildBlockList();
  void enableChannel();
  void disableChannel();

  // some utility functions
  void updateChannelLabel(IO::endpoint probe_info);
  void updateChannelScale(IO::endpoint probe_info);
  void updateChannelOffset(IO::endpoint probe_info);
  void updateChannelLineWidth(IO::endpoint probe_info);
  void updateChannelLineStyle(IO::endpoint probe_info);
  void updateChannelPenColor(IO::endpoint probe_info);

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

  std::vector<IO::Block*> blocks;
  // size_t downsample_rate;
};  // Panel

class Plugin : public Modules::Plugin
{
public:
  Plugin(const Plugin&) = delete;
  Plugin(Plugin&&) = delete;
  Plugin& operator=(const Plugin&) = delete;
  Plugin& operator=(Plugin&&) = delete;
  explicit Plugin(Event::Manager* ev_manager);
  ~Plugin() override;

  void receiveEvent(Event::Object* event) override;
  Oscilloscope::Component* getProbeComponent(IO::endpoint probeInfo);

  std::vector<Oscilloscope::channel_info> getChannelsList()
  {
    return this->chanInfoList;
  }
  bool addProbe(IO::endpoint probe_info);
  void removeProbe(IO::endpoint probe_info);
  Oscilloscope::Trigger::Info getTriggerInfo() { return this->trigger_info; }

private:
  // List to maintain multiple scopes
  std::list<Oscilloscope::Component> componentList;
  std::vector<channel_info> chanInfoList;
  Trigger::Info trigger_info;
};  // Plugin

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Modules::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(
    Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();
};  // namespace Oscilloscope

#endif  // OSCILLOSCOPE_H
