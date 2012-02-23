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

#include <debug.h>

#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <cmdline.h>
#include <main_window.h>
#include <dynamo_model_loader.h>

#define MODEL_MAKEFILE_PATH    "/usr/local/share/rtxi/Makefile.dynamo_compile"
#define MAKE_CMD_PREFIX        "make -f "
#define MODEL_SOURCE_SUFFIX    ".dynamo"
#define MODEL_SUFFIX           ".so"

extern "C" Plugin::Object *createRTXIPlugin(void *) {
	return DynamoModelLoader::getInstance();
}

DynamoModelLoader::DynamoModelLoader(void) {
	DEBUG_MSG("DynamoModelLoader::DynamoModelLoader : starting\n");
	menuID = MainWindow::getInstance()->createModuleMenuItem("Load DYNAMO Model",this,SLOT(load_dialog(void)));

	model_makefile_path = QString(MODEL_MAKEFILE_PATH);
	DEBUG_MSG("model_makefile_path = %s\n", model_makefile_path.ascii());
	DEBUG_MSG("DynamoModelLoader::DynamoModelLoader : finished\n");

	MainWindow::getInstance()->insertModuleMenuSeparator();

	// add recently used modules to the menu
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
	QStringList entries = userprefs.entryList("/recentFileList");
	int numRecentFiles = entries.size();
	QString listmodule;
	QString text;
	for (int i = 0; i < std::min(numRecentFiles-2,10); ++i) {
		listmodule = userprefs.readEntry("/recentFileList/" + entries[i]);
		text = tr("&%1 %2").arg(i).arg(listmodule);
		menuID = MainWindow::getInstance()->createModuleMenuItem(text,this,SLOT(load_recent(int)));
		MainWindow::getInstance()->setModuleMenuItemParameter(menuID, i);
	}

	// add recently used settings files to the menu
	entries = userprefs.entryList("/recentSettingsList");
	numRecentFiles = entries.size()-1;
	for (int i = 0; i < std::min(numRecentFiles,10); ++i) {
		listmodule = userprefs.readEntry("/recentSettingsList/" + entries[i]);
		text = tr("&%1 %2").arg(i).arg(listmodule);
		menuID = MainWindow::getInstance()->createFileMenuItem(text,this,SLOT(load_setting(int)));
		MainWindow::getInstance()->setFileMenuItemParameter(menuID, i);
	}

}

DynamoModelLoader::~DynamoModelLoader(void) {
	MainWindow::getInstance()->removeModuleMenuItem(menuID);
}

/* Set and retrieve the name of the model make file. */
QString DynamoModelLoader::get_model_makefile_path() const {
	return model_makefile_path;
}

int DynamoModelLoader::set_model_makefile_path(char *s) {
	if (s != NULL) {
		model_makefile_path = QString(s);
	} else
		return -EINVAL;

	return 0;
}

void DynamoModelLoader::load(char *srcpath) {
	int status;
	char *path;

	path = canonicalize_file_name(srcpath);
	QString model_name = QString(path);
	model_name.remove(MODEL_SOURCE_SUFFIX);

	if (model_name.isNull() || model_name.isEmpty())
		return;

	if (model_makefile_path.isNull() || model_makefile_path.isEmpty()) {
		ERROR_MSG("invalid model makefile name\n");
		return;
	}

	QString cmd = QString("cd %1; %2 %3 %4%5") .arg(getcwd(NULL, 0)) .arg(
			MAKE_CMD_PREFIX) .arg(model_makefile_path) .arg(model_name) .arg(
			MODEL_SUFFIX);

	QString module_name = QString("%1%2") .arg(model_name) .arg(MODEL_SUFFIX);

	DEBUG_MSG("about to compile model with command %s\n", cmd.ascii());
	status = CmdLine::getInstance()->execute(cmd.ascii());

	if (status != 0) {
		ERROR_MSG("build process error\n");
		return;
	}

	Plugin::Manager::getInstance()->load(module_name.latin1());

	if (path != NULL)
		free(path);
}

void DynamoModelLoader::load_dialog(void) {
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
	//QString settingsDir = userprefs.readEntry("/dirs/dynamomodels", getenv("HOME"));

	QString file_name = QFileDialog::getOpenFileName(userprefs.readEntry("/dirs/dynamomodels", getenv("HOME")),
			"Dynamo Models (*" MODEL_SOURCE_SUFFIX
			");;All (*.*)", MainWindow::getInstance());
	//QString file_name = QFileDialog::getOpenFileName(QString::null,"Dynamo Models (*" MODEL_SOURCE_SUFFIX \
						      ");;All (*.*)",MainWindow::getInstance());

	load((char *) file_name.latin1());
}


void DynamoModelLoader::load_recent(int i) {
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
	QString filename = userprefs.readEntry("/recentFileList/"+QString::number(i));
	Plugin::Manager::getInstance()->load(filename.latin1());
}

void DynamoModelLoader::load_setting(int i) {
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
	QString filename = userprefs.readEntry("/recentSettingsList/"+QString::number(i));
	printf("Loading settings file: %s\n",filename.latin1());
	Settings::Manager::getInstance()->load(filename.latin1());
}

static Mutex mutex;
DynamoModelLoader *DynamoModelLoader::instance = NULL;

DynamoModelLoader *DynamoModelLoader::getInstance(void) {
	if (instance != NULL)
		return instance;

	Mutex::Locker lock(&::mutex);
	if (instance == NULL)
		instance = new DynamoModelLoader();

	return instance;
}
