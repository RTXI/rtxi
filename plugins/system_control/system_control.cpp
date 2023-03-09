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

//#include <math.h>

#include <memory>

#include "system_control.h"

#include "daq.hpp"
#include "debug.hpp"
#include "main_window.hpp"
#include "rt.hpp"

// struct find_daq_t
//{
//   int index;
//   DAQ::Device* device;
// };

// static void findDAQDevice(DAQ::Device* dev, void* arg)
//{
//   struct find_daq_t* info = static_cast<struct find_daq_t*>(arg);
//   if (!info->index){
//     info->device = dev;
//   }
//   info->index--;
// }

// static void buildDAQDeviceList(DAQ::Device* dev, void* arg)
//{
//   QComboBox* deviceList = static_cast<QComboBox*>(arg);
//   deviceList->addItem(QString::fromStdString(dev->getName()));
// }

SystemControl::Panel::Panel(MainWindow* mw, Event::Manager* ev_manager)
    : Modules::Panel(std::string(SystemControl::MODULE_NAME), mw, ev_manager)
{
  setWhatsThis(
      "<p><b>System Control Panel:</b><br>This control panel allows you to "
      "configure "
      "the channels on your DAQ card. RTXI automatically detects the number "
      "and types "
      "of channels that are available. You should set the \"Scale\" of the "
      "channel "
      "to be the inverse of the gain that is applied to the signal through the "
      "combination of hardware and software that you are using. To acquire "
      "data from "
      "a channel, you must set it to be Active using the toggle button. Any "
      "parameter "
      "settings such as the \"Range\" or \"Scale\" must be set by clicking the "
      "\"Apply\" "
      "button.<br><br> The \"Thread\" tab allows you to set the period for "
      "real-time "
      "execution. You must click \"Apply\" to change this setting and "
      "propagate it "
      "to other user modules such as the Data Recorder. Custom user modules "
      "can "
      "execute special code when the real-time period is changed using the "
      "update(PERIOD) "
      "flag.</p>");
  rateUpdate = false;

  // Make Mdi
  subWindow = new QMdiSubWindow;
  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  mw->createMdi(subWindow);

  // Create main layout
  auto* layout = new QGridLayout;

  // Create child widget and layout for device block
  deviceGroup = new QGroupBox(tr("DAQ Setup"));
  auto* deviceLayout = new QGridLayout;

  // Create elements for device block
  deviceLayout->addWidget(new QLabel(tr("Device:")), 0, 0);

  deviceList = new QComboBox;
  deviceLayout->addWidget(deviceList, 0, 1, 1, 5);
  // DAQ::Manager::getInstance()->foreachDevice(buildDAQDeviceList, deviceList);
  QObject::connect(
      deviceList, SIGNAL(activated(int)), this, SLOT(updateDevice(void)));

  // Frequency box
  deviceLayout->addWidget(new QLabel(tr("Frequency:")), 1, 0);
  freqEdit = new QLineEdit;
  deviceLayout->addWidget(freqEdit, 1, 1);
  freqEdit->setText("1000");
  QObject::connect(freqEdit,
                   SIGNAL(textChanged(const QString&)),
                   this,
                   SLOT(updatePeriod(void)));
  freqUnitList = new QComboBox;
  freqUnitList->setFixedWidth(50);
  freqUnitList->addItem(" Hz");
  freqUnitList->addItem("kHz");
  deviceLayout->addWidget(freqUnitList, 1, 2);
  QObject::connect(
      freqUnitList, SIGNAL(activated(int)), this, SLOT(updatePeriod(void)));

  // Period box
  deviceLayout->addWidget(new QLabel(tr("Period:")), 1, 3);
  periodEdit = new QLineEdit;
  deviceLayout->addWidget(periodEdit, 1, 4);
  QObject::connect(periodEdit,
                   SIGNAL(textChanged(const QString&)),
                   this,
                   SLOT(updateFreq(void)));
  periodUnitList = new QComboBox;
  deviceLayout->addWidget(periodUnitList, 1, 5);
  periodUnitList->setFixedWidth(50);
  periodUnitList->addItem(" s");
  periodUnitList->addItem("ms");
  periodUnitList->addItem("us");
  periodUnitList->addItem("ns");
  QObject::connect(
      periodUnitList, SIGNAL(activated(int)), this, SLOT(updateFreq(void)));
  updatePeriod();

  // Assign layout to child widget
  deviceGroup->setLayout(deviceLayout);

  // Create child widget and layout for Analog block
  analogGroup = new QGroupBox(tr("Analog Channels"));
  QGridLayout* analogLayout = new QGridLayout;

  // Create elements for analog block
  analogLayout->addWidget(new QLabel(tr("Channel:")), 1, 0);

  analogSubdeviceList = new QComboBox;
  analogSubdeviceList->addItem("Input");
  analogSubdeviceList->addItem("Output");
  QObject::connect(analogSubdeviceList,
                   SIGNAL(activated(int)),
                   this,
                   SLOT(updateDevice(void)));
  analogLayout->addWidget(analogSubdeviceList, 1, 1);

  analogChannelList = new QComboBox;
  QObject::connect(
      analogChannelList, SIGNAL(activated(int)), this, SLOT(display(void)));
  analogLayout->addWidget(analogChannelList, 1, 2);

  analogActiveButton = new QPushButton("Active");
  analogActiveButton->setCheckable(true);
  analogLayout->addWidget(analogActiveButton, 1, 3);

  analogLayout->addWidget(new QLabel(tr("Range:")), 2, 0, 1, 1);
  analogRangeList = new QComboBox;
  analogLayout->addWidget(analogRangeList, 2, 1, 1, 2);

  analogReferenceList = new QComboBox;
  analogLayout->addWidget(analogReferenceList, 2, 3, 1, 2);

  analogLayout->addWidget(new QLabel(tr("Scale:")), 3, 0);
  analogGainEdit = new QLineEdit;
  analogGainEdit->setAlignment(Qt::AlignRight);
  analogLayout->addWidget(analogGainEdit, 3, 1);

  analogUnitPrefixList = new QComboBox;
  analogUnitPrefixList->addItem("yotta-");
  analogUnitPrefixList->addItem("zetta-");
  analogUnitPrefixList->addItem("exa-");
  analogUnitPrefixList->addItem("peta-");
  analogUnitPrefixList->addItem("tera-");
  analogUnitPrefixList->addItem("giga-");
  analogUnitPrefixList->addItem("mega-");
  analogUnitPrefixList->addItem("kilo-");
  analogUnitPrefixList->addItem("");
  analogUnitPrefixList->addItem("milli-");
  analogUnitPrefixList->addItem("micro-");
  analogUnitPrefixList->addItem("nano-");
  analogUnitPrefixList->addItem("pico-");
  analogUnitPrefixList->addItem("femto-");
  analogUnitPrefixList->addItem("atto-");
  analogUnitPrefixList->addItem("zepto-");
  analogUnitPrefixList->addItem("yocto-");
  analogLayout->addWidget(analogUnitPrefixList, 3, 2);

  analogUnitList = new QComboBox;
  analogLayout->addWidget(analogUnitList, 3, 3);
  analogLayout->addWidget(new QLabel(tr(" / Volt")), 3, 4);

  analogLayout->addWidget(new QLabel(tr("Offset:")), 4, 0);
  analogZeroOffsetEdit = new QLineEdit;
  analogZeroOffsetEdit->setText("0");
  analogZeroOffsetEdit->setAlignment(Qt::AlignRight);
  analogLayout->addWidget(analogZeroOffsetEdit, 4, 1);

  // Prefixes for offset
  analogUnitPrefixList2 = new QComboBox;
  analogUnitPrefixList2->addItem("yotta-");
  analogUnitPrefixList2->addItem("zetta-");
  analogUnitPrefixList2->addItem("exa-");
  analogUnitPrefixList2->addItem("peta-");
  analogUnitPrefixList2->addItem("tera-");
  analogUnitPrefixList2->addItem("giga-");
  analogUnitPrefixList2->addItem("mega-");
  analogUnitPrefixList2->addItem("kilo-");
  analogUnitPrefixList2->addItem("");
  analogUnitPrefixList2->addItem("milli-");
  analogUnitPrefixList2->addItem("micro-");
  analogUnitPrefixList2->addItem("nano-");
  analogUnitPrefixList2->addItem("pico-");
  analogUnitPrefixList2->addItem("femto-");
  analogUnitPrefixList2->addItem("atto-");
  analogUnitPrefixList2->addItem("zepto-");
  analogUnitPrefixList2->addItem("yocto-");
  analogLayout->addWidget(analogUnitPrefixList2, 4, 2, 1, 1);
  analogLayout->addWidget(new QLabel(tr(" Volt/Amps")), 4, 3);

  analogLayout->addWidget(new QLabel(tr("Downsample:")), 5, 0);
  analogDownsampleList = new QComboBox;
  analogDownsampleList->addItem("1");
  analogDownsampleList->addItem("2");
  analogDownsampleList->addItem("4");
  analogDownsampleList->addItem("6");
  analogDownsampleList->addItem("8");
  analogDownsampleList->addItem("10");
  analogLayout->addWidget(analogDownsampleList, 5, 1);

  // Assign layout to child widget
  analogGroup->setLayout(analogLayout);

  // Create child widget and layout for digital block
  digitalGroup = new QGroupBox(tr("Digital I/O"));
  QGridLayout* digitalLayout = new QGridLayout;

  // Create elements for digital block
  digitalLayout->addWidget(new QLabel(tr("Channel:")), 1, 0, 1, 1);

  digitalSubdeviceList = new QComboBox;
  digitalSubdeviceList->addItem("I/O");
  QObject::connect(digitalSubdeviceList,
                   SIGNAL(activated(int)),
                   this,
                   SLOT(updateDevice(void)));
  digitalLayout->addWidget(digitalSubdeviceList, 1, 1, 1, 1);

  digitalChannelList = new QComboBox;
  QObject::connect(
      digitalChannelList, SIGNAL(activated(int)), this, SLOT(display(void)));
  digitalLayout->addWidget(digitalChannelList, 1, 2, 1, 1);

  digitalDirectionList = new QComboBox;
  digitalDirectionList->addItem("Input");
  digitalDirectionList->addItem("Output");
  digitalLayout->addWidget(digitalDirectionList, 1, 3, 1, 1);

  digitalActiveButton = new QPushButton("Active");
  digitalActiveButton->setCheckable(true);
  digitalLayout->addWidget(digitalActiveButton, 1, 4, 1, 1);

  // Assign layout to child widget
  digitalGroup->setLayout(digitalLayout);

  // Create child widget
  buttonGroup = new QGroupBox;
  QHBoxLayout* buttonLayout = new QHBoxLayout;

  // Create elements for buttons
  QPushButton* applyButton = new QPushButton("Apply");
  QObject::connect(
      applyButton, SIGNAL(released(void)), this, SLOT(apply(void)));
  buttonLayout->addWidget(applyButton);
  QPushButton* cancelButton = new QPushButton("Close");
  QObject::connect(
      cancelButton, SIGNAL(released(void)), subWindow, SLOT(close()));
  buttonLayout->addWidget(cancelButton);

  // Assign layout to child widget
  buttonGroup->setLayout(buttonLayout);

  // Attach child widget to parent widget
  layout->addWidget(deviceGroup, 1, 0);
  layout->addWidget(analogGroup, 2, 0);
  layout->addWidget(digitalGroup, 3, 0);
  layout->addWidget(buttonGroup, 4, 0);

  // Attach layout to widget
  setLayout(layout);
  setWindowTitle("System Control Panel");

  // Set layout to Mdi
  subWindow->setWidget(this);
  subWindow->setFixedSize(subWindow->minimumSizeHint());

  // Updates settings for device and builds lists of channels
  updateDevice();
  display();
  show();
}

void SystemControl::Panel::apply()
{
  int index = this->deviceList->currentIndex();
  if (index == -1) {
    // Even if there is no valid device we should still change rt period
    double period = periodEdit->text().toDouble();
    period *= pow(10, 3 * (3 - periodUnitList->currentIndex()));
    Event::Object event(Event::Type::RT_PERIOD_EVENT);
    event.setParam("period", std::any(static_cast<int64_t>(period)));
    this->getRTXIEventManager()->postEvent(&event);
    return;
  }

  QString dev_name = deviceList->itemText(index);
  Event::Object device_query_event(Event::DAQ_DEVICE_QUERY_EVENT);
  device_query_event.setParam("name", std::any(dev_name.toStdString()));
  this->getRTXIEventManager()->postEvent(&device_query_event);
  DAQ::Device* dev = nullptr;
  try {
    dev = std::any_cast<DAQ::Device*>(device_query_event.getParam("device"));
  } catch (std::bad_any_cast&) {
    ERROR_MSG("The device {} was not found", dev_name.toStdString());
    return;
  }

  // Make sure we aren't getting a phony device
  if (dev == nullptr) {
    ERROR_MSG("DAQ device manager returned a nullptr for {}",
              dev_name.toStdString());
    return;
  }

  auto a_chan = static_cast<DAQ::index_t>(analogChannelList->currentIndex());
  auto a_type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());
  double a_gain = analogGainEdit->text().toDouble()
      * pow(10, -3 * (analogUnitPrefixList->currentIndex() - 8));
  double a_zerooffset = analogZeroOffsetEdit->text().toDouble()
      * pow(10, -3 * (analogUnitPrefixList2->currentIndex() - 8));

  dev->setChannelActive(a_type, a_chan, analogActiveButton->isChecked());
  dev->setAnalogGain(a_type, a_chan, a_gain);
  dev->setAnalogZeroOffset(a_type, a_chan, a_zerooffset);
  dev->setAnalogRange(
      a_type,
      a_chan,
      static_cast<DAQ::index_t>(analogRangeList->currentIndex()));
  dev->setAnalogReference(
      a_type,
      a_chan,
      static_cast<DAQ::index_t>(analogReferenceList->currentIndex()));
  dev->setAnalogUnits(
      a_type,
      a_chan,
      static_cast<DAQ::index_t>(analogUnitList->currentIndex()));
  dev->setAnalogDownsample(
      a_type,
      a_chan,
      (analogDownsampleList->itemData(analogDownsampleList->currentIndex(),
                                      Qt::DisplayRole))
          .toInt());
  dev->setAnalogCounter(a_type, a_chan);

  auto d_chan = static_cast<DAQ::index_t>(digitalChannelList->currentIndex());
  auto d_type =
      static_cast<DAQ::type_t>(digitalSubdeviceList->currentIndex() + DAQ::DIO);
  auto d_dir =
      static_cast<DAQ::direction_t>(digitalDirectionList->currentIndex());

  // Write digital channel configuration to DAQ
  dev->setChannelActive(d_type, d_chan, digitalActiveButton->isChecked());
  if (d_type == DAQ::DIO) {
    dev->setDigitalDirection(d_chan, d_dir);
  }

  // Apply thread settings
  double period = periodEdit->text().toDouble();
  period *= pow(10, 3 * (3 - periodUnitList->currentIndex()));

  Event::Object event(Event::Type::RT_PERIOD_EVENT);
  event.setParam("period", std::any(static_cast<int64_t>(period)));
  this->getRTXIEventManager()->postEvent(&event);
  display();
}

void SystemControl::Panel::updateDevice()
{
  // DAQ::Device* dev;
  // DAQ::type_t type;
  // {
  //   struct find_daq_t info = {
  //       deviceList->currentIndex(),
  //       0,
  //   };
  //   DAQ::Manager::getInstance()->foreachDevice(findDAQDevice, &info);
  //   dev = info.device;
  // }

  int index = this->deviceList->currentIndex();
  if (index == -1) {
    return;
  }

  QString dev_name = deviceList->itemText(index);
  Event::Object device_query_event(Event::Type::DAQ_DEVICE_QUERY_EVENT);
  device_query_event.setParam("name", std::any(dev_name.toStdString()));
  this->getRTXIEventManager()->postEvent(&device_query_event);
  DAQ::Device* dev = nullptr;
  try {
    dev = std::any_cast<DAQ::Device*>(device_query_event.getParam("device"));
  } catch (std::bad_any_cast&) {
    ERROR_MSG(
        "SystemContrlo::Panel::updateDevice : The device {} was not found",
        dev_name.toStdString());
    return;
  }

  // Make sure we aren't getting a phony device
  if (dev == nullptr) {
    ERROR_MSG(
        "SystemControl::Panel::updateDevice : DAQ device manager returned a "
        "nullptr for {}",
        dev_name.toStdString());
    return;
  }

  analogChannelList->clear();
  digitalChannelList->clear();

  auto type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());
  for (size_t i = 0; i < dev->getChannelCount(type); ++i) {
    analogChannelList->addItem(QString::number(i));
  }

  type =
      static_cast<DAQ::type_t>(digitalSubdeviceList->currentIndex() + DAQ::DIO);
  for (size_t i = 0; i < dev->getChannelCount(type); ++i) {
    digitalChannelList->addItem(QString::number(i));
  }

  display();
}

void SystemControl::Panel::updateFreq()
{
  /* This is to prevent recursive updates, not to provide mutual exclusion */
  if (rateUpdate) {
    return;
  }

  rateUpdate = true;
  int index = 0;

  /* Determine the Period */
  auto period = periodEdit->text().toDouble();
  period *= pow(10, -3 * periodUnitList->currentIndex());
  auto freq = 1 / period;

  if (freq > 1000) {
    freq /= 1000;
    index = 1;
  }

  freqEdit->setText(QString::number(freq));
  freqUnitList->setCurrentIndex(index);
  rateUpdate = false;
}

void SystemControl::Panel::updatePeriod()
{
  if (rateUpdate) {
    return;
  }

  rateUpdate = true;
  int index = 0;

  // Determine the Frequency
  auto freq = freqEdit->text().toDouble();
  if (freqUnitList->currentIndex()) {
    freq *= 1000;
  }

  auto period = 1 / freq;

  while (period < .001 && (index < 4)) {
    period *= 1000;
    index++;
  }

  periodEdit->setText(QString::number(period));
  periodUnitList->setCurrentIndex(index);
  rateUpdate = false;
}

void SystemControl::Panel::display()
{
  // Display channel info
  // DAQ::Device* dev;
  // {
  //   struct find_daq_t info = {
  //       deviceList->currentIndex(),
  //       0,
  //   };
  //   DAQ::Manager::getInstance()->foreachDevice(findDAQDevice, &info);
  //   dev = info.device;
  // }

  int index = this->deviceList->currentIndex();
  if (index == -1) {
    return;
  }

  QString dev_name = deviceList->itemText(index);
  Event::Object device_query_event(Event::Type::DAQ_DEVICE_QUERY_EVENT);
  device_query_event.setParam("name", std::any(dev_name.toStdString()));
  this->getRTXIEventManager()->postEvent(&device_query_event);
  DAQ::Device* dev = nullptr;
  try {
    dev = std::any_cast<DAQ::Device*>(device_query_event.getParam("device"));
  } catch (std::bad_any_cast&) {
    ERROR_MSG("SystemContrlo::Panel::display : The device {} was not found",
              dev_name.toStdString());
    return;
  }

  // Check to make sure DAQ is of the right type
  // if not, disable functions, else set
  if (dev == nullptr) {
    deviceList->setEnabled(false);
    analogSubdeviceList->setEnabled(false);
    digitalSubdeviceList->setEnabled(false);
  }

  if (dev == nullptr || analogChannelList->count() == 0) {
    analogActiveButton->setChecked(false);
    analogActiveButton->setEnabled(false);
    analogActiveButton->setChecked(false);
    analogChannelList->setEnabled(false);
    analogRangeList->setEnabled(false);
    analogDownsampleList->setEnabled(false);
    analogReferenceList->setEnabled(false);
    analogGainEdit->setEnabled(false);
    analogZeroOffsetEdit->setEnabled(false);
    analogUnitPrefixList->setEnabled(false);
    analogUnitPrefixList2->setEnabled(false);
    analogUnitList->setEnabled(false);
  } else {
    auto type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());
    auto chan = static_cast<DAQ::index_t>(analogChannelList->currentIndex());

    // Downsample is only enabled for AI
    if (type == DAQ::AI) {
      analogDownsampleList->setEnabled(true);
    } else {
      analogDownsampleList->setEnabled(false);
    }

    analogActiveButton->setEnabled(true);
    analogChannelList->setEnabled(true);
    analogRangeList->setEnabled(true);
    analogReferenceList->setEnabled(true);
    analogGainEdit->setEnabled(true);
    analogZeroOffsetEdit->setEnabled(true);
    analogUnitPrefixList->setEnabled(true);
    analogUnitPrefixList2->setEnabled(true);
    analogUnitList->setEnabled(true);

    analogRangeList->clear();
    for (size_t i = 0; i < dev->getAnalogRangeCount(type, chan); ++i) {
      analogRangeList->addItem(
          QString::fromStdString(dev->getAnalogRangeString(type, chan, i)));
    }

    analogReferenceList->clear();
    for (size_t i = 0; i < dev->getAnalogReferenceCount(type, chan); ++i) {
      analogReferenceList->addItem(
          QString::fromStdString(dev->getAnalogReferenceString(type, chan, i)));
    }

    analogUnitList->clear();
    for (size_t i = 0; i < dev->getAnalogUnitsCount(type, chan); ++i) {
      analogUnitList->addItem(
          QString::fromStdString(dev->getAnalogUnitsString(type, chan, i)));
    }
    analogActiveButton->setChecked(dev->getChannelActive(type, chan));
    analogRangeList->setCurrentIndex(dev->getAnalogRange(type, chan));
    analogDownsampleList->setCurrentIndex(analogDownsampleList->findData(
        QVariant::fromValue(dev->getAnalogDownsample(type, chan)),
        Qt::DisplayRole));
    analogReferenceList->setCurrentIndex(dev->getAnalogReference(type, chan));
    analogUnitList->setCurrentIndex(dev->getAnalogUnits(type, chan));

    // Determine the correct prefix for analog gain
    int index = 8;
    double tmp;
    bool sign = true;
    if (dev->getAnalogGain(type, chan) < 0.0) {
      sign = false;
    }  // Negative value
    tmp = fabs(dev->getAnalogGain(type, chan));
    while (((tmp >= 1000) && (index > 0)) || ((tmp < 1) && (index < 16))) {
      if (tmp >= 1000) {
        tmp /= 1000;
        index--;
      } else {
        tmp *= 1000;
        index++;
      }
    }
    if (sign) {
      analogGainEdit->setText(QString::number(tmp));
    } else {
      analogGainEdit->setText(QString::number(-tmp));
    }

    // Set gain prefix to computed index
    analogUnitPrefixList->setCurrentIndex(index);

    // Determine the correct prefix for analog offset
    index = 8;
    sign = true;
    if (dev->getAnalogZeroOffset(type, chan) < 0.0) {
      sign = false;
    }  // Negative value
    tmp = fabs(dev->getAnalogZeroOffset(type, chan));
    while (((tmp >= 1000) && (index > 0)) || ((tmp < 1) && (index < 16))) {
      if (tmp >= 1000) {
        tmp /= 1000;
        index--;
      } else {
        tmp *= 1000;
        index++;
      }
    }
    if (sign) {
      analogZeroOffsetEdit->setText(QString::number(tmp));
    } else {
      analogZeroOffsetEdit->setText(QString::number(-tmp));
    }

    // Set offset prefix to computed index
    analogUnitPrefixList2->setCurrentIndex(index);
  }

  if (dev == nullptr || digitalChannelList->count() == 0) {
    digitalActiveButton->setChecked(false);
    digitalActiveButton->setEnabled(false);
    digitalChannelList->setEnabled(false);
    digitalDirectionList->setEnabled(false);
  } else {
    auto type = static_cast<DAQ::type_t>(digitalSubdeviceList->currentIndex()
                                         + DAQ::DIO);
    auto chan = static_cast<DAQ::index_t>(digitalChannelList->currentIndex());

    digitalActiveButton->setEnabled(true);
    digitalChannelList->setEnabled(true);
    if (type == DAQ::DIO) {
      digitalDirectionList->setEnabled(true);
    } else {
      digitalDirectionList->setEnabled(false);
    }

    digitalActiveButton->setChecked(dev->getChannelActive(type, chan));

    if (type == DAQ::DIO) {
      digitalDirectionList->setEnabled(true);
      digitalDirectionList->setCurrentIndex(dev->getDigitalDirection(chan));
    } else {
      digitalDirectionList->setEnabled(false);
    }
  }

  // Display thread info
  index = 3;
  Event::Object get_period_event(Event::Type::RT_GET_PERIOD_EVENT);
  this->getRTXIEventManager()->postEvent(&get_period_event);
  auto tmp = std::any_cast<int64_t>(get_period_event.getParam("period"));
  // auto tmp = static_cast<long long>(wrapped_tmp);
  // long long tmp = RT::System::getInstance()->getPeriod();
  while ((tmp >= 1000) && (index)) {
    tmp /= 1000;
    index--;
  }
  periodEdit->setText(QString::number(static_cast<unsigned long>(tmp)));
  periodUnitList->setCurrentIndex(index);
  updateFreq();
}

std::unique_ptr<Modules::Plugin> SystemControl::createRTXIPlugin(
    Event::Manager* ev_manager, MainWindow* main_window)
{
  return std::make_unique<SystemControl::Plugin>(ev_manager, main_window);
}

Modules::Panel* SystemControl::createRTXIPanel(MainWindow* main_window,
                                               Event::Manager* ev_manager)
{
  return static_cast<Modules::Panel*>(
      new SystemControl::Panel(main_window, ev_manager));
}

std::unique_ptr<Modules::Component> SystemControl::createRTXIComponent(
    Modules::Plugin*)
{
  return std::unique_ptr<Modules::Component>(nullptr);
}

Modules::FactoryMethods SystemControl::getFactories()
{
  Modules::FactoryMethods fact;
  fact.createPanel = &SystemControl::createRTXIPanel;
  fact.createComponent = &SystemControl::createRTXIComponent;
  fact.createPlugin = &SystemControl::createRTXIPlugin;
  return fact;
}

