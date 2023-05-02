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

#include <cmath>
#include <sstream>

#include <qwt_plot_renderer.h>

#include "debug.hpp"
#include "main_window.hpp"
#include "rt.hpp"
#include "oscilloscope.h"

void Oscilloscope::Plugin::receiveEvent(Event::Object* event)
{
  switch (event->getType()){
    case Event::Type::RT_THREAD_INSERT_EVENT :
    case Event::Type::RT_DEVICE_INSERT_EVENT :
    case Event::Type::RT_THREAD_REMOVE_EVENT :
    case Event::Type::RT_DEVICE_REMOVE_EVENT :
      dynamic_cast<Oscilloscope::Panel*>(this->widget_panel)->updateBlockInfo();
      break;
    default :
      break;
  }
}

Oscilloscope::Component* Oscilloscope::Plugin::getProbeComponent(Oscilloscope::probe probeInfo)
{
  Oscilloscope::Component* component=nullptr;
  auto probe_loc = std::find_if(this->chanInfoList.begin(),
                                this->chanInfoList.end(),
                                [&](const Oscilloscope::channel_info& chan){
                                  return chan.probe.block == probeInfo.block &&
                                         chan.probe.port == probeInfo.port &&
                                         chan.probe.direction == probeInfo.direction;
                                });
  if(probe_loc != this->chanInfoList.end()){
    component = probe_loc->measuring_component;
  }
  return component;
}

void Oscilloscope::Panel::enableChannel()
{
  if(!this->activateButton->isChecked()) { return; }
  int block_list_index = this->blocksListDropdown->currentIndex();
  int port_list_index = this->channelsList->currentIndex();
  int direction_list_index = this->typesList->currentIndex();
  if(block_list_index < 0 || port_list_index < 0 || direction_list_index < 0) { return; }
  scope_channel chan;
  chan.block = this->blocks[static_cast<size_t>(this->blocksListDropdown->currentIndex())];
  chan.port = static_cast<size_t>(port_list_index);
  chan.direction = static_cast<IO::flags_t>(direction_list_index);
  chan.label = QString::number(chan.block->getID()) + 
               " " + 
               QString::fromStdString(chan.block->getName()) +
               " " +
               this->scalesList->currentText();
  chan.curve = new QwtPlotCurve(chan.label);
  int scale_index = this->scalesList->currentIndex();
  switch (scale_index % 4){
    case 0:
      chan.scale = pow(10, 1 - scale_index / 4);
      break;
    case 1:
      chan.scale = 5 * pow(10, -scale_index / 4);
      break;
    case 2:
      chan.scale = 2.5 * pow(10, -scale_index / 4);
      break;
    case 3:
      chan.scale = 2 * pow(10, -scale_index / 4);
      break;
    default:
      ERROR_MSG("Oscilloscope::Panel::applyChannelTab : invalid chan.scale selection\n");
      chan.scale = 1.0;
    } 
  chan.offset = this->offsetsEdit->text().toDouble() * pow(10, -3*offsetsList->currentIndex());
  chan.curve->setPen(Oscilloscope::penColors[this->colorsList->currentIndex()],
                     this->widthsList->currentIndex() + 1,
                     Oscilloscope::penStyles[this->stylesList->currentIndex()]);

  Oscilloscope::probe probe {chan.block, chan.port, chan.direction};
  auto* oscilloscope_plugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  oscilloscope_plugin->addProbe(chan);
}

void Oscilloscope::Panel::disableChannel()
{

}


void Oscilloscope::Panel::activateChannel(bool active)
{
  bool enable = active && blocksListDropdown->count() > 0 && channelsList->count() > 0;
  scalesList->setEnabled(enable);
  offsetsEdit->setEnabled(enable);
  offsetsList->setEnabled(enable);
  colorsList->setEnabled(enable);
  widthsList->setEnabled(enable);
  stylesList->setEnabled(enable);
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
  channelsList->clear();
  if (blocksListDropdown->count() <= 0) {
    return;
  }

  if (blocksListDropdown->currentIndex() < 0){
    blocksListDropdown->setCurrentIndex(0);
  }

  IO::Block* block = blocks[static_cast<size_t>(blocksListDropdown->currentIndex())];
  IO::flags_t type = IO::UNKNOWN;
  switch (typesList->currentIndex()) {
    case 0:
      type = IO::INPUT;
      break;
    case 1:
      type = IO::OUTPUT;
      break;
    default:
      ERROR_MSG(
          "Oscilloscope::Panel::buildChannelList : invalid type selection\n");
  }

  for (size_t i = 0; i < block->getCount(type); ++i)
    channelsList->addItem(QString::fromStdString(block->getChannelName(type, i)));

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

void Oscilloscope::Panel::setActivity(Oscilloscope::Component* comp, bool activity)
{
  Event::Type event_type = Event::Type::NOOP; 
  switch(activity){
    case true:
      event_type = Event::Type::RT_THREAD_UNPAUSE_EVENT;
      break;
    default:
      event_type = Event::Type::RT_THREAD_PAUSE_EVENT;
      break;
  }     
  Event::Object event(event_type);
  event.setParam("thread", std::any(static_cast<RT::Thread*>(comp)));
  this->getRTXIEventManager()->postEvent(&event);
}

void Oscilloscope::Panel::applyChannelTab()
{
  if (this->blocksListDropdown->count() <= 0 || this->channelsList->count() <= 0)
    return;

  int block_index = this->blocksListDropdown->currentIndex();
  int port_index = this->channelsList->currentIndex();
  int flags_index = this->typesList->currentIndex();
  if(block_index < 0 || port_index < 0) { return; }

  IO::Block* block = blocks[static_cast<size_t>(block_index)];
  auto port = static_cast<size_t>(this->channelsList->currentIndex());
  IO::flags_t type = static_cast<IO::flags_t>(flags_index);
  auto* host_plugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  Oscilloscope::probe probeInfo {block, port, type};
  Oscilloscope::Component* component = host_plugin->getProbeComponent(probeInfo);
  if (!activateButton->isChecked()) {
    if (component == nullptr) { return; }
    this->setActivity(component, false);
    scopeWindow->removeChannel(probeInfo);
    flushFifo();
  } else {

    host_plugin->addProbe(probeInfo);
    flushFifo();
  }

  //  double scale;
  //  switch (scalesList->currentIndex() % 4) {
  //    case 0:
  //      scale = pow(10, 1 - scalesList->currentIndex() / 4);
  //      break;
  //    case 1:
  //      scale = 5 * pow(10, -scalesList->currentIndex() / 4);
  //      break;
  //    case 2:
  //      scale = 2.5 * pow(10, -scalesList->currentIndex() / 4);
  //      break;
  //    case 3:
  //      scale = 2 * pow(10, -scalesList->currentIndex() / 4);
  //      break;
  //    default:
  //      ERROR_MSG(
  //          "Oscilloscope::Panel::applyChannelTab : invalid scale selection\n");
  //      scale = 1.0;
  //  }
  //  if (scale != i->getScale()) {
  //    scopeWindow->setChannelScale(i, scale);
  //    scopeWindow->setChannelLabel(
  //        i, info->name + " - " + scalesList->currentText().simplified());
  //  }
  //  scopeWindow->setChannelOffset(
  //      i,
  //      offsetsEdit->text().toDouble()
  //          * pow(10, -3 * offsetsList->currentIndex()));

  //  QPen pen;
  //  switch (colorsList->currentIndex()) {
  //    case 0:
  //      pen.setColor(QColor(255, 0, 16, 255));
  //      break;
  //    case 1:
  //      pen.setColor(QColor(255, 164, 5, 255));
  //      break;
  //    case 2:
  //      pen.setColor(QColor(43, 206, 72, 255));
  //      break;
  //    case 3:
  //      pen.setColor(QColor(0, 117, 220, 255));
  //      break;
  //    case 4:
  //      pen.setColor(QColor(178, 102, 255, 255));
  //      break;
  //    case 5:
  //      pen.setColor(QColor(0, 153, 143, 255));
  //      break;
  //    case 6:
  //      pen.setColor(QColor(83, 81, 84, 255));
  //      break;
  //    default:
  //      ERROR_MSG(
  //          "Oscilloscope::Panel::applyChannelTab : invalid color selection\n");
  //      pen.setColor(QColor(255, 0, 16, 255));
  //  }
  //  pen.setWidth(widthsList->currentIndex() + 1);
  //  switch (stylesList->currentIndex()) {
  //    case 0:
  //      pen.setStyle(Qt::SolidLine);
  //      break;
  //    case 1:
  //      pen.setStyle(Qt::DashLine);
  //      break;
  //    case 2:
  //      pen.setStyle(Qt::DotLine);
  //      break;
  //    case 3:
  //      pen.setStyle(Qt::DashDotLine);
  //      break;
  //    case 4:
  //      pen.setStyle(Qt::DashDotDotLine);
  //      break;
  //    default:
  //      ERROR_MSG(
  //          "Oscilloscope::Panel::applyChannelTab : invalid style selection\n");
  //      pen.setStyle(Qt::SolidLine);
  //  }
  //  scopeWindow->setChannelPen(i, pen);
  //}
  scopeWindow->replot();
  showChannelTab();
}

void Oscilloscope::Panel::applyDisplayTab()
{
  // Update X divisions
  double divT;
  if (timesList->currentIndex() % 3 == 1)
    divT = 2 * pow(10, 3 - timesList->currentIndex() / 3);
  else if (timesList->currentIndex() % 3 == 2)
    divT = pow(10, 3 - timesList->currentIndex() / 3);
  else
    divT = 5 * pow(10, 3 - timesList->currentIndex() / 3);
  scopeWindow->setDivT(divT);
  //scopeWindow->setPeriod(RT::System::getInstance()->getPeriod() * 1e-6);
  this->adjustDataSize();

  // Update trigger direction
  updateTrigger();
  
  // Update trigger threshold
  //double trigThreshold = trigsThreshEdit->text().toDouble()
  //    * pow(10, -3 * trigsThreshList->currentIndex());

  //// Update pre-trigger window for displaying
  //double trigWindow = trigWindowEdit->text().toDouble()
  //    * pow(10, -3 * trigWindowList->currentIndex());

  //std::list<Scope::Channel>::iterator trigChannel =
  //    scopeWindow->getChannelsEnd();
  //for (std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin(),
  //                                         end = scopeWindow->getChannelsEnd();
  //     i != end;
  //     ++i)
  //  if (i->getLabel() == trigsChanList->currentText()) {
  //    trigChannel = i;
  //    break;
  //  }
  //if (trigChannel == scopeWindow->getChannelsEnd())
  //  trigDirection = Scope::NONE;

  //scopeWindow->setTrigger(
  //    trigDirection, trigThreshold, trigChannel, trigWindow);

  adjustDataSize();
  scopeWindow->replot();
  showDisplayTab();
}

void Oscilloscope::Panel::buildBlockList()
{
  Event::Object event(Event::Type::IO_BLOCK_QUERY_EVENT);
  this->getRTXIEventManager()->postEvent(&event);
  auto blocklist = std::any_cast<std::vector<IO::Block*>>(event.getParam("blockList"));
  blocksListDropdown->clear();
  for(auto* block : blocklist) {
    this->blocksListDropdown->addItem(QString::fromStdString(block->getName()) + " " + 
                                      QString::number(block->getID()));
  }
  this->blocks = blocklist;
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
  QObject::connect(
      blocksListDropdown, SIGNAL(activated(int)), this, SLOT(buildChannelList()));
  row1Layout->addWidget(blocksListDropdown);

  // Create Type box
  typesList = new QComboBox(page);
  typesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  typesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  typesList->addItem("Output");
  typesList->addItem("Input");
  //typesList->addItem("Parameter");
  //typesList->addItem("State");
  QObject::connect(
      typesList, SIGNAL(activated(int)), this, SLOT(buildChannelList()));
  row1Layout->addWidget(typesList);

  // Create Channels box
  channelsList = new QComboBox(page);
  channelsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  channelsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  QObject::connect(
      channelsList, SIGNAL(activated(int)), this, SLOT(showChannelTab()));
  row1Layout->addWidget(channelsList);

  // Create elements for display box
  row1Layout->addSpacerItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  auto* scaleLabel = new QLabel(tr("Scale:"), page);
  row1Layout->addWidget(scaleLabel);
  scalesList = new QComboBox(page);
  row1Layout->addWidget(scalesList);
  QFont scalesListFont("DejaVu Sans Mono");
  scalesList->setFont(scalesListFont);
  scalesList->addItem("10 V/div");  // 0  case 0
  scalesList->addItem("5 V/div");  // 1  case 1
  scalesList->addItem("2.5 V/div");  // 2  case 2
  scalesList->addItem("2 V/div");  // 3  case 3
  scalesList->addItem("1 V/div");  // 4  case 0
  scalesList->addItem("500 mV/div");  // 5  case 1
  scalesList->addItem("250 mV/div");  // 6  case 2
  scalesList->addItem("200 mV/div");  // 7  case 3
  scalesList->addItem("100 mV/div");  // 8  case 0
  scalesList->addItem("50 mV/div");  // 9  case 1
  scalesList->addItem("25 mV/div");
  scalesList->addItem("20 mV/div");
  scalesList->addItem("10 mV/div");
  scalesList->addItem("5 mV/div");
  scalesList->addItem("2.5 mV/div");
  scalesList->addItem("2 mV/div");
  scalesList->addItem("1 mV/div");
  scalesList->addItem(QString::fromUtf8("500 µV/div"));
  scalesList->addItem(QString::fromUtf8("250 µV/div"));
  scalesList->addItem(QString::fromUtf8("200 µV/div"));
  scalesList->addItem(QString::fromUtf8("100 µV/div"));
  scalesList->addItem(QString::fromUtf8("50 µV/div"));
  scalesList->addItem(QString::fromUtf8("25 µV/div"));
  scalesList->addItem(QString::fromUtf8("20 µV/div"));
  scalesList->addItem(QString::fromUtf8("10 µV/div"));
  scalesList->addItem(QString::fromUtf8("5 µV/div"));
  scalesList->addItem(QString::fromUtf8("2.5 µV/div"));
  scalesList->addItem(QString::fromUtf8("2 µV/div"));
  scalesList->addItem(QString::fromUtf8("1 µV/div"));
  scalesList->addItem("500 nV/div");
  scalesList->addItem("250 nV/div");
  scalesList->addItem("200 nV/div");
  scalesList->addItem("100 nV/div");
  scalesList->addItem("50 nV/div");
  scalesList->addItem("25 nV/div");
  scalesList->addItem("20 nV/div");
  scalesList->addItem("10 nV/div");
  scalesList->addItem("5 nV/div");
  scalesList->addItem("2.5 nV/div");
  scalesList->addItem("2 nV/div");
  scalesList->addItem("1 nV/div");
  scalesList->addItem("500 pV/div");
  scalesList->addItem("250 pV/div");
  scalesList->addItem("200 pV/div");
  scalesList->addItem("100 pV/div");
  scalesList->addItem("50 pV/div");
  scalesList->addItem("25 pV/div");
  scalesList->addItem("20 pV/div");
  scalesList->addItem("10 pV/div");
  scalesList->addItem("5 pV/div");
  scalesList->addItem("2.5 pV/div");
  scalesList->addItem("2 pV/div");
  scalesList->addItem("1 pV/div");
  scalesList->addItem("500 fV/div");
  scalesList->addItem("250 fV/div");
  scalesList->addItem("200 fV/div");
  scalesList->addItem("100 fV/div");
  scalesList->addItem("50 fV/div");
  scalesList->addItem("25 fV/div");
  scalesList->addItem("20 fV/div");
  scalesList->addItem("10 fV/div");
  scalesList->addItem("5 fV/div");
  scalesList->addItem("2.5 fV/div");
  scalesList->addItem("2 fV/div");
  scalesList->addItem("1 fV/div");

  // Offset items
  auto* offsetLabel = new QLabel(tr("Offset:"), page);
  row1Layout->addWidget(offsetLabel);
  offsetsEdit = new QLineEdit(page);
  offsetsEdit->setMaximumWidth(offsetsEdit->minimumSizeHint().width() * 2);
  offsetsEdit->setValidator(new QDoubleValidator(offsetsEdit));
  row1Layout->addWidget(offsetsEdit);  //, Qt::AlignRight);
  offsetsList = new QComboBox(page);
  row1Layout->addWidget(offsetsList);  //, Qt::AlignRight);
  offsetsList->addItem("V");
  offsetsList->addItem("mV");
  offsetsList->addItem(QString::fromUtf8("µV"));
  offsetsList->addItem("nV");
  offsetsList->addItem("pV");

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
  tmp.fill(QColor(255, 0, 16, 255));
  colorsList->addItem(tmp, " Red");
  tmp.fill(QColor(255, 164, 5, 255));
  colorsList->addItem(tmp, " Orange");
  tmp.fill(QColor(43, 206, 72, 255));
  colorsList->addItem(tmp, " Green");
  tmp.fill(QColor(0, 117, 220, 255));
  colorsList->addItem(tmp, " Blue");
  tmp.fill(QColor(178, 102, 255, 255));
  colorsList->addItem(tmp, " Purple");
  tmp.fill(QColor(0, 153, 143, 255));
  colorsList->addItem(tmp, " Teal");
  tmp.fill(QColor(83, 81, 84, 255));
  colorsList->addItem(tmp, " Black");

  auto* widthLabel = new QLabel(tr("Width:"), page);
  row2Layout->addWidget(widthLabel);
  widthsList = new QComboBox(page);
  widthsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  widthsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  row2Layout->addWidget(widthsList);
  tmp.fill(Qt::white);
  QPainter painter(&tmp);
  for (int i = 1; i < 6; i++) {
    painter.setPen(QPen(QColor(83, 81, 84, 255), i));
    painter.drawLine(0, 12, 25, 12);
    widthsList->addItem(tmp, QString::number(i) + QString(" Pixels"));
  }

  // Create styles list
  QLabel* styleLabel = new QLabel(tr("Style:"), page);
  row2Layout->addWidget(styleLabel);
  stylesList = new QComboBox(page);
  stylesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  stylesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  row2Layout->addWidget(stylesList);
  tmp.fill(Qt::white);
  painter.setPen(QPen(QColor(83, 81, 84, 255), 3, Qt::SolidLine));
  painter.drawLine(0, 12, 25, 12);
  stylesList->addItem(tmp, QString(" Solid"));
  tmp.fill(Qt::white);
  painter.setPen(QPen(QColor(83, 81, 84, 255), 3, Qt::DashLine));
  painter.drawLine(0, 12, 25, 12);
  stylesList->addItem(tmp, QString(" Dash"));
  tmp.fill(Qt::white);
  painter.setPen(QPen(QColor(83, 81, 84, 255), 3, Qt::DotLine));
  painter.drawLine(0, 12, 25, 12);
  stylesList->addItem(tmp, QString(" Dot"));
  tmp.fill(Qt::white);
  painter.setPen(QPen(QColor(83, 81, 84, 255), 3, Qt::DashDotLine));
  painter.drawLine(0, 12, 25, 12);
  stylesList->addItem(tmp, QString(" Dash Dot"));
  tmp.fill(Qt::white);
  painter.setPen(QPen(QColor(83, 81, 84, 255), 3, Qt::DashDotDotLine));
  painter.drawLine(0, 12, 25, 12);
  stylesList->addItem(tmp, QString(" Dash Dot Dot"));

  // Activate button
  row2Layout->addSpacerItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  activateButton = new QPushButton("Enable Channel", page);
  row2Layout->addWidget(activateButton);
  activateButton->setCheckable(true);
  activateButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QObject::connect(
      activateButton, SIGNAL(toggled(bool)), this, SLOT(activateChannel(bool)));

  bttnLayout->addLayout(row1Layout, 0, 0);
  bttnLayout->addLayout(row2Layout, 1, 0);

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

  QWidget* page = new QWidget(parent);

  // Scope properties
  QGridLayout* displayTabLayout = new QGridLayout(page);

  // Create elements for time settings
  QHBoxLayout* row1Layout = new QHBoxLayout;
  row1Layout->addWidget(new QLabel(tr("Time/Div:"), page));
  timesList = new QComboBox(page);
  row1Layout->addWidget(timesList);
  QFont timeListFont("DejaVu Sans Mono");
  timesList->setFont(timeListFont);
  timesList->addItem("5 s/div");
  timesList->addItem("2 s/div");
  timesList->addItem("1 s/div");
  timesList->addItem("500 ms/div");
  timesList->addItem("200 ms/div");
  timesList->addItem("100 ms/div");
  timesList->addItem("50 ms/div");
  timesList->addItem("20 ms/div");
  timesList->addItem("10 ms/div");
  timesList->addItem("5 ms/div");
  timesList->addItem("2 ms/div");
  timesList->addItem("1 ms/div");
  timesList->addItem(QString::fromUtf8("500 µs/div"));
  timesList->addItem(QString::fromUtf8("200 µs/div"));
  timesList->addItem(QString::fromUtf8("100 µs/div"));
  timesList->addItem(QString::fromUtf8("50 µs/div"));
  timesList->addItem(QString::fromUtf8("20 µs/div"));
  timesList->addItem(QString::fromUtf8("10 µs/div"));
  timesList->addItem(QString::fromUtf8("5 µs/div"));
  timesList->addItem(QString::fromUtf8("2 µs/div"));
  timesList->addItem(QString::fromUtf8("1 µs/div"));
  timesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  timesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

  QLabel* refreshLabel = new QLabel(tr("Refresh:"), page);
  row1Layout->addWidget(refreshLabel);
  refreshDropdown = new QComboBox(page);
  row1Layout->addWidget(refreshDropdown);
  refreshDropdown->addItem("60 Hz");
  refreshDropdown->addItem("120 Hz");
  refreshDropdown->addItem("240 Hz");
  //refreshsSpin->setRange(100, 10000);
  //refreshsSpin->setValue(250);

  //QLabel* downsampleLabel = new QLabel(tr("Downsample:"), page);
  //row1Layout->addWidget(downsampleLabel);
  //ratesSpin = new QSpinBox(page);
  //row1Layout->addWidget(ratesSpin);
  //ratesSpin->setValue(downsample_rate);
  //ratesSpin->setEnabled(true);
  //ratesSpin->setRange(1, 2);
  //ratesSpin->setValue(1);

  // Display box for Buffer bit. Push it to the right.
  row1Layout->addSpacerItem(
      new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
  QLabel* bufferLabel = new QLabel(tr("Buffer Size (MB):"), page);
  row1Layout->addWidget(bufferLabel);
  sizesEdit = new QLineEdit(page);
  sizesEdit->setMaximumWidth(sizesEdit->minimumSizeHint().width() * 3);
  sizesEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  row1Layout->addWidget(sizesEdit);
  sizesEdit->setText(QString::number(scopeWindow->getDataSize()));
  sizesEdit->setEnabled(false);

  // Trigger box
  QHBoxLayout* row2Layout = new QHBoxLayout;
  row2Layout->addWidget(new QLabel(tr("Edge:"), page));
  trigsGroup = new QButtonGroup(page);

  QRadioButton* off = new QRadioButton(tr("Off"), page);
  trigsGroup->addButton(off, Oscilloscope::Trigger::NONE);
  row2Layout->addWidget(off);
  QRadioButton* plus = new QRadioButton(tr("+"), page);
  trigsGroup->addButton(plus, Oscilloscope::Trigger::POS);
  row2Layout->addWidget(plus);
  QRadioButton* minus = new QRadioButton(tr("-"), page);
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
  trigsThreshList->addItem("V");
  trigsThreshList->addItem("mV");
  trigsThreshList->addItem(QString::fromUtf8("µV"));
  trigsThreshList->addItem("nV");
  trigsThreshList->addItem("pV");

  row2Layout->addWidget(new QLabel(tr("Window:"), page));
  trigWindowEdit = new QLineEdit(page);
  trigWindowEdit->setText(QString::number(scopeWindow->getWindowTimewidth()));
  trigWindowEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
  trigWindowEdit->setMaximumWidth(trigWindowEdit->minimumSizeHint().width()
                                  * 3);
  trigWindowEdit->setValidator(new QDoubleValidator(trigWindowEdit));
  row2Layout->addWidget(trigWindowEdit);
  trigWindowList = new QComboBox(page);
  trigWindowList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
  row2Layout->addWidget(trigWindowList);
  trigWindowList->addItem("s");
  trigWindowList->addItem("ms");
  trigWindowList->addItem(QString::fromUtf8("µs"));
  trigWindowList->setCurrentIndex(1);

  displayTabLayout->addLayout(row1Layout, 0, 0);
  displayTabLayout->addLayout(row2Layout, 1, 0);

  return page;
}

void Oscilloscope::Panel::syncBlockInfo()
{
  this->buildBlockList();
}

// Aggregates all channel information to show for configuration
// in the display tab
void Oscilloscope::Panel::showChannelTab()
{
  int type_index = this->typesList->currentIndex();
  if (type_index < 0) {
    ERROR_MSG("Oscilloscope::Panel::showChannelTab : invalid type\n");
    typesList->setCurrentIndex(0);
    type_index = 0;
  }
  IO::flags_t type = static_cast<IO::flags_t>(type_index);  

  int block_index = this->blocksListDropdown->currentIndex();
  if (block_index < 0 || block_index >= blocks.size()){
    activateButton->setChecked(false);
    scalesList->setEnabled(false);
    offsetsEdit->setEnabled(false);
    offsetsList->setEnabled(false);
    colorsList->setEnabled(false);
    widthsList->setEnabled(false);
    stylesList->setEnabled(false);
    scalesList->setCurrentIndex(9);
    offsetsEdit->setText(QString::number(0));
    offsetsList->setCurrentIndex(0);
    colorsList->setCurrentIndex(0);
    widthsList->setCurrentIndex(0);
    stylesList->setCurrentIndex(0);
    return;
  }

  auto* block = static_cast<IO::Block*>(blocks[static_cast<size_t>(block_index)]);
  size_t port=0;
  if(this->channelsList->count() != 0) {
    port = static_cast<size_t>(this->channelsList->currentIndex());
  }
  Oscilloscope::probe chan {block, port, type};
  double scale = this->scopeWindow->getChannelScale(chan);
  double offset = this->scopeWindow->getChannelOffset(chan);
  scalesList->setCurrentIndex(static_cast<int>(round(4 * (log10(1 / scale) + 1))));
  int offsetUnits = 0;
  if(offset*std::pow(10, -3*offsetsList->count()-3) < 1) {
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

  // set color
  QPen* pen = this->scopeWindow->getChannelPen(chan);
  const auto* color_loc = std::find(Oscilloscope::penColors.begin(),
                                    Oscilloscope::penColors.end(),
                                    pen->color());
  if (color_loc == Oscilloscope::penColors.end()){
    ERROR_MSG(
        "Oscilloscope::Panel::displayChannelTab : invalid color "
        "selection\n");
    colorsList->setCurrentIndex(0);
  } else {
    colorsList->setCurrentIndex(static_cast<int>(color_loc - Oscilloscope::penColors.begin()));
  }

  // set width
  if(pen->width() < 0 || pen->width() > widthsList->count()){
    ERROR_MSG(
          "Oscilloscope::Panel::displayChannelTab : invalid width "
          "selection\n");
      widthsList->setCurrentIndex(0);
  } else {
    widthsList->setCurrentIndex(pen->width());
  }

  // set style
  switch (pen->style()) {
    case Qt::SolidLine:
      stylesList->setCurrentIndex(0);
      break;
    case Qt::DashLine:
      stylesList->setCurrentIndex(1);
      break;
    case Qt::DotLine:
      stylesList->setCurrentIndex(2);
      break;
    case Qt::DashDotLine:
      stylesList->setCurrentIndex(3);
      break;
    case Qt::DashDotDotLine:
      stylesList->setCurrentIndex(4);
      break;
    default:
      ERROR_MSG(
          "Oscilloscope::Panel::displayChannelTab : invalid style "
          "selection\n");
      stylesList->setCurrentIndex(0);
  }
}

void Oscilloscope::Panel::showDisplayTab()
{
  timesList->setCurrentIndex(
      static_cast<int>(round(3 * log10(1 / scopeWindow->getDivT()) + 11)));

  //refreshsSpin->setValue(scopeWindow->getRefresh());

  // Find current trigger value and update gui
  auto* oscilloscope_plugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  Oscilloscope::Trigger::Info trigInfo = oscilloscope_plugin->getTriggerInfo();
  static_cast<QRadioButton*>(trigsGroup->button(static_cast<int>(trigInfo.trigger_direction)))->setChecked(true);

  trigsChanList->clear();
  std::vector<Oscilloscope::channel_info> channelList = oscilloscope_plugin->getChannelsList();
  std::string direction_str = "";
  for (auto chanInfo : channelList){
    direction_str = chanInfo.probe.direction == IO::INPUT ? "INPUT" : "OUTPUT";
    trigsChanList->addItem(chanInfo.name + 
                           " " + QString::fromStdString(direction_str) +
                           " " + QString::number(chanInfo.probe.port));
  }
  trigsChanList->addItem("<None>");
  auto trig_list_iter = std::find_if(channelList.begin(),
                                     channelList.end(),
                                     [&](const Oscilloscope::channel_info& chan_info){
                                       return chan_info.probe.block == trigInfo.block &&
                                              chan_info.probe.port == trigInfo.port &&
                                              chan_info.probe.direction == trigInfo.io_direction;
                                     });
  if(trig_list_iter != channelList.end()){
    trigsChanList->setCurrentIndex(static_cast<int>(trig_list_iter - channelList.begin() + 1));
  }

  int trigThreshUnits = 0;
  double trigThresh = trigInfo.threshold;
  if (trigThresh*std::pow(10, -3*this->trigsThreshList->count()-1) < 1){
    trigThreshUnits = 0;
    trigThresh = 0;
  } else {
    while (fabs(trigThresh) < 1 && trigThreshUnits < this->trigsThreshList->count()) {
      trigThresh *= 1000;
      ++trigThreshUnits;
    }
  }
  trigsThreshList->setCurrentIndex(trigThreshUnits);
  trigsThreshEdit->setText(QString::number(trigThresh));

  sizesEdit->setText(QString::number(scopeWindow->getDataSize()));
}

Oscilloscope::Panel::Panel(MainWindow* mw, Event::Manager* event_manager) 
  : Modules::Panel(std::string(Oscilloscope::MODULE_NAME), mw, event_manager),
    subWindow(new QMdiSubWindow),
    tabWidget(new QTabWidget),
    scopeWindow(new Scope(this)),
    layout(new QVBoxLayout),
    scopeGroup(new QWidget(this)),
    setBttnGroup(new QGroupBox(this))
{
  // Make Mdi
  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint
                            | Qt::WindowMaximizeButtonHint);
  mw->createMdi(subWindow);

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
      "click the \"Active\" button for it to appear in the window. To change "
      "signal settings, "
      "you must click the \"Apply\" button. The right-click context \"Pause\" "
      "menu item "
      "allows you to start and stop real-time plotting.</p>");

  // Create tab widget
  tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  QObject::connect(
      tabWidget, SIGNAL(currentChanged(int)), this, SLOT(showTab(int)));

  auto* scopeLayout = new QHBoxLayout(this);
  scopeLayout->addWidget(scopeWindow);
  scopeGroup->setLayout(scopeLayout);
  auto* setBttnLayout = new QHBoxLayout(this);

  // Creat buttons
  pauseButton = new QPushButton("Pause");
  pauseButton->setCheckable(true);
  QObject::connect(
      pauseButton, SIGNAL(released()), this, SLOT(togglePause()));
  setBttnLayout->addWidget(pauseButton);
  applyButton = new QPushButton("Apply");
  QObject::connect(
      applyButton, SIGNAL(released()), this, SLOT(apply()));
  setBttnLayout->addWidget(applyButton);
  settingsButton = new QPushButton("Screenshot");
  QObject::connect(
      settingsButton, SIGNAL(released()), this, SLOT(screenshot()));
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
  buildChannelList();
  showDisplayTab();
  subWindow->setWidget(this);
  subWindow->setMinimumSize(subWindow->minimumSizeHint().width(), 450);
  subWindow->resize(subWindow->minimumSizeHint().width() + 50, 600);

  // Initialize vars
  setWindowTitle(QString::fromStdString(this->getName()));

  auto* otimer = new QTimer;
  otimer->setTimerType(Qt::PreciseTimer);
  QObject::connect(
      otimer, SIGNAL(timeout()), this, SLOT(timeoutEvent()));
  otimer->start(Oscilloscope::FrameRates::HZ60);

  QObject::connect(this, SIGNAL(updateBlockInfo()), this, SLOT(syncBlockInfo()));
  scopeWindow->replot();
  show();
}

// TODO: Handle trigger synchronization bettween oscilloscope components 
void Oscilloscope::Component::callback()
{
  Oscilloscope::sample sample {};
  auto state = getValue<Modules::Variable::state_t>(Oscilloscope::PARAMETER::STATE);
  switch(state)
  {
    case Modules::Variable::INIT:{
      this->setValue(Oscilloscope::PARAMETER::STATE, Modules::Variable::EXEC);
      break;
    }
    case Modules::Variable::EXEC:{
      auto triggering = getValue<Modules::Variable::state_t>(Oscilloscope::PARAMETER::TRIGGERING);
      sample.time = RT::OS::getTime();
      std::vector<double> value = this->readinput(0);
      sample.value = value[0];
      this->fifo->writeRT(&sample, sizeof(Oscilloscope::sample));
      break;
    }
    case Modules::Variable::UNPAUSE:{
      this->setValue(Oscilloscope::PARAMETER::STATE, Modules::Variable::EXEC);
      break;
    }
    case Modules::Variable::PAUSE :
    case Modules::Variable::MODIFY :
    case Modules::Variable::EXIT :
    case Modules::Variable::PERIOD :
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
  Event::Type event_type = Event::Type::NOOP;
  if(this->pauseButton->isChecked()) {
    event_type = Event::Type::RT_THREAD_PAUSE_EVENT;
  } else {
    event_type = Event::Type::RT_THREAD_UNPAUSE_EVENT;
  }
  auto* oscilloscope_plugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  std::vector<Oscilloscope::channel_info> channelList = oscilloscope_plugin->getChannelsList();
  std::vector<Event::Object> events;
  for(auto channel : channelList){
    events.emplace_back(event_type);
    events.back().setParam("thread", static_cast<RT::Thread*>(channel.measuring_component));
  }
  this->getRTXIEventManager()->postEvent(events);
}

void Oscilloscope::Component::flushFifo()
{
  Oscilloscope::sample sample;
  while(this->fifo->read(&sample, sizeof(Oscilloscope::sample)) > 0) {}
}

void Oscilloscope::Panel::adjustDataSize()
{
  Event::Object event(Event::Type::RT_GET_PERIOD_EVENT);
  this->getRTXIEventManager()->postEvent(&event);
  auto period = std::any_cast<int64_t>(event.getParam("period"));
  size_t size =
      ceil(scopeWindow->getDivT() * scopeWindow->getDivX() / period) + 1;
  scopeWindow->setDataSize(size);
  sizesEdit->setText(QString::number(scopeWindow->getDataSize()));
}

void Oscilloscope::Panel::timeoutEvent()
{
  Oscilloscope::sample sample;
  std::vector<Oscilloscope::sample> sample_vector;
  size_t sample_count = this->scopeWindow->getDataSize();
  auto* oscilloscope_plugin = dynamic_cast<Oscilloscope::Plugin*>(this->getHostPlugin());
  for(auto channel : oscilloscope_plugin->getChannelsList()){
    while(channel.fifo->read(&sample, sizeof(Oscilloscope::sample)) > 0){
      sample_vector.push_back(sample);
    }
    this->scopeWindow->setData(channel.probe, sample_vector);
    sample_vector.assign(sample_count, {0.0, 0}); 
  }
  this->scopeWindow->drawCurves();
  //size_t size;
  //while (fifo.read(&size, sizeof(size), false)) {
  //  double data[size];
  //  if (fifo.read(data, sizeof(data)))
  //    scopeWindow->setData(data, size);
  //}
}

Oscilloscope::Plugin::Plugin(Event::Manager* ev_manager, MainWindow* main_window)
  : Modules::Plugin(ev_manager, main_window, std::string(Oscilloscope::MODULE_NAME))
{}

Oscilloscope::Plugin::~Plugin()
{
  std::vector<Event::Object> unloadEvents;
  for(auto& oscilloscope_component : this->componentList){
    unloadEvents.emplace_back(Event::Type::RT_THREAD_REMOVE_EVENT);
    unloadEvents.back().setParam("thread", std::any(static_cast<RT::Thread*>(&oscilloscope_component)));
  }
  this->event_manager->postEvent(unloadEvents);
}

std::unique_ptr<Modules::Plugin> Oscilloscope::createRTXIPlugin(
    Event::Manager* ev_manager, MainWindow* main_window)
{
  return std::make_unique<Oscilloscope::Plugin>(ev_manager,
                                                main_window);
}

Modules::Panel* Oscilloscope::createRTXIPanel(
    MainWindow* main_window, Event::Manager* ev_manager)
{
  return static_cast<Modules::Panel*>(new Oscilloscope::Panel(
      main_window,
      ev_manager));
}

std::unique_ptr<Modules::Component> Oscilloscope::createRTXIComponent(
    Modules::Plugin* host_plugin)
{
  return std::unique_ptr<Oscilloscope::Component>(nullptr);
}

Modules::FactoryMethods Oscilloscope::getFactories()
{
  Modules::FactoryMethods fact;
  fact.createPanel = &Oscilloscope::createRTXIPanel;
  fact.createComponent = &Oscilloscope::createRTXIComponent;
  fact.createPlugin = &Oscilloscope::createRTXIPlugin;
  return fact;
}

// void Oscilloscope::Panel::doDeferred(const Settings::Object::State& s)
// {
//   bool active = setInactiveSync();
// 
//   for (size_t i = 0, nchans = s.loadInteger("Num Channels"); i < nchans; ++i) {
//     std::ostringstream str;
//     str << i;
// 
//     IO::Block* block =
//         dynamic_cast<IO::Block*>(Settings::Manager::getInstance()->getObject(
//             s.loadInteger(str.str() + " ID")));
//     if (!block)
//       continue;
// 
//     struct channel_info* info = new struct channel_info;
// 
//     info->block = block;
//     info->type = s.loadInteger(str.str() + " type");
//     info->index = s.loadInteger(str.str() + " index");
//     info->name = QString::number(block->getID()) + " "
//         + QString::fromStdString(block->getName(info->type, info->index));
//     info->previous = 0.0;
// 
//     QwtPlotCurve* curve = new QwtPlotCurve(info->name);
// 
//     std::list<Scope::Channel>::iterator chan = scopeWindow->insertChannel(
//         info->name,
//         s.loadDouble(str.str() + " scale"),
//         s.loadDouble(str.str() + " offset"),
//         QPen(QColor(QString::fromStdString(
//                  s.loadString(str.str() + " pen color"))),
//              s.loadInteger(str.str() + " pen width"),
//              Qt::PenStyle(s.loadInteger(str.str() + " pen style"))),
//         curve,
//         info);
// 
//     scopeWindow->setChannelLabel(
//         chan,
//         info->name + " - "
//             + scalesList
//                   ->itemText(static_cast<int>(
//                       round(4 * (log10(1 / chan->getScale()) + 1))))
//                   .simplified());
//   }
// 
//   flushFifo();
//   setActive(active);
// }
// 
// void Oscilloscope::Panel::doLoad(const Settings::Object::State& s)
// {
//   scopeWindow->setDataSize(s.loadInteger("Size"));
//   scopeWindow->setDivT(s.loadDouble("DivT"));
// 
//   if (s.loadInteger("Maximized"))
//     scopeWindow->showMaximized();
//   else if (s.loadInteger("Minimized"))
//     scopeWindow->showMinimized();
// 
//   if (scopeWindow->paused() != s.loadInteger("Paused"))
//     togglePause();
// 
//   scopeWindow->setRefresh(s.loadInteger("Refresh"));
// 
//   subWindow->resize(s.loadInteger("W"), s.loadInteger("H"));
//   parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
// }
// 
// void Oscilloscope::Panel::doSave(Settings::Object::State& s) const
// {
//   s.saveInteger("Size", scopeWindow->getDataSize());
//   s.saveInteger("DivX", scopeWindow->getDivX());
//   s.saveInteger("DivY", scopeWindow->getDivY());
//   s.saveDouble("DivT", scopeWindow->getDivT());
// 
//   if (isMaximized())
//     s.saveInteger("Maximized", 1);
//   else if (isMinimized())
//     s.saveInteger("Minimized", 1);
// 
//   s.saveInteger("Paused", scopeWindow->paused());
//   s.saveInteger("Refresh", scopeWindow->getRefresh());
// 
//   s.saveInteger("X", parentWidget()->pos().x());
//   s.saveInteger("Y", parentWidget()->pos().y());
//   s.saveInteger("W", width());
//   s.saveInteger("H", height());
// 
//   s.saveInteger("Num Channels", scopeWindow->getChannelCount());
//   size_t n = 0;
//   for (std::list<Scope::Channel>::const_iterator
//            i = scopeWindow->getChannelsBegin(),
//            end = scopeWindow->getChannelsEnd();
//        i != end;
//        ++i)
//   {
//     std::ostringstream str;
//     str << n++;
// 
//     const struct channel_info* info =
//         reinterpret_cast<const struct channel_info*>(i->getInfo());
//     s.saveInteger(str.str() + " ID", info->block->getID());
//     s.saveInteger(str.str() + " type", info->type);
//     s.saveInteger(str.str() + " index", info->index);
// 
//     s.saveDouble(str.str() + " scale", i->getScale());
//     s.saveDouble(str.str() + " offset", i->getOffset());
// 
//     s.saveString(str.str() + " pen color",
//                  i->getPen().color().name().toStdString());
//     s.saveInteger(str.str() + " pen style", i->getPen().style());
//     s.saveInteger(str.str() + " pen width", i->getPen().width());
//   }
// }

