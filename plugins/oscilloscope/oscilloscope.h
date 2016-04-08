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

/*
	 Oscilloscope namespace. The namespace works with
	 both Scope and Panel classes to instantiate an oscilloscope
	 plugin.
 */

#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QtWidgets>

#include <event.h>
#include <fifo.h>
#include <io.h>
#include <mutex.h>
#include <plugin.h>
#include <rt.h>
#include <settings.h>

#include "scope.h"

namespace Oscilloscope
{

class SpinBox;
class CheckBox;
class Panel;

class Plugin : public QObject, public ::Plugin::Object, public RT::Thread
{

    Q_OBJECT

    friend class Panel;

public:
    static Plugin *getInstance(void);

public slots:
    void createOscilloscopePanel(void);

protected:
    virtual void doDeferred(const Settings::Object::State &);
    virtual void doLoad(const Settings::Object::State &);
    virtual void doSave(Settings::Object::State &) const;

private:
    Plugin(void);
    ~Plugin(void);
    Plugin(const Plugin &) {};
    Plugin &operator=(const Plugin &)
    {
        return *getInstance();
    };
    static Plugin *instance;
    void removeOscilloscopePanel(Panel *);

    // List to maintain multiple scopes
    std::list<Panel *> panelList;
}; // Plugin

class Panel : public QWidget, public RT::Thread, public virtual Settings::Object, public Event::Handler
{

    Q_OBJECT

    friend class Scope;

public:
    Panel(QWidget * = NULL);
    virtual ~Panel(void);
    void execute(void);
    bool setInactiveSync(void);
    void flushFifo(void);
    void adjustDataSize(void);
    void doDeferred(const Settings::Object::State &);
    void doLoad(const Settings::Object::State &);
    void doSave(Settings::Object::State &) const;
    void receiveEvent(const ::Event::Object *);

public slots:
    void timeoutEvent(void);
    void togglePause(void);

protected:

private slots:
    void showChannelTab(void);
    void showDisplayTab(void);
    void buildChannelList(void);
    void screenshot(void);
    void apply(void);
    void showTab(int);
    void activateChannel(bool);

private:
    QMdiSubWindow *subWindow;

    // Tab Widget
    QTabWidget *tabWidget;

    // Create scope
    Scope *scopeWindow;

    // Create curve element
    QwtPlotCurve *curve;

    // Functions to initialize and
    // apply changes made in tabs
    void applyChannelTab(void);
    void applyDisplayTab(void);
    QWidget *createChannelTab(QWidget *parent);
    QWidget *createDisplayTab(QWidget *parent);

    // Group and layout information
    QVBoxLayout *layout;
    QWidget *scopeGroup;
    QGroupBox *setBttnGroup;

    // Properties
    QSpinBox *ratesSpin;
    QLineEdit *sizesEdit;
    QButtonGroup *trigsGroup;
    QComboBox *timesList;
    QComboBox *trigsChanList;
    QComboBox *trigsThreshList;
    QSpinBox *refreshsSpin;
    QLineEdit *trigsThreshEdit;
    QLineEdit *trigWindowEdit;
    QComboBox *trigWindowList;

    // Lists
    QComboBox *blocksList;
    QComboBox *typesList;
    QComboBox *channelsList;
    QComboBox *colorsList;
    QComboBox *offsetsList;
    QComboBox *scalesList;
    QComboBox *stylesList;
    QComboBox *widthsList;
    QLineEdit *offsetsEdit;

    // Buttons
    QPushButton *pauseButton;
    QPushButton *settingsButton;
    QPushButton *applyButton;
    QPushButton *activateButton;

    Fifo fifo;
    std::vector<IO::Block *> blocks;
    size_t counter;
    size_t downsample_rate;
}; // Panel
}; // Oscilloscope

#endif // OSCILLOSCOPE_H
