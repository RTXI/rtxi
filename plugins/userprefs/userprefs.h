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

#ifndef USERPREFS_H
#define USERPREFS_H

#include <qwidget.h>
#include <qsettings.h>

#include <plugin.h>
#include <qobject.h>
#include <rt.h>

class QLineEdit;

namespace UserPrefs
{

  class Panel;

  class Prefs : public QObject, public ::Plugin::Object
  {

    Q_OBJECT

    friend class Panel;

  public:

    static Prefs *
    getInstance(void);

public slots:

  void createPrefsPanel(void);

  private:

    Prefs(void);
    ~Prefs(void);
    Prefs(const Prefs &)
    {
    }
    ;
    Prefs &
    operator=(const Prefs &)
    {
      return *getInstance();
    }
    ;

    static Prefs *instance;

    int menuID;
    Panel *panel;

  }; // class Prefs

  class Panel : public QWidget
  {

  Q_OBJECT

  public:

    Panel(QWidget *);
    virtual
    ~Panel(void);

  public slots:

    void apply(void); // save and close
    void
    reset(void); // reset to defaults
    void
    cancel(void); // close with saving

    void
    chooseSettingsDir(void);
    void
    chooseDynamoDir(void);
    void
    chooseDataDir(void);

  private:

    int menuID;

    QSettings userprefs;

    QLineEdit *settingsDirEdit; // directory for settings files
    QLineEdit *dynamoDirEdit; // directory for DYNAMO files
    QLineEdit *dataDirEdit; // directory of most recent data file
    QLineEdit *HDFBufferEdit; // buffer size for HDF Data Recorder

  }; // class Panel

}
; // namespace USERPREFS

#endif /* USERPREFS */
