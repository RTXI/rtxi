/*
	 The Real-Time eXperiment Interface (RTXI)
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
#include <main_window.h>
#include <model_loader.h>
#include <algorithm>

extern "C" Plugin::Object * createRTXIPlugin(void *)
{
    return new ModelLoader();
}

ModelLoader::ModelLoader(void)
{
    action = MainWindow::getInstance()->createModuleMenuItem("Load Plugin", this, SLOT(load(void)));

    // Add recently used modules to the menu
    MainWindow::getInstance()->insertModuleMenuSeparator();
    QSettings userprefs;
    userprefs.setPath(QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
    userprefs.beginGroup("/recentFileList");
    QStringList entries = userprefs.childKeys();
    userprefs.endGroup();
    int numRecentFiles = entries.size();
    QString listmodule;
    QString text;
    for (int i = 0; i < std::min(numRecentFiles-2,10); ++i)
        {
            listmodule = userprefs.value("/recentFileList/" + entries[i]).toString();
            text = tr("&%1 %2").arg(i).arg(listmodule);
            MainWindow::getInstance()->createModuleMenuItem(text);
        }

    // Add recently used settings files to the menu
    userprefs.beginGroup("/recentSettingsList");
    entries = userprefs.childKeys();
    userprefs.endGroup();
    numRecentFiles = entries.size()-1;
    for (int i = 0; i < std::min(numRecentFiles,10); ++i)
        {
            listmodule = userprefs.value("/recentSettingsList/" + entries[i]).toString();
            text = tr("&%1 %2").arg(i).arg(listmodule);
            MainWindow::getInstance()->createFileMenuItem(text);
        }
}

ModelLoader::~ModelLoader(void)
{
    MainWindow::getInstance()->removeModuleMenuItem(action);
}

void ModelLoader::load(void)
{
    QString plugin_dir = QString(EXEC_PREFIX) + QString("/lib/rtxi/");
    QString filename = QFileDialog::getOpenFileName(0, tr("Load plugin"), plugin_dir, tr("Plugins (*.so);;All (*.*)"));

    if (filename.isNull()
            || filename.isEmpty()
            || filename.contains("model_loader")
            || filename.contains("analogy"))
        return;

    if (filename.startsWith(plugin_dir))
        filename.remove(0, plugin_dir.length());

    Plugin::Manager::getInstance()->load(filename.toLatin1());

    // load QSettings
    QSettings userprefs;
    userprefs.setPath(QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi");

    int oldestmodule = userprefs.value("/recentFileList/start", 0).toInt();
    if (oldestmodule == 0)
        userprefs.setValue("/recentFileList/start", 0);

    int num_module = userprefs.value("/recentFileList/num", 0).toInt();
    userprefs.beginGroup("/recentFileList");
    QStringList entries = userprefs.childKeys();
    userprefs.endGroup();

    int numRecentFiles = entries.size();
    QString listmodule;

    bool doesnotexist = true;

    for (int i = 0; i < numRecentFiles; ++i)
        {
            listmodule = userprefs.value("/recentFileList/" + entries[i]).toString();
            if (filename == listmodule)
                doesnotexist = false;
        }
    int index;
    if (doesnotexist)
        {
            if (num_module == 10)
                {
                    userprefs.setValue("/recentFileList/" + QString::number(oldestmodule), filename);
                    index = oldestmodule;
                    oldestmodule++;
                    if (oldestmodule == 10)
                        oldestmodule = 0;
                    userprefs.setValue("/recentFileList/start", oldestmodule);
                }
            else
                {
                    userprefs.setValue("/recentFileList/" + QString::number(num_module++), filename);
                    index = num_module;
                    userprefs.setValue("/recentFileList/num", num_module);
                }
        }
}
