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
         This class creates and controls the drawing parameters
         A control panel is instantiated for all the active channels/modules
         and user selection is enabled to change color, style, width and other
         oscilloscope properties.
 */

#include <QButtonGroup>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QMdiSubWindow>
#include <QPushButton>
#include <QRadioButton>
#include <QTimer>
#include <cmath>

#include "oscilloscope.hpp"

#include <qwt_plot_renderer.h>

#include "debug.hpp"
#include "rt.hpp"
#include "rtos.hpp"
#include "scope.hpp"

void Oscilloscope::Plugin::receiveEvent(Event::Object* event)
{
  auto* module_panel = dynamic_cast<Oscilloscope::Panel*>(this->getPanel());
  switch (event->getType()) {
    case Event::Type::RT_THREAD_INSERT_EVENT:
    case Event::Type::RT_DEVICE_INSERT_EVENT:
      module_panel->updateBlockInfo();
      break;
    case Event::Type::RT_THREAD_REMOVE_EVENT:
      module_panel->updateBlockChannels(
          std::any_cast<RT::Thread*>(event->getParam("thread")));
      module_panel->updateBlockInfo();
      break;
    case Event::Type::RT_DEVICE_REMOVE_EVENT:
      module_panel->updateBlockChannels(
          std::any_cast<RT::Device*>(event->getParam("device")));
      module_panel->updateBlockInfo();
      break;
    default:
      break;
  }
}

void Oscilloscope::Panel::updateChannelScale(IO::endpoint probe_info)
{
  const auto scale = this->scalesList->currentData().value<double>();
  this->scopeWindow->setChannelScale(probe_info, scale);
}

void Oscilloscope::Panel::updateChannelOffset(IO::endpoint probe_info)
{
  const double chanoffset = this->offsetsEdit->text().toDouble()
      * offsetsList->currentData().value<double>();
  this->scopeWindow->setChannelOffset(probe_info, chanoffset);
}

void Oscilloscope::Panel::updateChannelPen(IO::endpoint endpoint)
{
  QPen pen = QPen();
  pen.setColor(this->colorsList->currentData().value<QColor>());
  pen.setWidth(this->widthsList->currentData().value<int>());
  pen.setStyle(this->stylesList->currentData().value<Qt::PenStyle>());
  this->scopeWindow->setChannelPen(endpoint, pen);
}

void Oscilloscope::Panel::updateChannelCurveStyle(IO::endpoint endpoint)
{
  auto curveStyle = this->curveStylesList->currentData().value<QwtPlotCurve::CurveStyle>();
  this->scopeWindow->setChannelCurveStyle(endpoint, curveStyle);
}

void Oscilloscope::Panel::updateChannelLabel(IO::endpoint probe_info)
{
  const QString chanlabel = QString::number(probe_info.block->getID()) + " "
      + QString(probe_info.block->getName().c_str()) + " "
      + this->scalesList->currentText();

  this->scopeWindow->setChannelLabel(probe_info, chanlabel);
}

void Oscilloscope::Panel::updateWindowTimeDiv()
{
  auto divt = this->timesList->currentData().value<int64_t>();
  this->scopeWindow->setDivT(divt);
}

void Oscilloscope::Panel::enableChannel()
{
  // make some initial checks
  if (!this->activateButton->isChecked()) {
    return;
  }

  // create component before we create the channel proper
  auto* oscilloscope_plugin =
      dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  auto* chanblock = this->blocksListDropdown->currentData().value<IO::Block*>();
  auto chanport = this->channelsList->currentData().value<size_t>();
  auto chandirection = this->typesList->currentData().value<IO::flags_t>();

  // this will try to create the probing component first. return if something
  // goes wrong
  const IO::endpoint endpoint {chanblock, chanport, chandirection};
  RT::OS::Fifo* probe_fifo = oscilloscope_plugin->createProbe(endpoint);
  if (probe_fifo == nullptr) {
    ERROR_MSG(
        "Oscilloscope::Panel::enableChannel Unable to create probing channel "
        "for block {}",
        chanblock->getName());
    return;
  }

  this->scopeWindow->createChannel(endpoint, probe_fifo);
  // we were able to create the probe, so we should populate metainfo about it
  // in scope window
  this->updateChannelOffset(endpoint);
  this->updateChannelScale(endpoint);
  this->updateChannelPen(endpoint);
  this->updateChannelCurveStyle(endpoint);
}

void Oscilloscope::Panel::disableChannel()
{
  // make some initial checks
  if (!this->activateButton->isChecked()) {
    return;
  }

  auto* oscilloscope_plugin =
      dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  auto* chanblock = this->blocksListDropdown->currentData().value<IO::Block*>();
  auto chanport = this->channelsList->currentData().value<size_t>();
  auto chandirection = this->typesList->currentData().value<IO::flags_t>();

  const IO::endpoint probe {chanblock, chanport, chandirection};
  // we should remove the scope channel before we attempt to remove block
  this->scopeWindow->removeChannel(probe);
  oscilloscope_plugin->deleteProbe(probe);
}

void Oscilloscope::Panel::activateChannel(bool active)
{
  const bool enable =
      active && blocksListDropdown->count() > 0 && channelsList->count() > 0;
  scalesList->setEnabled(enable);
  offsetsEdit->setEnabled(enable);
  offsetsList->setEnabled(enable);
  colorsList->setEnabled(enable);
  widthsList->setEnabled(enable);
  stylesList->setEnabled(enable);
  curveStylesList->setEnabled(enable);
  this->activateButton->setChecked(enable);
}

void Oscilloscope::Panel::apply()
{
  switch (tabWidget->currentIndex()) {
    case 0:
      applyChannelTab();
      break;
    case 1:
      applyDisplayTab();
      break;
    default:
      ERROR_MSG("Oscilloscope::Panel::showTab : invalid tab\n");
  }
}

void Oscilloscope::Panel::buildChannelList()
{
  if (blocksListDropdown->count() <= 0) {
    return;
  }

  if (blocksListDropdown->currentIndex() < 0) {
    blocksListDropdown->setCurrentIndex(0);
  }

  auto* block = this->blocksListDropdown->currentData().value<IO::Block*>();
  auto chanport = this->channelsList->currentData().value<size_t>();
  auto type = this->typesList->currentData().value<IO::flags_t>();
  channelsList->clear();
  for (size_t i = 0; i < block->getCount(type); ++i) {
    channelsList->addItem(QString(block->getChannelName(type, i).c_str()),
                          QVariant::fromValue(i));
  }
  channelsList->setCurrentIndex(
      this->channelsList->findData(QVariant::fromValue(chanport)));
  showChannelTab();
}

void Oscilloscope::Panel::showTab(int index)
{
  switch (index) {
    case 0:
      showChannelTab();
      break;
    case 1:
      showDisplayTab();
      break;
    default:
      ERROR_MSG("Oscilloscope::Panel::showTab : invalid tab\n");
  }
}

void Oscilloscope::Panel::setActivity(IO::endpoint endpoint, bool activity)
{
  auto* hplugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  hplugin->setProbeActivity(endpoint, activity);
}

void Oscilloscope::Panel::applyChannelTab()
{
  if (this->blocksListDropdown->count() <= 0
      || this->channelsList->count() <= 0)
  {
    return;
  }

  auto* block = this->blocksListDropdown->currentData().value<IO::Block*>();
  auto port = this->channelsList->currentData().value<size_t>();
  auto type = this->typesList->currentData().value<IO::flags_t>();
  auto* host_plugin =
      dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  const IO::endpoint probeInfo {block, port, type};
  this->scopeWindow->setPause(/*value=*/true);
  if (!activateButton->isChecked()) {
    scopeWindow->removeChannel(probeInfo);
    host_plugin->deleteProbe(probeInfo);
  } else {
    if (!this->scopeWindow->channelRegistered(probeInfo)) {
      RT::OS::Fifo* fifo = host_plugin->createProbe(probeInfo);
      if (fifo != nullptr) {
        this->scopeWindow->createChannel(probeInfo, fifo);
      }
    }
    this->updateChannelScale(probeInfo);
    this->updateChannelOffset(probeInfo);
    this->updateChannelPen(probeInfo);
    this->updateChannelCurveStyle(probeInfo);
    this->updateChannelLabel(probeInfo);
  }
  scopeWindow->replot();
  this->scopeWindow->setPause(/*value=*/false);
  this->syncChannelProperties();
  showChannelTab();
}

void Oscilloscope::Panel::applyDisplayTab()
{
  updateTrigger();
  updateWindowTimeDiv();
  scopeWindow->replot();
  showDisplayTab();
}

void Oscilloscope::Panel::buildBlockList()
{
  Event::Object event(Event::Type::IO_BLOCK_QUERY_EVENT);
  this->getRTXIEventManager()->postEvent(&event);
  auto blocklist =
      std::any_cast<std::vector<IO::Block*>>(event.getParam("blockList"));
  auto* previous_block =
      this->blocksListDropdown->currentData().value<IO::Block*>();
  blocksListDropdown->clear();
  for (auto* block : blocklist) {
    // Ignore blocks created from oscilloscope (probing blocks),
    // and from recorder (recorder components)
    if (block->getName().find("Probe") != std::string::npos
        || block->getName().find("Recording") != std::string::npos)
    {
      continue;
    }
    this->blocksListDropdown->addItem(QString(block->getName().c_str()) + " "
                                          + QString::number(block->getID()),
                                      QVariant::fromValue(block));
  }
  blocksListDropdown->setCurrentIndex(
      this->blocksListDropdown->findData(QVariant::fromValue(previous_block)));
}

QWidget* Oscilloscope::Panel::createChannelTab(QWidget* parent)
{
  setWhatsThis(
      "<p><b>Oscilloscope: Channel Options</b><br>"
      "Use the dropdown boxes to select the signal streams you want to plot "
      "from "
      "any loaded modules or your DAQ device. You may change the plotting "
      "scale for "
      "the signal, apply a DC offset, and change the color and style of the "
      "line.</p>");

  auto* page = new QWidget(parent);

  // Create group and layout for buttons at bottom of scope
  auto* bttnLayout = new QGridLayout(page);

  // Create Channel box
  auto* row1Layout = new QHBoxLayout;
  auto* channelLabel = new QLabel(tr("Channel:"), page);
  row1Layout->addWidget(channelLabel);
  blocksListDropdown = new QComboBox(page);
  blocksListDropdown->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  blocksListDropdown->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  QObject::connect(blocksListDropdown,
                   QOverload<int>::of(&QComboBox::activated),
                   this,
                   &Oscilloscope::Panel::buildChannelList);
  row1Layout->addWidget(blocksListDropdown);

  // Create Type box
  typesList = new QComboBox(page);
  typesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  typesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  typesList->addItem("Output", QVariant::fromValue(IO::OUTPUT));
  typesList->addItem("Input", QVariant::fromValue(IO::INPUT));
  row1Layout->addWidget(typesList);
  QObject::connect(typesList,
                   QOverload<int>::of(&QComboBox::activated),
                   this,
                   &Oscilloscope::Panel::buildChannelList);

  // Create Channels box
  channelsList = new QComboBox(page);
  channelsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  channelsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  QObject::connect(channelsList,
                   QOverload<int>::of(&QComboBox::activated),
                   this,
                   &Oscilloscope::Panel::showChannelTab);
  row1Layout->addWidget(channelsList);

  // Create elements for display box
  row1Layout->addSpacerItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  auto* scaleLabel = new QLabel(tr("Scale:"), page);
  row1Layout->addWidget(scaleLabel);
  scalesList = new QComboBox(page);
  row1Layout->addWidget(scalesList);
  const QFont scalesListFont("DejaVu Sans Mono");
  const QString postfix = "/div";
  std::array<std::string, 6> unit_array = {"V", "mV", "µV", "nV", "pV", "fV"};
  size_t unit_array_index = 0;
  const std::array<double, 4> fixed_values = {10, 5, 2.5, 2};
  double value_scale = 1.0;
  scalesList->setFont(scalesListFont);
  const std::string formatting = "{:.1f} {}/div";
  double temp_value = 0.0;
  while (unit_array_index < unit_array.size()) {
    for (auto current_fixed_value : fixed_values) {
      temp_value =
          current_fixed_value * std::pow(1e3, unit_array_index) * value_scale;
      if (temp_value < 1) {
        unit_array_index++;
        if (unit_array_index >= 6) {
          break;
        }
        temp_value =
            current_fixed_value * std::pow(1e3, unit_array_index) * value_scale;
      }
      scalesList->addItem(
          QString(fmt::format(
                      formatting, temp_value, unit_array.at(unit_array_index))
                      .c_str()),
          current_fixed_value * value_scale);
    }
    value_scale = value_scale / 10.0;
  }
  // Offset items
  auto* offsetLabel = new QLabel(tr("Offset:"), page);
  row1Layout->addWidget(offsetLabel);
  offsetsEdit = new QLineEdit(page);
  offsetsEdit->setMaximumWidth(offsetsEdit->minimumSizeHint().width() * 2);
  offsetsEdit->setValidator(new QDoubleValidator(offsetsEdit));
  row1Layout->addWidget(offsetsEdit);  //, Qt::AlignRight);
  offsetsList = new QComboBox(page);
  row1Layout->addWidget(offsetsList);  //, Qt::AlignRight);
  offsetsList->addItem("V", 1.0);
  offsetsList->addItem("mV", 1e-3);
  offsetsList->addItem(QString::fromUtf8("µV"), 1e-6);
  offsetsList->addItem("nV", 1e-9);
  offsetsList->addItem("pV", 1e-12);

  // Create elements for graphic
  auto* row2Layout = new QHBoxLayout;  //(page);
  row2Layout->setAlignment(Qt::AlignLeft);
  auto* colorLabel = new QLabel(tr("Color:"), page);
  row2Layout->addWidget(colorLabel);
  colorsList = new QComboBox(page);
  colorsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  colorsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  row2Layout->addWidget(colorsList);
  QPixmap tmp(25, 25);
  std::string color_name;
  for (size_t i = 0; i < penColors.size(); i++) {
    tmp.fill(penColors.at(i));
    color_name = Oscilloscope::color2string.at(i);
    colorsList->addItem(
        tmp, QString(color_name.c_str()), Oscilloscope::penColors.at(i));
  }

  auto* widthLabel = new QLabel(tr("Width:"), page);
  row2Layout->addWidget(widthLabel);
  widthsList = new QComboBox(page);
  widthsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  widthsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  row2Layout->addWidget(widthsList);
  tmp.fill(Qt::white);
  QPainter painter(&tmp);
  for (int i = 1; i < 6; i++) {
    painter.setPen(
        QPen(Oscilloscope::penColors.at(Oscilloscope::ColorID::Black), i));
    painter.drawLine(0, 12, 25, 12);
    widthsList->addItem(tmp, QString::number(i) + QString(" Pixels"), i);
  }

  // Create styles list
  auto* styleLabel = new QLabel(tr("Line Style:"), page);
  row2Layout->addWidget(styleLabel);
  stylesList = new QComboBox(page);
  stylesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  stylesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  row2Layout->addWidget(stylesList);
  std::string temp_name;
  for (size_t i = 0; i < Oscilloscope::penStyles.size(); i++) {
    temp_name = Oscilloscope::penstyles2string.at(i);
    tmp.fill(Qt::white);
    painter.setPen(
        QPen(Oscilloscope::penColors.at(Oscilloscope::ColorID::Black),
             3,
             Oscilloscope::penStyles.at(i)));
    painter.drawLine(0, 12, 25, 12);
    stylesList->addItem(tmp,
                        QString(temp_name.c_str()),
                        QVariant::fromValue(Oscilloscope::penStyles.at(i)));
  }

  // Create curve styles list
  auto* curveStyleLabel = new QLabel(tr("Curve Style:"), page);
  row2Layout->addWidget(curveStyleLabel);
  curveStylesList = new QComboBox(page);
  curveStylesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  curveStylesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  row2Layout->addWidget(curveStylesList);
  for (size_t i = 0; i < Oscilloscope::curveStyles.size(); i++) {
    temp_name = Oscilloscope::curvestyles2string.at(i);
    tmp.fill(Qt::white);
    painter.setPen(
      QPen(Oscilloscope::penColors.at(Oscilloscope::ColorID::Black),
      3));
    (i == 0) ? painter.drawLine(0, 12, 25, 12) : painter.drawPoint(12, 12);
    curveStylesList->addItem(tmp,
                        QString(temp_name.c_str()),
                        QVariant::fromValue(Oscilloscope::curveStyles.at(i)));
  }

  // Activate button
  row2Layout->addSpacerItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  activateButton = new QPushButton("Enable Channel", page);
  row2Layout->addWidget(activateButton);
  activateButton->setCheckable(true);
  activateButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QObject::connect(activateButton,
                   &QPushButton::toggled,
                   this,
                   &Oscilloscope::Panel::activateChannel);
  activateChannel(/*active=*/false);

  bttnLayout->addLayout(row1Layout, 0, 0);
  bttnLayout->addLayout(row2Layout, 1, 0);

  QObject::connect(blocksListDropdown,
                   &QComboBox::currentTextChanged,
                   this,
                   &Oscilloscope::Panel::syncChannelProperties);
  QObject::connect(typesList,
                   &QComboBox::currentTextChanged,
                   this,
                   &Oscilloscope::Panel::syncChannelProperties);
  QObject::connect(channelsList,
                   &QComboBox::currentTextChanged,
                   this,
                   &Oscilloscope::Panel::syncChannelProperties);
  return page;
}

QWidget* Oscilloscope::Panel::createDisplayTab(QWidget* parent)
{
  setWhatsThis(
      "<p><b>Oscilloscope: Display Options</b><br>"
      "Use the dropdown box to select the time scale for the Oscilloscope. "
      "This "
      "scaling is applied to all signals plotted in the same window. You may "
      "also "
      "set a trigger on any signal that is currently plotted in the window. A "
      "yellow "
      "line will appear at the trigger threshold.</p>");

  auto* page = new QWidget(parent);

  // Scope properties
  auto* displayTabLayout = new QGridLayout(page);

  // Create elements for time settings
  auto* row1Layout = new QHBoxLayout;
  row1Layout->addWidget(new QLabel(tr("Time/Div:"), page));
  timesList = new QComboBox(page);
  row1Layout->addWidget(timesList);
  const QFont timeListFont("DejaVu Sans Mono");
  timesList->setFont(timeListFont);
  timesList->addItem("5 s/div", QVariant::fromValue(5000000000));
  timesList->addItem("2 s/div", QVariant::fromValue(2000000000));
  timesList->addItem("1 s/div", QVariant::fromValue(1000000000));
  timesList->addItem("500 ms/div", QVariant::fromValue(500000000));
  timesList->addItem("200 ms/div", QVariant::fromValue(200000000));
  timesList->addItem("100 ms/div", QVariant::fromValue(100000000));
  timesList->addItem("50 ms/div", QVariant::fromValue(50000000));
  timesList->addItem("20 ms/div", QVariant::fromValue(20000000));
  timesList->addItem("10 ms/div", QVariant::fromValue(10000000));
  timesList->addItem("5 ms/div", QVariant::fromValue(5000000));
  timesList->addItem("2 ms/div", QVariant::fromValue(2000000));
  timesList->addItem("1 ms/div", QVariant::fromValue(1000000));
  timesList->addItem(QString::fromUtf8("500 µs/div"),
                     QVariant::fromValue(500000));
  timesList->addItem(QString::fromUtf8("200 µs/div"),
                     QVariant::fromValue(200000));
  timesList->addItem(QString::fromUtf8("100 µs/div"),
                     QVariant::fromValue(100000));
  timesList->addItem(QString::fromUtf8("50 µs/div"),
                     QVariant::fromValue(50000));
  timesList->addItem(QString::fromUtf8("20 µs/div"),
                     QVariant::fromValue(20000));
  timesList->addItem(QString::fromUtf8("10 µs/div"),
                     QVariant::fromValue(10000));
  timesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  timesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  auto* refreshLabel = new QLabel(tr("Refresh:"), page);
  row1Layout->addWidget(refreshLabel);
  refreshDropdown = new QComboBox(page);
  row1Layout->addWidget(refreshDropdown);
  refreshDropdown->addItem("60 Hz",
                           QVariant::fromValue(Oscilloscope::FrameRates::HZ60));
  refreshDropdown->addItem(
      "120 Hz", QVariant::fromValue(Oscilloscope::FrameRates::HZ120));
  refreshDropdown->addItem(
      "240 Hz", QVariant::fromValue(Oscilloscope::FrameRates::HZ240));

  // Display box for Buffer bit. Push it to the right.
  row1Layout->addSpacerItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  auto* bufferLabel = new QLabel(tr("Buffer Size (MB):"), page);
  row1Layout->addWidget(bufferLabel);
  sizesEdit = new QLineEdit(page);
  sizesEdit->setMaximumWidth(sizesEdit->minimumSizeHint().width() * 3);
  sizesEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  row1Layout->addWidget(sizesEdit);
  const auto total_bytes = static_cast<double>(scopeWindow->getDataSize()
                                               * sizeof(Oscilloscope::sample));
  sizesEdit->setText(QString::number(total_bytes / 1e6));
  sizesEdit->setEnabled(false);

  // Trigger box
  auto* row2Layout = new QHBoxLayout;
  row2Layout->addWidget(new QLabel(tr("Edge:"), page));
  trigsGroup = new QButtonGroup(page);

  auto* off = new QRadioButton(tr("Off"), page);
  trigsGroup->addButton(off, Oscilloscope::Trigger::NONE);
  row2Layout->addWidget(off);
  auto* plus = new QRadioButton(tr("+"), page);
  trigsGroup->addButton(plus, Oscilloscope::Trigger::POS);
  row2Layout->addWidget(plus);
  auto* minus = new QRadioButton(tr("-"), page);
  trigsGroup->addButton(minus, Oscilloscope::Trigger::NEG);
  row2Layout->addWidget(minus);

  row2Layout->addWidget(new QLabel(tr("Channel:"), page));
  trigsChanList = new QComboBox(page);
  trigsChanList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  trigsChanList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  row2Layout->addWidget(trigsChanList);

  row2Layout->addWidget(new QLabel(tr("Threshold:"), page));
  trigsThreshEdit = new QLineEdit(page);
  trigsThreshEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  trigsThreshEdit->setMaximumWidth(trigsThreshEdit->minimumSizeHint().width()
                                   * 3);
  row2Layout->addWidget(trigsThreshEdit);
  trigsThreshEdit->setValidator(new QDoubleValidator(trigsThreshEdit));
  trigsThreshList = new QComboBox(page);
  trigsThreshList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  row2Layout->addWidget(trigsThreshList);
  trigsThreshList->addItem("V", 1.0);
  trigsThreshList->addItem("mV", 1e-3);
  trigsThreshList->addItem(QString::fromUtf8("µV"), 1e-6);
  trigsThreshList->addItem("nV", 1e-9);
  trigsThreshList->addItem("pV", 1e-12);

  // TODO: determine the proper implementation of trigger windows
  // row2Layout->addWidget(new QLabel(tr("Window:"), page));
  // trigWindowEdit = new QLineEdit(page);
  // trigWindowEdit->setText(QString::number(scopeWindow->getWindowTimewidth()));
  // trigWindowEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  // trigWindowEdit->setMaximumWidth(trigWindowEdit->minimumSizeHint().width() *
  // 3);

  // trigWindowEdit->setValidator(new QDoubleValidator(trigWindowEdit));
  // row2Layout->addWidget(trigWindowEdit);
  // trigWindowList = new QComboBox(page);
  // trigWindowList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  // row2Layout->addWidget(trigWindowList);
  // trigWindowList->addItem("s");
  // trigWindowList->addItem("ms");
  // trigWindowList->addItem(QString::fromUtf8("µs"));
  // trigWindowList->setCurrentIndex(1);

  displayTabLayout->addLayout(row1Layout, 0, 0);
  displayTabLayout->addLayout(row2Layout, 1, 0);

  return page;
}

void Oscilloscope::Panel::syncBlockInfo()
{
  this->buildBlockList();
  this->buildChannelList();
  this->showChannelTab();
}

void Oscilloscope::Panel::showChannelTab()
{
  auto type = static_cast<IO::flags_t>(this->typesList->currentData().toInt());
  auto* block = this->blocksListDropdown->currentData().value<IO::Block*>();
  auto port = this->channelsList->currentData().value<size_t>();
  const IO::endpoint chan {block, port, type};
  const double scale = this->scopeWindow->getChannelScale(chan);
  double offset = this->scopeWindow->getChannelOffset(chan);
  scalesList->setCurrentIndex(
      static_cast<int>(round(4 * (log10(1 / scale) + 1))));
  int offsetUnits = 0;
  if (offset > 0.0 && offset < 1.0) {
    while (fabs(offset) < 1 && offsetUnits < offsetsList->count()) {
      offset *= 1000;
      offsetUnits++;
    }
  }
  offsetsEdit->setText(QString::number(offset));
  offsetsList->setCurrentIndex(offsetUnits);
  this->activateButton->setChecked(this->scopeWindow->channelRegistered(chan));
}

void Oscilloscope::Panel::showDisplayTab()
{
  timesList->setCurrentIndex(
      timesList->findData(QVariant::fromValue(this->scopeWindow->getDivT())));

  // Find current trigger value and update gui
  IO::endpoint trigger_endpoint;
  auto* oscilloscope_plugin =
      dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  trigger_endpoint = this->scopeWindow->getTriggerEndpoint();
  this->trigsGroup->button(static_cast<int>(trigger_endpoint.direction))
      ->setChecked(true);

  trigsChanList->clear();
  std::vector<IO::endpoint> endpoint_list;
  if (oscilloscope_plugin != nullptr) {
    endpoint_list = oscilloscope_plugin->getTrackedEndpoints();
  }
  std::string direction_str;
  for (const auto& endpoint : endpoint_list) {
    direction_str = endpoint.direction == IO::INPUT ? "INPUT" : "OUTPUT";
    trigsChanList->addItem(QString(endpoint.block->getName().c_str()) + " "
                               + QString(direction_str.c_str()) + " "
                               + QString::number(endpoint.port),
                           QVariant::fromValue(endpoint));
  }
  trigsChanList->addItem("<None>");

  const int triglist_index =
      trigsChanList->findData(QVariant::fromValue(trigger_endpoint));
  trigsChanList->setCurrentIndex(triglist_index);

  int trigThreshUnits = 0;
  double trigThresh = this->scopeWindow->getTriggerThreshold();
  if (trigThresh * std::pow(10, -3 * this->trigsThreshList->count() - 1) < 1) {
    trigThreshUnits = 0;
    trigThresh = 0;
  } else {
    while (fabs(trigThresh) < 1
           && trigThreshUnits < this->trigsThreshList->count())
    {
      trigThresh *= 1000;
      ++trigThreshUnits;
    }
  }
  trigsThreshList->setCurrentIndex(trigThreshUnits);
  trigsThreshEdit->setText(QString::number(trigThresh));

  sizesEdit->setText(
      QString::number(static_cast<double>(scopeWindow->getDataSize()
                                          * sizeof(Oscilloscope::sample))
                      / 1e6));
}

Oscilloscope::Panel::Panel(QMainWindow* mw, Event::Manager* ev_manager)
    : Widgets::Panel(std::string(Oscilloscope::MODULE_NAME), mw, ev_manager)
    , tabWidget(new QTabWidget)
    , scopeWindow(new Scope(this))
    , layout(new QVBoxLayout)
    , scopeGroup(new QWidget(this))
    , setBttnGroup(new QGroupBox(this))
{
  setWhatsThis(
      "<p><b>Oscilloscope:</b><br>The Oscilloscope allows you to plot any "
      "signal "
      "in your workspace in real-time, including signals from your DAQ card "
      "and those "
      "generated by user modules. Multiple signals are overlaid in the window "
      "and "
      "different line colors and styles can be selected. When a signal is "
      "added, a legend "
      "automatically appears in the bottom of the window. Multiple "
      "oscilloscopes can "
      "be instantiated to give you multiple data windows. To select signals "
      "for plotting, "
      "use the right-click context \"Panel\" menu item. After selecting a "
      "signal, you must "
      "click the \"Enable\" button for it to appear in the window. To change "
      "signal settings, "
      "you must click the \"Apply\" button. The right-click context \"Pause\" "
      "menu item "
      "allows you to start and stop real-time plotting.</p>");

  // Create tab widget
  tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QObject::connect(tabWidget,
                   &QTabWidget::currentChanged,
                   this,
                   &Oscilloscope::Panel::showTab);

  auto* scopeLayout = new QHBoxLayout(this);
  scopeLayout->addWidget(scopeWindow);
  scopeGroup->setLayout(scopeLayout);
  auto* setBttnLayout = new QHBoxLayout(this);

  // Create buttons
  pauseButton = new QPushButton("Pause");
  pauseButton->setCheckable(true);
  QObject::connect(pauseButton,
                   &QPushButton::released,
                   this,
                   &Oscilloscope::Panel::togglePause);
  setBttnLayout->addWidget(pauseButton);
  applyButton = new QPushButton("Apply");
  QObject::connect(
      applyButton, &QPushButton::released, this, &Oscilloscope::Panel::apply);
  setBttnLayout->addWidget(applyButton);
  settingsButton = new QPushButton("Screenshot");
  QObject::connect(settingsButton,
                   &QPushButton::released,
                   this,
                   &Oscilloscope::Panel::screenshot);
  setBttnLayout->addWidget(settingsButton);

  // Attach layout
  setBttnGroup->setLayout(setBttnLayout);

  // Create tabs
  tabWidget->setTabPosition(QTabWidget::North);
  tabWidget->addTab(createChannelTab(this), "Channel");
  tabWidget->addTab(createDisplayTab(this), "Display");

  // Setup main layout
  layout->addWidget(scopeGroup);
  layout->addWidget(tabWidget);
  layout->addWidget(setBttnGroup);

  // Set
  setLayout(layout);

  // Show stuff
  adjustDataSize();
  showDisplayTab();
  getMdiWindow()->setMinimumSize(this->minimumSizeHint().width(), 450);
  getMdiWindow()->resize(this->minimumSizeHint().width() + 50, 600);

  // Initialize vars
  setWindowTitle(tr(std::string(Oscilloscope::MODULE_NAME).c_str()));

  auto* otimer = new QTimer(this);
  otimer->setTimerType(Qt::PreciseTimer);
  otimer->start(Oscilloscope::FrameRates::HZ60);

  qRegisterMetaType<IO::Block*>("IO::Block*");
  QObject::connect(this,
                   &Oscilloscope::Panel::updateBlockChannels,
                   this,
                   &Oscilloscope::Panel::removeBlockChannels);
  QObject::connect(this,
                   &Oscilloscope::Panel::updateBlockInfo,
                   this,
                   &Oscilloscope::Panel::syncBlockInfo);

  this->updateBlockInfo();
  this->buildChannelList();
  scopeWindow->replot();
}

Oscilloscope::Component::Component(Widgets::Plugin* hplugin,
                                   const std::string& probe_name)
    : Widgets::Component(hplugin,
                         probe_name,
                         Oscilloscope::get_default_channels(),
                         Oscilloscope::get_default_vars())
{
  if (RT::OS::getFifo(this->fifo, Oscilloscope::DEFAULT_BUFFER_SIZE) != 0) {
    ERROR_MSG("Unable to create xfifo for Oscilloscope Component {}",
              probe_name);
  }
}

// TODO: Handle trigger synchronization between oscilloscope components
void Oscilloscope::Component::execute()
{
  Oscilloscope::sample sample {};
  switch (this->getState()) {
    case RT::State::EXEC: {
      sample.time = RT::OS::getTime();
      sample.value = this->readinput(0);
      this->fifo->writeRT(&sample, sizeof(Oscilloscope::sample));
      break;
    }
    case RT::State::INIT:
    case RT::State::UNPAUSE:
      this->setState(RT::State::EXEC);
      break;
    case RT::State::PAUSE:
    case RT::State::MODIFY:
    case RT::State::EXIT:
    case RT::State::PERIOD:
    case RT::State::UNDEFINED:
      break;
  }
}

void Oscilloscope::Panel::screenshot()
{
  QwtPlotRenderer renderer;
  renderer.exportTo(scopeWindow, "screenshot.pdf");
}

void Oscilloscope::Panel::togglePause()
{
  this->scopeWindow->setPause(this->pauseButton->isChecked());
  auto* hplugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  hplugin->setAllProbesActivity(this->pauseButton->isChecked());
}

void Oscilloscope::Component::flushFifo()
{
  Oscilloscope::sample sample;
  while (this->fifo->read(&sample, sizeof(Oscilloscope::sample)) > 0) {
  }
}

// TODO: fix rt buffer size adjustments for components
void Oscilloscope::Panel::adjustDataSize()
{
  // Event::Object event(Event::Type::RT_GET_PERIOD_EVENT);
  // this->getRTXIEventManager()->postEvent(&event);
  // auto period = std::any_cast<int64_t>(event.getParam("period"));
  // const double timedivs = scopeWindow->getDivT();
  // const double xdivs =
  //     static_cast<double>(scopeWindow->getDivX()) /
  //     static_cast<double>(period);
  // const size_t size = static_cast<size_t>(ceil(timedivs + xdivs)) + 1;
  // scopeWindow->setDataSize(size);
  // sizesEdit->setText(QString::number(scopeWindow->getDataSize()));
}

void Oscilloscope::Panel::updateTrigger() {}

void Oscilloscope::Panel::removeBlockChannels(IO::Block* block)
{
  this->scopeWindow->removeBlockChannels(block);
  auto* hplugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  // Sometimes fired events may trigger this function after the plugin has been
  // unloaded. in that case just return and don't crash please.
  if (hplugin == nullptr) {
    return;
  }
  hplugin->deleteAllProbes(block);
}

void Oscilloscope::Panel::syncChannelProperties()
{
  IO::endpoint probe_info {};
  probe_info.block = blocksListDropdown->currentData().value<IO::Block*>();
  probe_info.direction = typesList->currentData().value<IO::flags_t>();
  probe_info.port = channelsList->currentData().value<size_t>();

  // we don't bother updating if channel is not active
  if (!scopeWindow->channelRegistered(probe_info)) {
    return;
  }

  const QColor color = scopeWindow->getChannelColor(probe_info);
  colorsList->setCurrentIndex(colorsList->findData(color));
  const int width = scopeWindow->getChannelWidth(probe_info);
  widthsList->setCurrentIndex(widthsList->findData(width));
  const Qt::PenStyle style = scopeWindow->getChannelStyle(probe_info);
  stylesList->setCurrentIndex(stylesList->findData(QVariant::fromValue(style)));
  const QwtPlotCurve::CurveStyle curveStyle = scopeWindow->getChannelCurveStyle(probe_info);
  curveStylesList->setCurrentIndex(curveStylesList->findData(QVariant::fromValue(curveStyle)));
  double offset = scopeWindow->getChannelOffset(probe_info);
  const double scale = scopeWindow->getChannelScale(probe_info);
  scalesList->setCurrentIndex(
      static_cast<int>(round(4 * (log10(1 / scale) + 1))));
  int offsetUnits = 0;
  if (offset * std::pow(10, -3 * offsetsList->count() - 3) < 1) {
    offset = 0;
    offsetUnits = 0;
  } else {
    while (fabs(offset) < 1 && offsetUnits < offsetsList->count()) {
      offset *= 1000;
      offsetUnits++;
    }
  }
  offsetsEdit->setText(QString::number(offset));
  offsetsList->setCurrentIndex(offsetUnits);
}

Oscilloscope::Plugin::Plugin(Event::Manager* ev_manager)
    : Widgets::Plugin(ev_manager, std::string(Oscilloscope::MODULE_NAME))
{
}

Oscilloscope::Plugin::~Plugin()
{
  std::vector<Event::Object> unloadEvents;
  for (auto& registry_entry : this->m_component_registry) {
    unloadEvents.emplace_back(Event::Type::RT_THREAD_REMOVE_EVENT);
    unloadEvents.back().setParam(
        "thread",
        std::any(static_cast<RT::Thread*>(registry_entry.component.get())));
  }
  this->getEventManager()->postEvent(unloadEvents);
}

// TODO:make this thread safe
RT::OS::Fifo* Oscilloscope::Plugin::createProbe(IO::endpoint probe_info)
{
  auto probe_loc = std::find_if(this->m_component_registry.begin(),
                                this->m_component_registry.end(),
                                [&](const registry_entry_t& entry)
                                { return entry.endpoint == probe_info; });
  if (probe_loc != this->m_component_registry.end()) {
    return probe_loc->component->getFifoPtr();
  }
  const std::string comp_name =
      fmt::format("{} Probe for Block {} Channel {} {} with Id {} ",
                  std::string(Oscilloscope::MODULE_NAME),
                  probe_info.block->getName(),
                  probe_info.direction == IO::OUTPUT ? "Output " : "Input ",
                  probe_info.port,
                  probe_info.block->getID());
  this->m_component_registry.push_back(
      {probe_info, std::make_unique<Oscilloscope::Component>(this, comp_name)});
  Oscilloscope::Component* measuring_component =
      this->m_component_registry.back().component.get();
  measuring_component->setActive(/*act=*/true);
  RT::block_connection_t connection;
  connection.src = probe_info.block;
  connection.src_port_type = probe_info.direction;
  connection.src_port = probe_info.port;
  connection.dest = measuring_component;
  connection.dest_port = 0;
  std::vector<Event::Object> events;
  events.emplace_back(Event::Type::RT_THREAD_INSERT_EVENT);
  events.back().setParam(
      "thread", std::any(static_cast<RT::Thread*>(measuring_component)));
  events.emplace_back(Event::Type::IO_LINK_INSERT_EVENT);
  events.back().setParam("connection", std::any(connection));
  this->getEventManager()->postEvent(events);
  return measuring_component->getFifoPtr();
  // TODO: complete proper handling of errors if not able to register probe
  // thread
}

void Oscilloscope::Plugin::deleteProbe(IO::endpoint probe_info)
{
  auto probe_loc = std::find_if(this->m_component_registry.begin(),
                                this->m_component_registry.end(),
                                [&](const registry_entry_t& entry)
                                { return entry.endpoint == probe_info; });
  if (probe_loc == this->m_component_registry.end()) {
    return;
  }
  Oscilloscope::Component* measuring_component = probe_loc->component.get();
  Event::Object event(Event::Type::RT_THREAD_REMOVE_EVENT);
  event.setParam("thread",
                 std::any(static_cast<RT::Thread*>(measuring_component)));
  this->getEventManager()->postEvent(&event);
  this->m_component_registry.erase(probe_loc);
}

void Oscilloscope::Plugin::deleteAllProbes(IO::Block* block)
{
  std::vector<IO::endpoint> all_endpoints;
  for (auto& entry : this->m_component_registry) {
    if (entry.endpoint.block == block) {
      all_endpoints.push_back(entry.endpoint);
    }
  }
  for (auto endpoint : all_endpoints) {
    this->deleteProbe(endpoint);
  }
}

void Oscilloscope::Plugin::setProbeActivity(IO::endpoint endpoint,
                                            bool activity)
{
  auto probe_loc = std::find_if(this->m_component_registry.begin(),
                                this->m_component_registry.end(),
                                [&](const registry_entry_t& entry)
                                { return entry.endpoint == endpoint; });
  if (probe_loc == this->m_component_registry.end()) {
    return;
  }
  const Event::Type event_type = activity
      ? Event::Type::RT_THREAD_PAUSE_EVENT
      : Event::Type::RT_THREAD_UNPAUSE_EVENT;
  Event::Object activity_event(event_type);
  activity_event.setParam("thread",
                          static_cast<RT::Thread*>(probe_loc->component.get()));
  this->getEventManager()->postEvent(&activity_event);
}

std::vector<IO::endpoint> Oscilloscope::Plugin::getTrackedEndpoints()
{
  std::vector<IO::endpoint> result;
  result.reserve(this->m_component_registry.size());
  for (const auto& entry : this->m_component_registry) {
    result.push_back(entry.endpoint);
  }
  return result;
}

void Oscilloscope::Plugin::setAllProbesActivity(bool activity)
{
  std::vector<Event::Object> events;
  events.reserve(this->m_component_registry.size());
  const Event::Type event_type = activity
      ? Event::Type::RT_THREAD_PAUSE_EVENT
      : Event::Type::RT_THREAD_UNPAUSE_EVENT;
  for (const auto& entry : this->m_component_registry) {
    events.emplace_back(event_type);
    events.back().setParam("thread",
                           static_cast<RT::Thread*>(entry.component.get()));
  }
  this->getEventManager()->postEvent(events);
}

Oscilloscope::Component* Oscilloscope::Plugin::getProbeComponentPtr(
    IO::endpoint endpoint)
{
  auto iter = std::find_if(this->m_component_registry.begin(),
                           this->m_component_registry.end(),
                           [&](const registry_entry_t& entry)
                           { return entry.endpoint == endpoint; });
  if (iter == this->m_component_registry.end()) {
    return nullptr;
  }
  return iter->component.get();
}

std::unique_ptr<Widgets::Plugin> Oscilloscope::createRTXIPlugin(
    Event::Manager* ev_manager)
{
  return std::make_unique<Oscilloscope::Plugin>(ev_manager);
}

Widgets::Panel* Oscilloscope::createRTXIPanel(QMainWindow* main_window,
                                              Event::Manager* ev_manager)
{
  return static_cast<Widgets::Panel*>(
      new Oscilloscope::Panel(main_window, ev_manager));
}

std::unique_ptr<Widgets::Component> Oscilloscope::createRTXIComponent(
    Widgets::Plugin* /*host_plugin*/)
{
  return std::unique_ptr<Oscilloscope::Component>(nullptr);
}

Widgets::FactoryMethods Oscilloscope::getFactories()
{
  Widgets::FactoryMethods fact;
  fact.createPanel = &Oscilloscope::createRTXIPanel;
  fact.createComponent = &Oscilloscope::createRTXIComponent;
  fact.createPlugin = &Oscilloscope::createRTXIPlugin;
  return fact;
}
