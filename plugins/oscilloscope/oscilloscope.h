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
  TRIGGERING = 0
};

const std::vector<Modules::Variable::Info> oscilloscope_vars
{
  {
    PARAMETER::TRIGGERING,
    "Trigger State",
    "Trigger activity for the oscilloscope",
    Modules::Variable::STATE,
    Modules::Variable::INIT
  }
};

namespace Trigger{
enum trig_t
{
  NONE,
  POS,
  NEG,
};

typedef struct Info {
  IO::Block* block;
  size_t port;
  Trigger::trig_t direction;
  double threshold;
}Info;
}; // namespace Trigger


typedef struct channel_info
{
  QString name;
  IO::Block* block;
  IO::flags_t type;
  size_t port;
} channel_info;  // channel_info

class Component : public Modules::Component
{
public:
  Component();
  void execute();

private:
  std::unique_ptr<RT::OS::Fifo> fifo;
};

class Panel : public Modules::Panel 
{
  Q_OBJECT

public:
  Panel(QWidget* = NULL);
  virtual ~Panel();
  //bool setInactiveSync();
  void flushFifo();
  void setActivity(Oscilloscope::Component* comp, bool activity);
  void adjustDataSize();
  void updateTrigger();
  void buildBlockList();
  //void doDeferred(const Settings::Object::State&);
  //void doLoad(const Settings::Object::State&);
  //void doSave(Settings::Object::State&) const;

public slots:
  void timeoutEvent();
  void togglePause();

private slots:
  void showChannelTab();
  void showDisplayTab();
  void buildChannelList();
  void screenshot();
  void apply();
  void showTab(int);
  void activateChannel(bool);

private:
  QMdiSubWindow* subWindow;

  // Tab Widget
  QTabWidget* tabWidget;

  // Create scope
  Scope* scopeWindow;

  // Create curve element
  QwtPlotCurve* curve;

  // Functions to initialize and
  // apply changes made in tabs
  void applyChannelTab();
  void applyDisplayTab();
  QWidget* createChannelTab(QWidget* parent);
  QWidget* createDisplayTab(QWidget* parent);

  // Group and layout information
  QVBoxLayout* layout;
  QWidget* scopeGroup;
  QGroupBox* setBttnGroup;

  // Properties
  QSpinBox* ratesSpin;
  QLineEdit* sizesEdit;
  QButtonGroup* trigsGroup;
  QComboBox* timesList;
  QComboBox* trigsChanList;
  QComboBox* trigsThreshList;
  QSpinBox* refreshsSpin;
  QLineEdit* trigsThreshEdit;
  QLineEdit* trigWindowEdit;
  QComboBox* trigWindowList;

  // Lists
  QComboBox* blocksListDropdown;
  QComboBox* typesList;
  QComboBox* channelsList;
  QComboBox* colorsList;
  QComboBox* offsetsList;
  QComboBox* scalesList;
  QComboBox* stylesList;
  QComboBox* widthsList;
  QLineEdit* offsetsEdit;

  // Buttons
  QPushButton* pauseButton;
  QPushButton* settingsButton;
  QPushButton* applyButton;
  QPushButton* activateButton;

  std::vector<IO::Block*> blocks;
  size_t counter;
  size_t downsample_rate;
};  // Panel

class Plugin : public Modules::Plugin
{
public:
  void receiveEvent(Event::Object* event) override;
  Oscilloscope::Component* getProbe(IO::Block* source, size_t port, IO::flags_t type);
  void addProbe(IO::Block* source, size_t port, IO::flags_t direction);
  void removeProbe(IO::Block* source, size_t port, IO::flags_t direction);

private:
  // List to maintain multiple scopes
  std::list<Oscilloscope::Component> componentList;
};  // Plugin


};  // namespace Oscilloscope

#endif  // OSCILLOSCOPE_H
