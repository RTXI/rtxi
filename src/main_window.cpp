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

#include <cmdline.h>
#include <debug.h>
#include <algorithm>
#include <settings.h>
#include <rtxi_config.h>
#include <stdlib.h>
#include <main_window.h>
#include <mutex.h>
#include <plugin.h>

MainWindow::MainWindow (void) : QMainWindow(NULL, Qt::Window)
{

    // Make central RTXI parent widget
    mdiArea = new QMdiArea;
    setCentralWidget(mdiArea);

    /* Initialize Window Settings */
    setWindowTitle("RTXI - Real-time eXperimental Interface");
    setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-icon.png"));

    /* Set Qt Settings Information */
    QCoreApplication::setOrganizationName("RTXI");
    QCoreApplication::setOrganizationDomain("rtxi.org");
    QCoreApplication::setApplicationName("RTXI");

    /* Initialize Menus */
    createFileActions();
    createFileMenu();

    /* Initialize Module Menu */
    createModuleMenu();

    /* Initialize Utilities menu */
    createUtilMenu();

    /* Initialize System Menu */
    createSystemMenu();

    /* Initialize Windows Menu */
    createWindowsMenu();

    /* Initialize Help Menu */
    createHelpActions();
    createHelpMenu();
}

MainWindow::~MainWindow (void)
{
}

QAction* MainWindow::insertModuleMenuSeparator (void)
{
    return moduleMenu->addSeparator();
}

QAction* MainWindow::createFileMenuItem(const QString &text)
{
    return fileMenu->addAction(text);
}

void MainWindow::clearFileMenu(void)
{
    // Clear but add back default actions
    fileMenu->clear();
    fileMenu->addAction(load);
    fileMenu->addAction(save);
    fileMenu->addAction(reset);
    fileMenu->addSeparator();
    fileMenu->addAction(quit);
    fileMenu->addSeparator();
}

QAction* MainWindow::createModuleMenuItem(const QString &text)
{
    return moduleMenu->addAction(text);
}

QAction* MainWindow::createModuleMenuItem(const QString &text, const QObject *receiver, const char *member)
{
    return moduleMenu->addAction(text, receiver, member);
}

void MainWindow::setModuleMenuItemParameter (QAction *action, int parameter)
{
    action->setData(parameter);
}

void MainWindow::clearModuleMenu (void)
{
    moduleMenu->clear();
}

void MainWindow::changeModuleMenuItem (QAction *action, QString text)
{
    action->setText(text);
}

void MainWindow::removeModuleMenuItem (QAction *action)
{
    QList<QAction *> actionList = moduleMenu->actions();
    if(!actionList.empty())
        moduleMenu->removeAction(action);
}

QAction* MainWindow::createUtilMenuItem(const QString &text, const QObject * receiver, const char *member)
{
    return utilMenu->addAction (text, receiver, member);
}

void MainWindow::createFileMenu()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(load);
    fileMenu->addAction(save);
    fileMenu->addAction(reset);
    fileMenu->addSeparator();
    fileMenu->addAction(quit);
    fileMenu->addSeparator();
    connect(fileMenu, SIGNAL(triggered(QAction*)), this, SLOT(fileMenuActivated(QAction*)));
}

void MainWindow::createModuleMenu()
{
    moduleMenu = menuBar()->addMenu(tr("&Modules"));
    connect(moduleMenu, SIGNAL(triggered(QAction*)), this, SLOT(modulesMenuActivated(QAction*)));
}

void MainWindow::createUtilMenu()
{
    utilMenu = menuBar()->addMenu(tr("&Utilities"));
    filtersSubMenu = utilMenu->addMenu(tr("&Filters"));
    signalsSubMenu = utilMenu->addMenu(tr("&Signals"));
    utilitiesSubMenu = utilMenu->addMenu(tr("&Utilities"));
    connect(utilMenu, SIGNAL(triggered(QAction*)), this, SLOT(utilitiesMenuActivated(QAction*)));

    QDir libsDir("/usr/local/lib/rtxi/");
    if(!libsDir.exists())
        return;

    libsDir.setNameFilters(QStringList("*.so"));
    for(size_t i = 0; i < libsDir.entryList().size(); i++)
        {
            utilItem = new QAction(libsDir.entryList().at(i), this);
            if(libsDir.entryList().at(i).contains("analysis"))
                utilitiesSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("sync"))
                utilitiesSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("mimic"))
                utilitiesSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("iir"))
                filtersSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("fir"))
                filtersSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("signal"))
                signalsSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("noise"))
                signalsSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("ttl"))
                signalsSubMenu->addAction(utilItem);
            else if(libsDir.entryList().at(i).contains("maker"))
                signalsSubMenu->addAction(utilItem);
        }
}

void MainWindow::createSystemMenu()
{
    systemMenu = menuBar()->addMenu(tr("&System"));
}

void MainWindow::createWindowsMenu()
{
    windowsMenu = menuBar()->addMenu(tr("&Windows"));
    connect(windowsMenu,SIGNAL(aboutToShow(void)),this,SLOT(windowsMenuAboutToShow(void)));
}

void MainWindow::createHelpMenu()
{
    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addSeparator();
    helpMenu->addAction(artxi);
    helpMenu->addAction(axeno);
    helpMenu->addAction(aqt);
    helpMenu->addSeparator();
    helpMenu->addAction(adocs);
    helpMenu->addAction(sub_issue);
}

void MainWindow::createFileActions()
{
    load = new QAction(tr("&Load Workspace"), this);
    load->setShortcuts(QKeySequence::Open);
    load->setStatusTip(tr("Load a saved workspace"));
    connect(load, SIGNAL(triggered()), this, SLOT(loadSettings()));

    save = new QAction(tr("&Save Workspace"), this);
    save->setShortcuts(QKeySequence::Save);
    save->setStatusTip(tr("Save current workspace"));
    connect(save, SIGNAL(triggered()), this, SLOT(saveSettings()));

    reset = new QAction(tr("&Reset Workspace"), this);
    reset->setStatusTip(tr("Reset to default RTXI workspace"));
    connect(reset, SIGNAL(triggered()), this, SLOT(resetSettings()));

    quit = new QAction(tr("&Quit"), this);
    quit->setShortcut(tr("Ctrl+Q"));
    quit->setStatusTip(tr("Quit RTXI"));
    connect(qApp, SIGNAL(aboutToQuit()), mdiArea, SLOT(closeAllSubWindows()));
    connect(quit, SIGNAL(triggered()), this, SLOT(close()));
}

void MainWindow::createMdi(QMdiSubWindow *subWindow)
{
    mdiArea->addSubWindow(subWindow);
}

void MainWindow::createHelpActions()
{
    artxi = new QAction(tr("About &RTXI"),this);
    connect(artxi, SIGNAL(triggered()), this, SLOT(about()));

    axeno = new QAction(tr("About &Xenomai"),this);
    connect(axeno, SIGNAL(triggered()), this, SLOT(aboutXeno()));

    aqt = new QAction(tr("About &Qt"),this);
    connect(aqt, SIGNAL(triggered()), this, SLOT(aboutQt()));

    adocs = new QAction(tr("&Documentation"), this);
    connect(adocs, SIGNAL(triggered()), this, SLOT(openDocs()));

    sub_issue = new QAction(tr("&Submit Issue"), this);
    connect(sub_issue, SIGNAL(triggered()), this, SLOT(openSubIssue()));
}

QAction* MainWindow::createSystemMenuItem (const QString &text, const QObject *receiver, const char *member)
{
    return systemMenu->addAction(text, receiver, member);
}

void MainWindow::about(void)
{
    QMessageBox::about(this, "About RTXI", "RTXI Version " + QString(VERSION)
                        +	"\n\nReleased under the GPLv3.\nSee www.rtxi.org for details.");
}

void MainWindow::aboutQt (void)
{
    QMessageBox::aboutQt(this);
}

void MainWindow::aboutXeno (void)
{
#if CONFIG_XENO_VERSION_MAJOR
	FILE *fp;
	char xeno_buff[8];
	fp = fopen("/proc/xenomai/version","r");
	fscanf(fp, "%s", xeno_buff);
	fclose(fp);
	QMessageBox::about(this, "About Xenomai", "Xenomai Version " + QString(xeno_buff));
#else
	QMessageBox::about(this, "About Xenomai", "Running POSIX (non-RT)");
#endif
}

void MainWindow::openDocs(void)
{
    QDesktopServices::openUrl(QUrl("http://rtxi.org/docs/", QUrl::TolerantMode));
}

void MainWindow::openSubIssue(void)
{
    QDesktopServices::openUrl(QUrl("https://github.com/rtxi/rtxi/issues", QUrl::TolerantMode));
}

/*
 * Load MainWindow settings
 */
void MainWindow::loadWindow(void)
{
    QSettings userprefs;
    userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
    userprefs.beginGroup("MainWindow");
    restoreGeometry(userprefs.value("geometry", saveGeometry()).toByteArray());
    move(userprefs.value("pos", pos()).toPoint());
    resize(userprefs.value("size", size()).toSize());
    if(userprefs.value("maximized", isMaximized()).toBool())
        showMaximized();
    userprefs.endGroup();
    show();
}

void MainWindow::loadSettings (void)
{
    QSettings userprefs;
    userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");

    QString filename = QFileDialog::getOpenFileName(this,
                       tr("Load saved workspace"), userprefs.value("/dirs/setfiles", getenv("HOME")).toString(), tr("Settings (*.set)"));

    if (QFile(filename).exists())
        {
            systemMenu->clear();
            mdiArea->closeAllSubWindows();
            Settings::Manager::getInstance()->load(filename.toStdString());
        }
}

void MainWindow::saveSettings(void)
{
    QSettings userprefs;
    userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");

    QString filename = QFileDialog::getSaveFileName(this,
                       tr("Save current workspace"), userprefs.value("/dirs/setfiles", getenv("HOME")).toString(), tr("Settings (*.set)"));

    if (!filename.isEmpty())
        {
            if (!filename.endsWith(".set"))
                filename = filename+".set";
            if (QFileInfo (filename).exists() && QMessageBox::warning(this,
                    "File Exists", "Do you wish to overwrite " + filename + "?",
                    QMessageBox::Yes | QMessageBox::Default,
                    QMessageBox::No | QMessageBox::Escape) != QMessageBox::Yes)
                {
                    DEBUG_MSG ("MainWindow::saveSettings : canceled overwrite\n");
                    return;
                }
            Settings::Manager::getInstance()->save(filename.toStdString());
        }
}

void MainWindow::resetSettings(void)
{
    systemMenu->clear();
    mdiArea->closeAllSubWindows();
    Settings::Manager::getInstance()->load("/usr/local/share/rtxi/rtxi.conf");
}

void MainWindow::utilitiesMenuActivated(QAction *id)
{
    Plugin::Manager::getInstance()->load(id->text());
}

void MainWindow::windowsMenuAboutToShow(void)
{

    // Clear previous entries
    windowsMenu->clear();

    // Add default options
    windowsMenu->addAction(tr("Cascade"),mdiArea,SLOT(cascadeSubWindows()));
    windowsMenu->addAction(tr("Tile"),mdiArea,SLOT(tileSubWindows()));
    windowsMenu->addSeparator();

    // Get list of open subwindows in Mdi Area
    subWindows = mdiArea->subWindowList();

    // Make sure it isn't empty
    if(subWindows.isEmpty())
        return;

    // Create windows list based off of what's open
    for(int i = 0; i < subWindows.size(); i++)
        {
            QAction *item = new QAction(subWindows.at(i)->widget()->windowTitle(), this);
            windowsMenu->addAction(item);
        }
    connect(windowsMenu, SIGNAL(triggered(QAction*)), this, SLOT(windowsMenuActivated(QAction*)));
}

void MainWindow::windowsMenuActivated(QAction *id)
{

    // Get list of open subwindows in Mdi Area
    subWindows = mdiArea->subWindowList();

    // Make sure it isn't empty
    if(subWindows.isEmpty())
        return;

    for(uint16_t i = 0; i < subWindows.size(); i++)
        if(subWindows.at(i)->widget()->windowTitle() == id->text())
            mdiArea->setActiveSubWindow(subWindows.at(i));
}

void MainWindow::modulesMenuActivated(QAction *id)
{
    // Annoying but the best way to do it is to tie an action to the entire menu
    // so we have to tell it to ignore the first two modules
    if(id->text().contains("Load Plugin"))
        return;

    // Have to trim the first three characters before loading
    // or else parser will include qstring formatting
    Plugin::Manager::getInstance()->load(id->text().remove(0,3));
}

void MainWindow::fileMenuActivated(QAction *id)
{
    // Annoying but the best way to do it is to tie an action to the entire menu
    // so we have to tell it to ignore the first three items
    if(id->text().contains("Load Workspace") ||
            id->text().contains("Save Workspace") ||
            id->text().contains("Reset Workspace") ||
            id->text().contains("Quit"))
        return;

    // Have to trim the first three characters before loading
    // or else parser will include qstring formatting
    systemMenu->clear();
    mdiArea->closeAllSubWindows();
    Settings::Manager::getInstance()->load(id->text().remove(0,3).toStdString());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    /*
     * Save MainWindow settings
     */
    QSettings userprefs;
    userprefs.setPath (QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
    userprefs.beginGroup("MainWindow");
    userprefs.setValue("geometry", saveGeometry());
    userprefs.setValue("maximized", isMaximized());
    if(!isMaximized())
        {
            userprefs.setValue("pos", pos());
            userprefs.setValue("size", size());
        }
    userprefs.endGroup();
}

static Mutex mutex;
MainWindow * MainWindow::instance = 0;

MainWindow * MainWindow::getInstance (void)
{
    if (instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but static allocation isn't *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock (&::mutex);
    if (!instance)
        {
            static MainWindow mainwindow;
            instance = &mainwindow;
        }
    return instance;
}
