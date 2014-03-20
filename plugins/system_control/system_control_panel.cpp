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

#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QWidget>
#include <QtGui>
#include <QWhatsThis>
#include <QPushButton>

#include <debug.h>
#include <daq.h>
#include <math.h>
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
	deviceList->addItem(QString::fromStdString(dev->getName()));
}

SystemControlPanel::SystemControlPanel(QWidget *parent) : QWidget(parent) {
	QWidget::setAttribute(Qt::WA_DeleteOnClose);
	setWindowTitle("System Control Panel");
	setWhatsThis(
			"<p><b>System Control Panel:</b><br>This control panel allows you to configure "
			"the channels on your DAQ card. RTXI automatically detects the number and types "
			"of channels that are available. You should set the \"Scale\" of the channel "
			"to be the inverse of the gain that is applied to the signal through the "
			"combination of hardware and software that you are using. To acquire data from "
			"a channel, you must set it to be Active using the toggle button. Any parameter "
			"settings such as the \"Range\" or \"Scale\" must be set by clicking the \"Apply\" "
			"button.<br><br> The \"Thread\" tab allows you to set the period for real-time "
			"execution. You must click \"Apply\" to change this setting and propagate it "
			"to other user modules such as the Data Recorder. Custom user modules can "
			"execute special code when the real-time period is changed using the update(PERIOD) "
			"flag.</p>");
	rateUpdate = false;

	QBoxLayout *layout = new QVBoxLayout(this);

	tabWidget = new QTabWidget(this);
	QObject::connect(tabWidget,SIGNAL(currentChanged(QWidget *)),this,SLOT(display(void)));
	layout->addWidget(tabWidget);

	QWidget *hbox0 = new QWidget;
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
	printf("coming from 1\n");
	switch(tabWidget->currentIndex()) {
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
		printf("coming from 2\n");
		struct find_daq_t info = { deviceList->currentIndex(), 0, };
		DAQ::Manager::getInstance()->foreachDevice(findDAQDevice,&info);
		dev = info.device;
	}

	analogChannelList->clear();
	digitalChannelList->clear();
	if(!dev) return;

	printf("coming from 3\n");
	type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());
	for(size_t i=0;i<dev->getChannelCount(type);++i)
		analogChannelList->addItem(QString::number(i));

	printf("coming from 4\n");
	type = static_cast<DAQ::type_t>(digitalSubdeviceList->currentIndex()+DAQ::DIO);
	for(size_t i=0;i<dev->getChannelCount(type);++i)
		digitalChannelList->addItem(QString::number(i));

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
	printf("coming from 5\n");
	period *= pow(10,-3*periodUnitList->currentIndex());

	freq = 1 / period;

	if(freq > 1000) {
		freq /= 1000;
		i = 1;
	}

	freqEdit->setText(QString::number(freq));
	freqUnitList->setCurrentIndex(i);

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
	printf("coming from 6\n");
	if(freqUnitList->currentIndex())
		freq *= 1000;

	period = 1 / freq;

	while(period < .001 && (i < 4)) {
		period *= 1000;
		i++;
	}

	periodEdit->setText(QString::number(period));
	periodUnitList->setCurrentIndex(i);

	rateUpdate = false;
}

void SystemControlPanel::display(void) {
	printf("coming from 7\n");
	switch(tabWidget->currentIndex()) {
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

	DAQ::Device *dev; {
		printf("coming from 8\n");
		struct find_daq_t info = { deviceList->currentIndex(), 0, };
		DAQ::Manager::getInstance()->foreachDevice(findDAQDevice,&info);
		dev = info.device;
	}

	printf("coming from 9\n");
	DAQ::index_t a_chan = analogChannelList->currentIndex();
	printf("coming from 10\n");
	DAQ::type_t a_type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());
	printf("coming from 11\n");
	double a_gain = analogGainEdit->text().toDouble()*pow(10,-3*(analogUnitPrefixList->currentIndex()-8));
	printf("coming from 12\n");
	double a_zerooffset = analogZeroOffsetEdit->text().toDouble()*pow(10,-3*(analogUnitPrefixList2->currentIndex()-8));

	dev->setChannelActive(a_type,a_chan,analogActiveButton->isChecked());
	dev->setAnalogCalibrationActive(a_type,a_chan,analogCalibrationButton->isChecked());
	dev->setAnalogGain(a_type,a_chan,a_gain);
	dev->setAnalogZeroOffset(a_type,a_chan,a_zerooffset);
	dev->setAnalogRange(a_type,a_chan,analogRangeList->currentIndex());
	dev->setAnalogReference(a_type,a_chan,analogReferenceList->currentIndex());
	dev->setAnalogUnits(a_type,a_chan,analogUnitList->currentIndex());
	dev->setAnalogCalibrationActive(a_type,a_chan,analogCalibrationButton->isChecked());

	printf("coming from 13\n");
	DAQ::index_t d_chan = digitalChannelList->currentIndex();
	DAQ::type_t d_type = static_cast<DAQ::type_t>(digitalSubdeviceList->currentIndex()+2);
	DAQ::direction_t d_dir = static_cast<DAQ::direction_t>(digitalDirectionList->currentIndex());

	dev->setChannelActive(d_type,d_chan,digitalActiveButton->isChecked());
	if(d_type == DAQ::DIO)
		dev->setDigitalDirection(d_chan,d_dir);
}

void SystemControlPanel::applyThreadTab(void) {
	double period = periodEdit->text().toDouble();
	printf("coming from 14\n");
	period *= pow(10,3*(3-periodUnitList->currentIndex()));
	RT::System::getInstance()->setPeriod(static_cast<long long>(period));
}

void SystemControlPanel::createChannelTab(void) {
	QWidget *hbox;
	QLabel *label;

	QBoxLayout *layout = new QVBoxLayout(channelTab);

	hbox = new QWidget(channelTab);
	//hbox->setSpacing(2);
	layout->addWidget(hbox);
	label = new QLabel("Device:",hbox);
	label->setFixedWidth(60);
	deviceList = new QComboBox(hbox);
	DAQ::Manager::getInstance()->foreachDevice(buildDAQDeviceList,deviceList);
	QObject::connect(deviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));

	QGroupBox *analogBox = new QGroupBox("Analog",channelTab);
	layout->addWidget(analogBox);

	QBoxLayout *analogLayout = new QVBoxLayout(analogBox);
	analogLayout->setMargin(15);

	hbox = new QWidget(analogBox);
	//hbox->setSpacing(2);
	analogLayout->addWidget(hbox);
	label = new QLabel("Channel:",hbox);
	label->setFixedWidth(60);
	analogSubdeviceList = new QComboBox(hbox);
	analogSubdeviceList->addItem("Input");
	analogSubdeviceList->addItem("Output");
	QObject::connect(analogSubdeviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));
	analogChannelList = new QComboBox(hbox);
	analogChannelList->setFixedWidth(60);
	QObject::connect(analogChannelList,SIGNAL(activated(int)),this,SLOT(display(void)));
	analogActiveButton = new QPushButton("Active",hbox);
	analogActiveButton->setCheckable(true);
	analogActiveButton->setFixedWidth(60);
	analogCalibrationButton = new QPushButton("Calibrate",hbox);
	analogCalibrationButton->setCheckable(true);
	analogCalibrationButton->setFixedWidth(60);

	hbox = new QWidget(analogBox);
	//hbox->setSpacing(2);
	analogLayout->addWidget(hbox);
	label = new QLabel("Range:",hbox);
	label->setFixedWidth(60);
	analogRangeList = new QComboBox(hbox);
	analogReferenceList = new QComboBox(hbox);

	hbox = new QWidget(analogBox);
	//hbox->setSpacing(2);
	analogLayout->addWidget(hbox);
	label = new QLabel("Scale:",hbox);
	label->setFixedWidth(60);
	analogGainEdit = new QLineEdit(hbox);
	analogGainEdit->setAlignment(Qt::AlignRight);
	analogUnitPrefixList = new QComboBox(hbox);
	analogUnitPrefixList->setFixedWidth(70);
	analogUnitPrefixList->addItem("yotta-");
	analogUnitPrefixList->addItem("zetta-");
	analogUnitPrefixList->addItem("exa-");
	analogUnitPrefixList->addItem("peta-");
	analogUnitPrefixList->addItem("tera-");
	analogUnitPrefixList->addItem("giga-");
	analogUnitPrefixList->addItem("mega-");
	analogUnitPrefixList->addItem("kilo-");
	analogUnitPrefixList->addItem("");
	analogUnitPrefixList->addItem("milli-");
	analogUnitPrefixList->addItem("micro-");
	analogUnitPrefixList->addItem("nano-");
	analogUnitPrefixList->addItem("pico-");
	analogUnitPrefixList->addItem("femto-");
	analogUnitPrefixList->addItem("atto-");
	analogUnitPrefixList->addItem("zepto-");
	analogUnitPrefixList->addItem("yocto-");
	analogUnitList = new QComboBox(hbox);
	analogUnitList->setFixedWidth(70);
	new QLabel(" / Volt\n",hbox);

	hbox = new QWidget(analogBox);
	//hbox->setSpacing(2);
	analogLayout->addWidget(hbox);
	label = new QLabel("Offset:",hbox);
	label->setFixedWidth(60);
	analogZeroOffsetEdit = new QLineEdit(hbox);
	analogZeroOffsetEdit->setAlignment(Qt::AlignRight);
	analogUnitPrefixList2 = new QComboBox(hbox);
	analogUnitPrefixList2->setFixedWidth(70);
	analogUnitPrefixList2->addItem("yotta-");
	analogUnitPrefixList2->addItem("zetta-");
	analogUnitPrefixList2->addItem("exa-");
	analogUnitPrefixList2->addItem("peta-");
	analogUnitPrefixList2->addItem("tera-");
	analogUnitPrefixList2->addItem("giga-");
	analogUnitPrefixList2->addItem("mega-");
	analogUnitPrefixList2->addItem("kilo-");
	analogUnitPrefixList2->addItem("");
	analogUnitPrefixList2->addItem("milli-");
	analogUnitPrefixList2->addItem("micro-");
	analogUnitPrefixList2->addItem("nano-");
	analogUnitPrefixList2->addItem("pico-");
	analogUnitPrefixList2->addItem("femto-");
	analogUnitPrefixList2->addItem("atto-");
	analogUnitPrefixList2->addItem("zepto-");
	analogUnitPrefixList2->addItem("yocto-");
	new QLabel(" Volt/Amps\n",hbox);

	QGroupBox *digitalBox = new QGroupBox("Digital",channelTab);
	layout->addWidget(digitalBox);

	QBoxLayout *digitalLayout = new QVBoxLayout(digitalBox);
	digitalLayout->setMargin(15);

	hbox = new QWidget(digitalBox);
	//hbox->setSpacing(2);
	digitalLayout->addWidget(hbox);
	label = new QLabel("Channel:",hbox);
	label->setFixedWidth(60);
	digitalSubdeviceList = new QComboBox(hbox);
	digitalSubdeviceList->addItem("Input / Output");
	digitalSubdeviceList->addItem("Input");
	digitalSubdeviceList->addItem("Output");
	QObject::connect(digitalSubdeviceList,SIGNAL(activated(int)),this,SLOT(updateDevice(void)));
	digitalChannelList = new QComboBox(hbox);
	digitalChannelList->setFixedWidth(60);
	QObject::connect(digitalChannelList,SIGNAL(activated(int)),this,SLOT(display(void)));
	digitalActiveButton = new QPushButton("Active",hbox);
	digitalActiveButton->setCheckable(true);
	digitalActiveButton->setFixedWidth(60);

	hbox = new QWidget(digitalBox);
	//hbox->setSpacing(2);
	digitalLayout->addWidget(hbox);
	label = new QLabel("Direction:",hbox);
	label->setFixedWidth(60);
	digitalDirectionList = new QComboBox(hbox);
	digitalDirectionList->addItem("Input");
	digitalDirectionList->addItem("Output");

}

void SystemControlPanel::createThreadTab(void) {
	QBoxLayout *layout = new QVBoxLayout(threadTab);

	QGroupBox *rateBox = new QGroupBox("Sample Rate",threadTab);
	layout->addWidget(rateBox);

	QBoxLayout *rateLayout = new QVBoxLayout(rateBox);
	rateLayout->setMargin(15);

	QWidget *hbox0 = new QWidget(rateBox);
	rateLayout->addWidget(hbox0);
	QLabel *label4 = new QLabel("Frequency:",hbox0);
	label4->setFixedWidth(70);
	freqEdit = new QLineEdit(hbox0);
	freqEdit->setAlignment(Qt::AlignRight);
	QObject::connect(freqEdit,SIGNAL(textChanged(const QString &)),this,SLOT(updatePeriod(void)));
	freqUnitList = new QComboBox(hbox0);
	freqUnitList->setFixedWidth(50);
	freqUnitList->addItem(" Hz");
	freqUnitList->addItem("kHz");  
	QObject::connect(freqUnitList,SIGNAL(activated(int)),this,SLOT(updatePeriod(void)));

	QWidget *hbox1 = new QWidget(rateBox);
	rateLayout->addWidget(hbox1);
	QLabel *label5 = new QLabel("Period:",hbox1);
	label5->setFixedWidth(70);
	periodEdit = new QLineEdit(hbox1);
	periodEdit->setAlignment(Qt::AlignRight);
	QObject::connect(periodEdit,SIGNAL(textChanged(const QString &)),this,SLOT(updateFreq(void)));
	periodUnitList = new QComboBox(hbox1);
	periodUnitList->setFixedWidth(50);
	periodUnitList->addItem(" s");
	periodUnitList->addItem("ms");
	periodUnitList->addItem("us");
	periodUnitList->addItem("ns");
	QObject::connect(periodUnitList,SIGNAL(activated(int)),this,SLOT(updateFreq(void)));
}

void SystemControlPanel::displayChannelTab(void) {
	DAQ::Device *dev;
	{
		printf("coming from 15\n");
		struct find_daq_t info = {deviceList->currentIndex(), 0, };
		DAQ::Manager::getInstance()->foreachDevice(findDAQDevice,&info);
		dev = info.device;
	}

	if(!dev) {
		deviceList->setEnabled(false);
		analogSubdeviceList->setEnabled(false);
		digitalSubdeviceList->setEnabled(false);
	}

	if(!dev || !analogChannelList->count()) {
		analogActiveButton->setChecked(false);        
		analogActiveButton->setEnabled(false);
		analogActiveButton->setChecked(false);
		analogCalibrationButton->setEnabled(false);
		analogChannelList->setEnabled(false);
		analogRangeList->setEnabled(false);
		analogReferenceList->setEnabled(false);
		analogGainEdit->setEnabled(false);
		analogZeroOffsetEdit->setEnabled(false);
		analogUnitPrefixList->setEnabled(false);
		analogUnitPrefixList2->setEnabled(false);
		analogUnitList->setEnabled(false);        

	} else {
		printf("coming from 16\n");
		DAQ::type_t type = static_cast<DAQ::type_t>(analogSubdeviceList->currentIndex());
		DAQ::index_t chan = static_cast<DAQ::index_t>(analogChannelList->currentIndex());

		analogActiveButton->setEnabled(true);        
		analogRangeList->setEnabled(true);
		analogReferenceList->setEnabled(true);
		analogGainEdit->setEnabled(true);
		analogZeroOffsetEdit->setEnabled(true);
		analogUnitPrefixList->setEnabled(true);
		analogUnitPrefixList2->setEnabled(true);
		analogUnitList->setEnabled(true);        

		analogRangeList->clear();
		for(size_t i=0;i < dev->getAnalogRangeCount(type,chan);++i)
			analogRangeList->addItem(QString::fromStdString(dev->getAnalogRangeString(type,chan,i)));
		analogReferenceList->clear();
		for(size_t i=0;i < dev->getAnalogReferenceCount(type,chan);++i)
			analogReferenceList->addItem(QString::fromStdString(dev->getAnalogReferenceString(type,chan,i)));
		analogUnitList->clear();
		for(size_t i=0;i < dev->getAnalogUnitsCount(type,chan);++i) {
			analogUnitList->addItem(QString::fromStdString(dev->getAnalogUnitsString(type,chan,i)));
		}
		analogActiveButton->setChecked(dev->getChannelActive(type,chan));
		analogCalibrationButton->setEnabled(dev->getAnalogCalibrationState(type,chan));
		analogCalibrationButton->setChecked(dev->getAnalogCalibrationActive(type,chan));        
		analogRangeList->setCurrentIndex(dev->getAnalogRange(type,chan));
		analogReferenceList->setCurrentIndex(dev->getAnalogReference(type,chan));
		analogUnitList->setCurrentIndex(dev->getAnalogUnits(type,chan));

		// Determine the Correct Prefix for analog gain
		int i = 8;
		double tmp;
		bool sign = true;
		if( dev->getAnalogGain(type,chan) < 0.0 )
			sign = false; // Negative value
		tmp = fabs( dev->getAnalogGain(type,chan) );
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
		if( sign )
			analogGainEdit->setText(QString::number(tmp));
		else
			analogGainEdit->setText(QString::number(-tmp));
		analogUnitPrefixList->setCurrentIndex(i);

		// Determine the Correct Prefix for analog offset
		i = 8;
		sign = true;
		if( dev->getAnalogZeroOffset(type,chan) < 0.0 )
			sign = false; // Negative value
		tmp = fabs( dev->getAnalogZeroOffset(type,chan) );
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
		if( sign )
			analogZeroOffsetEdit->setText(QString::number(tmp));
		else
			analogZeroOffsetEdit->setText(QString::number(-tmp));
		analogUnitPrefixList2->setCurrentIndex(i);
	}

	if(!dev || !digitalChannelList->count()) {
		digitalActiveButton->setChecked(false);
		digitalActiveButton->setEnabled(false);
		digitalChannelList->setEnabled(false);
		digitalDirectionList->setEnabled(false);
	} else {
		printf("coming from 16\n");
		DAQ::type_t type = static_cast<DAQ::type_t>(digitalSubdeviceList->currentIndex()+DAQ::DIO);
		DAQ::index_t chan = static_cast<DAQ::index_t>(digitalChannelList->currentIndex());

		digitalActiveButton->setEnabled(true);
		digitalChannelList->setEnabled(true);
		if(type == DAQ::DIO)
			digitalDirectionList->setEnabled(true);
		else
			digitalDirectionList->setEnabled(false);

		digitalActiveButton->setChecked(dev->getChannelActive(type,chan));

		if(type == DAQ::DIO) {
			digitalDirectionList->setEnabled(true);
			digitalDirectionList->setCurrentIndex(dev->getDigitalDirection(chan));
		} else
			digitalDirectionList->setEnabled(false);
	}
}

void SystemControlPanel::displayThreadTab(void) {
	int i = 3;
	long long tmp = RT::System::getInstance()->getPeriod();
	while((tmp >= 1000)&&(i)) {
		tmp /= 1000;
		i--;
	}
	periodEdit->setText(QString::number(static_cast<unsigned long>(tmp)));
	periodUnitList->setCurrentIndex(i);

	updateFreq();
}

void SystemControlPanel::receiveEvent(const Event::Object *event) {
	if(event->getName() == Event::RT_POSTPERIOD_EVENT)
		displayThreadTab();
	if(event->getName() == Event::SETTINGS_OBJECT_INSERT_EVENT ||
			event->getName() == Event::SETTINGS_OBJECT_REMOVE_EVENT) {
		deviceList->clear();
		DAQ::Manager::getInstance()->foreachDevice(buildDAQDeviceList,deviceList);
		updateDevice();
	}
}
