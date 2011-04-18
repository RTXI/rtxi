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

#ifndef SYSTEM_CONTROL_PANEL_H
#define SYSTEM_CONTROL_PANEL_H

#include <event.h>
#include <qwidget.h>

class QComboBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;
class QWidget;

class SystemControlPanel : public QWidget, public Event::Handler
{

    Q_OBJECT

public:

    SystemControlPanel(QWidget *); 
    virtual ~SystemControlPanel(void);

public slots:

    void apply(void);
    void okay(void); 
    void display(void);
    void updateDevice(void);
    void updateFreq(void);
    void updatePeriod(void);

private:

    void __display(void);
    void applyChannelTab(void);
    void applyThreadTab(void);
    void createChannelTab(void);
    void createThreadTab(void);
    void displayChannelTab(void);
    void displayThreadTab(void);
    void receiveEvent(const Event::Object *);

    QTabWidget *tabWidget;
    QWidget *channelTab;
    QWidget *threadTab;

    /* Channel Tab Items */
    QComboBox *deviceList;

    QComboBox *analogChannelList;
    QComboBox *analogRangeList;
    QComboBox *analogReferenceList;
    QComboBox *analogSubdeviceList;
    QComboBox *analogUnitPrefixList;
    QComboBox *analogUnitList;
    QLineEdit *analogGainEdit;
    QPushButton *analogActiveButton;

    QComboBox *digitalChannelList;
    QComboBox *digitalDirectionList;
    QComboBox *digitalSubdeviceList;
    QPushButton *digitalActiveButton;

    /* Thread Tab Items */
    bool rateUpdate;
    QComboBox *freqUnitList;
    QComboBox *periodUnitList;
    QLineEdit *freqEdit;
    QLineEdit *periodEdit;

};

#endif /* SYSTEM_CONTROL_PANEL_H */
