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

#ifndef USERPREFS_H
#define USERPREFS_H

#include <QtWidgets>

#include <plugin.h>
#include <rt.h>

namespace UserPrefs
{

class Panel;
class Prefs : public QObject, public ::Plugin::Object
{

    Q_OBJECT

    friend class Panel;

public:
    static Prefs *getInstance(void);

public slots:
    void createPrefsPanel(void);

private:
    Prefs(void);
    ~Prefs(void);
    Prefs(const Prefs &) {};
    Prefs & operator=(const Prefs &)
    {
        return *getInstance();
    };

    static Prefs *instance;
    Panel *panel;
}; // class Prefs

class Panel : public QWidget
{

    Q_OBJECT

public:
    Panel(QWidget *);
    virtual ~Panel(void);
    QLabel *status;

public slots:
    void apply(void); // save and close
    void reset(void); // reset to defaults

    void chooseSettingsDir(void);
    void chooseDataDir(void);

private:
    QMdiSubWindow *subWindow;
    QSettings userprefs;

    QGroupBox *dirGroup;
    QGroupBox *HDF;
    QGroupBox *buttons;

    QLineEdit *settingsDirEdit; // directory for settings files
    QLineEdit *dataDirEdit; // directory of most recent data file
    QLineEdit *HDFBufferEdit; // buffer size for HDF Data Recorder
}; // class Panel
}; // namespace USERPREFS
#endif /* USERPREFS */
