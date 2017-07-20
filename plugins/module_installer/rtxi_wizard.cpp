/*
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

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
#include <unistd.h>
#include <git2.h>
extern "C" {
#include <mkdio.h>
}
#include "rtxi_wizard.h"

/*
 * This module uses the QNetworkManager class to fetch information on our
 * GitHub repos using GitHub's own API.
 */
RTXIWizard::Panel::Panel(QWidget *parent) : QWidget(parent)
{
	setWhatsThis("Module Wizard enables management of all RTXI modules. You can download and install new modules directly from the GitHub site..etc");

	// Make Mdi
	subWindow = new QMdiSubWindow;
	subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
	subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint |
			Qt::WindowMinimizeButtonHint);
	subWindow->setAttribute(Qt::WA_DeleteOnClose);
	MainWindow::getInstance()->createMdi(subWindow);

	QGridLayout *customLayout = new QGridLayout;

	QGroupBox *buttonBox = new QGroupBox;
	QHBoxLayout *buttonLayout = new QHBoxLayout();
	buttonBox->setLayout(buttonLayout);
	syncButton = new QPushButton("Sync", this);
	cloneButton = new QPushButton("Install", this);
	cloneButton->setEnabled(false);
	buttonLayout->addWidget(syncButton);
	buttonLayout->addWidget(cloneButton);

	QGroupBox *installedBox = new QGroupBox("Installed");
	QVBoxLayout *installedLayout = new QVBoxLayout;
	installedBox->setLayout(installedLayout);
	installedListWidget = new QListWidget(installedBox);
	installedListWidget->setFixedWidth(175);
	installedListWidget->setSortingEnabled(true);
	installedLayout->addWidget(installedListWidget);

	QGroupBox *moduleBox = new QGroupBox("Available");
	QVBoxLayout *moduleLayout = new QVBoxLayout;
	moduleBox->setLayout(moduleLayout);
	availableListWidget = new QListWidget(this);
	availableListWidget->setFixedWidth(175);
	availableListWidget->setSortingEnabled(true);
	moduleLayout->addWidget(availableListWidget);

	readmeWindow = new QTextEdit;
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
	QObject::connect(availableListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(getReadme(void)));
	QObject::connect(availableListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateButton(void)));
	QObject::connect(installedListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(getReadme(void)));
	QObject::connect(installedListWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateButton(void)));

	setLayout(customLayout);
	setWindowTitle("Module Wizard");
	subWindow->setWidget(this);
	subWindow->resize(700, subWindow->sizeHint().height());
	getRepos();
	show();

	initParameters();
}

RTXIWizard::Panel::~Panel(void) 
{
	Plugin::getInstance()->removeRTXIWizardPanel(this);
}

void RTXIWizard::Panel::initParameters(void)
{

#if LIBGIT2_SOVERSION >= 22
	git_libgit2_init();
#else
	git_threads_init();
#endif

	// syntax here only works in c++11
	exclude_list = std::vector<QString> ({ 
			QString("rtxi"),
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
			QString("conference-2015")
			});
	button_mode = DOWNLOAD;

}

// Set the text to "Update" if the module is already installed or "Download
// and Install" if not.
void RTXIWizard::Panel::updateButton(void)
{
	QListWidget *parent = qobject_cast<QListWidget*>(sender());

	if ( parent == availableListWidget )
	{
		cloneButton->setText("Install");
		button_mode = DOWNLOAD;
	}
	else if ( parent == installedListWidget )
	{
		cloneButton->setText("Update");
		button_mode = UPDATE;
	}

}

// Clone the module currently highlighted in the QListWidget.
void RTXIWizard::Panel::cloneModule(void)
{
	cloneButton->setEnabled(false);
	availableListWidget->setDisabled(true);
	installedListWidget->setDisabled(true);

	QString name;
	switch(button_mode)
	{
		case DOWNLOAD:
			name = availableListWidget->currentItem()->text();
			break;

		case UPDATE:
			name = installedListWidget->currentItem()->text();
			break;

		default:
			std::cout<<"ERROR: default in switch block in cloneModule()"<<std::endl;
			break;
	}

	/*
	 * Two QByteArray variables are needed due to the way Qt stores binary data.
	 * Calling module->getCloneUrl().toString().toLatin1().data() will produce
	 * an error.
	 */

	QByteArray temp = modules[name].clone_url.toString().toLatin1();
	const char *url = temp.data();
	QByteArray temp2 = modules[name].location.toString().toLatin1();
	const char *path = temp2.data();

	int error = 0;

	// If the repo already exists, pull from master. If not, clone it.
	if ( (QDir(modules[name].location.toString())).exists() )
	{
		git_repository *repo = NULL;
		git_remote *remote = NULL;

		git_repository_open(&repo, path);
#if LIBGIT2_SOVERSION >= 22
		printGitError(git_remote_lookup(&remote, repo, "origin"));
#else
		printGitError(git_remote_load(&remote, repo, "origin"));
#endif

#if LIBGIT2_SOVERSION >= 24
		printGitError(git_remote_connect(remote, GIT_DIRECTION_FETCH, NULL, NULL, NULL));
		//printGitError(git_remote_connect(remote, GIT_DIRECTION_FETCH, NULL, NULL));
#else
		printGitError(git_remote_connect(remote, GIT_DIRECTION_FETCH));
#endif

#if LIBGIT2_SOVERSION >= 24
		printGitError(git_remote_download(remote, NULL, NULL));
#elif LIBGIT2_SOVERSION >= 22
		printGitError(git_remote_download(remote, NULL));
#elif LIBGIT2_SOVERSION >= 21
		printGitError(git_remote_download(remote));
#else
		printGitError(git_remote_download(remote, NULL, NULL));
#endif

		git_remote_disconnect(remote);
		git_remote_free(remote);
		git_repository_free(repo);
	}
	else
	{
		git_repository *repo = NULL;
		printGitError(git_clone(&repo, url, path, NULL));
		git_repository_free(repo);
	}

	if (error)
	{
		std::cout<<"git ERROR"<<std::endl;
	}
	else
	{
		// Add module to list of already installed modules.
		modules[name].installed = true;

		// Define the commands to be run.
		QString make_cmd = "/usr/bin/make -j2 -C " + modules[name].location.toString();
		QString make_install_cmd;

		// If RTXI is root, no need to call gksudo.
		if (getuid()) make_install_cmd = "gksudo \"/usr/bin/make install -C" + modules[name].location.toString() + "\"";
		else make_install_cmd = "/usr/bin/make install -C" + modules[name].location.toString();

		// Compile and instal handled by QProcess.
		QProcess *make = new QProcess();
		QProcess *make_install = new QProcess();
		make->start(make_cmd);

		if (!make->waitForFinished())
		{
			std::cout<<"make -C "<<path<<" failed"<<std::endl;
		}
		else
		{
			make_install->start(make_install_cmd);
			if (!make_install->waitForFinished())
			{
				std::cout<<"make install -C"<<path<<" failed..."<<std::endl;
				std::cout<<"...despite make -C succeeding."<<std::endl;
			}
		}

		make->close();
		make_install->close();
	}

	// Re-enable buttons only after compilation is done. Otherwise you get race
	// conditions if buttons are pressed before modules are done compiling.
	cloneButton->setEnabled(true);
	rebuildListWidgets();
	availableListWidget->setDisabled(false);
	installedListWidget->setDisabled(false);
}

// Download the list of repos from GitHub's API. Call parseRepos for the JSON.
void RTXIWizard::Panel::getRepos()
{
	availableListWidget->setDisabled(true);
	installedListWidget->setDisabled(true);

	if (!availableListWidget->count())
	{
		QUrl url("https://api.github.com/orgs/rtxi/repos?per_page=100");
		reply = qnam.get(QNetworkRequest(url));
		QObject::connect(reply, SIGNAL(finished()), this, SLOT(parseRepos(void)));
		//connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
		//connect(reply, SIGNAL(downloadProgress(qint64,qint64)),
		// 	      this, SLOT(updateDataReadProgress(qint64,qint64)));
	}
	else
	{
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
void RTXIWizard::Panel::getReadme(void)
{
	availableListWidget->setDisabled(true);
	installedListWidget->setDisabled(true);

	QListWidget *parent = qobject_cast<QListWidget*>(sender());
	QString name = parent->currentItem()->text();

	// If the README hasn't been downloaded before, download it now.
	if (modules[parent->currentItem()->text()].readme == "")
	{
		reply = qnam.get(QNetworkRequest(modules[name].readme_url));
		QObject::connect(reply, SIGNAL(finished()), this, SLOT(parseReadme()));
	}
	else
	{
		// Disable buttons until all logic is done.
		readmeWindow->setHtml(modules[parent->currentItem()->text()].readme);
		cloneButton->setEnabled(true);
		availableListWidget->setDisabled(false);
		installedListWidget->setDisabled(false);
	}
}

// READMEs are downloaded as markdown. Convert them to HTML and display them
// within a QTextWidget.
void RTXIWizard::Panel::parseReadme(void)
{
	const char* raw_data = (reply->readAll()).constData();
	MMIOT *m = mkd_string(raw_data, strlen(raw_data), 0);
	mkd_compile(m, 0);

	char* text;
	int len = mkd_document(m, &text);
	std::string html(text, text+len);

	mkd_cleanup(m);
	QString fileText = QString::fromStdString(html);

	// QObject::disconnect(reply, SIGNAL(finished()), this, SLOT(parseReadme(void)));
	reply->deleteLater();
	reply = 0;

	switch(button_mode)
	{
		case DOWNLOAD:
			modules[availableListWidget->currentItem()->text()].readme = fileText;
			break;

		case UPDATE:
			modules[installedListWidget->currentItem()->text()].readme = fileText;
			break;

		default:
			std::cout<<"ERROR: default in swtich block in cloneModule()"<<std::endl;
			break;
	}

	readmeWindow->setHtml(fileText);
	readmeWindow->show();

	// The README is now displayed, so free the user to start clicking around.
	cloneButton->setEnabled(true);
	availableListWidget->setDisabled(false);
	installedListWidget->setDisabled(false);
}

// GitHub's API returns a JSON array. Parse it with QtJson functions.
void RTXIWizard::Panel::parseRepos(void)
{
	QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll().data());
	QJsonArray jsonArr = jsonDoc.array();

	QString readmeUrlPrefix = "https://raw.githubusercontent.com/RTXI/";
	QString readmeUrlSuffix = "/master/README.md";
	// QString locationPrefix = "/usr/local/lib/rtxi_modules/";

	QString locationPrefix;
	if (getuid())
	{
		locationPrefix = QString(getenv("HOME")) +  "/.config/RTXI/";
	}
	else
	{
		locationPrefix = "/usr/local/lib/rtxi_modules/";
	}

	for (int idx = 0; idx < jsonArr.size(); idx++)
	{
		QJsonObject newObj = (jsonArr.at(idx)).toObject();
		newObj.find("name").key();

		// if the current module isn't in the exclude_list
		if (std::find(exclude_list.begin(), exclude_list.end(), newObj.value("name").toString()) == exclude_list.end())
		{
			module_t module;

			QString name = newObj.value("name").toString();
			module.readme_url = QUrl(readmeUrlPrefix + newObj.value("name").toString() + readmeUrlSuffix);
			module.clone_url = QUrl(newObj.value("clone_url").toString());
			module.location = QString(locationPrefix + name);
			module.readme = "";

			if ( (QDir(module.location.toString())).exists() )
			{
				module.installed = true;
			}
			else
			{
				module.installed = false;
			}
			modules[name] = module;
		}
	}

	// QObject::disconnect(reply, SIGNAL(finished()), this, SLOT(parseRepos(void)));
	reply->deleteLater();
	reply = 0;

	rebuildListWidgets();
	availableListWidget->setDisabled(false);
	installedListWidget->setDisabled(false);
}

void RTXIWizard::Panel::rebuildListWidgets(void)
{
	availableListWidget->clear();
	installedListWidget->clear();

	for (std::map<QString,module_t>::iterator i = modules.begin(); i != modules.end(); ++i) {
		if (i->second.installed) installedListWidget->addItem(i->first);
		else availableListWidget->addItem(i->first);
	}

	installedListWidget->sortItems(Qt::AscendingOrder);
	availableListWidget->sortItems(Qt::AscendingOrder);
}


/*
 * Public function, not for use in this module. It gets called by other
 * classes. The function is basically a truncated version of cloneModule().
 */
void RTXIWizard::Panel::installFromString( std::string module_name )
{
	std::string cloneUrl = "https://github.com/rtxi/" + module_name;
	// QString locationPrefix = "/usr/local/lib/rtxi_modules/";

	std::string locationUrl;
	if (getuid())
	{
		locationUrl = std::string(getenv("HOME")) +  "/.config/RTXI/" + module_name;
	}
	else
	{
		locationUrl = "/usr/local/lib/rtxi_modules/" + module_name;
	}

	const char *url = cloneUrl.c_str();
	const char *path = locationUrl.c_str();

	int error = 0;
	if ( (QDir(QString::fromStdString(locationUrl))).exists() )
	{
		git_repository *repo = NULL;
		git_remote *remote = NULL;

		git_repository_open(&repo, path);
#if LIBGIT2_SOVERSION >= 22
		printGitError(git_remote_lookup(&remote, repo, "origin"));
#else
		printGitError(git_remote_load(&remote, repo, "origin"));
#endif

#if LIBGIT2_SOVERSION >= 24
		printGitError(git_remote_connect(remote, GIT_DIRECTION_FETCH, NULL, NULL, NULL));
		//printGitError(git_remote_connect(remote, GIT_DIRECTION_FETCH, NULL, NULL));
#else
		printGitError(git_remote_connect(remote, GIT_DIRECTION_FETCH));
#endif

#if LIBGIT2_SOVERSION >= 24
		printGitError(git_remote_download(remote, NULL, NULL));
#elif LIBGIT2_SOVERSION >= 22
		printGitError(git_remote_download(remote, NULL));
#elif LIBGIT2_SOVERSION >= 21
		printGitError(git_remote_download(remote));
#else
		printGitError(git_remote_download(remote, NULL, NULL));
#endif

		git_remote_disconnect(remote);
		git_remote_free(remote);
		git_repository_free(repo);

	}
	else
	{
		git_repository *repo = NULL;
		printGitError(git_clone(&repo, url, path, NULL));
		git_repository_free(repo);
	}

	if (error)
	{
		std::cout<<"git ERROR"<<std::endl;
		return;
	}

	QString make_cmd = "/usr/bin/make -j2 -C " + QString::fromStdString(locationUrl);
	QString make_install_cmd;
	if (getuid()) make_install_cmd = "gksudo \"/usr/bin/make install -C" + QString::fromStdString(locationUrl) + "\"";
	else make_install_cmd = "/usr/bin/make install -C" + QString::fromStdString(locationUrl);

	QProcess *make = new QProcess();
	QProcess *make_install = new QProcess();
	make->start(make_cmd);

	if (!make->waitForFinished())
	{
		std::cout<<"make -C "<<path<<" failed"<<std::endl;
	}
	else
	{
		make_install->start(make_install_cmd);
		if (!make_install->waitForFinished())
		{
			std::cout<<"make install -C"<<path<<" failed..."<<std::endl;
			std::cout<<"...despite make -C succeeding."<<std::endl;
		}
	}

	make->close();
	make_install->close();

}

int RTXIWizard::Panel::printGitError(int error) {
	if (error) {
		const git_error *e = giterr_last();
		printf("Error %d/%d: %s\n", error, e->klass, e->message);
	}
	return error; 
}

extern "C" Plugin::Object *createRTXIPlugin(void *)
{
	return RTXIWizard::Plugin::getInstance();
}

RTXIWizard::Plugin::Plugin(void) : panel(0)
{
	MainWindow::getInstance()->createSystemMenuItem("Plugin Manager",this,SLOT(showRTXIWizardPanel(void)));
}

RTXIWizard::Plugin::~Plugin(void)
{
	if(panel)
		delete panel;
	instance = 0;
}

void RTXIWizard::Plugin::showRTXIWizardPanel(void)
{
	if(!panel)
		panel = new Panel(MainWindow::getInstance()->centralWidget());
	panel->show();
}

void RTXIWizard::Plugin::removeRTXIWizardPanel(RTXIWizard::Panel *p)
{
	if(p == panel)
		panel = NULL;
}

static Mutex mutex;
RTXIWizard::Plugin *RTXIWizard::Plugin::instance = 0;

RTXIWizard::Plugin *RTXIWizard::Plugin::getInstance(void)
{
	if(instance)
		return instance;

	/*************************************************************************
	 * Seems like alot of hoops to jump through, but allocation isn't        *
	 *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
	 *************************************************************************/

	Mutex::Locker lock(&::mutex);
	if(!instance)
		instance = new Plugin();

	return instance;
}
