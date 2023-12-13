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

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QProcess>
#include <QtGlobal>

#include "rtxi_wizard.hpp"

#include <unistd.h>

#include "rtxiConfig.h"

/*
 * This module uses the QNetworkManager class to fetch information on our
 * GitHub repos using GitHub's own API.
 */
RTXIWizard::Panel::Panel(QMainWindow* mwindow, Event::Manager* ev_manager)
    : Widgets::Panel(std::string(RTXIWizard::MODULE_NAME), mwindow, ev_manager)
    , readmeWindow(new QTextEdit)
    , availableListWidget(new QListWidget(this))
{
  setWhatsThis(
      "Module Wizard enables management of all RTXI modules. You can download "
      "and install new modules directly from the GitHub site..etc");
  install_prefix.setPath(QCoreApplication::applicationDirPath()
                         + QString("/rtxi_modules/"));
  auto* customLayout = new QGridLayout;

  auto* buttonBox = new QGroupBox;
  auto* buttonLayout = new QHBoxLayout();
  buttonBox->setLayout(buttonLayout);
  syncButton = new QPushButton("Sync", this);
  cloneButton = new QPushButton("Install", this);
  updateAllButton = new QPushButton("Update Installed", this);
  cloneButton->setEnabled(false);
  buttonLayout->addWidget(syncButton);
  buttonLayout->addWidget(cloneButton);

  auto* installedBox = new QGroupBox("Installed");
  auto* installedLayout = new QVBoxLayout;
  installedBox->setLayout(installedLayout);
  installedListWidget = new QListWidget(installedBox);
  installedListWidget->setFixedWidth(175);
  installedListWidget->setSortingEnabled(true);
  installedLayout->addWidget(installedListWidget);

  auto* moduleBox = new QGroupBox("Available");
  auto* moduleLayout = new QVBoxLayout;
  moduleBox->setLayout(moduleLayout);

  availableListWidget->setFixedWidth(175);
  availableListWidget->setSortingEnabled(true);
  moduleLayout->addWidget(availableListWidget);

  readmeWindow->setReadOnly(true);
  readmeWindow->show();

  customLayout->addWidget(buttonBox, 0, 0);
  customLayout->addWidget(updateAllButton, 1, 0);
  customLayout->addWidget(moduleBox, 2, 0);
  customLayout->addWidget(installedBox, 3, 0);
  customLayout->addWidget(readmeWindow, 0, 1, 4, 1);
  customLayout->setColumnStretch(0, 0);
  customLayout->setColumnStretch(1, 1);

  QObject::connect(syncButton, 
                   &QPushButton::clicked, 
                   this, 
                   &RTXIWizard::Panel::getRepos);
  QObject::connect(cloneButton, 
                   &QPushButton::clicked, 
                   this, 
                   &RTXIWizard::Panel::cloneModule);
  QObject::connect(updateAllButton,
                   &QPushButton::clicked,
                   this,
                   &RTXIWizard::Panel::updateAllInstalledModules);
  QObject::connect(availableListWidget,
                   &QListWidget::itemClicked,
                   this,
                   &RTXIWizard::Panel::getReadme);
  QObject::connect(availableListWidget,
                   &QListWidget::itemClicked,
                   this,
                   &RTXIWizard::Panel::updateButton);
  QObject::connect(installedListWidget,
                   &QListWidget::itemClicked,
                   this,
                   &RTXIWizard::Panel::getReadme);
  QObject::connect(installedListWidget,
                   &QListWidget::itemClicked,
                   this,
                   &RTXIWizard::Panel::updateButton);

  setLayout(customLayout);
  setWindowTitle("Module Wizard");
  getRepos();
  initParameters();
  getMdiWindow()->resize(1000, 800);
}

void RTXIWizard::Panel::initParameters()
{
  // syntax here only works in c++11
  exclude_list = std::vector<QString>({QString("rtxi"),
                                       QString("rtxi.github.io"),
                                       QString("rtxi-crawler"),
                                       QString("matlab-tools"),
                                       QString("tutorials"),
                                       QString("autapse"),
                                       QString("camera-control"),
                                       QString("gen-net"),
                                       QString("dynamo-examples"),
                                       QString("plot-lib"),
                                       QString("python-plugin"),
                                       QString("poster"),
                                       QString("user-manual"),
                                       QString("logos"),
                                       QString("live-image"),
                                       QString("conference-2015")});
  button_mode = DOWNLOAD;
}

// Set the text to "Update" if the module is already installed or "Download
// and Install" if not.
void RTXIWizard::Panel::updateButton()
{
  auto* parent = qobject_cast<QListWidget*>(sender());

  if (parent == availableListWidget) {
    cloneButton->setText("Install");
    button_mode = DOWNLOAD;
  } else if (parent == installedListWidget) {
    cloneButton->setText("Update");
    button_mode = UPDATE;
  }
}

// Clone the module currently highlighted in the QListWidget.
void RTXIWizard::Panel::cloneModule()
{
  cloneButton->setEnabled(false);
  availableListWidget->setDisabled(true);
  installedListWidget->setDisabled(true);
  // QApplication::processEvents();

  QString name;
  switch (button_mode) {
    case DOWNLOAD:
      name = availableListWidget->currentItem()->text();
      break;
    case UPDATE:
      name = installedListWidget->currentItem()->text();
      break;
    default:
      ERROR_MSG("ERROR: default in switch block in cloneModule()");
      break;
  }

  installFromString(name.toStdString());
}

void RTXIWizard::Panel::updateAllInstalledModules()
{
  if (installedListWidget->count() <= 0) {
    return;
  }
  int plugin_count = installedListWidget->count();
  auto* progress = new QProgressDialog(
      "Updating installed plugins", "Cancel", 0, plugin_count + 1, this);
  progress->move(this->pos());
  progress->setMinimumDuration(0);
  for (int widget_index = 0; widget_index < plugin_count; widget_index++) {
    if (progress->wasCanceled()) {
      progress->close();
      return;
    }
    progress->setLabelText(QString("Installing ")
                           + installedListWidget->item(widget_index)->text());
    installFromString(
        installedListWidget->item(widget_index)->text().toStdString());
    progress->setValue(widget_index + 1);
  }
  progress->close();
}

// Download the list of repos from GitHub's API. Call parseRepos for the JSON.
void RTXIWizard::Panel::getRepos()
{
  availableListWidget->setDisabled(true);
  installedListWidget->setDisabled(true);

  if (availableListWidget->count() == 0) {
    const QUrl url("https://api.github.com/orgs/rtxi/repos?per_page=100");
    reposNetworkReply = qnam.get(QNetworkRequest(url));
    QObject::connect(
        reposNetworkReply, SIGNAL(finished()), this, SLOT(parseRepos()));
  } else {
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
  }
}

/*
 * Download the README (markdown) for the highlighted repo just clicked. If the
 *  README has already been downloaded, the function will just keep that and
 * not make another network request.
 *
 * The module doesn't save READMEs for after it closes. If you reopen the
 * module, you'll have to redownload the repos.
 */
void RTXIWizard::Panel::getReadme()
{
  auto* parent = qobject_cast<QListWidget*>(sender());
  if (parent->currentRow() < 0) {
    return;
  }
  const QString name = parent->currentItem()->text();
  availableListWidget->setDisabled(true);
  installedListWidget->setDisabled(true);

  // If the README hasn't been downloaded before, download it now.
  if (modules[parent->currentItem()->text()].readme == "") {
    readmeNetworkReply = qnam.get(QNetworkRequest(modules[name].readme_url));
    QObject::connect(
        readmeNetworkReply, SIGNAL(finished()), this, SLOT(parseReadme()));
  } else {
    // Disable buttons until all logic is done.
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    readmeWindow->setText(modules[parent->currentItem()->text()].readme);
#else
    readmeWindow->setMarkdown(modules[parent->currentItem()->text()].readme);
#endif
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
  }
}

// READMEs are downloaded as markdown. Convert them to HTML and display them
// within a QTextWidget.
void RTXIWizard::Panel::parseReadme()
{
  const QByteArray network_reply_data = readmeNetworkReply->readAll();
  const QString markdown_data = QString(network_reply_data.constData());
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  this->readmeWindow->setText(markdown_data);
#else
  this->readmeWindow->setMarkdown(markdown_data);
#endif

  switch (button_mode) {
    case DOWNLOAD:
      modules[availableListWidget->currentItem()->text()].readme =
          markdown_data;
      break;
    case UPDATE:
      modules[installedListWidget->currentItem()->text()].readme =
          markdown_data;
      break;
    default:
      ERROR_MSG("ERROR: default in switch block in cloneModule()");
      break;
  }

  // readmeWindow->setHtml(fileText);
  readmeWindow->show();
  readmeNetworkReply->deleteLater();

  // The README is now displayed, so free the user to start clicking around.
  cloneButton->setEnabled(true);
  availableListWidget->setDisabled(false);
  installedListWidget->setDisabled(false);
}

// GitHub's API returns a JSON array. Parse it with QtJson functions.
void RTXIWizard::Panel::parseRepos()
{
  const QJsonDocument jsonDoc =
      QJsonDocument::fromJson(reposNetworkReply->readAll().data());
  const QJsonArray jsonArr = jsonDoc.array();

  const QString readmeUrlPrefix = "https://raw.githubusercontent.com/RTXI/";
  const QString readmeUrlSuffix = "/master/README.md";

  for (auto&& idx : jsonArr) {
    QJsonObject newObj = (idx).toObject();
    newObj.find("name").key();

    // if the current module isn't in the exclude_list
    if (std::find(exclude_list.begin(),
                  exclude_list.end(),
                  newObj.value("name").toString())
        == exclude_list.end())
    {
      module_t module;

      const QString name = newObj.value("name").toString();
      module.readme_url = QUrl(readmeUrlPrefix + newObj.value("name").toString()
                               + readmeUrlSuffix);
      module.clone_url = QUrl(newObj.value("clone_url").toString());
      module.readme = "";
      module.installed =
          QFileInfo::exists(install_prefix.path() + QString("/")
                            + QString("lib") + name + QString(".so"));
      modules[name] = module;
    }
  }

  reposNetworkReply->deleteLater();

  rebuildListWidgets();
  availableListWidget->setDisabled(false);
  installedListWidget->setDisabled(false);
}

void RTXIWizard::Panel::rebuildListWidgets()
{
  availableListWidget->clear();
  installedListWidget->clear();

  for (auto& module : modules) {
    if (module.second.installed) {
      installedListWidget->addItem(module.first);
    } else {
      availableListWidget->addItem(module.first);
    }
  }

  installedListWidget->sortItems(Qt::AscendingOrder);
  availableListWidget->sortItems(Qt::AscendingOrder);
}

/*
 * Public function, not for use in this module. It gets called by other
 * classes. The function is basically a truncated version of cloneModule().
 */
void RTXIWizard::Panel::installFromString(const std::string& module_name)
{
  const QString name = QString(module_name.c_str());
  // Let the user know that RTXI is installing the plugin.
  auto* progress =
      new QProgressDialog("Installing plugin", "Cancel", 0, 5, this);
  progress->setMinimumDuration(0);
  progress->setWindowModality(Qt::WindowModal);
  progress->setLabelText("Starting...");
  /*
   * Two QByteArray variables are needed due to the way Qt stores binary data.
   * Calling module->getCloneUrl().toString().toLatin1().data() will produce
   * an error.
   */
  const QByteArray url = modules[name].clone_url.toString().toLatin1();
  const QByteArray installpath = install_prefix.path().toLatin1();
  const QString mod_location =
      QDir::temp().absolutePath() + QDir::separator() + QString("rtxi_modules");
  const QString source_location =
      mod_location + QDir::separator() + QString(module_name.c_str());
  QDir(source_location).removeRecursively();

  progress->setLabelText("Downloading extension...");
  progress->setValue(1);
  // QApplication::processEvents();
  //  If the repo already exists, pull from master. If not, clone it.
  auto* command = new QProcess();
  const QStringList clone_args = {
      "clone",
      "-b",
      "master",
      url,
      source_location,
  };

  const std::string git_command(GIT_COMMAND);
  if (!(QDir(source_location)).exists()) {
    command->start(QString::fromStdString(git_command), clone_args);
    command->waitForFinished();
  }

  if (command->exitStatus() != QProcess::NormalExit) {
    ERROR_MSG("Could not complete installation for module {}",
              name.toStdString());
    // Re-enable buttons only after compilation is done. Otherwise you get race
    // conditions if buttons are pressed before modules are done compiling.
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    return;
  }
  if (progress->wasCanceled()) {
    progress->close();
    return;
  }
  // Define the commands to be run.
  QDir package_dir = install_prefix;
  package_dir.cdUp();
  package_dir.cdUp();
  const QString build_location = source_location + QString("/build");
  const QString make_cmd = "cmake";
  const QStringList make_config_args = {
      "-S",
      source_location,
      "-B",
      build_location,
      QString("-DRTXI_PACKAGE_PATH=") + package_dir.path(),
      QString("-DCMAKE_BUILD_TYPE=")
          + QString::fromStdString(std::string(RTXI_BUILD_TYPE)),
      QString("-DRTXI_CMAKE_SCRIPTS=")
          + QString::fromStdString(std::string(RTXI_CMAKE_SCRIPTS))};
  const QStringList make_build_args = {"--build", build_location, "-j2"};
  const QStringList make_install_args = {"--install", build_location};

  progress->setLabelText("Configuring...");
  progress->setValue(2);
  // QApplication::processEvents();

  // Compile and install handled by QProcess.
  command->start(make_cmd, make_config_args);
  command->waitForFinished();
  if (command->exitStatus() != QProcess::NormalExit || command->exitCode() != 0)
  {
    QMessageBox::critical(
        nullptr,
        "Error",
        "Could not Configure plugin. Email help@rtxi.org for assistance");
    const QByteArray err_str = command->readAllStandardError();
    ERROR_MSG("{}", err_str.toStdString());
    ERROR_MSG("Configure command for module {} failed with command {}",
              name.toStdString(),
              make_cmd.toStdString() + std::string(" ")
                  + make_config_args.join(" ").toStdString());
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    progress->close();
    return;
  }
  if (progress->wasCanceled()) {
    progress->close();
    return;
  }

  progress->setLabelText("Building...");
  progress->setValue(3);
  // QApplication::processEvents();
  command->start(make_cmd, make_build_args);
  command->waitForFinished();
  if (command->exitStatus() != QProcess::NormalExit || command->exitCode() != 0)
  {
    QMessageBox::critical(
        nullptr,
        "Error",
        "Could not build plugin. Email help@rtxi.org for assistance");
    const QByteArray err_str = command->readAllStandardError();
    ERROR_MSG("{}", err_str.toStdString());
    ERROR_MSG("Build command for module {} failed with command {}",
              name.toStdString(),
              make_cmd.toStdString() + std::string(" ")
                  + make_build_args.join(" ").toStdString());
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    return;
  }

  progress->setLabelText("Installing...");
  progress->setValue(4);
  // QApplication::processEvents();
  command->start(make_cmd, make_install_args);
  command->waitForFinished();
  if (command->exitStatus() != QProcess::NormalExit || command->exitCode() != 0)
  {
    QMessageBox::critical(
        nullptr,
        "Error",
        "Could not install plugin. Email help@rtxi.org for assistance");
    const QByteArray err_str = command->readAllStandardError();
    ERROR_MSG("{}", err_str.toStdString());
    ERROR_MSG("Install command for module {} failed with command {}",
              name.toStdString(),
              make_cmd.toStdString() + std::string(" ")
                  + make_install_args.join(" ").toStdString());
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    return;
  }
  if (progress->wasCanceled()) {
    progress->close();
    return;
  }
  // Add module to list of already installed modules.
  modules[name].installed = true;
  progress->setValue(5);
  // QApplication::processEvents();
  command->close();
  progress->close();
  cloneButton->setEnabled(true);
  rebuildListWidgets();
  availableListWidget->setDisabled(false);
  installedListWidget->setDisabled(false);
}

std::unique_ptr<Widgets::Plugin> RTXIWizard::createRTXIPlugin(
    Event::Manager* ev_manager)
{
  return std::make_unique<RTXIWizard::Plugin>(ev_manager);
}

Widgets::Panel* RTXIWizard::createRTXIPanel(QMainWindow* main_window,
                                            Event::Manager* ev_manager)
{
  return static_cast<Widgets::Panel*>(
      new RTXIWizard::Panel(main_window, ev_manager));
}

std::unique_ptr<Widgets::Component> RTXIWizard::createRTXIComponent(
    Widgets::Plugin* /*unused*/)
{
  return {nullptr};
}

Widgets::FactoryMethods RTXIWizard::getFactories()
{
  Widgets::FactoryMethods fact;
  fact.createPanel = &RTXIWizard::createRTXIPanel;
  fact.createComponent = &RTXIWizard::createRTXIComponent;
  fact.createPlugin = &RTXIWizard::createRTXIPlugin;
  return fact;
}
