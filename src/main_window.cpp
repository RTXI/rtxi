/*
 * Copyright (C) 2004 Boston University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <compiler.h>
#include <debug.h>
#include <main_window.h>
#include <mutex.h>
#include <qapplication.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qobjectlist.h>
#include <qpopupmenu.h>
#include <qworkspace.h>
#include <settings.h>

MainWindow::MainWindow(void)
    : QMainWindow(NULL,NULL,Qt::WType_TopLevel)
{
    /* Initialize Window Settings */
    setCaption("RTXI - Realtime Experimental Interface");

    /* Insert Menu Items */
    int id;
    QPopupMenu *fileMenu = new QPopupMenu(this);
    id = fileMenu->insertItem("&Load",this,SLOT(loadSettings()),CTRL+Key_L);
    fileMenu->setWhatsThis(id,"Load System Settings");
    id = fileMenu->insertItem("&Save",this,SLOT(saveSettings()),CTRL+Key_S);
    fileMenu->setWhatsThis(id,"Save System Settings");
    fileMenu->insertSeparator();
    id = fileMenu->insertItem("&Quit",qApp,SLOT(closeAllWindows()),CTRL+Key_Q);
    fileMenu->setWhatsThis(id,"Quits the Application");
    menuBar()->insertItem("&File",fileMenu);
    controlMenu = new QPopupMenu(this);
    menuBar()->insertItem("&Control",controlMenu);
    windowsMenu = new QPopupMenu(this);
    windowsMenu->setCheckable(true);
    QObject::connect(windowsMenu,SIGNAL(aboutToShow(void)),this,SLOT(windowsMenuAboutToShow(void)));
    menuBar()->insertItem("&Windows",windowsMenu);
    menuBar()->insertSeparator();
    QPopupMenu *helpMenu = new QPopupMenu(this);
    id = helpMenu->insertItem("&About",this,SLOT(about()));
    helpMenu->setWhatsThis(id,"Opens a Window Containing Information About this Software Package");
    id = helpMenu->insertItem("About &Qt",this,SLOT(aboutQt()));
    helpMenu->setWhatsThis(id,"Opens a Window Containing Information About the QT Widget Toolkit");
    helpMenu->insertSeparator();
    helpMenu->insertItem("What's &This",this,SLOT(whatsThis()),SHIFT+Key_F1);
    menuBar()->insertItem("&Help",helpMenu);

    /* Create and Initialize the Workspace */
    setCentralWidget(new QWorkspace(this,NULL));
    ((QWorkspace *)centralWidget())->setScrollBarsEnabled(true);
}

MainWindow::~MainWindow(void)
{
}

int MainWindow::createControlMenuItem(const std::string &text,const QObject *receiver,const char *member) {
    if(unlikely(!instance))
        getInstance();

    return instance->controlMenu->insertItem(text,receiver,member);
}

void MainWindow::removeControlMenuItem(int id) {
    if(unlikely(!instance))
        getInstance();

    instance->controlMenu->removeItem(id);
}

void MainWindow::about(void)
{
    QMessageBox::about(this,"About RTXI","This software is provided under the GPL we make no guarantees... blah blah blah");
}

void MainWindow::aboutQt(void)
{
    QMessageBox::aboutQt(this);
}

void MainWindow::loadSettings(void) {
    QFileDialog dialog(this,0,true);
    dialog.setDir(".");
    dialog.setFilters(QString("Settings (*.set);;All Files (*)"));
    dialog.setCaption("Choose a Settings File");
    dialog.setMode(QFileDialog::ExistingFile);

    dialog.exec();

    QString filename = dialog.selectedFile();
    if(filename != "/")
        Settings::Manager::load(filename);
}

void MainWindow::saveSettings(void) {
    QFileDialog dialog(this,0,true);
    dialog.setDir(".");
    dialog.setFilters(QString("Settings (*.set);;All Files (*)"));
    dialog.setCaption("Choose a Settings File");
    dialog.setMode(QFileDialog::AnyFile);

    dialog.exec();

    QString filename = dialog.selectedFile();
    if(filename != "/") {
        if((dialog.selectedFilter() == "Settings (*.set)") && !filename.endsWith(".set"))
            filename += ".set";
        if(QFileInfo(filename).exists() &&
           QMessageBox::warning(this,"File Exists","Do you wish to overwrite " + filename + "?",QMessageBox::Yes|QMessageBox::Default,QMessageBox::No|QMessageBox::Escape) != QMessageBox::Yes) {
            DEBUG_MSG("MainWindow::saveSettings : canceled overwrite\n");
            return;
        }
        Settings::Manager::save(filename);
    }
}

void MainWindow::windowsMenuAboutToShow(void) {
    windowsMenu->clear();

    QWorkspace *ws = dynamic_cast<QWorkspace *>(centralWidget());
    if(!ws) {
        ERROR_MSG("MainWindow::windowsMenuAboutToShow : centralWidget() not a QWorkspace?\n");
        return;
    }

    int cascadeID = windowsMenu->insertItem("&Cascade",ws,SLOT(cascade(void)));
    int tileID = windowsMenu->insertItem("&Tile",ws,SLOT(tile(void)));
    if(ws->windowList().isEmpty()) {
        windowsMenu->setItemEnabled(cascadeID,false);
        windowsMenu->setItemEnabled(tileID,false);
    }

    windowsMenu->insertSeparator();
    QWidgetList windows = ws->windowList();
    for(size_t i = 0;i < windows.count();++i) {
        int id = windowsMenu->insertItem(windows.at(i)->caption(),this,SLOT(windowsMenuActivated(int)));
        windowsMenu->setItemParameter(id,i);
        windowsMenu->setItemChecked(id,ws->activeWindow() == windows.at(i));
    }
}

void MainWindow::windowsMenuActivated(int id) {
    QWorkspace *ws = dynamic_cast<QWorkspace *>(centralWidget());
    if(!ws) {
        ERROR_MSG("MainWindow::windowsMenuActivated : centralWidget() not a QWorkspace?\n");
        return;
    }

    QWidget *w = ws->windowList().at(id);
    if(w) {
        w->showNormal();
        w->setFocus();
    }
}

static Mutex mutex;
MainWindow *MainWindow::instance = 0;

MainWindow *MainWindow::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but static allocation isn't *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static MainWindow mainwindow;
        instance = &mainwindow;
    }

    return instance;
}
