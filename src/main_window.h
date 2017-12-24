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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QAction>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QtWidgets>
#include <QObject>

class QMenu;

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
     * MainWindow is a Singleton, which means that there can only be one
     *   instance. This function returns a pointer to that single instance.
     *
     * \return The instance of MainWindow.
     */

    static MainWindow *getInstance(void);

    /*!
     * Add an item to the File menu.
     *
     * \param label The text that will appear in the menu.
     * \param handler The object that will handle signals from the menu.
     * \param slot The slot in the handler that the signal will activate.
     * \return The index of the new menu item.
     */

    QAction* createFileMenuItem(const QString &label);//, const QObject *handler, const char *slot);

    /*!
     * Clears the File menu.
     */

    void clearFileMenu(void);


    /*!
     * Insert separator
     */
    QAction* insertModuleMenuSeparator(void);

    /*!
     * Add an item to the Modules menu.
     *
     * \param label The text that will appear in the menu.
     *
     * \param handler The object that will handle signals from the menu.
     * \param slot The slot in the handler that the signal will activate.
     * \return The index of the new menu item.
     */

    QAction* createModuleMenuItem(const QString & text);
    QAction* createModuleMenuItem(const QString & text, const QObject *handler, const char *slot);

    /*!
     * Sets the parameter value of a menu item in the Modules menu.
     *
     * \param menuid The index of the item to be removed.
     * \param parameter The parameter value to set.
     */

    void setModuleMenuItemParameter(QAction *action, int parameter);

    /*!
     * Clears the Modules menu.
     */

    void clearModuleMenu(void);

    /*!
     * Change the text associated with the Modules menu item.
     *
     * \param id The index of the item to change.
     * \param text The next text to assign to that menu item.
     */

    void changeModuleMenuItem(QAction *action, QString text);

    /*!
     * Remove an item from the Modules menu.
     *
     * \param index The index of the item to be removed.
     */

    void removeModuleMenuItem(QAction *action);

    /*!
     * Add an item to the Utilities menu.
     *
     * \param label The text that will appear in the menu.
     * \param handler The object that will handle signals from the menu.
     * \param slot The slot in the handler that the signal will activate.
     * \return The index of the new menu item.
     */

    QAction* createUtilMenuItem(const QString &label, const QObject *handler,
                                const char *slot);

    /*!
     * Add an item to the System menu.
     *
     * \param label The text that will appear in the menu.
     * \param handler The object that will handle signals from the menu.
     * \param slot The slot in the handler that the signal will activate.
     * \return The index of the new menu item.
     */

    QAction* createSystemMenuItem(const QString &label, const QObject *handler, const char *slot);

    /*
     * Create a window for the widget in the main window
     */
    void createMdi(QMdiSubWindow *);

    /*!
     * Load window geometry based upon previously stored settings
     */
    void loadWindow(void);

private slots:
    void about(void);
    void aboutXeno(void);
    void aboutQt(void);
    void openDocs(void);
    void openSubIssue(void);

    void loadSettings(void);
    void saveSettings(void);
    void resetSettings(void);

    void windowsMenuAboutToShow(void);
    void windowsMenuActivated(QAction *);
    void modulesMenuActivated(QAction *);
    void fileMenuActivated(QAction *);
    void utilitiesMenuActivated(QAction *);

private:

    /****************************************************************
     * The constructor, destrutos, and assignment operator are made *
     *   private to control instantiation of the class.             *
     ****************************************************************/

    MainWindow(void);
    MainWindow(const MainWindow &) {};
    MainWindow &operator = (const MainWindow &)
    {
        return *getInstance();
    };

    static MainWindow *instance;
    QMdiArea *mdiArea;
    QList<QMdiSubWindow *> subWindows;

    QMenu *fileMenu;
    QMenu *moduleMenu;
    QMenu *utilMenu;
    QMenu *filtersSubMenu;
    QMenu *signalsSubMenu;
    QMenu *utilitiesSubMenu;
    QMenu *systemMenu;
    QMenu *windowsMenu;
    QMenu *helpMenu;

    QAction *load;
    QAction *save;
    QAction *reset;
    QAction *quit;
    QAction *artxi;
		QAction *axeno;
    QAction *aqt;
    QAction *adocs;
    QAction *sub_issue;
    QAction *utilItem;

    void createFileMenu();
    void createModuleMenu();
    void createUtilMenu();
    void createSystemMenu();
    void createWindowsMenu();
    void createHelpMenu();
    void createFileActions();
    void createHelpActions();
    void closeEvent(QCloseEvent *);
};
#endif /* MAIN_WINDOW_H */
