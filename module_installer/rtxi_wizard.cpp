#include <QtNetwork>
#include <iostream>
#include <unistd.h>
#include <git2.h>
extern "C" {
#include <mkdio.h>
}

#include "rtxi_wizard.h"

extern "C" Plugin::Object *createRTXIPlugin(void *) {
	return new RTXIWizard();
}

static DefaultGUIModel::variable_t vars[] = {};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

RTXIWizard::RTXIWizard(void) : DefaultGUIModel("Module Installer", ::vars, ::num_vars) {

	DefaultGUIModel::createGUI(vars, num_vars);
	initParameters();
	customizeGUI();

	QTimer::singleShot(0, this, SLOT(resizeMe()));

}

RTXIWizard::~RTXIWizard(void) {}

void RTXIWizard::initParameters(void) {

	git_threads_init();

   allModules = new QList<RTXIModule*>;
   installedModules = new QList<RTXIModule*>;

   // syntax here only works in c++11
   exclude_list = std::vector<QString> ({ QString("rtxi"),
                                          QString("rtxi.github.io"),
                                          QString("analysis-tools"),
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
                                          QString("conference-2015") });
   button_mode = DOWNLOAD;

}

void RTXIWizard::customizeGUI(void) {

	QGridLayout *customLayout = DefaultGUIModel::getLayout();
	customLayout->itemAtPosition(1,0)->widget()->setVisible(false);
	customLayout->itemAtPosition(10,0)->widget()->setVisible(false);

   QGroupBox *buttonBox = new QGroupBox;
   QVBoxLayout *buttonLayout = new QVBoxLayout();
   buttonBox->setLayout(buttonLayout);
   syncButton = new QPushButton("Sync Repos", this);
   cloneButton = new QPushButton("Download and Install", this);
   cloneButton->setEnabled(false);
   buttonLayout->addWidget(syncButton);
   buttonLayout->addWidget(cloneButton);

   QGroupBox *installedBox = new QGroupBox("Installed");
   QVBoxLayout *installedLayout = new QVBoxLayout;
   installedBox->setLayout(installedLayout);
   installedList = new QListWidget(installedBox);
   installedList->setFixedWidth(200);
   installedLayout->addWidget(installedList);

   QGroupBox *moduleBox = new QGroupBox("All Modules");
   QVBoxLayout *moduleLayout = new QVBoxLayout;
   moduleBox->setLayout(moduleLayout);
   moduleList = new QListWidget(this);
   moduleList->setFixedWidth(200);
   moduleLayout->addWidget(moduleList);

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
   QObject::connect(moduleList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(getReadme(void)));
   QObject::connect(moduleList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateButton(void)));
   QObject::connect(installedList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(getReadme(void)));
   QObject::connect(installedList, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateButton(void)));

	setLayout(customLayout);
}

void RTXIWizard::updateButton(void) {
   QListWidget *parent = qobject_cast<QListWidget*>(sender());

   if ( parent == moduleList ) {
      cloneButton->setText("Download and Install");
      button_mode = DOWNLOAD;
   } else if ( parent == installedList ) {
      cloneButton->setText("Update");
      button_mode = UPDATE;
   }

}

void RTXIWizard::cloneModule(void) {

	cloneButton->setEnabled(false);
	moduleList->setDisabled(true);
	installedList->setDisabled(true);
//	QPushButton *parent = qobject_cast<QPushButton*>(sender());

	RTXIModule *module = nullptr;
	int module_idx = 0;

/*
	if ( parent->text() == "Download and Install" ) {
		module_idx = moduleList->currentRow();
		module = allModules->at(module_idx);
	} else if ( parent->text() == "Update" ) {
		module_idx = installedList->currentRow();
		module = installedModules->at(module_idx);
	}
*/
	
	switch(button_mode) {
		case DOWNLOAD:
			module_idx = moduleList->currentRow();
			module = allModules->at(module_idx);
			break;

		case UPDATE:
			module_idx = installedList->currentRow();
			module = installedModules->at(module_idx);
			break;

		default:
			std::cout<<"ERROR: default in swtich block in cloneModule()"<<std::endl;
			break;
	}

/*
	RTXIModule *module = nullptr;
	if ( parent == moduleList ) {
		module_idx = moduleList->currentRow();
		module = allModules->at(module_idx);
	} else if ( parent == installedList ) {
		module_idx = installedList->currentRow();
		module = installedModules->at(module_idx);
	}
*/
//	int module_idx = moduleList->currentRow();
//	RTXIModule* module = allModules->at(module_idx);

	QByteArray temp = module->getCloneUrl().toString().toLatin1();
	const char *url = temp.data();
	QByteArray temp2 = module->getLocation().toString().toLatin1();
	const char *path = temp2.data(); 

	int error = 0;
	if ( (QDir(module->getLocation().toString())).exists() ) {
		git_repository *repo = NULL;
		git_remote *remote = NULL;

		git_repository_open(&repo, path);
//std::cout<<"before git_remote_load"<<std::endl;
		error = error | git_remote_load(&remote, repo, "origin");
//std::cout<<"before git_remote_connect"<<std::endl;
		error = error | git_remote_connect(remote, GIT_DIRECTION_FETCH);
//std::cout<<"before git_remote_download"<<std::endl;
		error = error | git_remote_download(remote, NULL, NULL);

//std::cout<<"before git_remote_disconnect"<<std::endl;
		git_remote_disconnect(remote);
//std::cout<<"before git_remote_free"<<std::endl;
		git_remote_free(remote);
//std::cout<<"before git_repository_free"<<std::endl;
		git_repository_free(repo);

	} else {
		git_repository *repo = NULL;
//std::cout<<"before git_clone"<<std::endl;
		error = git_clone(&repo, url, path, NULL);
		git_repository_free(repo);
	}
	
	if (error) {
		std::cout<<"git ERROR"<<std::endl;
	} else {

		if (module->installed == false) {
			installedList->addItem(module->getName());
			installedModules->append(module);
			module->installed = true;

			allModules->removeAt(module_idx);
			moduleList->takeItem(module_idx);
		}

		QString make_cmd = "/usr/bin/make -j2 -C " + module->getLocation().toString();
		QString make_install_cmd;
		if (getuid()) make_install_cmd = "gksudo \"/usr/bin/make install -C" + module->getLocation().toString() + "\"";
		else make_install_cmd = "/usr/bin/make install -C" + module->getLocation().toString();

		QProcess *make = new QProcess();
		QProcess *make_install = new QProcess();
		make->start(make_cmd);

		if (!make->waitForFinished()) {
			std::cout<<"make -C "<<path<<" failed"<<std::endl;
		} else {
			make_install->start(make_install_cmd);
			if (!make_install->waitForFinished()) {
				std::cout<<"make install -C"<<path<<" failed..."<<std::endl;
				std::cout<<"...despite make -C succeeding."<<std::endl;
			}
		}

		make->close();
		make_install->close();
		
	}

	cloneButton->setEnabled(true);
	moduleList->setDisabled(false);
	installedList->setDisabled(false);

}

void RTXIWizard::getRepos() {

	moduleList->setDisabled(true);
	installedList->setDisabled(true);

	if (!moduleList->count()) {
		QUrl url("https://api.github.com/orgs/rtxi/repos?per_page=100");
		reply = qnam.get(QNetworkRequest(url));
		QObject::connect(reply, SIGNAL(finished()), this, SLOT(parseRepos(void)));
//		connect(reply, SIGNAL(readyRead()), this, SLOT(httpReadyRead()));
//		connect(reply, SIGNAL(downloadProgress(qint64,qint64)), 
//	   	     this, SLOT(updateDataReadProgress(qint64,qint64)));
	} else {
		moduleList->setDisabled(false);
		installedList->setDisabled(false);
	}
}

void RTXIWizard::getReadme(void) {
	moduleList->setDisabled(true);
	installedList->setDisabled(true);

	QListWidget *parent = qobject_cast<QListWidget*>(sender());

	RTXIModule *selectedModule = nullptr;
	if ( parent == moduleList ) {
		selectedModule = allModules->at(parent->currentRow());
	} else if ( parent == installedList ) {
		selectedModule = installedModules->at(parent->currentRow());
	}

	if (selectedModule->getReadme() == "") {
		reply = qnam.get(QNetworkRequest(selectedModule->getReadmeUrl()));
		QObject::connect(reply, SIGNAL(finished()), this, SLOT(parseReadme()));
	} else {
		readmeWindow->setHtml(selectedModule->getReadme());
		cloneButton->setEnabled(true);
		moduleList->setDisabled(false);
		installedList->setDisabled(false);
	}
}

void RTXIWizard::parseReadme(void) {
	
	const char* raw_data = (reply->readAll()).constData();
	MMIOT *m = mkd_string(raw_data, strlen(raw_data), 0);
	mkd_compile(m, 0);

	char* text;
	int len = mkd_document(m, &text);
	std::string html(text, text+len);

	mkd_cleanup(m);
	QString fileText = QString::fromStdString(html);

//	QObject::disconnect(reply, SIGNAL(finished()), this, SLOT(parseReadme(void)));
	reply->deleteLater();
	reply = 0;

	RTXIModule *selectedModule = nullptr;
	switch(button_mode) {
		case DOWNLOAD:
			selectedModule = allModules->at(moduleList->currentRow());
			break;

		case UPDATE:
			selectedModule = installedModules->at(installedList->currentRow());
			break;

		default:
			std::cout<<"ERROR: default in swtich block in cloneModule()"<<std::endl;
			break;
	}

	selectedModule->setReadme(fileText);

	readmeWindow->setHtml(fileText);
	readmeWindow->show();

	cloneButton->setEnabled(true);
	moduleList->setDisabled(false);
	installedList->setDisabled(false);
}

void RTXIWizard::parseRepos(void) {

	QJsonDocument jsonDoc = QJsonDocument::fromJson(reply->readAll().data());
	QJsonArray jsonArr = jsonDoc.array();

	QString readmeUrlPrefix = "https://raw.githubusercontent.com/RTXI/";
	QString readmeUrlSuffix = "/master/README.md";

	QString locationPrefix;
	if (getuid()) {
		locationPrefix = QString(getenv("HOME")) +  "/.config/rtxi/";
	} else {
		locationPrefix = "/usr/local/lib/rtxi_modules/";
	}

	for (int idx = 0; idx < jsonArr.size(); idx++) {
		QJsonObject newObj = (jsonArr.at(idx)).toObject();
		newObj.find("name").key();

		// if the current module isn't in the exclude_list
		if (std::find(exclude_list.begin(), exclude_list.end(), newObj.value("name").toString()) 
				== exclude_list.end()) {

			RTXIModule* module = new RTXIModule;
		
			module->setReadmeUrl( readmeUrlPrefix + newObj.value("name").toString() + readmeUrlSuffix);
			module->setCloneUrl(newObj.value("clone_url").toString());
			module->setName(newObj.value("name").toString());
			module->setLocation(locationPrefix + newObj.value("name").toString());
			if ( (QDir(module->getLocation().toString())).exists() ) {
				module->installed = true;
				installedList->addItem(module->getName());
				installedModules->append(module);
			} else {
				module->installed = false;
				moduleList->addItem(module->getName());
				allModules->append(module);
			}
		}
	}

//	QObject::disconnect(reply, SIGNAL(finished()), this, SLOT(parseRepos(void)));
	reply->deleteLater();
	reply = 0;
	
	moduleList->setDisabled(false);
	installedList->setDisabled(false);
}

void RTXIWizard::installFromString( std::string module_name ) {

	std::string cloneUrl = "https://github.com/rtxi/" + module_name;

	std::string locationUrl;
	if (getuid()) {
		locationUrl = std::string(getenv("HOME")) +  "/.config/rtxi/" + module_name;
	} else {
		locationUrl = "/usr/local/lib/rtxi_modules/" + module_name;
	}

	const char *url = cloneUrl.c_str(); 
	const char *path = locationUrl.c_str();

	int error = 0;
	if ( (QDir(QString::fromStdString(locationUrl))).exists() ) {
		git_repository *repo = NULL;
		git_remote *remote = NULL;

		git_repository_open(&repo, path);
		error = error | git_remote_load(&remote, repo, "origin");
		error = error | git_remote_connect(remote, GIT_DIRECTION_FETCH);
		error = error | git_remote_download(remote, NULL, NULL);

		git_remote_disconnect(remote);
		git_remote_free(remote);
		git_repository_free(repo);

	} else {
		git_repository *repo = NULL;
		error = git_clone(&repo, url, path, NULL);
		git_repository_free(repo);
	}
	
	if (error) {
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

	if (!make->waitForFinished()) {
		std::cout<<"make -C "<<path<<" failed"<<std::endl;
	} else {
		make_install->start(make_install_cmd);
		if (!make_install->waitForFinished()) {
			std::cout<<"make install -C"<<path<<" failed..."<<std::endl;
			std::cout<<"...despite make -C succeeding."<<std::endl;
		}
	}

	make->close();
	make_install->close();

}
