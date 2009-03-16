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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qmainwindow.h>

class QPopupMenu;

/*!
 * The primary graphical object in the program.
 */
class MainWindow : public QMainWindow
{

    Q_OBJECT

    friend class QApplication;

public:

    virtual ~MainWindow(void);

    /*!
     * Get a pointer to the MainWindow.
     *   Needed for adding new GUI components.
     *
     * \return A pointer to the MainWindow widget.
     */
    static MainWindow *getInstance(void);

    /*!
     * Add an item to the control menu.
     *
     * \param label The text that will appear in the menu.
     * \param handler The object that will handle signals from the menu.
     * \param slot The slot in the handler that the signal will activate.
     * \return The index of the new menu item.
     */
    static int createControlMenuItem(const std::string &label,const QObject *handler,const char *slot);
    /*!
     * Remove an item from the control menu.
     *
     * \param index The index of the item to be removed.
     */
    static void removeControlMenuItem(int index);

private slots:

    void about(void);
    void aboutQt(void);
    void loadSettings(void);
    void saveSettings(void);
    void windowsMenuAboutToShow(void);
    void windowsMenuActivated(int);

private:

    /****************************************************************
     * The constructor, destrutos, and assignment operator are made *
     *   private to control instantiation of the class.             *
     ****************************************************************/

    MainWindow(void);
    MainWindow(const MainWindow &) {};
    MainWindow &operator=(const MainWindow &) { return *instance; };

    static MainWindow *instance;

    QPopupMenu *controlMenu;
    QPopupMenu *windowsMenu;

};

#endif /* MAIN_WINDOW_H */
