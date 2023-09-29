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

#ifndef SYSTEM_CONTROL_H
#define SYSTEM_CONTROL_H

#include <utility>

#include <QComboBox>

#include "daq.hpp"
#include "event.hpp"
#include "main_window.hpp"
#include "module.hpp"

Q_DECLARE_METATYPE(DAQ::ChannelType::type_t)
Q_DECLARE_METATYPE(DAQ::Device*)
Q_DECLARE_METATYPE(DAQ::direction_t)
Q_DECLARE_METATYPE(DAQ::Reference::reference_t)
Q_DECLARE_METATYPE(DAQ::analog_range_t)

namespace SystemControl
{

constexpr std::string_view MODULE_NAME = "Control Panel";

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(QMainWindow* mw, Event::Manager* ev_manager);

  void receiveEvent(const Event::Object* event);

public slots:
  void apply();
  void display();
  void displayAnalogGroup();
  void displayDigitalGroup();
  void updateDevice();
  void updateFreq();
  void updatePeriod();

private:
  void buildDAQDeviceList();

  QGroupBox* deviceGroup = nullptr;
  QGroupBox* analogGroup = nullptr;
  QGroupBox* digitalGroup = nullptr;
  QGroupBox* buttonGroup = nullptr;

  QMdiSubWindow* subWindow = nullptr;

  QComboBox* deviceList = nullptr;
  QComboBox* analogChannelList = nullptr;
  QComboBox* analogRangeList = nullptr;
  QComboBox* analogDownsampleList = nullptr;
  QComboBox* analogReferenceList = nullptr;
  QComboBox* analogSubdeviceList = nullptr;
  QComboBox* analogUnitPrefixList = nullptr;
  QComboBox* analogUnitList = nullptr;
  QComboBox* analogUnitPrefixList2 = nullptr;
  QComboBox* analogUnitList2 = nullptr;
  QLineEdit* analogGainEdit = nullptr;
  QLineEdit* analogZeroOffsetEdit = nullptr;
  QPushButton* analogActiveButton = nullptr;
  QPushButton* analogCalibrationButton = nullptr;

  QComboBox* digitalChannelList = nullptr;
  QComboBox* digitalDirectionList = nullptr;
  QComboBox* digitalSubdeviceList = nullptr;
  QPushButton* digitalActiveButton = nullptr;

  bool rateUpdate;
  QComboBox* freqUnitList = nullptr;
  QComboBox* periodUnitList = nullptr;
  QLineEdit* freqEdit = nullptr;
  QLineEdit* periodEdit = nullptr;
};

class Plugin : public Modules::Plugin
{
public:
  explicit Plugin(Event::Manager* ev_manager)
      : Modules::Plugin(ev_manager, std::string(MODULE_NAME))
  {
  }
};

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Modules::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(
    Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();

}  // namespace SystemControl
#endif /* SYSTEM_CONTROL_H */
