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

class Panel : public Modules::Panel 
{
  Q_OBJECT

public:
  Panel(QWidget* = NULL);
  virtual ~Panel();
  void execute();
  bool setInactiveSync();
  void flushFifo();
  void adjustDataSize();
  //void doDeferred(const Settings::Object::State&);
  //void doLoad(const Settings::Object::State&);
  //void doSave(Settings::Object::State&) const;
  void receiveEvent(const ::Event::Object*);

public slots:
  void timeoutEvent();
  void togglePause();

protected:
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
  QComboBox* blocksList;
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

  std::unique_ptr<RT::OS::Fifo> fifo;
  std::vector<IO::Block*> blocks;
  size_t counter;
  size_t downsample_rate;
};  // Panel

class Plugin : public Modules::Plugin
{
private:
  // List to maintain multiple scopes
  std::list<std::unique_ptr<Oscilloscope::Panel>> panelList;
};  // Plugin


};  // namespace Oscilloscope

#endif  // OSCILLOSCOPE_H
