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

#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <qmainwindow.h>
#include <qaction.h>

class QPopupMenu;

/*!
 * The primary graphical object in the program.
 */
class MainWindow : public QMainWindow
{

  Q_OBJECT

  friend class QApplication;

public:

  virtual
  ~MainWindow(void);

  /*!
   * MainWindow is a Singleton, which means that there can only be one
   *   instance. This function returns a pointer to that single instance.
   *
   * \return The instance of MainWindow.
   */

  static MainWindow *
  getInstance(void);

  /*!
   * Add an item to the File menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */

  int
  createFileMenuItem(const std::string &label, const QObject *handler,
      const char *slot);

  /*!
   * Sets the parameter value of a menu item in the File menu.
   *
   * \param menuid The index of the item to be removed.
   * \param parameter The parameter value to set.
   */

  void
  setFileMenuItemParameter(int menuid, int parameter);

  /*!
   * Clears the File menu.
   */

  void
  clearFileMenu(void);

  /*!
   * Add an item to the Modules menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */

  int
  createModuleMenuItem(const std::string &label, const QObject *handler,
      const char *slot);

  /*!
   * Sets the parameter value of a menu item in the Modules menu.
   *
   * \param menuid The index of the item to be removed.
   * \param parameter The parameter value to set.
   */

  void
  setModuleMenuItemParameter(int menuid, int parameter);

  /*!
   * Insert a line separator into the Modules menu.
   */

  int
  insertModuleMenuSeparator();

  /*!
   * Clears the Modules menu.
   */

  void
  clearModuleMenu(void);

  /*!
   * Change the text associated with the Modules menu item.
   *
   * \param id The index of the item to change.
   * \param text The next text to assign to that menu item.
   */

  void
  changeModuleMenuItem(int id, QString text);

  /*!
   * Remove an item from the Modules menu.
   *
   * \param index The index of the item to be removed.
   */

  void
  removeModuleMenuItem(int index);

  /*!
   * Remove an item from the Modules menu.
   *
   * \param index The position of the item to be removed.
   */

  void
  removeModuleMenuItemAt(int index);

  /*!
   * Add an item to the Utilities menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */

  int
  createUtilMenuItem(const std::string &label, const QObject *handler,
      const char *slot);

  /*!
   * Sets the parameter value of a menu item in the Utilities menu.
   */

  void
  setUtilMenuItemParameter(int, int);

  /*!
   * Clears the Utilities menu.
   */

  void
  clearUtilMenu(void);

  /*!
   * Change the text associated with the Utilities menu item.
   *
   * \param id The index of the item to change.
   * \param text The next text to assign to that menu item.
   */

  void
  changeUtilMenuItem(int id, QString text);

  /*!
   * Remove an item from the Utilities menu.
   *
   * \param index The index of the item to be removed.
   */
  void
  removeUtilMenuItem(int index);

  /*!
   * Add an item to the Utilities->Patch Clamp menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */

  int
  createPatchClampMenuItem(const std::string &label, const QObject *handler,
      const char *slot);
    
  /*!
   * Add an item to the System menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */

  int
  createSystemMenuItem(const std::string &label, const QObject *handler,
      const char *slot);
    
  /*!
   * Insert a line separator into the System menu.
   */

  int
  insertSystemMenuSeparator();

  /*!
   * Remove an item from the System menu.
   *
   * \param index The index of the item to be removed.
   */
  void
  removeSystemMenuItem(int index);

private slots:

  void about(void);
  void
  aboutQt(void);
  void
  aboutComedi(void);
  void
  loadSettings(void);
  void
  saveSettings(void);
  void
  windowsMenuAboutToShow(void);
  void
  windowsMenuActivated(int);
  void loadUtil(int);
  void loadSignal(int);
  void loadFilter(int);

private:

  /****************************************************************
   * The constructor, destrutos, and assignment operator are made *
   *   private to control instantiation of the class.             *
   ****************************************************************/

  MainWindow(void);
  MainWindow(const MainWindow &)
  {
  }
  ;
  MainWindow &
  operator=(const MainWindow &)
  {
    return *getInstance();
  }
  ;

  static MainWindow *instance;

  QPopupMenu *fileMenu;
  QPopupMenu *moduleMenu;
  QPopupMenu *utilMenu;
  QPopupMenu *patchClampSubMenu;
  QPopupMenu *systemMenu;
  QPopupMenu *windowsMenu;

  void updateUtilModules();
};

#endif /* MAIN_WINDOW_H */
