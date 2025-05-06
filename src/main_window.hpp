/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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

#include <QDialog>
#include <QMainWindow>
#include <unordered_map>

#include "event.hpp"

class QSettings;
class QAction;
class QMdiArea;
class QMdiSubWindow;
class QMenu;
class QTextEdit;
class QTextItem;

namespace Widgets
{
class Plugin;
}  // namespace Widgets

namespace IO
{
class Block;
}  // namespace IO

class SaveSettingsDialog : public QDialog
{
public:
private:
  QTextItem* settings_group_desc;  // NOLINT
  QTextEdit* settings_group_edit;  // NOLINT
};

/*!
 * The primary graphical object in the program.
 */
class MainWindow : public QMainWindow
{
  Q_OBJECT
public:
  explicit MainWindow(Event::Manager* ev_manager);

  /*!
   * Add an item to the File menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */
  QAction* createFileMenuItem(const QString& label);

  /*!
   * Clears the File menu.
   */
  void clearFileMenu();

  /*!
   * Insert separator
   */
  QAction* insertWidgetMenuSeparator();

  /*!
   * Add an item to the Widgets menu.
   *
   * \param label The text that will appear in the menu.
   *
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */
  QAction* createWidgetMenuItem(const QString& text);
  QAction* createWidgetMenuItem(const QString& text,
                                const QObject* receiver,
                                const char* member);

  /*!
   * Sets the parameter value of a menu item in the Widgets menu.
   *
   * \param menuid The index of the item to be removed.
   * \param parameter The parameter value to set.
   */
  static void setWidgetMenuItemParameter(QAction* action, int parameter);

  /*!
   * Clears the Widgets menu.
   */
  void clearWidgetMenu();

  /*!
   * Change the text associated with the Widgets menu item.
   *
   * \param id The index of the item to change.
   * \param text The next text to assign to that menu item.
   */
  static void changeWidgetMenuItem(QAction* action, const QString& text);

  /*!
   * Remove an item from the Widgets menu.
   *
   * \param index The index of the item to be removed.
   */
  void removeWidgetMenuItem(QAction* action);

  /*!
   * Add an item to the Utilities menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */
  QAction* createUtilMenuItem(const QString& label,
                              const QObject* handler,
                              const char* slot);

  /*!
   * Add an item to the System menu.
   *
   * \param label The text that will appear in the menu.
   * \param handler The object that will handle signals from the menu.
   * \param slot The slot in the handler that the signal will activate.
   * \return The index of the new menu item.
   */
  QAction* createSystemMenuItem(const QString& label,
                                const QObject* handler,
                                const char* slot);

  /*!
   * Load window geometry based upon previously stored settings
   */
  void loadWindow();

private slots:
  void about();
  void aboutXeno();
  void aboutQt();
  static void openDocs();
  static void openSubIssue();

  void loadSettings();
  void saveSettings();
  void resetSettings();

  void windowsMenuAboutToShow();
  void windowsMenuActivated(QAction* id);
  void modulesMenuActivated(QAction* /*unused*/);
  void fileMenuActivated(QAction* id);
  void utilitiesMenuActivated(QAction* /*unused*/);
  void systemMenuActivated(QAction* id);

private:
  void loadWidget(const QString& module_name);
  void loadWidget(const QString& module_name,
                  Widgets::Plugin*& rtxi_plugin_pointer);
  inline void savePeriodSettings(QSettings& userprefs);
  inline void loadPeriodSettings(QSettings& userprefs);
  inline void saveDAQSettings(QSettings& userprefs);
  inline void loadDAQSettings(QSettings& userprefs,
                              std::unordered_map<int, IO::Block*>& block_cache);
  inline void saveWidgetSettings(QSettings& userprefs);
  inline void loadWidgetSettings(
      QSettings& userprefs, std::unordered_map<int, IO::Block*>& block_cache);
  inline void saveConnectionSettings(QSettings& userprefs);
  inline void loadConnectionSettings(
      QSettings& userprefs, std::unordered_map<int, IO::Block*>& block_cache);
  Event::Manager* event_manager;
  QMdiArea* mdiArea = nullptr;
  QList<QMdiSubWindow*> subWindows;

  QMenu* fileMenu = nullptr;
  QMenu* moduleMenu = nullptr;
  QMenu* utilMenu = nullptr;
  QMenu* filtersSubMenu = nullptr;
  QMenu* signalsSubMenu = nullptr;
  QMenu* utilitiesSubMenu = nullptr;
  QMenu* systemMenu = nullptr;
  QMenu* windowsMenu = nullptr;
  QMenu* helpMenu = nullptr;

  QAction* load = nullptr;
  QAction* save = nullptr;
  QAction* reset = nullptr;
  QAction* quit = nullptr;
  QAction* artxi = nullptr;
  QAction* axeno = nullptr;
  QAction* aqt = nullptr;
  QAction* adocs = nullptr;
  QAction* sub_issue = nullptr;
  QAction* utilItem = nullptr;
  QAction* loadDynamicWidget = nullptr;
  QAction* openDataRecorder = nullptr;
  QAction* openRTBenchmarks = nullptr;
  QAction* openUserPrefs = nullptr;
  QAction* openControlPanel = nullptr;
  QAction* openConnector = nullptr;
  QAction* openOscilloscope = nullptr;
  QAction* openRTXIWizard = nullptr;

  void createFileMenu();
  void createWidgetMenu();
  void createUtilMenu();
  void createSystemMenu();
  void createSystemActions();
  void createWindowsMenu();
  void createHelpMenu();
  void createFileActions();
  void createHelpActions();
  void closeEvent(QCloseEvent* event) override;
};
#endif /* MAIN_WINDOW_H */
