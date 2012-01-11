#include <debug.h>
#include <main_window.h>
#include <model_loader.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <algorithm>

extern "C" Plugin::Object *createRTXIPlugin(void *) {
	return new ModelLoader();
}

ModelLoader::ModelLoader(void) {
	menuID = MainWindow::getInstance()->createControlMenuItem("Load User Module",this,SLOT(load(void)));

}

ModelLoader::~ModelLoader(void) {
	MainWindow::getInstance()->removeControlMenuItem(menuID);
}

void ModelLoader::load(void) {
	QString plugin_dir = QString(EXEC_PREFIX) + QString("/lib/rtxi/");
	QString filename = QFileDialog::getOpenFileName(plugin_dir,
			"Plugins (*.so);;All (*.*)", MainWindow::getInstance());

	if (filename.isNull() || filename.isEmpty())
		return;

	if (filename.startsWith(plugin_dir))
		filename.remove(0, plugin_dir.length());

	Plugin::Manager::getInstance()->load(filename.latin1());

	// load QSettings
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);

	int oldestmodule = userprefs.readNumEntry("/recentFileList/start", 0);
	if (oldestmodule == 0)
		userprefs.writeEntry("/recentFileList/start", 0);
	int num_module = userprefs.readNumEntry("/recentFileList/num", 0);

	QStringList entries = userprefs.entryList("/recentFileList");
	int numRecentFiles = entries.size();
	QString listmodule;
	bool doesnotexist = true;

	for (int i = 0; i < numRecentFiles; ++i) {
		listmodule = userprefs.readEntry("/recentFileList/" + entries[i]);
		if (filename == listmodule)
			doesnotexist = false;
	}
	int index;
	if (doesnotexist) {
		if (num_module == 10) {
			userprefs.writeEntry("/recentFileList/" + QString::number(
					oldestmodule), filename);
			index = oldestmodule;
			oldestmodule++;
			if (oldestmodule == 10)
				oldestmodule = 0;
			userprefs.writeEntry("/recentFileList/start", oldestmodule);
		} else {
			userprefs.writeEntry("/recentFileList/" + QString::number(
					num_module++), filename);
			index = num_module;
			userprefs.writeEntry("/recentFileList/num", num_module);
		}
		updateRecentModules(filename, index);

	}


}

void ModelLoader::updateRecentModules(QString filename, int index) {
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
	QStringList entries = userprefs.entryList("/recentFileList");
	int numRecentFiles = entries.size();

	// remove list and re-add them all
	for (int i = 3; i < std::min(numRecentFiles-2,10)+3; ++i) {
		MainWindow::getInstance()->removeControlMenuItemAt(3);
	}
	QString listmodule;
	QString text;
	for (int i = 0; i < std::min(numRecentFiles-2,10); ++i) {
		listmodule = userprefs.readEntry("/recentFileList/" + entries[i]);
		if (i==index)
			text = tr("&%1 %2").arg(i).arg(filename);
		else
			text = tr("&%1 %2").arg(i).arg(listmodule);
		menuID = MainWindow::getInstance()->createControlMenuItem(text,this,SLOT(load_recent(int)));
		MainWindow::getInstance()->changeControlMenuItem(menuID,text);
		MainWindow::getInstance()->setControlMenuItemParameter(menuID, i);
	}

}

void ModelLoader::load_recent(int i) {
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
	QString filename = userprefs.readEntry("/recentFileList/"+QString::number(i));
	Plugin::Manager::getInstance()->load(filename.latin1());
}
