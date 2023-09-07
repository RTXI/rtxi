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

#include <iostream>

#include <git2.h>
#include <unistd.h>
extern "C" {
#include <mkdio.h>
}
#include <QMessageBox>
#include <QJsonArray>
#include <QJsonDocument>
#include <QCoreApplication>
#include <QDir>
#include <QJsonObject>
#include <QProcess>
#include "rtxi_wizard.h"

/*
 * This module uses the QNetworkManager class to fetch information on our
 * GitHub repos using GitHub's own API.
 */
RTXIWizard::Panel::Panel(QMainWindow* mwindow, Event::Manager* ev_manager)
    : Modules::Panel(std::string(RTXIWizard::MODULE_NAME), mwindow, ev_manager)
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
  customLayout->addWidget(moduleBox, 1, 0);
  customLayout->addWidget(installedBox, 2, 0);
  customLayout->addWidget(readmeWindow, 0, 1, 3, 1);
  customLayout->setColumnStretch(0, 0);
  customLayout->setColumnStretch(1, 1);

  QObject::connect(syncButton, SIGNAL(clicked()), this, SLOT(getRepos()));
  QObject::connect(cloneButton, SIGNAL(clicked()), this, SLOT(cloneModule()));
  QObject::connect(availableListWidget,
                   SIGNAL(itemClicked(QListWidgetItem*)),
                   this,
                   SLOT(getReadme()));
  QObject::connect(availableListWidget,
                   SIGNAL(itemClicked(QListWidgetItem*)),
                   this,
                   SLOT(updateButton()));
  QObject::connect(installedListWidget,
                   SIGNAL(itemClicked(QListWidgetItem*)),
                   this,
                   SLOT(getReadme()));
  QObject::connect(installedListWidget,
                   SIGNAL(itemClicked(QListWidgetItem*)),
                   this,
                   SLOT(updateButton()));

  setLayout(customLayout);
  setWindowTitle("Module Wizard");
  getRepos();
  initParameters();
  getMdiWindow()->resize(1000, 800);
}

void RTXIWizard::Panel::initParameters()
{
  git_libgit2_init();

  // syntax here only works in c++11
  exclude_list = std::vector<QString>({QString("rtxi"),
                                       QString("rtxi.github.io"),
                                       QString("genicam-camera"),
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
  //QApplication::processEvents();

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

// Download the list of repos from GitHub's API. Call parseRepos for the JSON.
void RTXIWizard::Panel::getRepos()
{
  availableListWidget->setDisabled(true);
  installedListWidget->setDisabled(true);

  if (availableListWidget->count() == 0) {
    const QUrl url("https://api.github.com/orgs/rtxi/repos?per_page=100");
    reposNetworkReply = qnam.get(QNetworkRequest(url));
    QObject::connect(reposNetworkReply, SIGNAL(finished()), this, SLOT(parseRepos()));
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
  if(parent->currentRow() < 0) { return; }
  const QString name = parent->currentItem()->text();
  availableListWidget->setDisabled(true);
  installedListWidget->setDisabled(true);

  // If the README hasn't been downloaded before, download it now.
  if (modules[parent->currentItem()->text()].readme == "") {
    readmeNetworkReply = qnam.get(QNetworkRequest(modules[name].readme_url));
    QObject::connect(readmeNetworkReply, SIGNAL(finished()), this, SLOT(parseReadme()));
  } else {
    // Disable buttons until all logic is done.
    readmeWindow->setHtml(modules[parent->currentItem()->text()].readme);
    cloneButton->setEnabled(true);
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
  }
}

// READMEs are downloaded as markdown. Convert them to HTML and display them
// within a QTextWidget.
void RTXIWizard::Panel::parseReadme()
{
  QByteArray data = readmeNetworkReply->readAll();
  const char* raw_data = data.constData();
  MMIOT* m = mkd_string(raw_data, data.size(), 0);
  mkd_compile(m, 0);

  char* text = nullptr;
  auto len = static_cast<size_t>(mkd_document(m, &text));
  const std::string html(text, len);

  mkd_cleanup(m);
  const QString fileText = QString::fromStdString(html);

  switch (button_mode) {
    case DOWNLOAD:
      modules[availableListWidget->currentItem()->text()].readme = fileText;
      break;
    case UPDATE:
      modules[installedListWidget->currentItem()->text()].readme = fileText;
      break;
    default:
      ERROR_MSG("ERROR: default in swtich block in cloneModule()");
      break;
  }

  readmeWindow->setHtml(fileText);
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
  const QJsonDocument jsonDoc = QJsonDocument::fromJson(reposNetworkReply->readAll().data());
  const QJsonArray jsonArr = jsonDoc.array();

  const QString readmeUrlPrefix = "https://raw.githubusercontent.com/RTXI/";
  const QString readmeUrlSuffix = "/master/README.md";
  const QString locationPrefix = install_prefix.path();

  for (auto && idx : jsonArr) {
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
      module.install_location = (locationPrefix + name);
      module.readme = "";

      module.installed = (QDir(module.install_location)).exists();
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

  for (auto & module : modules)
  {
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
  const QString name = QString::fromStdString(module_name);
  // Let the user know that RTXI is installing the plugin.
  auto* progress = new QProgressDialog("Installing plugin", 
                                       "Cancel", 
                                       0, 
                                       5, 
                                       this);
  progress->setMinimumDuration(0);
  progress->setWindowModality(Qt::WindowModal);
  progress->setLabelText("Starting...");
  /*
   * Two QByteArray variables are needed due to the way Qt stores binary data.
   * Calling module->getCloneUrl().toString().toLatin1().data() will produce
   * an error.
   */
  const QByteArray url = modules[name].clone_url.toString().toLatin1();
  const QByteArray installpath = modules[name].install_location.toLatin1();
  const QString source_location = QDir::temp().absolutePath() 
    + QString("/rtxi_modules/") 
    + QString::fromStdString(module_name);
  QDir(source_location).removeRecursively(); 
  const QByteArray clonepath = source_location.toLatin1();

  int error = 0;

  progress->setLabelText("Downloading extension...");
  progress->setValue(1);
  //QApplication::processEvents();
  // If the repo already exists, pull from master. If not, clone it.
  if(!(QDir(source_location)).exists()) {
    git_repository* repo = nullptr;
    git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
    opts.checkout_branch = "rtxi3";
    error = printGitError(git_clone(&repo, url, clonepath, &opts));
    git_repository_free(repo);
    if(error != 0) { return; }
  }

  git_repository* repo = nullptr;
  git_remote* remote = nullptr;
  git_repository_open(&repo, clonepath);
  error += printGitError(git_remote_lookup(&remote, repo, "origin"));
  error += printGitError(
      git_remote_connect(remote, GIT_DIRECTION_FETCH, nullptr, nullptr, nullptr));
  error += printGitError(git_remote_download(remote, nullptr, nullptr));

  git_remote_disconnect(remote);
  git_remote_free(remote);
  git_repository_free(repo);

  if (error != 0) {
    ERROR_MSG("Could not complete installation for module {}", name.toStdString());
    // Re-enable buttons only after compilation is done. Otherwise you get race
    // conditions if buttons are pressed before modules are done compiling.
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    return;
  } 
  // Define the commands to be run.
  QDir package_dir = install_prefix;
  package_dir.cdUp();
  package_dir.cdUp();
  const QString build_location = source_location + QString("/build");
  const QString make_cmd = "cmake";
  const QStringList make_config_args = 
    {"-S", source_location , "-B", build_location, QString("-DRTXI_PACKAGE_PATH=")+package_dir.path()};
  const QStringList make_build_args = {"--build", build_location, "-j2"};
  const QStringList make_install_args = 
    {"--install", build_location};

  progress->setLabelText("Configuring...");
  progress->setValue(2);
  //QApplication::processEvents();

  // Compile and instal handled by QProcess.
  auto* command = new QProcess();
  command->start(make_cmd, make_config_args);
  command->waitForFinished();
  if(command->exitStatus() != QProcess::NormalExit || command->exitCode() != 0) {
    QMessageBox::critical(
        nullptr,
        "Error",
        "Could not Configure plugin. Email help@rtxi.org for assistance");
    QByteArray err_str = command->readAllStandardError();
    ERROR_MSG("{}", err_str.toStdString());
    ERROR_MSG("Configure command for module {} failed with command {}", 
              name.toStdString(),
              make_cmd.toStdString()+std::string(" ")+make_config_args.join(" ").toStdString());
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    progress->close();
    return;
  } 

  progress->setLabelText("Building...");
  progress->setValue(3);
  //QApplication::processEvents();
  command->start(make_cmd, make_build_args);
  command->waitForFinished();
  if (command->exitStatus() != QProcess::NormalExit || command->exitCode() != 0) {
    QMessageBox::critical(
        nullptr,
        "Error",
        "Could not build plugin. Email help@rtxi.org for assistance");
    QByteArray err_str = command->readAllStandardError();
    ERROR_MSG("{}", err_str.toStdString());
    ERROR_MSG("Build command for module {} failed with command {}", 
              name.toStdString(),
              make_cmd.toStdString()+std::string(" ")+make_build_args.join(" ").toStdString());
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    progress->close();
    return;
  }

  progress->setLabelText("Installing...");
  progress->setValue(4);
  //QApplication::processEvents();
  command->start(make_cmd, make_install_args);
  command->waitForFinished();
  if (command->exitStatus() != QProcess::NormalExit || command->exitCode() != 0) {
    QMessageBox::critical(
        nullptr,
        "Error",
        "Could not install plugin. Email help@rtxi.org for assistance");
    QByteArray err_str = command->readAllStandardError();
    ERROR_MSG("{}", err_str.toStdString());
    ERROR_MSG("Install command for module {} failed with command {}",
              name.toStdString(),
              make_cmd.toStdString()+std::string(" ")+make_install_args.join(" ").toStdString());
    cloneButton->setEnabled(true);
    rebuildListWidgets();
    availableListWidget->setDisabled(false);
    installedListWidget->setDisabled(false);
    progress->close();
    return;
  }

  // Add module to list of already installed modules.
  modules[name].installed = true;
  progress->setValue(5);
  //QApplication::processEvents();
  command->close();
  progress->close();
  cloneButton->setEnabled(true);
  rebuildListWidgets();
  availableListWidget->setDisabled(false);
  installedListWidget->setDisabled(false);
}

int RTXIWizard::Panel::printGitError(int error)
{
  if (error != 0) {
    const git_error* e = giterr_last();
    printf("Error %d/%d: %s\n", error, e->klass, e->message);
  }
  return error;
}

std::unique_ptr<Modules::Plugin> RTXIWizard::createRTXIPlugin(
    Event::Manager* ev_manager)
{
  return std::make_unique<RTXIWizard::Plugin>(ev_manager);
}

Modules::Panel* RTXIWizard::createRTXIPanel(QMainWindow* main_window,
                                           Event::Manager* ev_manager)
{
  return static_cast<Modules::Panel*>(
      new RTXIWizard::Panel(main_window, ev_manager));
}

std::unique_ptr<Modules::Component> RTXIWizard::createRTXIComponent(
    Modules::Plugin* /*unused*/)
{
  return {nullptr};
}

Modules::FactoryMethods RTXIWizard::getFactories()
{
  Modules::FactoryMethods fact;
  fact.createPanel = &RTXIWizard::createRTXIPanel;
  fact.createComponent = &RTXIWizard::createRTXIComponent;
  fact.createPlugin = &RTXIWizard::createRTXIPlugin;
  return fact;
}
