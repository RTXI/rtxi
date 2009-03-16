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

#include <debug.h>
#include <daq.h>
#include <math.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtabwidget.h>
#include <rt.h>
#include <system_control.h>
#include <system_control_panel.h>

struct find_daq_t {
    int index;
    DAQ::Device *device;
};

static void findDAQDevice(DAQ::Device *dev,void *arg) {
    struct find_daq_t *info = static_cast<struct find_daq_t *>(arg);
    if(!info->index)
        info->device = dev;
    info->index--;
}

static void buildDAQDeviceList(DAQ::Device *dev,void *arg) {
    QComboBox *deviceList = static_cast<QComboBox *>(arg);
    deviceList->insertItem(dev->getName());
}

SystemControlPanel::SystemControlPanel(QWidget *parent)
    : QWidget(parent,NULL,Qt::WStyle_NormalBorder | Qt::WDestructiveClose) {
    setCaption("System Control Panel");

    rateUpdate = false;

    QBoxLayout *layout = new QVBoxLayout(this);

    tabWidget = new QTabWidget(this);
    QObject::connect(tabWidget,SIGNAL(currentChanged(QWidget *)),this,SLOT(display(void)));
    layout->addWidget(tabWidget);

    QHBox *hbox0 = new QHBox(this);
    layout->addWidget(hbox0);
    QPushButton *applyButton = new QPushButton("Apply",hbox0);
    QObject::connect(applyButton,SIGNAL(clicked(void)),this,SLOT(apply(void)));
    QPushButton *okayButton = new QPushButton("Okay",hbox0);
    QObject::connect(okayButton,SIGNAL(clicked(void)),this,SLOT(okay(void)));
    QPushButton *cancelButton = new QPushButton("Cancel",hbox0);
    QObject::connect(cancelButton,SIGNAL(clicked(void)),this,SLOT(close(void)));

    channelTab = new QWidget(tabWidget);
    tabWidget->addTab(channelTab,"Channel");
    createChannelTab();

    threadTab = new QWidget(tabWidget);
    tabWidget->addTab(threadTab,"Thread");
    createThreadTab();

    updateDevice();
    show();
}

SystemControlPanel::~SystemControlPanel(void) {
    SystemControl::getInstance()->removeControlPanel(this);
}

void SystemControlPanel::apply(void) {
    switch(tabWidget->currentPageIndex()) {
      case 0:
          applyChannelTab();
          break;
      case 1:
          applyThreadTab();
          break;
      default:
          ERROR_MSG("SystemControl::apply : invalid page request\n");
    }

    // Refresh the View
    display();
}

void SystemControlPanel::okay(void) {
    apply();
    close();
}

void SystemControlPanel::updateDevice(void) {
    DAQ::Device *dev;
    DAQ::type_t type;
    {
        struct find_daq_t info = { deviceList->currentItem(), 0, };
        DAQ::Manager::foreachDevice(findDAQDevice,&info);
        dev = info.device;
    }

    analogChannelList->clear();
    digitalChannelList->clear();
    if(!dev) return;

    type = static_cast<DAQ::type_t>(analogSubdeviceList->currentItem());
    for(size_t i=0;i<dev->getChannelCount(type);++i)
        analogChannelList->insertItem(QString::number(i));

    type = static_cast<DAQ::type_t>(digitalSubdeviceList->currentItem()+DAQ::DIO);
    for(size_t i=0;i<dev->getChannelCount(type);++i)
        digitalChannelList->insertItem(QString::number(i));

    display();
}

void SystemControlPanel::updateFreq(void) {
    int i = 0;
    double freq, period;

    /* This is to prevent recursive updates, not to provide mutual exclusion */
    if(rateUpdate)
        return;
    else
        rateUpdate = true;

    /* Determine the Period */
    period = periodEdit->text().toDouble();
    period *= pow(10,-3*periodUnitList->currentItem());

    freq = 1 / period;

    if(freq > 1000) {
        freq /= 1000;
        i = 1;
    }

    freqEdit->setText(QString::number(freq));
    freqUnitList->setCurrentItem(i);

    rateUpdate = false;
}

void SystemControlPanel::updatePeriod(void) {
    int i = 0;
    double freq, period;

    if(rateUpdate)
        return;
    else
        rateUpdate = true;

    /* Determine the Frequency */
    freq = freqEdit->text().toDouble();
    if(freqUnitList->currentItem())
        freq *= 1000;

    period = 1 / freq;

    while(period < .001 && (i < 4)) {
        period *= 1000;
        i++;
    }

    periodEdit->setText(QString::number(period));
    periodUnitList->setCurrentItem(i);

    rateUpdate = false;
}

void SystemControlPanel::display(void) {
    switch(tabWidget->currentPageIndex()) {
      case 0:
          displayChannelTab();
          break;
      case 1:
          displayThreadTab();
          break;
      default:
          ERROR_MSG("SystemControl::display : invalid page request\n");
    }
}

void SystemControlPanel::applyChannelTab(void) {

    DAQ::Device *dev;
    {
        struct find_daq_t info = { deviceList->currentItem(), 0, };
        DAQ::Manager::foreachDevice(findDAQDevice,&info);
        dev = info.device;
    }

    DAQ::index_t a_chan = analogChannelList->currentItem();
    DAQ::type_t a_type = static_cast<DAQ::type_t>(analogSubdeviceList->currentItem());
    double a_gain = analogGainEdit->text().toDouble()*pow(10,-3*(analogUnitPrefixList->currentItem()-8));

    dev->setChannelActive(a_type,a_chan,analogActiveButton->isOn());
    dev->setAnalogGain(a_type,a_chan,a_gain);
    dev->setAnalogRange(a_type,a_chan,analogRangeList->currentItem());
    dev->setAnalogReference(a_type,a_chan,analogReferenceList->currentItem());
    dev->setAnalogUnits(a_type,a_chan,analogUnitList->currentItem());

    DAQ::index_t d_chan = digitalChannelList->currentItem();
    DAQ::type_t d_type = static_cast<DAQ::type_t>(digitalSubdeviceList->currentItem()+2);
    DAQ::direction_t d_dir = static_cast<DAQ::direction_t>(digitalDirectionList->currentItem());

    dev->setChannelActive(d_type,d_chan,digitalActiveButton->isOn());
    if(d_type == DAQ::DIO)
        dev->setDigitalDirection(d_chan,d_dir);
}

void SystemControlPanel::applyThreadTab(void) {
    double period = periodEdit->text().toDouble();
    period *= pow(10,3*(3-periodUnitList->currentItem()));

    RT::System::setPeriod(static_cast<long long>(period));
}

void SystemControlPanel::createChannelTab(void) {
    QHBox *hbox;
    QLabel *label;

    QBoxLayout *layout = new QVBoxLayout(channelTab);

    hbox = new QHBox(channelTab);
    hbox->setSpacing(2);
    layout->addWidget(hbox);
    label = new QLabel("Device:",hbox);
    label->setFixedWidth(60);
    deviceList = new QComboBox(hbox);
    DAQ::Manager::foreachDevice(buildDAQDeviceList,deviceList);
    QObject::connect(deviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));

    QGroupBox *analogBox = new QGroupBox("Analog",channelTab);
    layout->addWidget(analogBox);

    QBoxLayout *analogLayout = new QVBoxLayout(analogBox);
    analogLayout->setMargin(15);

    hbox = new QHBox(analogBox);
    hbox->setSpacing(2);
    analogLayout->addWidget(hbox);
    label = new QLabel("Channel:",hbox);
    label->setFixedWidth(60);
    analogSubdeviceList = new QComboBox(hbox);
    analogSubdeviceList->insertItem("Input");
    analogSubdeviceList->insertItem("Output");
    QObject::connect(analogSubdeviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));
    analogChannelList = new QComboBox(hbox);
    analogChannelList->setFixedWidth(60);
    QObject::connect(analogChannelList,SIGNAL(activated(int)),this,SLOT(display(void)));
    analogActiveButton = new QPushButton("Active",hbox);
    analogActiveButton->setToggleButton(true);
    analogActiveButton->setFixedWidth(60);

    hbox = new QHBox(analogBox);
    hbox->setSpacing(2);
    analogLayout->addWidget(hbox);
    label = new QLabel("Range:",hbox);
    label->setFixedWidth(60);
    analogRangeList = new QComboBox(hbox);
    analogReferenceList = new QComboBox(hbox);

    hbox = new QHBox(analogBox);
    hbox->setSpacing(2);
    analogLayout->addWidget(hbox);
    label = new QLabel("Scale:",hbox);
    label->setFixedWidth(60);
    analogGainEdit = new QLineEdit(hbox);
    analogGainEdit->setAlignment(Qt::AlignRight);
    analogUnitPrefixList = new QComboBox(hbox);
    analogUnitPrefixList->setFixedWidth(70);
    analogUnitPrefixList->insertItem("yotta-");
    analogUnitPrefixList->insertItem("zetta-");
    analogUnitPrefixList->insertItem("exa-");
    analogUnitPrefixList->insertItem("peta-");
    analogUnitPrefixList->insertItem("tera-");
    analogUnitPrefixList->insertItem("giga-");
    analogUnitPrefixList->insertItem("mega-");
    analogUnitPrefixList->insertItem("kilo-");
    analogUnitPrefixList->insertItem("");
    analogUnitPrefixList->insertItem("milli-");
    analogUnitPrefixList->insertItem("micro-");
    analogUnitPrefixList->insertItem("nano-");
    analogUnitPrefixList->insertItem("pico-");
    analogUnitPrefixList->insertItem("femto-");
    analogUnitPrefixList->insertItem("atto-");
    analogUnitPrefixList->insertItem("zepto-");
    analogUnitPrefixList->insertItem("yocto-");
    analogUnitList = new QComboBox(hbox);
    analogUnitList->setFixedWidth(70);
    new QLabel(" / Volt\n",hbox);

    QGroupBox *digitalBox = new QGroupBox("Digital",channelTab);
    layout->addWidget(digitalBox);

    QBoxLayout *digitalLayout = new QVBoxLayout(digitalBox);
    digitalLayout->setMargin(15);

    hbox = new QHBox(digitalBox);
    hbox->setSpacing(2);
    digitalLayout->addWidget(hbox);
    label = new QLabel("Channel:",hbox);
    label->setFixedWidth(60);
    digitalSubdeviceList = new QComboBox(hbox);
    digitalSubdeviceList->insertItem("Input / Output");
    digitalSubdeviceList->insertItem("Input");
    digitalSubdeviceList->insertItem("Output");
    QObject::connect(digitalSubdeviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));
    digitalChannelList = new QComboBox(hbox);
    digitalChannelList->setFixedWidth(60);
    QObject::connect(digitalChannelList,SIGNAL(activated(int)),this,SLOT(display(void)));
    digitalActiveButton = new QPushButton("Active",hbox);
    digitalActiveButton->setToggleButton(true);
    digitalActiveButton->setFixedWidth(60);

    hbox = new QHBox(digitalBox);
    hbox->setSpacing(2);
    digitalLayout->addWidget(hbox);
    label = new QLabel("Direction:",hbox);
    label->setFixedWidth(60);
    digitalDirectionList = new QComboBox(hbox);
    digitalDirectionList->insertItem("Input");
    digitalDirectionList->insertItem("Output");
}

void SystemControlPanel::createThreadTab(void) {
    QBoxLayout *layout = new QVBoxLayout(threadTab);

    QGroupBox *rateBox = new QGroupBox("Sample Rate",threadTab);
    layout->addWidget(rateBox);

    QBoxLayout *rateLayout = new QVBoxLayout(rateBox);
    rateLayout->setMargin(15);

    QHBox *hbox0 = new QHBox(rateBox);
    rateLayout->addWidget(hbox0);
    QLabel *label4 = new QLabel("Frequency:",hbox0);
    label4->setFixedWidth(70);
    freqEdit = new QLineEdit(hbox0);
    freqEdit->setAlignment(Qt::AlignRight);
    QObject::connect(freqEdit,SIGNAL(textChanged(const QString &)),this,SLOT(updatePeriod(void)));
    freqUnitList = new QComboBox(hbox0);
    freqUnitList->setFixedWidth(50);
    freqUnitList->insertItem(" Hz");
    freqUnitList->insertItem("kHz");  
    QObject::connect(freqUnitList,SIGNAL(activated(int)),this,SLOT(updatePeriod(void)));

    QHBox *hbox1 = new QHBox(rateBox);
    rateLayout->addWidget(hbox1);
    QLabel *label5 = new QLabel("Period:",hbox1);
    label5->setFixedWidth(70);
    periodEdit = new QLineEdit(hbox1);
    periodEdit->setAlignment(Qt::AlignRight);
    QObject::connect(periodEdit,SIGNAL(textChanged(const QString &)),this,SLOT(updateFreq(void)));
    periodUnitList = new QComboBox(hbox1);
    periodUnitList->setFixedWidth(50);
    periodUnitList->insertItem(" s");
    periodUnitList->insertItem("ms");
    periodUnitList->insertItem("us");
    periodUnitList->insertItem("ns");
    QObject::connect(periodUnitList,SIGNAL(activated(int)),this,SLOT(updateFreq(void)));
}

void SystemControlPanel::displayChannelTab(void) {
    DAQ::Device *dev;
    {
        struct find_daq_t info = { deviceList->currentItem(), 0, };
        DAQ::Manager::foreachDevice(findDAQDevice,&info);
        dev = info.device;
    }

    if(!dev) {
        deviceList->setEnabled(false);
        analogSubdeviceList->setEnabled(false);
        digitalSubdeviceList->setEnabled(false);
    }

    if(!dev || !analogChannelList->count()) {
        analogActiveButton->setOn(false);
        analogActiveButton->setEnabled(false);
        analogChannelList->setEnabled(false);
        analogRangeList->setEnabled(false);
        analogReferenceList->setEnabled(false);
        analogGainEdit->setEnabled(false);
        analogUnitPrefixList->setEnabled(false);
        analogUnitList->setEnabled(false);
    } else {
        DAQ::type_t type = static_cast<DAQ::type_t>(analogSubdeviceList->currentItem());
        DAQ::index_t chan = static_cast<DAQ::index_t>(analogChannelList->currentItem());

        analogActiveButton->setEnabled(true);
        analogRangeList->setEnabled(true);
        analogReferenceList->setEnabled(true);
        analogGainEdit->setEnabled(true);
        analogUnitPrefixList->setEnabled(true);
        analogUnitList->setEnabled(true);

        analogRangeList->clear();
        for(size_t i=0;i < dev->getAnalogRangeCount(type,chan);++i)
            analogRangeList->insertItem(dev->getAnalogRangeString(type,chan,i));
        analogReferenceList->clear();
        for(size_t i=0;i < dev->getAnalogReferenceCount(type,chan);++i)
            analogReferenceList->insertItem(dev->getAnalogReferenceString(type,chan,i));
        analogUnitList->clear();
        for(size_t i=0;i < dev->getAnalogUnitsCount(type,chan);++i)
            analogUnitList->insertItem(dev->getAnalogUnitsString(type,chan,i));

        analogActiveButton->setOn(dev->getChannelActive(type,chan));
        analogRangeList->setCurrentItem(dev->getAnalogRange(type,chan));
        analogReferenceList->setCurrentItem(dev->getAnalogReference(type,chan));
        analogUnitList->setCurrentItem(dev->getAnalogUnits(type,chan));

        // Determine the Correct Prefix
        int i = 8;
        double tmp;
        tmp = dev->getAnalogGain(type,chan);
        if(tmp != 0.0)
            while(((tmp >= 1000)&&(i > 0))||((tmp < 1)&&(i < 16))) {
                if(tmp >= 1000) {
                    tmp /= 1000;
                    i--;
                }
                else {
                    tmp *= 1000;
                    i++;
                }
            }
        analogGainEdit->setText(QString::number(tmp));
        analogUnitPrefixList->setCurrentItem(i);
    }

    if(!dev || !digitalChannelList->count()) {
        digitalActiveButton->setOn(false);
        digitalActiveButton->setEnabled(false);
        digitalChannelList->setEnabled(false);
        digitalDirectionList->setEnabled(false);
    } else {
        DAQ::type_t type = static_cast<DAQ::type_t>(digitalSubdeviceList->currentItem()+DAQ::DIO);
        DAQ::index_t chan = static_cast<DAQ::index_t>(digitalChannelList->currentItem());

        digitalActiveButton->setEnabled(true);
        digitalChannelList->setEnabled(true);
        if(type == DAQ::DIO)
            digitalDirectionList->setEnabled(true);
        else
            digitalDirectionList->setEnabled(false);

        digitalActiveButton->setOn(dev->getChannelActive(type,chan));

        if(type == DAQ::DIO) {
            digitalDirectionList->setEnabled(true);
            digitalDirectionList->setCurrentItem(dev->getDigitalDirection(chan));
        } else
            digitalDirectionList->setEnabled(false);
    }
}

void SystemControlPanel::displayThreadTab(void) {
    int i = 3;
    long long tmp = RT::System::getPeriod();
    while((tmp >= 1000)&&(i)) {
        tmp /= 1000;
        i--;
    }
    periodEdit->setText(QString::number(static_cast<unsigned long>(tmp)));
    periodUnitList->setCurrentItem(i);

    updateFreq();
}

void SystemControlPanel::receiveEvent(const Event::Object *event) {
    if(event->getName() == Event::RT_POSTPERIOD_EVENT)
        displayThreadTab();
    if(event->getName() == Event::SETTINGS_OBJECT_INSERT_EVENT ||
       event->getName() == Event::SETTINGS_OBJECT_REMOVE_EVENT) {
        deviceList->clear();
        DAQ::Manager::foreachDevice(buildDAQDeviceList,deviceList);
        updateDevice();
    }
}
