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

#ifndef SYSTEM_CONTROL_PANEL_H
#define SYSTEM_CONTROL_PANEL_H

#include <QtWidgets>

#include <event.h>

class SystemControlPanel : public QWidget, public Event::Handler
{

    Q_OBJECT

public:
    SystemControlPanel(QWidget *);
    virtual ~SystemControlPanel(void);

public slots:
    void apply(void);
    void display(void);
    void updateDevice(void);
    void updateFreq(void);
    void updatePeriod(void);

private:
    void __display(void);
    void receiveEvent(const Event::Object *);

    QGroupBox *deviceGroup;
    QGroupBox *analogGroup;
    QGroupBox *digitalGroup;
    QGroupBox *buttonGroup;

    QMdiSubWindow *subWindow;

    QComboBox *deviceList;
    QComboBox *analogChannelList;
    QComboBox *analogRangeList;
    QComboBox *analogDownsampleList;
    QComboBox *analogReferenceList;
    QComboBox *analogSubdeviceList;
    QComboBox *analogUnitPrefixList;
    QComboBox *analogUnitList;
    QComboBox *analogUnitPrefixList2;
    QComboBox *analogUnitList2;
    QLineEdit *analogGainEdit;
    QLineEdit *analogZeroOffsetEdit;
    QPushButton *analogActiveButton;
    QPushButton *analogCalibrationButton;

    QComboBox *digitalChannelList;
    QComboBox *digitalDirectionList;
    QComboBox *digitalSubdeviceList;
    QPushButton *digitalActiveButton;

    bool rateUpdate;
    QComboBox *freqUnitList;
    QComboBox *periodUnitList;
    QLineEdit *freqEdit;
    QLineEdit *periodEdit;
};

#endif /* SYSTEM_CONTROL_PANEL_H */
