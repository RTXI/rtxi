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


/* 
  This class creates and controls the drawing parameters
	A control panel is instantiated for all the active channels/modules
	and user selection is enabled to change color, style, width and other 
	oscilloscope properties.
 */

#include <QtGui>

#include <qwt_plot_renderer.h>

#include <debug.h>
#include <main_window.h>
#include <rt.h>
#include <workspace.h>
#include <cmath>
#include <sstream>

#include "oscilloscope.h"

namespace {
	class SyncEvent : public RT::Event {
		public:
			int callback(void) {
				return 0;
			};
	}; // SyncEvent

	struct channel_info {
		QString name;
		IO::Block *block;
		IO::flags_t type;
		size_t index;
		double previous; // stores previous value for trigger and downsample buffer
	}; // channel_info
} // namespace


////////// #Plugin
extern "C" Plugin::Object * createRTXIPlugin(void *) {
	return Oscilloscope::Plugin::getInstance();
}

// Create and insert scope into menu
Oscilloscope::Plugin::Plugin(void) {
	MainWindow::getInstance()->createSystemMenuItem("Oscilloscope",this,SLOT(createOscilloscopePanel(void)));
}

// Kill me
Oscilloscope::Plugin::~Plugin(void) {
	while (scopeList.size())
		delete scopeList.front();
	instance = 0;
}

// Create settings panel for oscilloscope(s)
void Oscilloscope::Plugin::createOscilloscopePanel(void) {
	Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
	scopeList.push_back(panel);
}

void Oscilloscope::Plugin::removeOscilloscopePanel(Oscilloscope::Panel *panel) {
	scopeList.remove(panel);
}

void Oscilloscope::Plugin::doDeferred(const Settings::Object::State &s) {
	size_t i = 0;
	for (std::list<Panel *>::iterator j = scopeList.begin(), end =
			scopeList.end(); j != end; ++j)
		(*j)->deferred(s.loadState(QString::number(i++).toStdString()));
}

void Oscilloscope::Plugin::doLoad(const Settings::Object::State &s) {
	for (size_t i = 0; i < static_cast<size_t> (s.loadInteger("Num Panels")); ++i) {
		Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
		scopeList.push_back(panel);
		panel->load(s.loadState(QString::number(i).toStdString()));
	}
}

void Oscilloscope::Plugin::doSave(Settings::Object::State &s) const {
	s.saveInteger("Num Panels", scopeList.size());
	size_t n = 0;
	for (std::list<Panel *>::const_iterator i = scopeList.begin(), end =
			scopeList.end(); i != end; ++i)
		s.saveState(QString::number(n++).toStdString(), (*i)->save());
}

////////// #Properties
/*Oscilloscope::Properties::Properties(Oscilloscope::Panel *parent) : QDialog(MainWindow::getInstance()), panel(parent) {

	// Create tab widget
	tabWidget = new QTabWidget;
	QGridLayout *layout = new QGridLayout;
	layout->addWidget(tabWidget);
	QObject::connect(tabWidget,SIGNAL(currentChanged(QWidget *)),this,SLOT(showTab(void)));

	// Create child layout for buttons
	buttonGroup = new QGroupBox;
	QHBoxLayout *buttonLayout = new QHBoxLayout;
	QPushButton *applyButton = new QPushButton("Apply");
	QObject::connect(applyButton,SIGNAL(clicked(void)),this,SLOT(apply(void)));
	buttonLayout->addWidget(applyButton);
	QPushButton *okayButton = new QPushButton("Okay");
	QObject::connect(okayButton,SIGNAL(clicked(void)),this,SLOT(okay(void)));
	buttonLayout->addWidget(okayButton);
	QPushButton *cancelButton = new QPushButton("Cancel");
	QObject::connect(cancelButton,SIGNAL(clicked(void)),this,SLOT(close(void)));
	buttonLayout->addWidget(cancelButton);

	// Setupt layout
	buttonGroup->setLayout(buttonLayout);

	// Create two tabs
	createChannelTab();
	createDisplayTab();

	// Set main widget properties
	layout->addWidget(buttonGroup);
	setLayout(layout);
	setWindowTitle(QString::number(parent->getID()) + " Oscilloscope Properties");
}

Oscilloscope::Properties::~Properties(void) {
}

void Oscilloscope::Properties::receiveEvent(const ::Event::Object *event) {
	if (event->getName() == Event::IO_BLOCK_INSERT_EVENT) {
		IO::Block *block =
			reinterpret_cast<IO::Block *> (event->getParam("block"));

		if (block) {
			// Update the list of blocks
			blockList->addItem(QString::fromStdString(block->getName())+" "+QString::number(block->getID()));
			panel->blocks.push_back(block);

			//if (blockList->count() == 1)
				//buildChannelList();
		}
	}
	else if (event->getName() == Event::IO_BLOCK_REMOVE_EVENT) {
		IO::Block *block =
			reinterpret_cast<IO::Block *> (event->getParam("block"));

		if (block) {
			// Find the index of the block in the blocks vector
			size_t index;
			for (index = 0; index < panel->blocks.size() && panel->blocks[index]
					!= block; ++index);

			if (index < panel->blocks.size()) {
				bool found = false;
				// Stop displaying channels coming from the removed block

				for (std::list<Scope::Channel>::iterator i =panel->getChannelsBegin(), end = panel->getChannelsEnd(); i != end;) {
					struct channel_info *info = reinterpret_cast<struct channel_info *> (i->getInfo());
					if (info->block == block)	{
						found = true;

						// If triggering on this channel disable triggering
						if (i->getLabel()	== panel->getTriggerChannel()->getLabel()){
							panel->setTrigger(Scope::NONE,
									panel->getTriggerThreshold(),
									panel->getChannelsEnd(),
									panel->getTriggerHolding(),
									panel->getTriggerHoldoff());
							//showDisplayTab();
						}

						struct channel_info
							*info = reinterpret_cast<struct channel_info *> (i->getInfo());

						std::list<Scope::Channel>::iterator chan = i++;

						bool active = panel->setInactiveSync();
						panel->removeChannel(chan);
						panel->flushFifo();
						panel->setActive(active);

						delete info;
					}
					else
						++i;
				}

				// Update the list of blocks
				size_t current = blockList->currentIndex();
				blockList->removeItem(index);
				panel->blocks.erase(panel->blocks.begin() + index);

				if (current == index) {
					blockList->setCurrentIndex(0);
					//buildChannelList();
				}

				showTab();
			}
			else
				DEBUG_MSG("Oscilloscope::Properties::receiveEvent : removed block never inserted\n");
		}
	}
	else if (event->getName() == Event::RT_POSTPERIOD_EVENT) {
		panel->setPeriod(RT::System::getInstance()->getPeriod() * 1e-6);
		panel->adjustDataSize();
		showTab();
	}
}

void Oscilloscope::Properties::closeEvent(QCloseEvent *e) {
	e->ignore();
	hide();
}

void Oscilloscope::Properties::activateChannel(bool active) {
	printf("activate clicked\n");
	bool enable = active && blockList->count() && channelList->count();
}

void Oscilloscope::Properties::apply(void) {
	switch (tabWidget->currentIndex()) {
		case 0:
			applyChannelTab();
			break;
		case 1:
			applyDisplayTab();
			break;
		default:
			ERROR_MSG("Oscilloscope::Properties::showTab : invalid tab\n");
	}
}

void Oscilloscope::Panel::buildChannelList(void) {

	channelsList->clear();
	if (!blocksList->count())
		return;

	if (blocksList->currentIndex() < 0)
		blocksList->setCurrentIndex(0);

	IO::Block *block = blocks[blocksList->currentIndex()];
	IO::flags_t type;
	switch (typesList->currentIndex()) {
		case 0:
			type = Workspace::OUTPUT;
			break;
		case 1:
			type = Workspace::INPUT;
			break;
		case 2:
			type = Workspace::PARAMETER;
			break;
		case 3:
			type = Workspace::STATE;
			break;
		default:
			ERROR_MSG("Oscilloscope::Properties::buildChannelList : invalid type selection\n");
			type = Workspace::INPUT;
	}

	for (size_t i = 0; i < block->getCount(type); ++i)
		channelsList->addItem(QString::fromStdString(block->getName(type, i)));

	//showChannelTab();
}

void Oscilloscope::Properties::okay(void) {
	apply();
	close();
}

void Oscilloscope::Properties::showTab(void) {
	switch (tabWidget->currentIndex()) {
		case 0:
			//showChannelTab();
			break;
		case 1:
			//showDisplayTab();
			break;
		default:
			ERROR_MSG("Oscilloscope::Properties::showTab : invalid tab\n");
	}
}

void Oscilloscope::Properties::applyChannelTab(void) {
	if (blockList->count() <= 0 || channelList->count() <= 0)
		return;

	IO::Block *block = panel->blocks[blockList->currentIndex()];
	IO::flags_t type;
	switch (typeList->currentIndex()) {
		case 0:
			type = Workspace::OUTPUT;
			break;
		case 1:
			type = Workspace::INPUT;
			break;
		case 2:
			type = Workspace::PARAMETER;
			break;
		case 3:
			type = Workspace::STATE;
			break;
		default:
			ERROR_MSG("Oscilloscope::Properties::applyChannelTab : invalid type\n");
			typeList->setCurrentIndex(0);
			type = Workspace::INPUT;
	}

	struct channel_info *info;
	std::list<Scope::Channel>::iterator i = panel->getChannelsBegin();
	for (std::list<Scope::Channel>::iterator end = panel->getChannelsEnd(); i
			!= end; ++i) {
		info = reinterpret_cast<struct channel_info *> (i->getInfo());
		if (info->block == block && info->type == type && info->index
				== static_cast<size_t> (channelList->currentIndex()))
			break;
	}

	if (!activateButton->isChecked()) {
		if (i != panel->getChannelsEnd()) {
			// If triggering on this channel disable triggering
			if (i->getLabel() == panel->getTriggerChannel()->getLabel())
				panel->setTrigger(Scope::NONE, panel->getTriggerThreshold(),
						panel->getChannelsEnd(), panel->getTriggerHolding(),
						panel->getTriggerHoldoff());

			bool active = panel->setInactiveSync();
			panel->removeChannel(i);
			panel->flushFifo();
			panel->setActive(active);

			delete info;
		}
	}
	else {
		if (i == panel->getChannelsEnd()) {
			info = new struct channel_info;

			info->block = block;
			info->type = type;
			info->index = channelList->currentIndex();
			info->previous = 0.0;

			info->name = QString::number(block->getID())+" "+QString::fromStdString(block->getName(type, channelList->currentIndex()));

			bool active = panel->setInactiveSync();

			i = panel->insertChannel(info->name + " 2 V/div", 2.0, 0.0, QPen(Qt::red, 1, Qt::SolidLine), info);

			panel->flushFifo();
			panel->setActive(active);
		}

		double scale;
		switch (scaleList->currentIndex() % 4) {
			case 0:
				scale = pow(10, 1 - scaleList->currentIndex() / 4);
				break;
			case 1:
				scale = 5 * pow(10, -scaleList->currentIndex() / 4);
				break;
			case 2:
				scale = 2.5 * pow(10, -scaleList->currentIndex() / 4);
				break;
			case 3:
				scale = 2 * pow(10, -scaleList->currentIndex() / 4);
				break;
			default:
				ERROR_MSG("Oscilloscope::Properties::applyChannelTab : invalid scale selection\n");
				scale = 2.0;
		}
		if (scale != i->getScale()) {
			panel->setChannelScale(i, scale);
			panel->setChannelLabel(i, info->name + " "
					+ scaleList->currentText().simplified());
		}
		panel->setChannelOffset(i, offsetEdit->text().toDouble() * pow(10, -3
					* offsetList->currentIndex()));

		QPen pen;
		switch (colorList->currentIndex()) {
			case 0:
				pen.setColor(Qt::red);
				break;
			case 1:
				pen.setColor(Qt::yellow);
				break;
			case 2:
				pen.setColor(Qt::green);
				break;
			case 3:
				pen.setColor(Qt::blue);
				break;
			case 4:
				pen.setColor(Qt::magenta);
				break;
			case 5:
				pen.setColor(Qt::cyan);
				break;
			case 6:
				pen.setColor(Qt::black);
				break;
			default:
				ERROR_MSG("Oscilloscope::Properties::applyChannelTab : invalid color selection\n");
				pen.setColor(Qt::red);
		}
		pen.setWidth(widthList->currentIndex() + 1);
		switch (styleList->currentIndex()) {
			case 0:
				pen.setStyle(Qt::SolidLine);
				break;
			case 1:
				pen.setStyle(Qt::DashLine);
				break;
			case 2:
				pen.setStyle(Qt::DotLine);
				break;
			case 3:
				pen.setStyle(Qt::DashDotLine);
				break;
			case 4:
				pen.setStyle(Qt::DashDotDotLine);
				break;
			default:
				ERROR_MSG("Oscilloscope::Properties::applyChannelTab : invalid style selection\n");
				pen.setStyle(Qt::SolidLine);
		}
		panel->setChannelPen(i, pen);

		//i->label.setColor(i->getPen().color());
		//for(std::vector<QCanvasLine>::iterator j = i->lines.begin(),end = i->lines.end();j != end;++j)
		//    j->setPen(info->pen);
*/
		/*
			 if(&*i == panel->trigChan)
			 panel->trigLine->setPoints(0,panel->val2pix(panel->trigThresh,*i),
			 width(),panel->val2pix(panel->trigThresh,*i));
		 */
	//}
	//showChannelTab();
//}
/*
void Oscilloscope::Properties::applyDisplayTab(void) {
	panel->setRefresh(refreshSpin->value());

	double divT;
	if (timeList->currentIndex() % 3 == 1)
		divT = 2 * pow(10, 3 - timeList->currentIndex() / 3);
	else if (timeList->currentIndex() % 3 == 2)
		divT = pow(10, 3 - timeList->currentIndex() / 3);
	else
		divT = 5 * pow(10, 3 - timeList->currentIndex() / 3);
	panel->setDivT(divT);
	panel->setPeriod(RT::System::getInstance()->getPeriod() * 1e-6);
	panel->adjustDataSize();

	Scope::trig_t trigDirection = static_cast<Scope::trig_t> (trigGroup->id(trigGroup->checkedButton()));
	double trigThreshold = trigThreshEdit->text().toDouble() * pow(10, -3 * trigThreshList->currentIndex());

	std::list<Scope::Channel>::iterator trigChannel = panel->getChannelsEnd();
	for (std::list<Scope::Channel>::iterator i = panel->getChannelsBegin(), end =
			panel->getChannelsEnd(); i != end; ++i)
		if (i->getLabel() == trigChanList->currentText()) {
			trigChannel = i;
			break;
		}
	if (trigChannel == panel->getChannelsEnd())
		trigDirection = Scope::NONE;

	bool trigHolding = trigHoldingCheck->isChecked();
	double trigHoldoff = trigHoldoffEdit->text().toDouble() * pow(10, -3 * trigHoldoffList->currentIndex());

	panel->setTrigger(trigDirection, trigThreshold, trigChannel, trigHolding, trigHoldoff);

	//panel->setDivXY(divXSpin->value(), divYSpin->value());
	panel->adjustDataSize();

	//showDisplayTab();
}

struct block_list_info_t {
	QComboBox *blockList;
	std::vector<IO::Block *> *blocks;
};

static void buildBlockList(IO::Block *block, void *arg) {
	block_list_info_t *info = static_cast<block_list_info_t *> (arg);
	info->blockList->addItem(QString::fromStdString(block->getName())+" "+QString::number(block->getID()));
	info->blocks->push_back(block);
}

void Oscilloscope::Properties::createChannelTab(void) {

	setWhatsThis("<p><b>Oscilloscope: Channel Options</b><br>"
			"Use the dropdown boxes to select the signal streams you want to plot from "
			"any loaded modules or your DAQ device. You may change the plotting scale for "
			"the signal, apply a DC offset, and change the color and style of the line.</p>");

	// Make parent widget and layout
	QWidget *channelTab = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout;

	// Create child widgets and layouts
	channelGroup = new QGroupBox(tr("Channel Selection"));
	QHBoxLayout *channelLayout = new QHBoxLayout;

	// Create elements for channel box
	channelLayout->addWidget(new QLabel(tr("Channel:")));
	blockList = new QComboBox;
	blockList->setFixedWidth(150);
	channelLayout->addWidget(blockList);
	block_list_info_t info = { blockList, &panel->blocks };
	IO::Connector::getInstance()->foreachBlock(::buildBlockList, &info);
	//QObject::connect(blockList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));

	typeList = new QComboBox;
	channelLayout->addWidget(typeList);
	typeList->setFixedWidth(150);
	typeList->addItem("Input");
	typeList->addItem("Output");
	typeList->addItem("Parameter");
	typeList->addItem("State");
	//QObject::connect(typeList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));

	channelList = new QComboBox;
	channelLayout->addWidget(channelList);
	channelList->setFixedWidth(150);
	//QObject::connect(channelList,SIGNAL(activated(int)),this,SLOT(showChannelTab(void)));

	// Activate button
	activateButton = new QPushButton("Active");
	channelLayout->addWidget(activateButton);
	activateButton->setCheckable(true);
	activateButton->setFixedWidth(55);
	QObject::connect(activateButton,SIGNAL(toggled(bool)),this,SLOT(activateChannel(bool)));

	channelGroup->setLayout(channelLayout);

	// Display child widget and layout
	displayGroup = new QGroupBox(tr("Display Properties"));
	QGridLayout *displayLayout = new QGridLayout;

	// Create elements for display box
	displayLayout->addWidget(new QLabel(tr("Scale:")), 0, 0);
	scaleList = new QComboBox;
	displayLayout->addWidget(scaleList, 0, 1, 1, 1);
	QFont scaleListFont("DejaVu Sans Mono");
	scaleList->setFont(scaleListFont);
	scaleList->addItem("10 V/div"); // 0  case 0
	scaleList->addItem("5 V/div"); // 1  case 1
	scaleList->addItem("2.5 V/div");	// 2  case 2
	scaleList->addItem("2 V/div"); // 3  case 3
	scaleList->addItem("1 V/div"); // 4  case 0
	scaleList->addItem("500 mV/div"); // 5  case 1
	scaleList->addItem("250 mV/div"); // 6  case 2
	scaleList->addItem("200 mV/div"); // 7  case 3
	scaleList->addItem("100 mV/div"); // 8  case 0
	scaleList->addItem("50 mV/div"); // 9  case 1
	scaleList->addItem("25 mV/div");
	scaleList->addItem("20 mV/div");
	scaleList->addItem("10 mV/div");
	scaleList->addItem("5 mV/div");
	scaleList->addItem("2.5 mV/div");
	scaleList->addItem("2 mV/div");
	scaleList->addItem("1 mV/div");
	QChar mu = QChar(0x3BC);
	QString suffix = QString("V/div");
	QString text = QString("500 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("250 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("200 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("100 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("50 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("25 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("20 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("10 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("5 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("2.5 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("2 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);
	text = QString("1 ");
	text.append(mu);
	text.append(suffix);
	scaleList->addItem(text);

	scaleList->addItem("500 nV/div");
	scaleList->addItem("250 nV/div");
	scaleList->addItem("200 nV/div");
	scaleList->addItem("100 nV/div");
	scaleList->addItem("50 nV/div");
	scaleList->addItem("25 nV/div");
	scaleList->addItem("20 nV/div");
	scaleList->addItem("10 nV/div");
	scaleList->addItem("5 nV/div");
	scaleList->addItem("2.5 nV/div");
	scaleList->addItem("2 nV/div");
	scaleList->addItem("1 nV/div");
	scaleList->addItem("500 pV/div");
	scaleList->addItem("250 pV/div");
	scaleList->addItem("200 pV/div");
	scaleList->addItem("100 pV/div");
	scaleList->addItem("50 pV/div");
	scaleList->addItem("25 pV/div");
	scaleList->addItem("20 pV/div");
	scaleList->addItem("10 pV/div");
	scaleList->addItem("5 pV/div");
	scaleList->addItem("2.5 pV/div");
	scaleList->addItem("2 pV/div");
	scaleList->addItem("1 pV/div");
	scaleList->addItem("500 fV/div");
	scaleList->addItem("250 fV/div");
	scaleList->addItem("200 fV/div");
	scaleList->addItem("100 fV/div");
	scaleList->addItem("50 fV/div");
	scaleList->addItem("25 fV/div");
	scaleList->addItem("20 fV/div");
	scaleList->addItem("10 fV/div");
	scaleList->addItem("5 fV/div");
	scaleList->addItem("2.5 fV/div");
	scaleList->addItem("2 fV/div");
	scaleList->addItem("1 fV/div");

	// Offset items
	displayLayout->addWidget(new QLabel(tr("Offset:")), 0, 2, 1, 1);
	offsetEdit = new QLineEdit;
	offsetEdit->setValidator(new QDoubleValidator(offsetEdit));
	displayLayout->addWidget(offsetEdit, 0, 3, 1, 1);
	offsetList = new QComboBox;
	displayLayout->addWidget(offsetList, 0, 4, 1, 1);
	offsetList->addItem("V");
	offsetList->addItem("mV");
	offsetList->addItem("uV");
	offsetList->addItem("nV");
	offsetList->addItem("pV");

	// Create elements for graphic
	displayLayout->addWidget(new QLabel(tr("   Color: ")), 1, 0, 1, 1);
	colorList = new QComboBox;
	colorList->setFixedWidth(80);
	displayLayout->addWidget(colorList, 1, 1, 1, 1);
	QPixmap tmp(25, 25);
	tmp.fill(Qt::red);
	colorList->addItem(tmp, " Red");
	tmp.fill(Qt::yellow);
	colorList->addItem(tmp, " Yellow");
	tmp.fill(Qt::green);
	colorList->addItem(tmp, " Green");
	tmp.fill(Qt::blue);
	colorList->addItem(tmp, " Blue");
	tmp.fill(Qt::magenta);
	colorList->addItem(tmp, " Magenta");
	tmp.fill(Qt::cyan);
	colorList->addItem(tmp, " Cyan");
	tmp.fill(Qt::black);
	colorList->addItem(tmp, " Black");

	displayLayout->addWidget(new QLabel(tr("   Width: ")), 1, 2, 1, 1);
	widthList = new QComboBox;
	displayLayout->addWidget(widthList, 1, 3, 1, 1);
	tmp.fill(Qt::white);
	QPainter painter(&tmp);
	for (int i = 1; i < 6; i++) {
		painter.setPen(QPen(Qt::black, i));
		painter.drawLine(0, 12, 25, 12);
		widthList->addItem(tmp, QString::number(i) + QString(" Pixels"));
	}

	displayLayout->addWidget(new QLabel(tr("   Style: ")), 1, 4, 1, 1);
	styleList = new QComboBox;
	displayLayout->addWidget(styleList, 1, 5, 1, 1);
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::SolidLine));
	painter.drawLine(0, 12, 25, 12);
	styleList->addItem(tmp, QString(" Solid"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DashLine));
	painter.drawLine(0, 12, 25, 12);
	styleList->addItem(tmp, QString(" Dash"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DotLine));
	painter.drawLine(0, 12, 25, 12);
	styleList->addItem(tmp, QString(" Dot"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DashDotLine));
	painter.drawLine(0, 12, 25, 12);
	styleList->addItem(tmp, QString(" Dash Dot"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DashDotDotLine));
	painter.drawLine(0, 12, 25, 12);
	styleList->addItem(tmp, QString(" Dash Dot Dot"));

	displayGroup->setLayout(displayLayout);

	layout->addWidget(channelGroup);
	layout->addWidget(displayGroup);
	channelTab->setLayout(layout);

	tabWidget->addTab(channelTab, tr("Channel"));
	//buildChannelList();
}

void Oscilloscope::Properties::createDisplayTab(void) {

	setWhatsThis("<p><b>Oscilloscope: Display Options</b><br>"
			"Use the dropdown box to select the time scale for the Oscilloscope. This "
			"scaling is applied to all signals plotted in the same window. You may also "
			"set a trigger on any signal that is currently plotted in the window. A yellow "
			"line will appear at the trigger threshold.</p>");

	// Parent widget and layout
	QWidget *displayTab = new QWidget;
	QVBoxLayout *layout = new QVBoxLayout(displayTab);

	// Create child elements
	timeGroup = new QGroupBox(tr("Time Properties"));
	QGridLayout *timeLayout = new QGridLayout;

	// Create elements for child
	QChar mu = QChar(0x3BC);
	timeLayout->addWidget(new QLabel("Time Scale:"), 0, 0, 1, 1);
	timeList = new QComboBox;
	timeLayout->addWidget(timeList, 0, 1, 1, 1);
	QFont timeListFont("DejaVu Sans Mono");
	timeList->setFont(timeListFont);
	timeList->addItem("5 s/div");
	timeList->addItem("2 s/div");
	timeList->addItem("1 s/div");
	timeList->addItem("500 ms/div");
	timeList->addItem("200 ms/div");
	timeList->addItem("100 ms/div");
	timeList->addItem("50 ms/div");
	timeList->addItem("20 ms/div");
	timeList->addItem("10 ms/div");
	timeList->addItem("5 ms/div");
	timeList->addItem("2 ms/div");
	timeList->addItem("1 ms/div");
	QString suffix = QString("s/div");
	QString text = QString("500 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("200 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("100 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("50 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("20 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("10 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("5 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("2 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);
	text = QString("1 ");
	text.append(mu);
	text.append(suffix);
	timeList->addItem(text);

	timeLayout->addWidget(new QLabel("Screen Refresh:"), 0, 2, 1, 1);
	refreshSpin = new QSpinBox;
	refreshSpin->setFixedWidth(70);
	timeLayout->addWidget(refreshSpin, 0, 3, 1, 1);
	refreshSpin->setRange(10,10000);
	refreshSpin->setValue(250);

	timeLayout->addWidget(new QLabel("  X Divisions: "), 1, 0, 1, 1);
	divXSpin = new QSpinBox;
	divXSpin->setFixedWidth(70);
	timeLayout->addWidget(divXSpin, 1, 1, 1, 1);
	divXSpin->setRange(1,25);
	divXSpin->setValue(8);

	timeLayout->addWidget(new QLabel("  Y Divisions: "), 1, 2, 1, 1);
	divYSpin = new QSpinBox;
	divYSpin->setFixedWidth(70);
	timeLayout->addWidget(divYSpin, 1, 3, 1, 1);
	divYSpin->setRange(1,25);
	divYSpin->setValue(10);

	timeLayout->addWidget(new QLabel("Downsampling Rate: "), 2, 0, 1, 1);
	rateSpin = new QSpinBox;
	rateSpin->setFixedWidth(70);
	timeLayout->addWidget(rateSpin, 2, 1, 1, 1);
	rateSpin->setValue(panel->downsample_rate);
	QObject::connect(rateSpin,SIGNAL(valueChanged(int)),this,SLOT(updateDownsampleRate(int)));
	rateSpin->setEnabled(true);
	rateSpin->setRange(1,2);
	rateSpin->setValue(1);

	timeLayout->addWidget(new QLabel(tr("Data Buffer Size: ")), 2, 2, 1, 1);
	sizeEdit = new QLineEdit;
	sizeEdit->setFixedWidth(70);
	timeLayout->addWidget(sizeEdit, 2, 3, 1, 1);
	sizeEdit->setText(QString::number(panel->getDataSize()));
	sizeEdit->setEnabled(false);

	timeGroup->setLayout(timeLayout);

	// Trigger box
	triggerGroup = new QGroupBox(tr("Trigger Properties"));
	QGridLayout *triggerLayout = new QGridLayout;

	triggerLayout->addWidget(new QLabel(tr("Trigger:")), 0, 0, 1, 1);
	trigGroup = new QButtonGroup;

	QRadioButton *off = new QRadioButton(tr("Off"));
	trigGroup->addButton(off, Scope::NONE);
	triggerLayout->addWidget(off, 0, 1, 1, 1);
	QRadioButton *plus = new QRadioButton(tr("+"));
	trigGroup->addButton(plus, Scope::POS);
	triggerLayout->addWidget(plus, 0, 2, 1, 1);
	QRadioButton *minus = new QRadioButton(tr("-"));
	trigGroup->addButton(minus, Scope::NEG);
	triggerLayout->addWidget(minus, 0, 3, 1, 1);

	triggerLayout->addWidget(new QLabel(tr("Channel:")), 1, 0, 1, 1);
	trigChanList = new QComboBox;
	triggerLayout->addWidget(trigChanList, 1, 1, 1, 1);

	triggerLayout->addWidget(new QLabel(tr("Threshold:")), 1, 2, 1, 1);
	trigThreshEdit = new QLineEdit;
	trigThreshEdit->setFixedWidth(50);
	triggerLayout->addWidget(trigThreshEdit, 1, 3, 1, 1);
	trigThreshEdit->setValidator(new QDoubleValidator(trigThreshEdit));
	trigThreshList = new QComboBox;
	triggerLayout->addWidget(trigThreshList, 1, 4, 1, 1);
	trigThreshList->addItem("V");
	trigThreshList->addItem("mV");
	trigThreshList->addItem("uV");
	trigThreshList->addItem("nV");
	trigThreshList->addItem("pV");

	triggerLayout->addWidget(new QLabel(tr("Holding:")), 2, 0, 1, 1);
	trigHoldingCheck = new QCheckBox;
	triggerLayout->addWidget(trigHoldingCheck, 2, 1, 1, 1);

	triggerLayout->addWidget(new QLabel(tr("Holdoff:")), 2, 2, 1, 1);
	trigHoldoffEdit = new QLineEdit;
	trigHoldoffEdit->setFixedWidth(50);
	triggerLayout->addWidget(trigHoldoffEdit, 2, 3, 1, 1);
	trigHoldoffEdit->setValidator(new QDoubleValidator(trigHoldoffEdit));
	trigHoldoffList = new QComboBox;
	triggerLayout->addWidget(trigHoldoffList, 2, 4, 1, 1);
	trigHoldoffList->addItem("ms");
	trigHoldoffList->addItem("us");
	trigHoldoffList->addItem("ns");

	triggerGroup->setLayout(triggerLayout);

	layout->addWidget(timeGroup);
	layout->addWidget(triggerGroup);

	displayTab->setLayout(layout);
	tabWidget->addTab(displayTab, tr("Display"));
}

// Aggregates all channel information to show for configuration
void Oscilloscope::Panel::showChannelTab(void) {

	IO::flags_t type;
	switch (typesList->currentIndex()) {
		case 0:
			type = Workspace::OUTPUT;
			break;
		case 1:
			type = Workspace::OUTPUT;
			break;
		case 2:
			type = Workspace::PARAMETER;
			break;
		case 3:
			type = Workspace::STATE;
			break;
		default:
			ERROR_MSG("Oscilloscope::Properties::showChannelTab : invalid type\n");
			typesList->setCurrentIndex(0);
			type = Workspace::OUTPUT;
	}

	bool found = false;

	for (std::list<Scope::Channel>::iterator i = Scope::getChannelsBegin(), end = Scope::getChannelsEnd(); i != end; ++i) {
		struct channel_info *info =
			reinterpret_cast<struct channel_info *> (i->getInfo());
		if (!info)
			continue;
		if (info->block && info->block == blocks[blocksList->currentIndex()]
				&& info->type == type && info->index == static_cast<size_t> (channelsList->currentIndex())) {
			found = true;

			scalesList->setCurrentIndex(static_cast<int> (round(4 * (log10(1/i->getScale()) + 1))));

			double offset = i->getOffset();
			int offsetUnits = 0;
			if (offset)
				while (fabs(offset) < 1) {
					offset *= 1000;
					offsetUnits++;
				}
			offsetsEdit->setText(QString::number(offset));
			offsetsList->setCurrentIndex(offsetUnits);

			if (i->getPen().color() == Qt::red)
				colorsList->setCurrentIndex(0);
			else if (i->getPen().color() == Qt::yellow)
				colorsList->setCurrentIndex(1);
			else if (i->getPen().color() == Qt::green)
				colorsList->setCurrentIndex(2);
			else if (i->getPen().color() == Qt::blue)
				colorsList->setCurrentIndex(3);
			else if (i->getPen().color() == Qt::magenta)
				colorsList->setCurrentIndex(4);
			else if (i->getPen().color() == Qt::cyan)
				colorsList->setCurrentIndex(5);
			else if (i->getPen().color() == Qt::black)
				colorsList->setCurrentIndex(6);
			else {
				ERROR_MSG("Oscilloscope::Properties::displayChannelTab : invalid color selection\n");
				colorsList->setCurrentIndex(0);
			}

			switch (i->getPen().style()) {
				case Qt::SolidLine:
					stylesList->setCurrentIndex(0);
					break;
				case Qt::DashLine:
					stylesList->setCurrentIndex(1);
					break;
				case Qt::DotLine:
					stylesList->setCurrentIndex(2);
					break;
				case Qt::DashDotLine:
					stylesList->setCurrentIndex(3);
					break;
				case Qt::DashDotDotLine:
					stylesList->setCurrentIndex(4);
					break;
				default:
					ERROR_MSG("Oscilloscope::Properties::displayChannelTab : invalid style selection\n");
					stylesList->setCurrentIndex(0);
			}
			break;
		}
	}

	activateButton->setCheckable(found);
	if (!found) {
		scalesList->setCurrentIndex(3);
		offsetsEdit->setText(QString::number(0));
		offsetsList->setCurrentIndex(0);
		colorsList->setCurrentIndex(0);
		widthsList->setCurrentIndex(0);
		stylesList->setCurrentIndex(0);
	}
}

void Oscilloscope::Panel::showDisplayTab(void) {
	printf("in here now\n");
	timesList->setCurrentIndex(static_cast<int> (round(3 * log10(1/Scope::getDivT()) + 11)));
	refreshSpin->setValue(Scope::getRefresh());

	// Find current trigger value and update gui
	static_cast<QRadioButton *>(trigsGroup->button(static_cast<int>(getTriggerDirection())))->setChecked(true);

	trigsChanList->clear();
	for (std::list<Scope::Channel>::iterator i = Scope::getChannelsBegin(), end =	Scope::getChannelsEnd(); i != end; ++i) {
		trigsChanList->addItem(i->getLabel());
		if (i == Scope::getTriggerChannel())
			trigsChanList->setCurrentIndex(trigsChanList->count() - 1);
	}
	trigsChanList->addItem("<None>");
	if (Scope::getTriggerChannel() == Scope::getChannelsEnd())
		trigsChanList->setCurrentIndex(trigsChanList->count() - 1);

	int trigThreshUnits = 0;
	double trigThresh = Scope::getTriggerThreshold();
	if (trigThresh != 0.0)
		while (fabs(trigThresh) < 1) {
			trigThresh *= 1000;
			++trigThreshUnits;
		}
	trigsThreshList->setCurrentIndex(trigThreshUnits);
	trigsThreshEdit->setText(QString::number(trigThresh));
	trigsHoldingCheck->setChecked(Scope::getTriggerHolding());
	int trigHoldoffUnits = 0;
	double trigHoldoff = Scope::getTriggerHoldoff();
	if (trigHoldoff != 0.0)
		while (fabs(trigHoldoff) < 1)	{
			trigHoldoff *= 1000;
			++trigHoldoffUnits;
		}
	trigsHoldoffList->setCurrentIndex(trigHoldoffUnits);
	trigsHoldoffEdit->setText(QString::number(trigHoldoff));

	//rateSpin->setValue(panel->rate);
	sizeEdit->setText(QString::number(Scope::getDataSize()));

	divXSpin->setValue(Scope::getDivX());
	divYSpin->setValue(Scope::getDivY());
}

void Oscilloscope::Properties::updateDownsampleRate(int r) {
	downsample_rate = r;
	panel->updateDownsampleRate(downsample_rate);
}
*/
////////// #Panel
Oscilloscope::Panel::Panel(QWidget *parent) : Scope(parent),	RT::Thread(0), fifo(10 * 1048576) {

	// Make Mdi
	subWindow = new QMdiSubWindow;
	subWindow->setMinimumSize(800,700);
	subWindow->setAttribute(Qt::WA_DeleteOnClose);
	MainWindow::getInstance()->createMdi(subWindow);

	setWhatsThis("<p><b>Oscilloscope:</b><br>The Oscilloscope allows you to plot any signal "
			"in your workspace in real-time, including signals from your DAQ card and those "
			"generated by user modules. Multiple signals are overlaid in the window and "
			"different line colors and styles can be selected. When a signal is added, a legend "
			"automatically appears in the bottom of the window. Multiple oscilloscopes can "
			"be instantiated to give you multiple data windows. To select signals for plotting, "
			"use the right-click context \"Properties\" menu item. After selecting a signal, you must "
			"click the \"Active\" button for it to appear in the window. To change signal settings, "
			"you must click the \"Apply\" button. The right-click context \"Pause\" menu item "
			"allows you to start and stop real-time plotting.</p>");

	adjustDataSize();
	//properties = new Properties(this);

	layout = new QGridLayout;

	// Create scope group
	scopeGroup = new QGroupBox(this);
	QHBoxLayout *scopeLayout = new QHBoxLayout(this);

	// Create scope
	scopeWindow = new Scope(this);

	// Attach scope to layout
	scopeLayout->addWidget(scopeWindow);

	// Create group and layout for buttons at bottom of scope
	bttnGroup = new QGroupBox(tr("Channel Settings"));
	QGridLayout *bttnLayout = new QGridLayout(this);

	// Create Channel box
	QLabel *channelLabel = new QLabel(tr("Channel:"));
	channelLabel->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
	bttnLayout->addWidget(channelLabel, 0, 0);
	blocksList = new QComboBox;
	block_list_info_t info = {blocksList, &this->blocks};
	IO::Connector::getInstance()->foreachBlock(::buildBlockList, &info);
	QObject::connect(blocksList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));
	bttnLayout->addWidget(blocksList, 0, 1, 1, 2);

	// Create Type box
	typesList = new QComboBox;
	typesList->addItem("Input");
	typesList->addItem("Output");
	typesList->addItem("Parameter");
	typesList->addItem("State");
	QObject::connect(typesList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));
	bttnLayout->addWidget(typesList, 0, 3, 1, 2);

	// Create Channels box
	channelsList = new QComboBox;
	QObject::connect(channelsList,SIGNAL(activated(int)),this,SLOT(showChannelTab(void)));
	bttnLayout->addWidget(channelsList, 0, 5, 1, 3);

	// Create elements for display box
	QLabel *scaleLabel = new QLabel(tr("Scale:"));
	scaleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignRight);
	bttnLayout->addWidget(scaleLabel, 0, 8, 1, 1);
	scalesList = new QComboBox;
	bttnLayout->addWidget(scalesList, 0, 9, 1, 1);
	QFont scalesListFont("DejaVu Sans Mono");
	scalesList->setFont(scalesListFont);
	scalesList->addItem("10 V/div"); // 0  case 0
	scalesList->addItem("5 V/div"); // 1  case 1
	scalesList->addItem("2.5 V/div");	// 2  case 2
	scalesList->addItem("2 V/div"); // 3  case 3
	scalesList->addItem("1 V/div"); // 4  case 0
	scalesList->addItem("500 mV/div"); // 5  case 1
	scalesList->addItem("250 mV/div"); // 6  case 2
	scalesList->addItem("200 mV/div"); // 7  case 3
	scalesList->addItem("100 mV/div"); // 8  case 0
	scalesList->addItem("50 mV/div"); // 9  case 1
	scalesList->addItem("25 mV/div");
	scalesList->addItem("20 mV/div");
	scalesList->addItem("10 mV/div");
	scalesList->addItem("5 mV/div");
	scalesList->addItem("2.5 mV/div");
	scalesList->addItem("2 mV/div");
	scalesList->addItem("1 mV/div");
	QChar mu = QChar(0x3BC);
	QString suffix = QString("V/div");
	QString text = QString("500 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("250 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("200 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("100 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("50 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("25 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("20 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("10 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("5 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("2.5 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("2 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);
	text = QString("1 ");
	text.append(mu);
	text.append(suffix);
	scalesList->addItem(text);

	scalesList->addItem("500 nV/div");
	scalesList->addItem("250 nV/div");
	scalesList->addItem("200 nV/div");
	scalesList->addItem("100 nV/div");
	scalesList->addItem("50 nV/div");
	scalesList->addItem("25 nV/div");
	scalesList->addItem("20 nV/div");
	scalesList->addItem("10 nV/div");
	scalesList->addItem("5 nV/div");
	scalesList->addItem("2.5 nV/div");
	scalesList->addItem("2 nV/div");
	scalesList->addItem("1 nV/div");
	scalesList->addItem("500 pV/div");
	scalesList->addItem("250 pV/div");
	scalesList->addItem("200 pV/div");
	scalesList->addItem("100 pV/div");
	scalesList->addItem("50 pV/div");
	scalesList->addItem("25 pV/div");
	scalesList->addItem("20 pV/div");
	scalesList->addItem("10 pV/div");
	scalesList->addItem("5 pV/div");
	scalesList->addItem("2.5 pV/div");
	scalesList->addItem("2 pV/div");
	scalesList->addItem("1 pV/div");
	scalesList->addItem("500 fV/div");
	scalesList->addItem("250 fV/div");
	scalesList->addItem("200 fV/div");
	scalesList->addItem("100 fV/div");
	scalesList->addItem("50 fV/div");
	scalesList->addItem("25 fV/div");
	scalesList->addItem("20 fV/div");
	scalesList->addItem("10 fV/div");
	scalesList->addItem("5 fV/div");
	scalesList->addItem("2.5 fV/div");
	scalesList->addItem("2 fV/div");
	scalesList->addItem("1 fV/div");

	// Offset items
	QLabel *offsetLabel = new QLabel(tr("Offset:"));
	offsetLabel->setAlignment(Qt::AlignCenter | Qt::AlignRight);
	bttnLayout->addWidget(offsetLabel, 0, 10, 1, 1);
	offsetsEdit = new QLineEdit;
	offsetsEdit->setValidator(new QDoubleValidator(offsetsEdit));
	bttnLayout->addWidget(offsetsEdit, 0, 11, 1, 1);
	offsetsList = new QComboBox;
	offsetsList->setFixedWidth(40);
	bttnLayout->addWidget(offsetsList, 0, 12, 1, 1);
	offsetsList->addItem("V");
	offsetsList->addItem("mV");
	offsetsList->addItem("uV");
	offsetsList->addItem("nV");
	offsetsList->addItem("pV");

	// Activate button
	activateButton = new QPushButton("Enable Channel");
	activateButton->setCheckable(true);
	QObject::connect(activateButton,SIGNAL(toggled(bool)),this,SLOT(activateChannel(bool)));
	bttnLayout->addWidget(activateButton, 1, 9, 1, 4);

	// Create elements for graphic
	bttnLayout->addWidget(new QLabel(tr("Color:")), 1, 0, Qt::AlignVCenter| Qt::AlignRight);
	colorsList = new QComboBox;
	bttnLayout->addWidget(colorsList, 1, 1, 1, 2);
	QPixmap tmp(25, 25);
	tmp.fill(Qt::red);
	colorsList->addItem(tmp, " Red");
	tmp.fill(Qt::yellow);
	colorsList->addItem(tmp, " Yellow");
	tmp.fill(Qt::green);
	colorsList->addItem(tmp, " Green");
	tmp.fill(Qt::blue);
	colorsList->addItem(tmp, " Blue");
	tmp.fill(Qt::magenta);
	colorsList->addItem(tmp, " Magenta");
	tmp.fill(Qt::cyan);
	colorsList->addItem(tmp, " Cyan");
	tmp.fill(Qt::black);
	colorsList->addItem(tmp, " Black");

	QLabel *widthLabel = new QLabel(tr("Width:"));
	widthLabel->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
	bttnLayout->addWidget(widthLabel, 1, 3);
	widthsList = new QComboBox;
	bttnLayout->addWidget(widthsList, 1, 4, 1, 2);
	tmp.fill(Qt::white);
	QPainter painter(&tmp);
	for (int i = 1; i < 6; i++) {
		painter.setPen(QPen(Qt::black, i));
		painter.drawLine(0, 12, 25, 12);
		widthsList->addItem(tmp, QString::number(i) + QString(" Pixels"));
	}

	// Create styles list
	QLabel *styleLabel = new QLabel(tr("Style:"));
	bttnLayout->addWidget(styleLabel, 1, 6);
	styleLabel->setAlignment(Qt::AlignCenter | Qt::AlignLeft);
	stylesList = new QComboBox;
	bttnLayout->addWidget(stylesList, 1, 7, 1, 2);
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::SolidLine));
	painter.drawLine(0, 12, 25, 12);
	stylesList->addItem(tmp, QString(" Solid"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DashLine));
	painter.drawLine(0, 12, 25, 12);
	stylesList->addItem(tmp, QString(" Dash"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DotLine));
	painter.drawLine(0, 12, 25, 12);
	stylesList->addItem(tmp, QString(" Dot"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DashDotLine));
	painter.drawLine(0, 12, 25, 12);
	stylesList->addItem(tmp, QString(" Dash Dot"));
	tmp.fill(Qt::white);
	painter.setPen(QPen(Qt::black, 3, Qt::DashDotDotLine));
	painter.drawLine(0, 12, 25, 12);
	stylesList->addItem(tmp, QString(" Dash Dot Dot"));

	// Scope properties 
	scopeBttnGroup = new QGroupBox(tr("Scope Configuration"));
	QGridLayout *scopeBttnsLayout = new QGridLayout(this);

	// Create elements for time settings
	scopeBttnsLayout->addWidget(new QLabel("Time/Div:"), 2, 0);
	timesList = new QComboBox;
	scopeBttnsLayout->addWidget(timesList, 2, 1, 1, 1);
	QFont timeListFont("DejaVu Sans Mono");
	timesList->setFont(timeListFont);
	timesList->addItem("5 s/div");
	timesList->addItem("2 s/div");
	timesList->addItem("1 s/div");
	timesList->addItem("500 ms/div");
	timesList->addItem("200 ms/div");
	timesList->addItem("100 ms/div");
	timesList->addItem("50 ms/div");
	timesList->addItem("20 ms/div");
	timesList->addItem("10 ms/div");
	timesList->addItem("5 ms/div");
	timesList->addItem("2 ms/div");
	timesList->addItem("1 ms/div");
	QString tsuffix = QString("s/div");
	text = QString("500 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("200 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("100 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("50 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("20 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("10 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("5 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("2 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);
	text = QString("1 ");
	text.append(mu);
	text.append(tsuffix);
	timesList->addItem(text);

	QLabel *refreshLabel = new QLabel(tr("Refresh\nRate:"));
	refreshLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	scopeBttnsLayout->addWidget(refreshLabel, 2, 6);
	refreshsSpin = new QSpinBox;
	scopeBttnsLayout->addWidget(refreshsSpin, 2, 7);
	refreshsSpin->setRange(10,10000);
	refreshsSpin->setValue(250);

	scopeBttnsLayout->addWidget(new QLabel("X Divs:"), 2, 2);
	divsXSpin = new QSpinBox;
	scopeBttnsLayout->addWidget(divsXSpin, 2, 3);
	divsXSpin->setRange(1,25);
	divsXSpin->setValue(8);

	scopeBttnsLayout->addWidget(new QLabel("Y Divs:"), 2, 4);
	divsYSpin = new QSpinBox;
	scopeBttnsLayout->addWidget(divsYSpin, 2, 5);
	divsYSpin->setRange(1,25);
	divsYSpin->setValue(10);

	QLabel *downsampleLabel = new QLabel(tr("Downsample\nRate:"));
	downsampleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	scopeBttnsLayout->addWidget(downsampleLabel, 2, 8);
	ratesSpin = new QSpinBox;
	ratesSpin->setFixedWidth(40);
	scopeBttnsLayout->addWidget(ratesSpin, 2, 9);
	ratesSpin->setValue(downsample_rate);
	QObject::connect(ratesSpin,SIGNAL(valueChanged(int)),this,SLOT(updateDownsampleRate(int)));
	ratesSpin->setEnabled(true);
	ratesSpin->setRange(1,2);
	ratesSpin->setValue(1);

	QLabel *bufferLabel = new QLabel(tr("Data Buffer\nSize (MB):"));
	bufferLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
	scopeBttnsLayout->addWidget(bufferLabel, 2, 10);
	sizesEdit = new QLineEdit;
	scopeBttnsLayout->addWidget(sizesEdit, 2, 11);
	sizesEdit->setText(QString::number(getDataSize()));
	sizesEdit->setEnabled(false);

	// Trigger properties 
	triggerBttnGroup = new QGroupBox(tr("Trigger Settings"));
	QGridLayout *triggerBttnsLayout = new QGridLayout(this);

	// Trigger box
	triggerBttnsLayout->addWidget(new QLabel(tr("Trigger:")), 0, 0);
	trigsGroup = new QButtonGroup;

	QRadioButton *off = new QRadioButton(tr("Off"));
	trigsGroup->addButton(off, Scope::NONE);
	triggerBttnsLayout->addWidget(off, 0, 1);
	QRadioButton *plus = new QRadioButton(tr("+"));
	trigsGroup->addButton(plus, Scope::POS);
	triggerBttnsLayout->addWidget(plus, 0, 2);
	QRadioButton *minus = new QRadioButton(tr("-"));
	trigsGroup->addButton(minus, Scope::NEG);
	triggerBttnsLayout->addWidget(minus, 0, 3);

	triggerBttnsLayout->addWidget(new QLabel(tr("Channel:")), 0, 4);
	trigsChanList = new QComboBox;
	triggerBttnsLayout->addWidget(trigsChanList, 0, 5, 1, 2);

	triggerBttnsLayout->addWidget(new QLabel(tr("Threshold:")), 0, 7);
	trigsThreshEdit = new QLineEdit;
	trigsThreshEdit->setFixedWidth(50);
	triggerBttnsLayout->addWidget(trigsThreshEdit, 0, 8);
	trigsThreshEdit->setValidator(new QDoubleValidator(trigsThreshEdit));
	trigsThreshList = new QComboBox;
	triggerBttnsLayout->addWidget(trigsThreshList, 0, 9);
	trigsThreshList->addItem("V");
	trigsThreshList->addItem("mV");
	trigsThreshList->addItem("uV");
	trigsThreshList->addItem("nV");
	trigsThreshList->addItem("pV");

	triggerBttnsLayout->addWidget(new QLabel(tr("Holding:")), 0, 10);
	trigsHoldingCheck = new QCheckBox;
	triggerBttnsLayout->addWidget(trigsHoldingCheck, 0, 11);

	triggerBttnsLayout->addWidget(new QLabel(tr("Holdoff:")), 0, 12);
	trigsHoldoffEdit = new QLineEdit;
	trigsHoldoffEdit->setFixedWidth(50);
	triggerBttnsLayout->addWidget(trigsHoldoffEdit, 0, 13);
	trigsHoldoffEdit->setValidator(new QDoubleValidator(trigsHoldoffEdit));
	trigsHoldoffList = new QComboBox;
	triggerBttnsLayout->addWidget(trigsHoldoffList, 0, 14) ;
	trigsHoldoffList->addItem("ms");
	trigsHoldoffList->addItem("us");
	trigsHoldoffList->addItem("ns");

	// Set buttons items
	setBttnGroup = new QGroupBox(this);
	QGridLayout *setBttnsLayout = new QGridLayout(this);

	// Create buttons
	pauseButton = new QPushButton("Pause");
	pauseButton->setCheckable(true);
	QObject::connect(pauseButton,SIGNAL(clicked()),this,SLOT(togglePause()));
	setBttnsLayout->addWidget(pauseButton, 2, 11);
	applyButton = new QPushButton("Apply");
	QObject::connect(applyButton,SIGNAL(clicked(void)),this,SLOT(apply(void)));
	setBttnsLayout->addWidget(applyButton, 2, 12);
	settingsButton = new QPushButton("Screenshot");
	QObject::connect(settingsButton,SIGNAL(clicked()),this,SLOT(screenshot()));
	setBttnsLayout->addWidget(settingsButton, 2, 10);

	// Attach to layout
	scopeGroup->setLayout(scopeLayout);
	bttnGroup->setLayout(bttnLayout);
	scopeBttnGroup->setLayout(scopeBttnsLayout);
	triggerBttnGroup->setLayout(triggerBttnsLayout);
	setBttnGroup->setLayout(setBttnsLayout);
	
	// Setup main layout
	layout->addWidget(scopeGroup, 0, 0, 9, 16);
	layout->addWidget(bttnGroup, 9, 0, 2, 16);
	layout->addWidget(scopeBttnGroup, 11, 0, 1, 16);
	layout->addWidget(triggerBttnGroup, 13, 0, 1, 16);
	layout->addWidget(setBttnGroup, 14, 0, 1, 16);

	// Set
	setLayout(layout);

	// Show stuff
	buildChannelList();
	//showDisplayTab();
	subWindow->setWidget(this);
	show();

	// Initialize vars
	counter = 0;
	downsample_rate = 1;
	setActive(true);
	setWindowTitle(QString::number(getID()) + " Oscilloscope");

	QTimer *otimer = new QTimer;
	QObject::connect(otimer,SIGNAL(timeout(void)),this,SLOT(timeoutEvent(void)));
	otimer->start(25);
}

Oscilloscope::Panel::~Panel(void) {
	while (getChannelsBegin() != getChannelsEnd())
		delete reinterpret_cast<struct channel_info *> (removeChannel(getChannelsBegin()));

	Plugin::getInstance()->removeOscilloscopePanel(this);
	delete properties;
}

void Oscilloscope::Panel::updateDownsampleRate(int r) {
	downsample_rate = r;
}

void Oscilloscope::Panel::execute(void) {
	size_t nchans = getChannelCount();

	if (nchans) {
		size_t idx = 0;
		size_t token = nchans;
		double data[nchans];

		if (!counter++) {
			for (std::list<Scope::Channel>::iterator i = getChannelsBegin(), end =
					getChannelsEnd(); i != end; ++i) {
				struct channel_info *info =
					reinterpret_cast<struct channel_info *> (i->getInfo());

				double value = info->block->getValue(info->type, info->index);

				if (i == getTriggerChannel()) {
					double thresholdValue = getTriggerThreshold();

					if ((thresholdValue > value && thresholdValue
								< info->previous) || (thresholdValue < value
									&& thresholdValue > info->previous)) {
						Event::Object event(Event::THRESHOLD_CROSSING_EVENT);
						int direction = (thresholdValue > value) ? 1 : -1;

						event.setParam("block", info->block);
						event.setParam("type", &info->type);
						event.setParam("index", &info->index);
						event.setParam("direction", &direction);
						event.setParam("threshold", &thresholdValue);

						Event::Manager::getInstance()->postEventRT(&event);
					}
				}
				info->previous = value; // automatically buffers a single value
				data[idx++] = value;
			}

			fifo.write(&token, sizeof(token));
			fifo.write(data, sizeof(data));
		}
		else {
			double prevdata[nchans];
			for (std::list<Scope::Channel>::iterator i = getChannelsBegin(), end =
					getChannelsEnd(); i != end; ++i) {
				struct channel_info *info =
					reinterpret_cast<struct channel_info *> (i->getInfo());
				prevdata[idx++] = info->previous;
			}
			fifo.write(&token, sizeof(token));
			fifo.write(prevdata, sizeof(prevdata));
		}
	}
	counter %= downsample_rate;
}

void Oscilloscope::Panel::screenshot() {
	QwtPlotRenderer renderer;
	renderer.exportTo(scopeWindow,"screenshot.pdf");
}

bool Oscilloscope::Panel::setInactiveSync(void) {
	bool active = getActive();
	setActive(false);
	SyncEvent event;
	RT::System::getInstance()->postEvent(&event);
	return active;
}

void Oscilloscope::Panel::flushFifo(void) {
	char yogi;
	while (fifo.read(&yogi, sizeof(yogi), false))
		;
}

void Oscilloscope::Panel::adjustDataSize(void) {
	double period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
	size_t size = ceil(getDivT() * getDivX() / period) + 1;
	setDataSize(size);
}

void Oscilloscope::Panel::showProperties(void) {
	properties->show();
	properties->raise();
	properties->move(mapToGlobal(rect().center()) - properties->rect().center());
}

void Oscilloscope::Panel::timeoutEvent(void) {
	size_t size;

	while (fifo.read(&size, sizeof(size), false)) {
		double data[size];
		if (fifo.read(data, sizeof(data)))
			setData(data, size);
	}
}

void Oscilloscope::Panel::mouseDoubleClickEvent(QMouseEvent *e) {
	if (e->button() == Qt::LeftButton && getTriggerChannel() != getChannelsEnd()) {
		double scale = height() / (getTriggerChannel()->getScale() * getDivY());
		double offset = getTriggerChannel()->getOffset();
		double threshold = (height() / 2 - e->y()) / scale - offset;

		setTrigger(getTriggerDirection(), threshold, getTriggerChannel(),
				getTriggerHolding(), getTriggerHoldoff());
		showDisplayTab();
	}
}

void Oscilloscope::Panel::doDeferred(const Settings::Object::State &s) {
	bool active = setInactiveSync();

	for (size_t i = 0, nchans = s.loadInteger("Num Channels"); i < nchans; ++i)	{
		std::ostringstream str;
		str << i;

		IO::Block
			*block = dynamic_cast<IO::Block *> (Settings::Manager::getInstance()->getObject(s.loadInteger(str.str() + " ID")));
		if (!block)
			continue;

		struct channel_info *info = new struct channel_info;

		info->block = block;
		info->type = s.loadInteger(str.str() + " type");
		info->index = s.loadInteger(str.str() + " index");
		info->name = QString::number(block->getID())+" "+QString::fromStdString(block->getName(info->type, info->index));
		info->previous = 0.0;

		std::list<Scope::Channel>::iterator chan =
			insertChannel(info->name, s.loadDouble(str.str() + " scale"),
					s.loadDouble(str.str() + " offset"), QPen(QColor(QString::fromStdString(s.loadString(str.str() + " pen color"))), s.loadInteger(str.str()
							+ " pen width"), static_cast<Qt::PenStyle> (s.loadInteger(str.str() + " pen style"))), info);

		setChannelLabel(chan, info->name + " " + properties->
				scaleList->itemText(static_cast<int> (round(4 * (log10(1/chan->getScale()) + 1)))).simplified());
	}

	flushFifo();
	setActive(active);
}

void Oscilloscope::Panel::doLoad(const Settings::Object::State &s) {
	setDataSize(s.loadInteger("Size"));
	//setDivXY(s.loadInteger("DivX"), s.loadInteger("DivY"));
	setDivT(s.loadDouble("DivT"));

	if (s.loadInteger("Maximized"))
		showMaximized();
	else if (s.loadInteger("Minimized"))
		showMinimized();

	if (paused() != s.loadInteger("Paused"))
		togglePause();

	setRefresh(s.loadInteger("Refresh"));

	resize(s.loadInteger("W"), s.loadInteger("H"));
	parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
}

void Oscilloscope::Panel::doSave(Settings::Object::State &s) const {
	s.saveInteger("Size", getDataSize());
	s.saveInteger("DivX", getDivX());
	s.saveInteger("DivY", getDivY());
	s.saveDouble("DivT", getDivT());

	if (isMaximized())
		s.saveInteger("Maximized", 1);
	else if (isMinimized())
		s.saveInteger("Minimized", 1);

	s.saveInteger("Paused", paused());
	s.saveInteger("Refresh", getRefresh());

	QPoint pos = parentWidget()->pos();
	s.saveInteger("X", pos.x());
	s.saveInteger("Y", pos.y());
	s.saveInteger("W", width());
	s.saveInteger("H", height());

	s.saveInteger("Num Channels", getChannelCount());
	size_t n = 0;
	for (std::list<Channel>::const_iterator i = getChannelsBegin(), end = getChannelsEnd(); i != end; ++i) {
		std::ostringstream str;
		str << n++;

		const struct channel_info *info =
			reinterpret_cast<const struct channel_info *> (i->getInfo());

		s.saveInteger(str.str() + " ID", info->block->getID());
		s.saveInteger(str.str() + " type", info->type);
		s.saveInteger(str.str() + " index", info->index);

		s.saveDouble(str.str() + " scale", i->getScale());
		s.saveDouble(str.str() + " offset", i->getOffset());

		QPen pen = i->getPen();
		s.saveString(str.str() + " pen color", pen.color().name().toStdString());
		s.saveInteger(str.str() + " pen style", pen.style());
		s.saveInteger(str.str() + " pen width", pen.width());
	}
}

static Mutex mutex;
Oscilloscope::Plugin *Oscilloscope::Plugin::instance = 0;

Oscilloscope::Plugin * Oscilloscope::Plugin::getInstance(void) {
	if (instance)
		return instance;

	/*************************************************************************
	 * Seems like alot of hoops to jump through, but allocation isn't        *
	 *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
	 *************************************************************************/

	Mutex::Locker lock(&::mutex);
	if (!instance)
		instance = new Plugin();

	return instance;
}
