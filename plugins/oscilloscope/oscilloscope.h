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

#include "fifo.hpp"
#include "io.hpp"
#include "rt.hpp"
#include "event.hpp"
#include "module.hpp"

#include "scope.h"

namespace Oscilloscope
{

constexpr std::string_view MODULE_NAME = "Oscilloscope";

enum PARAMETER : size_t {
  STATE = 0,
  TRIGGERING,
};

const std::vector<Modules::Variable::Info> oscilloscope_vars
{
  {
    PARAMETER::STATE,
    "Oscilloscope Probe State",
    "State of the probing component within the oscilloscope",
    Modules::Variable::STATE,
    Modules::Variable::INIT
  },
  {
    PARAMETER::TRIGGERING,
    "Trigger State",
    "Trigger activity for the oscilloscope",
    Modules::Variable::STATE,
    Modules::Variable::INIT
  }
};

namespace Trigger{
enum trig_t : int
{
  NONE = 0,
  POS,
  NEG,
};

typedef struct Info {
  IO::Block* block;
  size_t port;
  IO::flags_t io_direction;
  Trigger::trig_t trigger_direction;
  double threshold;
}Info;
}; // namespace Trigger

class Component;

typedef struct channel_info
{
  QString name;
  Oscilloscope::probe probe;
  Oscilloscope::Component* measuring_component;
  RT::OS::Fifo* fifo;
} channel_info;  // channel_info

class Component : public Modules::Component
{
public:
  Component();
  void flushFifo();

private:
  void callback();
  std::unique_ptr<RT::OS::Fifo> fifo;
};

class Panel : public Modules::Panel 
{
  Q_OBJECT

public:
  Panel(MainWindow* mw, Event::Manager* event_manager);

  //bool setInactiveSync();
  void flushFifo();
  void setActivity(Oscilloscope::Component* comp, bool activity);
  void adjustDataSize();
  void updateTrigger();
  //void doDeferred(const Settings::Object::State&);
  //void doLoad(const Settings::Object::State&);
  //void doSave(Settings::Object::State&) const;

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
  QMdiSubWindow* subWindow=nullptr;

  // Tab Widget
  QTabWidget* tabWidget=nullptr;

  // Create scope
  Scope* scopeWindow=nullptr;

  // Create curve element
  QwtPlotCurve* curve=nullptr;

  // Functions to initialize and
  // apply changes made in tabs
  void applyChannelTab();
  void applyDisplayTab();
  QWidget* createChannelTab(QWidget* parent);
  QWidget* createDisplayTab(QWidget* parent);

  // Group and layout information
  QVBoxLayout* layout=nullptr;
  QWidget* scopeGroup=nullptr;
  QGroupBox* setBttnGroup=nullptr;

  // Properties
  //QSpinBox* ratesSpin=nullptr;
  QLineEdit* sizesEdit=nullptr;
  QButtonGroup* trigsGroup=nullptr;
  QComboBox* timesList=nullptr;
  QComboBox* trigsChanList=nullptr;
  QComboBox* trigsThreshList=nullptr;
  QComboBox* refreshDropdown=nullptr;
  QLineEdit* trigsThreshEdit=nullptr;
  QLineEdit* trigWindowEdit=nullptr;
  QComboBox* trigWindowList=nullptr;

  // Lists
  QComboBox* blocksListDropdown=nullptr;
  QComboBox* typesList=nullptr;
  QComboBox* channelsList=nullptr;
  QComboBox* colorsList=nullptr;
  QComboBox* offsetsList=nullptr;
  QComboBox* scalesList=nullptr;
  QComboBox* stylesList=nullptr;
  QComboBox* widthsList=nullptr;
  QLineEdit* offsetsEdit=nullptr;

  // Buttons
  QPushButton* pauseButton=nullptr;
  QPushButton* settingsButton=nullptr;
  QPushButton* applyButton=nullptr;
  QPushButton* activateButton=nullptr;

  std::vector<IO::Block*> blocks;
  size_t counter;
  //size_t downsample_rate;
};  // Panel

class Plugin : public Modules::Plugin
{
public:
  Plugin(Event::Manager* ev_manager, MainWindow* main_window);
  ~Plugin() override;

  void receiveEvent(Event::Object* event) override;
  Oscilloscope::Component* getProbeComponent(probe probeInfo);

  std::vector<Oscilloscope::channel_info> getChannelsList(){ return this->chanInfoList; }
  void addProbe(channel_info chaninfo);
  void removeProbe(channel_info chaninfo);
  Oscilloscope::Trigger::Info getTriggerInfo(){ return this->trigger_info; }

private:
  // List to maintain multiple scopes
  std::list<Oscilloscope::Component> componentList;
  std::vector<channel_info> chanInfoList;
  Trigger::Info trigger_info;
};  // Plugin

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager, MainWindow* main_window);

Modules::Panel* createRTXIPanel(MainWindow* main_window, Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();
};  // namespace Oscilloscope

#endif  // OSCILLOSCOPE_H
