/*
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

#ifndef RTXI_WIZARD_H
#define RTXI_WIZARD_H

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QTextEdit>
#include <QListWidget>
#include <QDir>
#include <map>
#include "module.hpp"

namespace RTXIWizard
{

constexpr std::string_view MODULE_NAME = "Module Wizard";

class Panel : public Modules::Panel 
{
  Q_OBJECT

public:
  Panel(QMainWindow* mwindow, Event::Manager* ev_manager);
  void installFromString(const std::string& module_name);
  void rebuildListWidgets();

private slots:
  void cloneModule();
  void getRepos();
  void getReadme();
  void updateButton();
  void parseRepos();
  void parseReadme();

private:
  QDir install_prefix;

  struct module_t
  {
    QUrl readme_url;
    QUrl clone_url;
    QString install_location;
    QString readme;
    bool installed;
  };

  std::map<QString, module_t> modules;
  void initParameters();
  static int printGitError(int);
  enum button_mode_t
  {
    DOWNLOAD,
    UPDATE
  } button_mode;

  QNetworkAccessManager qnam;
  QNetworkReply* reposNetworkReply=nullptr;
  QNetworkReply* readmeNetworkReply=nullptr;
  QProgressDialog* progressDialog=nullptr;

  QTextEdit* readmeWindow=nullptr;
  QListWidget* availableListWidget=nullptr;
  QListWidget* installedListWidget=nullptr;

  QPushButton* cloneButton=nullptr;
  QPushButton* syncButton=nullptr;

  std::vector<QString> exclude_list;
};

class Plugin : public Modules::Plugin 
{
public:
  explicit Plugin(Event::Manager* ev_manager)
    : Modules::Plugin(ev_manager, std::string(RTXIWizard::MODULE_NAME)) {}
};  // class Plugin

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Modules::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(
    Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();

};  // namespace RTXIWizard

#endif
