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
#include <main_window.h>
#include <model_loader.h>
#include <qfiledialog.h>
#include <qsettings.h>
#include <algorithm>

extern "C" Plugin::Object *
createRTXIPlugin(void *)
{
  return new ModelLoader();
}

ModelLoader::ModelLoader(void)
{
menuID = MainWindow::getInstance()->createModuleMenuItem("Load User Module",this,SLOT(load(void)));

}

ModelLoader::~ModelLoader(void)
{
  MainWindow::getInstance()->removeModuleMenuItem(menuID);
}

void
ModelLoader::load(void)
{
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

  for (int i = 0; i < numRecentFiles; ++i)
    {
      listmodule = userprefs.readEntry("/recentFileList/" + entries[i]);
      if (filename == listmodule)
        doesnotexist = false;
    }
  int index;
  if (doesnotexist)
    {
      if (num_module == 10)
        {
          userprefs.writeEntry("/recentFileList/" + QString::number(
              oldestmodule), filename);
          index = oldestmodule;
          oldestmodule++;
          if (oldestmodule == 10)
            oldestmodule = 0;
          userprefs.writeEntry("/recentFileList/start", oldestmodule);
        }
      else
        {
          userprefs.writeEntry("/recentFileList/" + QString::number(
              num_module++), filename);
          index = num_module;
          userprefs.writeEntry("/recentFileList/num", num_module);
        }
      updateRecentModules(filename, index);

    }

}

void
ModelLoader::updateRecentModules(QString filename, int index)
{
  QSettings userprefs;
  userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
  QStringList entries = userprefs.entryList("/recentFileList");
  int numRecentFiles = entries.size();

  // remove list and re-add them all
  for (int i = 3; i < std::min(numRecentFiles - 2, 10) + 3; ++i)
    {
      MainWindow::getInstance()->removeModuleMenuItemAt(3);
    }
  QString listmodule;
  QString text;
  for (int i = 0; i < std::min(numRecentFiles - 2, 10); ++i)
    {
      listmodule = userprefs.readEntry("/recentFileList/" + entries[i]);
      if (i == index)
        text = tr("&%1 %2").arg(i).arg(filename);
      else
        text = tr("&%1 %2").arg(i).arg(listmodule);
      menuID = MainWindow::getInstance()->createModuleMenuItem(text,this,SLOT(load_recent(int)));
      MainWindow::getInstance()->changeModuleMenuItem(menuID, text);
      MainWindow::getInstance()->setModuleMenuItemParameter(menuID, i);
    }

}

void
ModelLoader::load_recent(int i)
{
  QSettings userprefs;
  userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
  QString filename = userprefs.readEntry("/recentFileList/"
      + QString::number(i));
  Plugin::Manager::getInstance()->load(filename.latin1());
}

