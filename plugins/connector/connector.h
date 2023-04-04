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

#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <vector>

#include <QMdiArea>
#include <QWidget>
#include <QtWidgets>

#include "io.hpp"
#include "fifo.hpp"
#include "event.hpp"
#include "module.hpp"

namespace Connector
{
constexpr std::string_view MODULE_NAME = "Connector";

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(MainWindow* mw, Event::Manager* event_manager);

signals:
  void updateBlockInfo();

private slots:
  void buildInputChannelList();
  void buildOutputChannelList();
  void highlightConnectionBox(QListWidgetItem*);
  void toggleConnection(bool);
  void updateConnectionButton();
  void syncBlockInfo();

private:
  void buildConnectionList();
  void buildBlockList();

  QMdiSubWindow* subWindow;

  QGroupBox* connectionGroup;
  QGroupBox* buttonGroup;
  QGroupBox* outputGroup;
  QGroupBox* inputGroup;
  QComboBox* inputBlock;
  QComboBox* inputChannel;
  QComboBox* outputBlock;
  QComboBox* outputChannel;
  QListWidget* connectionBox;
  QPushButton* connectionButton;
  std::vector<IO::Block*> blocks;
  std::vector<RT::block_connection_t> links;
};  // class Panel

class Plugin : public Modules::Plugin
{
public:
  Plugin(Event::Manager* ev_manager, MainWindow* mw);
  void receiveEvent(Event::Object* event) override;

private:
  void updatePanelInfo();
  //void receiveEvent(Event::Object* event) override;

};  // class Plugin

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager, MainWindow* main_window);

Modules::Panel* createRTXIPanel(MainWindow* main_window, Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();

};  // namespace Connector
#endif  // CONNECTOR_H
