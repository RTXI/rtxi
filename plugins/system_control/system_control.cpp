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

// #include <math.h>

#include <cmath>
#include <memory>

#include "system_control.h"

#include "daq.hpp"
#include "debug.hpp"
#include "main_window.hpp"
#include "rt.hpp"

SystemControl::Panel::Panel(QMainWindow* mw, Event::Manager* ev_manager)
    : Widgets::Panel(std::string(SystemControl::MODULE_NAME), mw, ev_manager)
    , buttonGroup(new QGroupBox)
    , deviceList(new QComboBox)
    , analogChannelList(new QComboBox)
    , analogRangeList(new QComboBox)
    , analogDownsampleList(new QComboBox)
    , analogReferenceList(new QComboBox)
    , analogSubdeviceList(new QComboBox)
    , analogUnitPrefixList(new QComboBox)
    , analogUnitList(new QComboBox)
    , analogUnitPrefixList2(new QComboBox)
    , analogGainEdit(new QLineEdit)
    , analogZeroOffsetEdit(new QLineEdit)
    , digitalChannelList(new QComboBox)
    , digitalDirectionList(new QComboBox)
    , digitalSubdeviceList(new QComboBox)
    , rateUpdate(false)
    , freqUnitList(new QComboBox)
    , periodUnitList(new QComboBox)
    , freqEdit(new QLineEdit)
    , periodEdit(new QLineEdit)
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
  // Create main layout
  auto* layout = new QGridLayout;

  // Create child widget and layout for device block
  deviceGroup = new QGroupBox(tr("DAQ Setup"));
  auto* deviceLayout = new QGridLayout;

  // Create elements for device block
  deviceLayout->addWidget(new QLabel(tr("Device:")), 0, 0);

  deviceLayout->addWidget(deviceList, 0, 1, 1, 5);
  buildDAQDeviceList();
  QObject::connect(
      deviceList, SIGNAL(activated(int)), this, SLOT(updateDevice()));

  // Frequency box
  deviceLayout->addWidget(new QLabel(tr("Frequency:")), 1, 0);
  deviceLayout->addWidget(freqEdit, 1, 1);

  Event::Object get_period_event(Event::Type::RT_GET_PERIOD_EVENT);
  this->getRTXIEventManager()->postEvent(&get_period_event);
  auto period = std::any_cast<int64_t>(get_period_event.getParam("period"));
  freqEdit->setText(
      std::to_string(RT::OS::SECONDS_TO_NANOSECONDS / period).c_str());

  QObject::connect(
      freqEdit, SIGNAL(textEdited(const QString&)), this, SLOT(updatePeriod()));
  freqUnitList->setFixedWidth(50);
  freqUnitList->addItem(" Hz", 1);
  freqUnitList->addItem("kHz", 1000);
  deviceLayout->addWidget(freqUnitList, 1, 2);
  QObject::connect(
      freqUnitList, SIGNAL(activated(int)), this, SLOT(updatePeriod()));

  // Period box
  deviceLayout->addWidget(new QLabel(tr("Period:")), 1, 3);
  deviceLayout->addWidget(periodEdit, 1, 4);
  QObject::connect(
      periodEdit, SIGNAL(textEdited(const QString&)), this, SLOT(updateFreq()));
  deviceLayout->addWidget(periodUnitList, 1, 5);
  periodUnitList->setFixedWidth(50);
  periodUnitList->addItem(" s", 1.0);
  periodUnitList->addItem("ms", 1e-3);
  periodUnitList->addItem("us", 1e-6);
  periodUnitList->addItem("ns", 1e-9);
  QObject::connect(
      periodUnitList, SIGNAL(activated(int)), this, SLOT(updateFreq()));
  updatePeriod();

  // Assign layout to child widget
  deviceGroup->setLayout(deviceLayout);

  // Create child widget and layout for Analog block
  analogGroup = new QGroupBox(tr("Analog Channels"));
  auto* analogLayout = new QGridLayout;

  // Create elements for analog block
  analogLayout->addWidget(new QLabel(tr("Channel:")), 1, 0);

  analogSubdeviceList->addItem("Input",
                               QVariant::fromValue(DAQ::ChannelType::AI));
  analogSubdeviceList->addItem("Output",
                               QVariant::fromValue(DAQ::ChannelType::AO));
  QObject::connect(
      analogSubdeviceList, SIGNAL(activated(int)), this, SLOT(updateDevice()));
  analogLayout->addWidget(analogSubdeviceList, 1, 1);

  QObject::connect(
      analogChannelList, SIGNAL(activated(int)), this, SLOT(display()));
  analogLayout->addWidget(analogChannelList, 1, 2);

  analogActiveButton = new QPushButton("Active");
  analogActiveButton->setCheckable(true);
  analogLayout->addWidget(analogActiveButton, 1, 3);

  analogLayout->addWidget(new QLabel(tr("Range:")), 2, 0, 1, 1);
  analogLayout->addWidget(analogRangeList, 2, 1, 1, 2);
  analogRangeList->clear();
  const std::string formatting = "{:.1f}";
  std::string range_list_text;
  DAQ::index_t indx = 0;
  for (auto range : DAQ::get_default_ranges()) {
    auto [min, max] = range;
    range_list_text = fmt::format(formatting, min) + std::string(" to ")
        + fmt::format(formatting, max);
    analogRangeList->addItem(QString(range_list_text.c_str()),
                             QVariant::fromValue(indx));
    indx++;
  }

  analogLayout->addWidget(analogReferenceList, 2, 3, 1, 2);
  analogLayout->addWidget(new QLabel(tr("Scale:")), 3, 0);
  analogReferenceList->clear();
  analogReferenceList->addItem(
      "Ground",
      QVariant::fromValue(static_cast<DAQ::index_t>(DAQ::Reference::GROUND)));
  analogReferenceList->addItem(
      "Common",
      QVariant::fromValue(static_cast<DAQ::index_t>(DAQ::Reference::COMMON)));
  analogReferenceList->addItem("Differential",
                               QVariant::fromValue(static_cast<DAQ::index_t>(
                                   DAQ::Reference::DIFFERENTIAL)));
  analogReferenceList->addItem(
      "Other",
      QVariant::fromValue(static_cast<DAQ::index_t>(DAQ::Reference::OTHER)));

  analogGainEdit->setAlignment(Qt::AlignRight);
  analogLayout->addWidget(analogGainEdit, 3, 1);

  analogUnitPrefixList->addItem("yotta-", 1e24);
  analogUnitPrefixList->addItem("zetta-", 1e21);
  analogUnitPrefixList->addItem("exa-", 1e18);
  analogUnitPrefixList->addItem("peta-", 1e15);
  analogUnitPrefixList->addItem("tera-", 1e12);
  analogUnitPrefixList->addItem("giga-", 1e9);
  analogUnitPrefixList->addItem("mega-", 1e6);
  analogUnitPrefixList->addItem("kilo-", 1e3);
  analogUnitPrefixList->addItem("", 1);
  analogUnitPrefixList->addItem("milli-", 1e-3);
  analogUnitPrefixList->addItem("micro-", 1e-6);
  analogUnitPrefixList->addItem("nano-", 1e-9);
  analogUnitPrefixList->addItem("pico-", 1e-12);
  analogUnitPrefixList->addItem("femto-", 1e-15);
  analogUnitPrefixList->addItem("atto-", 1e-18);
  analogUnitPrefixList->addItem("zepto-", 1e-21);
  analogUnitPrefixList->addItem("yocto-", 1e-24);
  analogLayout->addWidget(analogUnitPrefixList, 3, 2);

  analogLayout->addWidget(analogUnitList, 3, 3);
  analogUnitList->clear();
  DAQ::index_t units_index = 0;
  for (const auto& units : DAQ::get_default_units()) {
    analogUnitList->addItem(QString(units.c_str()),
                            QVariant::fromValue(units_index));
    units_index++;
  }
  analogLayout->addWidget(new QLabel(tr(" / Volt")), 3, 4);
  analogLayout->addWidget(new QLabel(tr("Offset:")), 4, 0);
  analogZeroOffsetEdit->setText("0");
  analogZeroOffsetEdit->setAlignment(Qt::AlignRight);
  analogLayout->addWidget(analogZeroOffsetEdit, 4, 1);

  // Prefixes for offset
  analogUnitPrefixList2->addItem("yotta-", 1e24);
  analogUnitPrefixList2->addItem("zetta-", 1e21);
  analogUnitPrefixList2->addItem("exa-", 1e18);
  analogUnitPrefixList2->addItem("peta-", 1e15);
  analogUnitPrefixList2->addItem("tera-", 1e12);
  analogUnitPrefixList2->addItem("giga-", 1e9);
  analogUnitPrefixList2->addItem("mega-", 1e6);
  analogUnitPrefixList2->addItem("kilo-", 1e3);
  analogUnitPrefixList2->addItem("", 1);
  analogUnitPrefixList2->addItem("milli-", 1e-3);
  analogUnitPrefixList2->addItem("micro-", 1e-6);
  analogUnitPrefixList2->addItem("nano-", 1e-9);
  analogUnitPrefixList2->addItem("pico-", 1e-12);
  analogUnitPrefixList2->addItem("femto-", 1e-15);
  analogUnitPrefixList2->addItem("atto-", 1e-18);
  analogUnitPrefixList2->addItem("zepto-", 1e-21);
  analogUnitPrefixList2->addItem("yocto-", 1e-24);

  analogLayout->addWidget(analogUnitPrefixList2, 4, 2, 1, 1);
  analogLayout->addWidget(new QLabel(tr(" Volt/Amps")), 4, 3);
  analogLayout->addWidget(new QLabel(tr("Downsample:")), 5, 0);
  analogDownsampleList->addItem("1", 1);
  analogDownsampleList->addItem("2", 2);
  analogDownsampleList->addItem("4", 4);
  analogDownsampleList->addItem("6", 6);
  analogDownsampleList->addItem("8", 8);
  analogDownsampleList->addItem("10", 10);
  analogLayout->addWidget(analogDownsampleList, 5, 1);

  // Assign layout to child widget
  analogGroup->setLayout(analogLayout);

  // Create child widget and layout for digital block
  digitalGroup = new QGroupBox(tr("Digital I/O"));
  auto* digitalLayout = new QGridLayout;

  // Create elements for digital block
  digitalLayout->addWidget(new QLabel(tr("Channel:")), 1, 0, 1, 1);

  digitalSubdeviceList->addItem("I/O");
  QObject::connect(
      digitalSubdeviceList, SIGNAL(activated(int)), this, SLOT(updateDevice()));
  digitalLayout->addWidget(digitalSubdeviceList, 1, 1, 1, 1);

  QObject::connect(
      digitalChannelList, SIGNAL(activated(int)), this, SLOT(display()));
  digitalLayout->addWidget(digitalChannelList, 1, 2, 1, 1);

  digitalDirectionList->addItem("Input",
                                QVariant::fromValue(DAQ::ChannelType::DI));
  digitalDirectionList->addItem("Output",
                                QVariant::fromValue(DAQ::ChannelType::DO));
  digitalLayout->addWidget(digitalDirectionList, 1, 3, 1, 1);

  digitalActiveButton = new QPushButton("Active");
  digitalActiveButton->setCheckable(true);
  digitalLayout->addWidget(digitalActiveButton, 1, 4, 1, 1);

  // Assign layout to child widget
  digitalGroup->setLayout(digitalLayout);

  // Create child widget
  auto* buttonLayout = new QHBoxLayout;

  // Create elements for buttons
  auto* applyButton = new QPushButton("Apply");
  QObject::connect(applyButton, SIGNAL(released()), this, SLOT(apply()));
  buttonLayout->addWidget(applyButton);
  auto* cancelButton = new QPushButton("Close");
  QObject::connect(
      cancelButton, SIGNAL(released()), parentWidget(), SLOT(close()));
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
  this->getMdiWindow()->setFixedSize(this->minimumSizeHint());

  // Updates settings for device and builds lists of channels
  updateDevice();
  display();
}

void SystemControl::Panel::submitAnalogChannelUpdate()
{
  auto* dev = deviceList->currentData().value<DAQ::Device*>();
  auto a_chan = analogChannelList->currentData().value<DAQ::index_t>();
  auto a_type =
      analogSubdeviceList->currentData().value<DAQ::ChannelType::type_t>();
  const double a_gain = analogGainEdit->text().toDouble()
      * analogUnitPrefixList->currentData().toDouble();
  const double a_zerooffset = analogZeroOffsetEdit->text().toDouble()
      * analogUnitPrefixList2->currentData().toDouble();

  dev->setChannelActive(a_type, a_chan, analogActiveButton->isChecked());
  if (!analogActiveButton->isChecked()) {
    return;
  }
  dev->setAnalogGain(a_type, a_chan, a_gain);
  dev->setAnalogZeroOffset(a_type, a_chan, a_zerooffset);
  dev->setAnalogRange(
      a_type, a_chan, analogRangeList->currentData().value<DAQ::index_t>());
  dev->setAnalogReference(
      a_type, a_chan, analogReferenceList->currentData().value<DAQ::index_t>());
  dev->setAnalogUnits(
      a_type, a_chan, analogUnitList->currentData().value<DAQ::index_t>());
  const int value =
      analogDownsampleList
          ->itemData(analogDownsampleList->currentIndex(), Qt::DisplayRole)
          .toInt();
  dev->setAnalogDownsample(a_type, a_chan, static_cast<size_t>(value));
  dev->setAnalogCounter(a_type, a_chan);
}

void SystemControl::Panel::submitDigitalChannelUpdate()
{
  auto* dev = deviceList->currentData().value<DAQ::Device*>();
  auto d_chan = digitalChannelList->currentData().value<DAQ::index_t>();
  auto d_dir = digitalDirectionList->currentData().value<DAQ::direction_t>();
  auto d_type =
      d_dir == DAQ::INPUT ? DAQ::ChannelType::DI : DAQ::ChannelType::DO;

  dev->setChannelActive(d_type, d_chan, digitalActiveButton->isChecked());
  if (!digitalActiveButton->isChecked()) {
    return;
  }
  if (d_type == DAQ::ChannelType::DI || d_type == DAQ::ChannelType::DO) {
    dev->setDigitalDirection(d_chan, d_dir);
  }
}

void SystemControl::Panel::apply()
{
  int64_t freq = freqEdit->text().toInt();
  freq *= freqUnitList->currentData().toInt();
  int64_t period = RT::OS::SECONDS_TO_NANOSECONDS/freq;
  Event::Object event(Event::Type::RT_PERIOD_EVENT);
  event.setParam("period", std::any(period));
  this->getRTXIEventManager()->postEvent(&event);

  // We don't bother continuing if no valid device info is present/selected
  if (deviceList->count() == 0 || deviceList->currentIndex() < 0) {
    return;
  }

  auto* device = this->deviceList->currentData().value<DAQ::Device*>();
  Event::Object disable_device(Event::Type::RT_DEVICE_PAUSE_EVENT);
  disable_device.setParam("device", std::any(static_cast<RT::Device*>(device)));
  Event::Object enable_device(Event::Type::RT_DEVICE_UNPAUSE_EVENT);
  enable_device.setParam("device", std::any(static_cast<RT::Device*>(device)));

  this->getRTXIEventManager()->postEvent(&disable_device);
  if (analogActiveButton->isEnabled()) {
    this->submitAnalogChannelUpdate();
  }
  if (digitalActiveButton->isEnabled()) {
    this->submitDigitalChannelUpdate();
  }
  this->getRTXIEventManager()->postEvent(&enable_device); 
  // Display changes
  display();
}

void SystemControl::Panel::updateDevice()
{
  if (this->deviceList->currentIndex() < 0) {
    return;
  }

  auto* dev = deviceList->currentData().value<DAQ::Device*>();
  analogChannelList->clear();
  digitalChannelList->clear();

  auto type =
      analogSubdeviceList->currentData().value<DAQ::ChannelType::type_t>();
  for (size_t i = 0; i < dev->getChannelCount(type); ++i) {
    analogChannelList->addItem(
        QString::number(i), QVariant::fromValue(static_cast<DAQ::index_t>(i)));
  }

  type = digitalSubdeviceList->currentData().value<DAQ::ChannelType::type_t>();
  for (size_t i = 0; i < dev->getChannelCount(type); ++i) {
    digitalChannelList->addItem(
        QString::number(i), QVariant::fromValue(static_cast<DAQ::index_t>(i)));
  }

  display();
}

void SystemControl::Panel::updateFreq()
{
  /* Determine the Period */
  auto period = periodEdit->text().toDouble();
  period *= periodUnitList->currentData().toDouble();
  auto freq = 1 / period;
  int index = 0;
  if (freq > 1000) {
    freq /= 1000;
    index = 1;
  }

  freqEdit->setText(QString::number(freq));
  freqUnitList->setCurrentIndex(index);
}

void SystemControl::Panel::updatePeriod()
{
  int index = 0;

  // Determine the Frequency
  auto freq = freqEdit->text().toDouble();
  freq *= freqUnitList->currentData().toDouble();

  auto period = 1 / freq;

  while (period < .001 && (index < 4)) {
    period *= 1000;
    index++;
  }

  periodEdit->setText(QString::number(period));
  periodUnitList->setCurrentIndex(index);
}

void SystemControl::Panel::buildDAQDeviceList()
{
  Event::Object device_list_request(Event::Type::DAQ_DEVICE_QUERY_EVENT);
  this->getRTXIEventManager()->postEvent(&device_list_request);
  auto devices = std::any_cast<std::vector<DAQ::Device*>>(
      device_list_request.getParam("devices"));
  for (auto* device : devices) {
    this->deviceList->addItem(QString(device->getName().c_str()),
                              QVariant::fromValue(device));
  }
}

// TODO: improve simplicity of display function
void SystemControl::Panel::display()
{
  if (deviceList->count() == 0) {
    deviceList->setEnabled(false);
    analogSubdeviceList->setEnabled(false);
    digitalSubdeviceList->setEnabled(false);
  }
  displayAnalogGroup();
  displayDigitalGroup();
  // Display thread info
  int index = 3;
  Event::Object get_period_event(Event::Type::RT_GET_PERIOD_EVENT);
  this->getRTXIEventManager()->postEvent(&get_period_event);
  auto tmp = std::any_cast<int64_t>(get_period_event.getParam("period"));
  while ((tmp >= 1000) && ((index) != 0)) {
    tmp /= 1000;
    index--;
  }
  periodEdit->setText(QString::number(static_cast<int64_t>(tmp)));
  periodUnitList->setCurrentIndex(index);
  updateFreq();
}

void SystemControl::Panel::displayAnalogGroup()
{
  if (deviceList->count() == 0 || deviceList->currentIndex() < 0) {
    return;
  }
  if (analogChannelList->count() == 0 || analogChannelList->currentIndex() < 0)
  {
    analogActiveButton->setChecked(false);
    analogActiveButton->setEnabled(false);
    analogRangeList->setEnabled(false);
    analogDownsampleList->setEnabled(false);
    analogReferenceList->setEnabled(false);
    analogGainEdit->setEnabled(false);
    analogZeroOffsetEdit->setEnabled(false);
    analogUnitPrefixList->setEnabled(false);
    analogUnitPrefixList2->setEnabled(false);
    analogUnitList->setEnabled(false);
    return;
  }
  auto* dev = deviceList->currentData().value<DAQ::Device*>();
  auto type =
      analogSubdeviceList->currentData().value<DAQ::ChannelType::type_t>();
  auto chan = analogChannelList->currentData().value<DAQ::index_t>();

  // Downsample is only enabled for AI
  analogDownsampleList->setEnabled(type == DAQ::ChannelType::AI);
  analogActiveButton->setEnabled(true);
  analogChannelList->setEnabled(true);
  analogRangeList->setEnabled(true);
  analogReferenceList->setEnabled(true);
  analogGainEdit->setEnabled(true);
  analogZeroOffsetEdit->setEnabled(true);
  analogUnitPrefixList->setEnabled(true);
  analogUnitPrefixList2->setEnabled(true);
  analogUnitList->setEnabled(true);

  analogActiveButton->setChecked(dev->getChannelActive(type, chan));
  analogRangeList->setCurrentIndex(
      static_cast<int>(dev->getAnalogRange(type, chan)));
  analogDownsampleList->setCurrentIndex(analogDownsampleList->findData(
      QVariant::fromValue(dev->getAnalogDownsample(type, chan)),
      Qt::DisplayRole));
  analogReferenceList->setCurrentIndex(analogReferenceList->findData(
      QVariant::fromValue(dev->getAnalogReference(type, chan))));
  analogUnitList->setCurrentIndex(analogUnitList->findData(
      QVariant::fromValue(dev->getAnalogUnits(type, chan))));

  // Determine the correct prefix for analog gain
  int indx = 8;
  double tmp = NAN;
  double gain = dev->getAnalogGain(type, chan);
  tmp = fabs(gain);
  while (((tmp >= 1000) && (indx > 0)) || ((tmp < 1) && (indx < 16))) {
    if (tmp >= 1000) {
      tmp /= 1000;
      indx--;
    } else {
      tmp *= 1000;
      indx++;
    }
  }
  analogGainEdit->setText(QString::number(gain));

  // Set gain prefix to computed index
  analogUnitPrefixList->setCurrentIndex(indx);

  // Determine the correct prefix for analog offset
  indx = 8;
  double offset = dev->getAnalogZeroOffset(type, chan);
  tmp = fabs(offset);
  while (((tmp >= 1000) && (indx > 0)) || ((tmp < 1) && (indx < 16))) {
    if (tmp >= 1000) {
      tmp /= 1000;
      indx--;
    } else {
      tmp *= 1000;
      indx++;
    }
  }
  analogZeroOffsetEdit->setText(QString::number(offset));
  // Set offset prefix to computed index
  analogUnitPrefixList2->setCurrentIndex(indx);
}

void SystemControl::Panel::displayDigitalGroup()
{
  if (deviceList->count() == 0 || deviceList->currentIndex() < 0) {
    return;
  }
  if (digitalChannelList->count() == 0) {
    digitalActiveButton->setChecked(false);
    digitalActiveButton->setEnabled(false);
    digitalChannelList->setEnabled(false);
    digitalDirectionList->setEnabled(false);
    return;
  }
  auto* dev = deviceList->currentData().value<DAQ::Device*>();
  auto chan = digitalChannelList->currentData().value<DAQ::index_t>();
  auto direction =
      digitalDirectionList->currentData().value<DAQ::direction_t>();

  digitalActiveButton->setEnabled(true);
  digitalChannelList->setEnabled(true);
  digitalDirectionList->setCurrentIndex(
      digitalDirectionList->findData(QVariant::fromValue(direction)));
  auto type =
      direction == DAQ::INPUT ? DAQ::ChannelType::DI : DAQ::ChannelType::DO;
  digitalActiveButton->setChecked(dev->getChannelActive(type, chan));
}

std::unique_ptr<Widgets::Plugin> SystemControl::createRTXIPlugin(
    Event::Manager* ev_manager)
{
  return std::make_unique<SystemControl::Plugin>(ev_manager);
}

Widgets::Panel* SystemControl::createRTXIPanel(QMainWindow* main_window,
                                               Event::Manager* ev_manager)
{
  return static_cast<Widgets::Panel*>(
      new SystemControl::Panel(main_window, ev_manager));
}

std::unique_ptr<Widgets::Component> SystemControl::createRTXIComponent(
    Widgets::Plugin* /*unused*/)
{
  return {nullptr};
}

Widgets::FactoryMethods SystemControl::getFactories()
{
  Widgets::FactoryMethods fact;
  fact.createPanel = &SystemControl::createRTXIPanel;
  fact.createComponent = &SystemControl::createRTXIComponent;
  fact.createPlugin = &SystemControl::createRTXIPlugin;
  return fact;
}
