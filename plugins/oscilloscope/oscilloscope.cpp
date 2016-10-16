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
	 This class creates and controls the drawing parameters
	 A control panel is instantiated for all the active channels/modules
	 and user selection is enabled to change color, style, width and other
	 oscilloscope properties.
 */

#include <qwt_plot_renderer.h>

#include <debug.h>
#include <main_window.h>
#include <rt.h>
#include <workspace.h>
#include <cmath>
#include <sstream>

#include "oscilloscope.h"

namespace
{
class SyncEvent : public RT::Event
{
public:
    int callback(void)
    {
        return 0;
    };
}; // SyncEvent

struct channel_info
{
    QString name;
    IO::Block *block;
    IO::flags_t type;
    size_t index;
    double previous; // stores previous value for trigger and downsample buffer
}; // channel_info
} // namespace

////////// #Plugin
extern "C" Plugin::Object * createRTXIPlugin(void *)
{
    return Oscilloscope::Plugin::getInstance();
}

// Create and insert scope into menu
Oscilloscope::Plugin::Plugin(void)
{
    MainWindow::getInstance()->createSystemMenuItem("Oscilloscope",this,SLOT(createOscilloscopePanel(void)));
}

// Kill me
Oscilloscope::Plugin::~Plugin(void)
{
    while (panelList.size())
        delete panelList.front();
    instance = 0;
}

// Create oscilloscope(s) and puts them in the list
void Oscilloscope::Plugin::createOscilloscopePanel(void)
{
    Panel *d_panel = new Panel(MainWindow::getInstance()->centralWidget());
    panelList.push_back(d_panel);
}

void Oscilloscope::Plugin::removeOscilloscopePanel(Panel *d_panel)
{
    panelList.remove(d_panel);
}

void Oscilloscope::Plugin::doDeferred(const Settings::Object::State &s)
{
    size_t i = 0;
    for (std::list<Panel *>::iterator j = panelList.begin(), end = panelList.end(); j != end; ++j)
        (*j)->deferred(s.loadState(QString::number(i++).toStdString()));
}

void Oscilloscope::Plugin::doLoad(const Settings::Object::State &s)
{
    for (size_t i = 0; i < static_cast<size_t> (s.loadInteger("Num Panels")); ++i)
        {
            Panel *d_panel = new Panel(MainWindow::getInstance()->centralWidget());
            panelList.push_back(d_panel);
            d_panel->load(s.loadState(QString::number(i).toStdString()));
        }
}

void Oscilloscope::Plugin::doSave(Settings::Object::State &s) const
{
    s.saveInteger("Num Panels", panelList.size());
    size_t n = 0;
    for (std::list<Panel *>::const_iterator i = panelList.begin(), end = panelList.end(); i != end; ++i)
        s.saveState(QString::number(n++).toStdString(), (*i)->save());
}

void Oscilloscope::Panel::receiveEvent(const ::Event::Object *event)
{
    if (event->getName() == Event::IO_BLOCK_INSERT_EVENT)
        {
            IO::Block *block = reinterpret_cast<IO::Block *> (event->getParam("block"));
            if (block)
                {
                    // Update the list of blocks
                    blocksList->addItem(QString::fromStdString(block->getName())+" "+QString::number(block->getID()));
                    blocks.push_back(block);
                    if (blocksList->count() == 1)
                        buildChannelList();
                }
        }
    else if (event->getName() == Event::IO_BLOCK_REMOVE_EVENT)
        {
            IO::Block *block = reinterpret_cast<IO::Block *> (event->getParam("block"));
            if (block)
                {
                    // Find the index of the block in the blocks vector
                    size_t index;
                    for (index = 0; index < blocks.size() && blocks[index]
                            != block; ++index);

                    if (index < blocks.size())
                        {
                            bool found = false;
                            // Stop displaying channels coming from the removed block
                            for (std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin(), end = scopeWindow->getChannelsEnd(); i != end;)
                                {
                                    struct channel_info *info = reinterpret_cast<struct channel_info *> (i->getInfo());
                                    if (info->block == block)
                                        {
                                            found = true;
                                            // If triggering on this channel disable triggering
                                            if(trigsChanList->currentText() != "<None>")
                                                if (i->getLabel()	== scopeWindow->getTriggerChannel()->getLabel())
                                                    {
                                                        scopeWindow->setTrigger(Scope::NONE, scopeWindow->getTriggerThreshold(),
                                                                                scopeWindow->getChannelsEnd(), scopeWindow->getTriggerWindow());
                                                        showDisplayTab();
                                                    }

                                            struct channel_info *info = reinterpret_cast<struct channel_info *> (i->getInfo());
                                            std::list<Scope::Channel>::iterator chan = i++;
                                            bool active = setInactiveSync();
                                            scopeWindow->removeChannel(chan);
                                            flushFifo();
                                            setActive(active);
                                            delete info;
                                        }
                                    else
                                        ++i;
                                }

                            // Update the list of blocks
                            size_t current = blocksList->currentIndex();
                            blocksList->removeItem(index);
                            blocks.erase(blocks.begin() + index);

                            if (current == index)
                                {
                                    blocksList->setCurrentIndex(0);
                                    buildChannelList();
                                }
                            showTab(0);
                        }
                    else
                        DEBUG_MSG("Oscilloscope::Panel::receiveEvent : removed block never inserted\n");
                }
        }
    else if (event->getName() == Event::RT_POSTPERIOD_EVENT)
        {
            scopeWindow->setPeriod(RT::System::getInstance()->getPeriod() * 1e-6);
            adjustDataSize();
            showTab(0);
        }
}

// Slot for enabling user specified channel
void Oscilloscope::Panel::activateChannel(bool active)
{
    bool enable = active && blocksList->count() && channelsList->count();
    scalesList->setEnabled(enable);
    offsetsEdit->setEnabled(enable);;
    offsetsList->setEnabled(enable);
    colorsList->setEnabled(enable);
    widthsList->setEnabled(enable);
    stylesList->setEnabled(enable);
}

void Oscilloscope::Panel::apply(void)
{
    switch (tabWidget->currentIndex())
        {
        case 0:
            applyChannelTab();
            break;
        case 1:
            applyDisplayTab();
            break;
        default:
            ERROR_MSG("Oscilloscope::Panel::showTab : invalid tab\n");
        }
}

void Oscilloscope::Panel::buildChannelList(void)
{

    channelsList->clear();
    if (!blocksList->count())
        return;

    if (blocksList->currentIndex() < 0)
        blocksList->setCurrentIndex(0);

    IO::Block *block = blocks[blocksList->currentIndex()];
    IO::flags_t type;
    switch (typesList->currentIndex())
        {
        case 0:
            type = Workspace::INPUT;
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
            ERROR_MSG("Oscilloscope::Panel::buildChannelList : invalid type selection\n");
            type = Workspace::INPUT;
        }

    for (size_t i = 0; i < block->getCount(type); ++i)
        channelsList->addItem(QString::fromStdString(block->getName(type, i)));

    showChannelTab();
}

void Oscilloscope::Panel::showTab(int index)
{
    switch (index)
        {
        case 0:
            showChannelTab();
            break;
        case 1:
            showDisplayTab();
            break;
        default:
            ERROR_MSG("Oscilloscope::Panel::showTab : invalid tab\n");
        }
}

void Oscilloscope::Panel::applyChannelTab(void)
{
    if (blocksList->count() <= 0 || channelsList->count() <= 0)
        return;

    IO::Block *block = blocks[blocksList->currentIndex()];
    IO::flags_t type;
    switch (typesList->currentIndex())
        {
        case 0:
            type = Workspace::INPUT;
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
            ERROR_MSG("Oscilloscope::Panel::applyChannelTab : invalid type\n");
            typesList->setCurrentIndex(0);
            type = Workspace::INPUT;
        }

    struct channel_info *info;
    std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin();
    for (std::list<Scope::Channel>::iterator end = scopeWindow->getChannelsEnd(); i
            != end; ++i)
        {
            info = reinterpret_cast<struct channel_info *> (i->getInfo());
            if (info->block == block && info->type == type && info->index
                    == static_cast<size_t> (channelsList->currentIndex()))
                break;
        }

    if (!activateButton->isChecked())
        {
            if (i != scopeWindow->getChannelsEnd())
                {
                    // If triggering on this channel disable triggering
                    if(trigsChanList->currentText() != "<None>")
                        if (i->getLabel() == scopeWindow->getTriggerChannel()->getLabel())
                            scopeWindow->setTrigger(Scope::NONE, scopeWindow->getTriggerThreshold(),
                                                    scopeWindow->getChannelsEnd(), scopeWindow->getTriggerWindow());

                    bool active = setInactiveSync();
                    scopeWindow->removeChannel(i);
                    flushFifo();
                    setActive(active);
                    delete info;
                }
        }
    else
        {
            if (i == scopeWindow->getChannelsEnd())
                {
                    info = new struct channel_info;

                    info->block = block;
                    info->type = type;
                    info->index = channelsList->currentIndex();
                    info->previous = 0.0;
                    info->name = QString::number(block->getID())+" "+QString::fromStdString(block->getName(type, channelsList->currentIndex()));

                    bool active = setInactiveSync();
                    QwtPlotCurve *curve = new QwtPlotCurve(info->name);
                    i = scopeWindow->insertChannel(info->name + " 50 mV/div", 2.0, 0.0, QPen(QColor(255,0,16,255), 1, Qt::SolidLine), curve, info);
                    flushFifo();
                    setActive(active);
                }

            double scale;
            switch (scalesList->currentIndex() % 4)
                {
                case 0:
                    scale = pow(10, 1 - scalesList->currentIndex() / 4);
                    break;
                case 1:
                    scale = 5 * pow(10, -scalesList->currentIndex() / 4);
                    break;
                case 2:
                    scale = 2.5 * pow(10, -scalesList->currentIndex() / 4);
                    break;
                case 3:
                    scale = 2 * pow(10, -scalesList->currentIndex() / 4);
                    break;
                default:
                    ERROR_MSG("Oscilloscope::Panel::applyChannelTab : invalid scale selection\n");
                    scale = 1.0;
                }
            if (scale != i->getScale())
                {
                    scopeWindow->setChannelScale(i, scale);
                    scopeWindow->setChannelLabel(i, info->name + " - " + scalesList->currentText().simplified());
                }
            scopeWindow->setChannelOffset(i, offsetsEdit->text().toDouble() * pow(10, -3 * offsetsList->currentIndex()));

            QPen pen;
            switch (colorsList->currentIndex())
                {
                case 0:
                    pen.setColor(QColor(255,0,16,255));
                    break;
                case 1:
                    pen.setColor(QColor(255,164,5,255));
                    break;
                case 2:
                    pen.setColor(QColor(43,206,72,255));
                    break;
                case 3:
                    pen.setColor(QColor(0,117,220,255));
                    break;
                case 4:
                    pen.setColor(QColor(178,102,255,255));
                    break;
                case 5:
                    pen.setColor(QColor(0,153,143,255));
                    break;
                case 6:
                    pen.setColor(QColor(83,81,84,255));
                    break;
                default:
                    ERROR_MSG("Oscilloscope::Panel::applyChannelTab : invalid color selection\n");
                    pen.setColor(QColor(255,0,16,255));
                }
            pen.setWidth(widthsList->currentIndex() + 1);
            switch (stylesList->currentIndex())
                {
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
                    ERROR_MSG("Oscilloscope::Panel::applyChannelTab : invalid style selection\n");
                    pen.setStyle(Qt::SolidLine);
                }
            scopeWindow->setChannelPen(i, pen);
        }
    scopeWindow->replot();
    showChannelTab();
}

void Oscilloscope::Panel::applyDisplayTab(void)
{
    // Update downsample rate
    downsample_rate = ratesSpin->value();

    // Update refresh rate for oscillscope
    scopeWindow->setRefresh(refreshsSpin->value());

    // Update X divisions
    double divT;
    if (timesList->currentIndex() % 3 == 1)
        divT = 2 * pow(10, 3 - timesList->currentIndex() / 3);
    else if (timesList->currentIndex() % 3 == 2)
        divT = pow(10, 3 - timesList->currentIndex() / 3);
    else
        divT = 5 * pow(10, 3 - timesList->currentIndex() / 3);
    scopeWindow->setDivT(divT);
    scopeWindow->setPeriod(RT::System::getInstance()->getPeriod() * 1e-6);
    adjustDataSize();

    // Update trigger direction
    Scope::trig_t trigDirection = static_cast<Scope::trig_t> (trigsGroup->id(trigsGroup->checkedButton()));

    // Update trigger threshold
    double trigThreshold = trigsThreshEdit->text().toDouble() * pow(10, -3 * trigsThreshList->currentIndex());

    // Update pre-trigger window for displaying
    double trigWindow = trigWindowEdit->text().toDouble() * pow(10, -3 * trigWindowList->currentIndex());

    std::list<Scope::Channel>::iterator trigChannel = scopeWindow->getChannelsEnd();
    for (std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin(), end = scopeWindow->getChannelsEnd(); i != end; ++i)
        if (i->getLabel() == trigsChanList->currentText())
            {
                trigChannel = i;
                break;
            }
    if (trigChannel == scopeWindow->getChannelsEnd())
        trigDirection = Scope::NONE;

    scopeWindow->setTrigger(trigDirection, trigThreshold, trigChannel, trigWindow);

    adjustDataSize();
    scopeWindow->replot();
    showDisplayTab();
}

struct block_list_info_t
{
    QComboBox *blockList;
    std::vector<IO::Block *> *blocks;
};

static void buildBlockList(IO::Block *block, void *arg)
{
    block_list_info_t *info = static_cast<block_list_info_t *> (arg);
    info->blockList->addItem(QString::fromStdString(block->getName())+" "+QString::number(block->getID()));
    info->blocks->push_back(block);
}

QWidget * Oscilloscope::Panel::createChannelTab(QWidget *parent)
{

    setWhatsThis("<p><b>Oscilloscope: Channel Options</b><br>"
                 "Use the dropdown boxes to select the signal streams you want to plot from "
                 "any loaded modules or your DAQ device. You may change the plotting scale for "
                 "the signal, apply a DC offset, and change the color and style of the line.</p>");

    QWidget *page = new QWidget(parent);

    // Create group and layout for buttons at bottom of scope
    QGridLayout *bttnLayout = new QGridLayout(page);

    // Create Channel box
    QHBoxLayout *row1Layout = new QHBoxLayout;
    QLabel *channelLabel = new QLabel(tr("Channel:"),page);
    row1Layout->addWidget(channelLabel);
    blocksList = new QComboBox(page);
    blocksList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    blocksList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    block_list_info_t info = {blocksList, &this->blocks};
    IO::Connector::getInstance()->foreachBlock(::buildBlockList, &info);
    QObject::connect(blocksList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));
    row1Layout->addWidget(blocksList);

    // Create Type box
    typesList = new QComboBox(page);
    typesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    typesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    typesList->addItem("Input");
    typesList->addItem("Output");
    typesList->addItem("Parameter");
    typesList->addItem("State");
    QObject::connect(typesList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));
    row1Layout->addWidget(typesList);

    // Create Channels box
    channelsList = new QComboBox(page);
    channelsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    channelsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    QObject::connect(channelsList,SIGNAL(activated(int)),this,SLOT(showChannelTab(void)));
    row1Layout->addWidget(channelsList);

    // Create elements for display box
    row1Layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    QLabel *scaleLabel = new QLabel(tr("Scale:"),page);
    row1Layout->addWidget(scaleLabel);
    scalesList = new QComboBox(page);
    row1Layout->addWidget(scalesList);
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
    scalesList->addItem(QString::fromUtf8("500 µV/div"));
    scalesList->addItem(QString::fromUtf8("250 µV/div"));
    scalesList->addItem(QString::fromUtf8("200 µV/div"));
    scalesList->addItem(QString::fromUtf8("100 µV/div"));
    scalesList->addItem(QString::fromUtf8("50 µV/div"));
    scalesList->addItem(QString::fromUtf8("25 µV/div"));
    scalesList->addItem(QString::fromUtf8("20 µV/div"));
    scalesList->addItem(QString::fromUtf8("10 µV/div"));
    scalesList->addItem(QString::fromUtf8("5 µV/div"));
    scalesList->addItem(QString::fromUtf8("2.5 µV/div"));
    scalesList->addItem(QString::fromUtf8("2 µV/div"));
    scalesList->addItem(QString::fromUtf8("1 µV/div"));
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
    QLabel *offsetLabel = new QLabel(tr("Offset:"),page);
    row1Layout->addWidget(offsetLabel);
    offsetsEdit = new QLineEdit(page);
    offsetsEdit->setMaximumWidth(offsetsEdit->minimumSizeHint().width()*2);
    offsetsEdit->setValidator(new QDoubleValidator(offsetsEdit));
    row1Layout->addWidget(offsetsEdit);//, Qt::AlignRight);
    offsetsList = new QComboBox(page);
    row1Layout->addWidget(offsetsList);//, Qt::AlignRight);
    offsetsList->addItem("V");
    offsetsList->addItem("mV");
    offsetsList->addItem(QString::fromUtf8("µV"));
    offsetsList->addItem("nV");
    offsetsList->addItem("pV");

    // Create elements for graphic
    QHBoxLayout *row2Layout = new QHBoxLayout;//(page);
    row2Layout->setAlignment(Qt::AlignLeft);
    QLabel *colorLabel = new QLabel(tr("Color:"), page);
    row2Layout->addWidget(colorLabel);
    colorsList = new QComboBox(page);
    colorsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    colorsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    row2Layout->addWidget(colorsList);
    QPixmap tmp(25, 25);
    tmp.fill(QColor(255,0,16,255));
    colorsList->addItem(tmp, " Red");
    tmp.fill(QColor(255,164,5,255));
    colorsList->addItem(tmp, " Orange");
    tmp.fill(QColor(43,206,72,255));
    colorsList->addItem(tmp, " Green");
    tmp.fill(QColor(0,117,220,255));
    colorsList->addItem(tmp, " Blue");
    tmp.fill(QColor(178,102,255,255));
    colorsList->addItem(tmp, " Purple");
    tmp.fill(QColor(0,153,143,255));
    colorsList->addItem(tmp, " Teal");
    tmp.fill(QColor(83,81,84,255));
    colorsList->addItem(tmp, " Black");

    QLabel *widthLabel = new QLabel(tr("Width:"),page);
    row2Layout->addWidget(widthLabel);
    widthsList = new QComboBox(page);
    widthsList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    widthsList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    row2Layout->addWidget(widthsList);
    tmp.fill(Qt::white);
    QPainter painter(&tmp);
    for (int i = 1; i < 6; i++)
        {
            painter.setPen(QPen(QColor(83,81,84,255), i));
            painter.drawLine(0, 12, 25, 12);
            widthsList->addItem(tmp, QString::number(i) + QString(" Pixels"));
        }

    // Create styles list
    QLabel *styleLabel = new QLabel(tr("Style:"),page);
    row2Layout->addWidget(styleLabel);
    stylesList = new QComboBox(page);
    stylesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    stylesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    row2Layout->addWidget(stylesList);
    tmp.fill(Qt::white);
    painter.setPen(QPen(QColor(83,81,84,255), 3, Qt::SolidLine));
    painter.drawLine(0, 12, 25, 12);
    stylesList->addItem(tmp, QString(" Solid"));
    tmp.fill(Qt::white);
    painter.setPen(QPen(QColor(83,81,84,255), 3, Qt::DashLine));
    painter.drawLine(0, 12, 25, 12);
    stylesList->addItem(tmp, QString(" Dash"));
    tmp.fill(Qt::white);
    painter.setPen(QPen(QColor(83,81,84,255), 3, Qt::DotLine));
    painter.drawLine(0, 12, 25, 12);
    stylesList->addItem(tmp, QString(" Dot"));
    tmp.fill(Qt::white);
    painter.setPen(QPen(QColor(83,81,84,255), 3, Qt::DashDotLine));
    painter.drawLine(0, 12, 25, 12);
    stylesList->addItem(tmp, QString(" Dash Dot"));
    tmp.fill(Qt::white);
    painter.setPen(QPen(QColor(83,81,84,255), 3, Qt::DashDotDotLine));
    painter.drawLine(0, 12, 25, 12);
    stylesList->addItem(tmp, QString(" Dash Dot Dot"));

    // Activate button
    row2Layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    activateButton = new QPushButton("Enable Channel",page);
    row2Layout->addWidget(activateButton);
    activateButton->setCheckable(true);
    activateButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(activateButton, SIGNAL(toggled(bool)), this, SLOT(activateChannel(bool)));

    bttnLayout->addLayout(row1Layout, 0, 0);
    bttnLayout->addLayout(row2Layout, 1, 0);

    return page;
}

QWidget *Oscilloscope::Panel::createDisplayTab(QWidget *parent)
{

    setWhatsThis("<p><b>Oscilloscope: Display Options</b><br>"
                 "Use the dropdown box to select the time scale for the Oscilloscope. This "
                 "scaling is applied to all signals plotted in the same window. You may also "
                 "set a trigger on any signal that is currently plotted in the window. A yellow "
                 "line will appear at the trigger threshold.</p>");

    QWidget *page = new QWidget(parent);

    // Scope properties
    QGridLayout *displayTabLayout = new QGridLayout(page);

    // Create elements for time settings
    QHBoxLayout *row1Layout = new QHBoxLayout;
    row1Layout->addWidget(new QLabel(tr("Time/Div:"), page));
    timesList = new QComboBox(page);
    row1Layout->addWidget(timesList);
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
    timesList->addItem(QString::fromUtf8("500 µs/div"));
    timesList->addItem(QString::fromUtf8("200 µs/div"));
    timesList->addItem(QString::fromUtf8("100 µs/div"));
    timesList->addItem(QString::fromUtf8("50 µs/div"));
    timesList->addItem(QString::fromUtf8("20 µs/div"));
    timesList->addItem(QString::fromUtf8("10 µs/div"));
    timesList->addItem(QString::fromUtf8("5 µs/div"));
    timesList->addItem(QString::fromUtf8("2 µs/div"));
    timesList->addItem(QString::fromUtf8("1 µs/div"));
    timesList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    timesList->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    QLabel *refreshLabel = new QLabel(tr("Refresh:"),page);
    row1Layout->addWidget(refreshLabel);
    refreshsSpin = new QSpinBox(page);
    row1Layout->addWidget(refreshsSpin);
    refreshsSpin->setRange(100,10000);
    refreshsSpin->setValue(250);

    QLabel *downsampleLabel = new QLabel(tr("Downsample:"),page);
    row1Layout->addWidget(downsampleLabel);
    ratesSpin = new QSpinBox(page);
    row1Layout->addWidget(ratesSpin);
    ratesSpin->setValue(downsample_rate);
    ratesSpin->setEnabled(true);
    ratesSpin->setRange(1,2);
    ratesSpin->setValue(1);

    // Display box for Buffer bit. Push it to the right.
    row1Layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Minimum));
    QLabel *bufferLabel = new QLabel(tr("Buffer Size (MB):"),page);
    row1Layout->addWidget(bufferLabel);
    sizesEdit = new QLineEdit(page);
    sizesEdit->setMaximumWidth(sizesEdit->minimumSizeHint().width()*3);
    sizesEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    row1Layout->addWidget(sizesEdit);
    sizesEdit->setText(QString::number(scopeWindow->getDataSize()));
    sizesEdit->setEnabled(false);

    // Trigger box
    QHBoxLayout *row2Layout = new QHBoxLayout;
    row2Layout->addWidget(new QLabel(tr("Edge:"),page));
    trigsGroup = new QButtonGroup(page);

    QRadioButton *off = new QRadioButton(tr("Off"),page);
    trigsGroup->addButton(off, Scope::NONE);
    row2Layout->addWidget(off);
    QRadioButton *plus = new QRadioButton(tr("+"),page);
    trigsGroup->addButton(plus, Scope::POS);
    row2Layout->addWidget(plus);
    QRadioButton *minus = new QRadioButton(tr("-"),page);
    trigsGroup->addButton(minus, Scope::NEG);
    row2Layout->addWidget(minus);

    row2Layout->addWidget(new QLabel(tr("Channel:"),page));
    trigsChanList = new QComboBox(page);
    trigsChanList->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    trigsChanList->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    row2Layout->addWidget(trigsChanList);

    row2Layout->addWidget(new QLabel(tr("Threshold:"),page));
    trigsThreshEdit = new QLineEdit(page);
    trigsThreshEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    trigsThreshEdit->setMaximumWidth(trigsThreshEdit->minimumSizeHint().width()*3);
    row2Layout->addWidget(trigsThreshEdit);
    trigsThreshEdit->setValidator(new QDoubleValidator(trigsThreshEdit));
    trigsThreshList = new QComboBox(page);
    trigsThreshList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    row2Layout->addWidget(trigsThreshList);
    trigsThreshList->addItem("V");
    trigsThreshList->addItem("mV");
    trigsThreshList->addItem(QString::fromUtf8("µV"));
    trigsThreshList->addItem("nV");
    trigsThreshList->addItem("pV");

    row2Layout->addWidget(new QLabel(tr("Window:"),page));
    trigWindowEdit = new QLineEdit(page);
    trigWindowEdit->setText(QString::number(scopeWindow->getTriggerWindow()));
    trigWindowEdit->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
    trigWindowEdit->setMaximumWidth(trigWindowEdit->minimumSizeHint().width()*3);
    trigWindowEdit->setValidator(new QDoubleValidator(trigWindowEdit));
    row2Layout->addWidget(trigWindowEdit);
    trigWindowList = new QComboBox(page);
    trigWindowList->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    row2Layout->addWidget(trigWindowList);
    trigWindowList->addItem("s");
    trigWindowList->addItem("ms");
    trigWindowList->addItem(QString::fromUtf8("µs"));
    trigWindowList->setCurrentIndex(1);

    displayTabLayout->addLayout(row1Layout, 0, 0);
    displayTabLayout->addLayout(row2Layout, 1, 0);

    return page;
}

// Aggregates all channel information to show for configuration
// in the display tab
void Oscilloscope::Panel::showChannelTab(void)
{

    IO::flags_t type;
    switch (typesList->currentIndex())
        {
        case 0:
            type = Workspace::INPUT;
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
            ERROR_MSG("Oscilloscope::Panel::showChannelTab : invalid type\n");
            typesList->setCurrentIndex(0);
            type = Workspace::OUTPUT;
        }

    bool found = false;

    for (std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin(), end = scopeWindow->getChannelsEnd(); i != end; ++i)
        {
            struct channel_info *info =
            reinterpret_cast<struct channel_info *> (i->getInfo());
            if (!info)
                continue;
            if (info->block && info->block == blocks[blocksList->currentIndex()]
                    && info->type == type && info->index == static_cast<size_t> (channelsList->currentIndex()))
                {
                    found = true;

                    scalesList->setCurrentIndex(static_cast<int> (round(4 * (log10(1/i->getScale()) + 1))));

                    double offset = i->getOffset();
                    int offsetUnits = 0;
                    if (offset)
                        while (fabs(offset) < 1)
                            {
                                offset *= 1000;
                                offsetUnits++;
                            }
                    offsetsEdit->setText(QString::number(offset));
                    offsetsList->setCurrentIndex(offsetUnits);

                    if (i->getPen().color() == QColor(255,0,16,255))
                        colorsList->setCurrentIndex(0);
                    else if (i->getPen().color() == QColor(255,164,5,255))
                        colorsList->setCurrentIndex(1);
                    else if (i->getPen().color() == QColor(43,206,72,255))
                        colorsList->setCurrentIndex(2);
                    else if (i->getPen().color() == QColor(0,117,220,255))
                        colorsList->setCurrentIndex(3);
                    else if (i->getPen().color() == QColor(178,102,255,255))
                        colorsList->setCurrentIndex(4);
                    else if (i->getPen().color() == QColor(0,153,143,255))
                        colorsList->setCurrentIndex(5);
                    else if (i->getPen().color() == QColor(83,81,84,255))
                        colorsList->setCurrentIndex(6);
                    else
                        {
                            ERROR_MSG("Oscilloscope::Panel::displayChannelTab : invalid color selection\n");
                            colorsList->setCurrentIndex(0);
                        }

                    switch (i->getPen().width())
                        {
                        case 1:
                            widthsList->setCurrentIndex(0);
                            break;
                        case 2:
                            widthsList->setCurrentIndex(1);
                            break;
                        case 3:
                            widthsList->setCurrentIndex(2);
                            break;
                        case 4:
                            widthsList->setCurrentIndex(3);
                            break;
                        case 5:
                            widthsList->setCurrentIndex(4);
                            break;
                        default:
                            ERROR_MSG("Oscilloscope::Panel::displayChannelTab : invalid width selection\n");
                            widthsList->setCurrentIndex(0);
                        }

                    switch (i->getPen().style())
                        {
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
                            ERROR_MSG("Oscilloscope::Panel::displayChannelTab : invalid style selection\n");
                            stylesList->setCurrentIndex(0);
                        }
                    break;
                }
        }

    activateButton->setChecked(found);
    scalesList->setEnabled(found);
    offsetsEdit->setEnabled(found);
    offsetsList->setEnabled(found);
    colorsList->setEnabled(found);
    widthsList->setEnabled(found);
    stylesList->setEnabled(found);
    if (!found)
        {
            scalesList->setCurrentIndex(9);
            offsetsEdit->setText(QString::number(0));
            offsetsList->setCurrentIndex(0);
            colorsList->setCurrentIndex(0);
            widthsList->setCurrentIndex(0);
            stylesList->setCurrentIndex(0);
        }
}

void Oscilloscope::Panel::showDisplayTab(void)
{
    timesList->setCurrentIndex(static_cast<int> (round(3 * log10(1/scopeWindow->getDivT()) + 11)));

    refreshsSpin->setValue(scopeWindow->getRefresh());

    // Find current trigger value and update gui
    static_cast<QRadioButton *>(trigsGroup->button(static_cast<int>(scopeWindow->getTriggerDirection())))->setChecked(true);

    trigsChanList->clear();
    for (std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin(), end =	scopeWindow->getChannelsEnd(); i != end; ++i)
        {
            trigsChanList->addItem(i->getLabel());
            if (i == scopeWindow->getTriggerChannel())
                trigsChanList->setCurrentIndex(trigsChanList->count() - 1);
        }
    trigsChanList->addItem("<None>");
    if (scopeWindow->getTriggerChannel() == scopeWindow->getChannelsEnd())
        trigsChanList->setCurrentIndex(trigsChanList->count() - 1);

    int trigThreshUnits = 0;
    double trigThresh = scopeWindow->getTriggerThreshold();
    if (trigThresh != 0.0)
        while (fabs(trigThresh) < 1)
            {
                trigThresh *= 1000;
                ++trigThreshUnits;
            }
    trigsThreshList->setCurrentIndex(trigThreshUnits);
    trigsThreshEdit->setText(QString::number(trigThresh));

    sizesEdit->setText(QString::number(scopeWindow->getDataSize()));
}

////////// #Panel
Oscilloscope::Panel::Panel(QWidget *parent) :	QWidget(parent), RT::Thread(0), fifo(25 * 1048576)
{

    // Make Mdi
    subWindow = new QMdiSubWindow;
    subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint |
                              Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    MainWindow::getInstance()->createMdi(subWindow);

    setWhatsThis("<p><b>Oscilloscope:</b><br>The Oscilloscope allows you to plot any signal "
                 "in your workspace in real-time, including signals from your DAQ card and those "
                 "generated by user modules. Multiple signals are overlaid in the window and "
                 "different line colors and styles can be selected. When a signal is added, a legend "
                 "automatically appears in the bottom of the window. Multiple oscilloscopes can "
                 "be instantiated to give you multiple data windows. To select signals for plotting, "
                 "use the right-click context \"Panel\" menu item. After selecting a signal, you must "
                 "click the \"Active\" button for it to appear in the window. To change signal settings, "
                 "you must click the \"Apply\" button. The right-click context \"Pause\" menu item "
                 "allows you to start and stop real-time plotting.</p>");

    // Create tab widget
    tabWidget = new QTabWidget;
    tabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    QObject::connect(tabWidget,SIGNAL(currentChanged(int)),this,SLOT(showTab(int)));

    // Create main layout
    layout = new QVBoxLayout;

    // Create scope group
    scopeGroup = new QWidget(this);
    QHBoxLayout *scopeLayout = new QHBoxLayout(this);

    // Create scope
    scopeWindow = new Scope(this);

    // Attach scope to layout
    scopeLayout->addWidget(scopeWindow);

    // Attach to layout
    scopeGroup->setLayout(scopeLayout);

    // Create group
    setBttnGroup = new QGroupBox(this);
    QHBoxLayout *setBttnLayout = new QHBoxLayout(this);

    // Creat buttons
    pauseButton = new QPushButton("Pause");
    pauseButton->setCheckable(true);
    QObject::connect(pauseButton,SIGNAL(released(void)),this,SLOT(togglePause(void)));
    setBttnLayout->addWidget(pauseButton);
    applyButton = new QPushButton("Apply");
    QObject::connect(applyButton,SIGNAL(released(void)),this,SLOT(apply(void)));
    setBttnLayout->addWidget(applyButton);
    settingsButton = new QPushButton("Screenshot");
    QObject::connect(settingsButton,SIGNAL(released(void)),this,SLOT(screenshot(void)));
    setBttnLayout->addWidget(settingsButton);

    // Attach layout
    setBttnGroup->setLayout(setBttnLayout);

    // Create tabs
    tabWidget->setTabPosition(QTabWidget::North);
    tabWidget->addTab(createChannelTab(this), "Channel");
    tabWidget->addTab(createDisplayTab(this), "Display");

    // Setup main layout
    layout->addWidget(scopeGroup);
    layout->addWidget(tabWidget);
    layout->addWidget(setBttnGroup);

    // Set
    setLayout(layout);

    // Show stuff
    adjustDataSize();
    buildChannelList();
    showDisplayTab();
    subWindow->setWidget(this);
    subWindow->setMinimumSize(subWindow->minimumSizeHint().width(),450);
    subWindow->resize(subWindow->minimumSizeHint().width()+50,600);

    // Initialize vars
    counter = 0;
    downsample_rate = 1;
    setActive(true);
    setWindowTitle(QString::number(getID()) + " Oscilloscope");

    QTimer *otimer = new QTimer;
    otimer->setTimerType(Qt::PreciseTimer);
    QObject::connect(otimer,SIGNAL(timeout(void)),this,SLOT(timeoutEvent(void)));
    otimer->start(25);

    scopeWindow->replot();
    show();
}

Oscilloscope::Panel::~Panel(void)
{
    while (scopeWindow->getChannelsBegin() != scopeWindow->getChannelsEnd())
        delete reinterpret_cast<struct channel_info *> (scopeWindow->removeChannel(scopeWindow->getChannelsBegin()));

    Oscilloscope::Plugin::getInstance()->removeOscilloscopePanel(this);
}

void Oscilloscope::Panel::execute(void)
{
    size_t nchans = scopeWindow->getChannelCount();

    if (nchans)
        {
            size_t idx = 0;
            size_t token = nchans;
            double data[nchans];

            if (!counter++)
                {
                    for (std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin(), end = scopeWindow->getChannelsEnd(); i != end; ++i)
                        {
                            struct channel_info *info =
                            reinterpret_cast<struct channel_info *> (i->getInfo());

                            double value = info->block->getValue(info->type, info->index);

                            if (i == scopeWindow->getTriggerChannel())
                                {
                                    double thresholdValue = scopeWindow->getTriggerThreshold();

                                    if ((thresholdValue > value && thresholdValue
                                            < info->previous) || (thresholdValue < value
                                                                  && thresholdValue > info->previous))
                                        {
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
                            data[idx++] = value;	// sample from DAQ
                        }
                    fifo.write(&token, sizeof(token));
                    fifo.write(data, sizeof(data));
                }
            else
                {
                    double prevdata[nchans];
                    for (std::list<Scope::Channel>::iterator i = scopeWindow->getChannelsBegin(), end = scopeWindow->getChannelsEnd(); i != end; ++i)
                        {
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

void Oscilloscope::Panel::screenshot()
{
    QwtPlotRenderer renderer;
    renderer.exportTo(scopeWindow,"screenshot.pdf");
}

void Oscilloscope::Panel::togglePause(void)
{
    scopeWindow->isPaused = !(scopeWindow->isPaused);
}

bool Oscilloscope::Panel::setInactiveSync(void)
{
    bool active = getActive();
    setActive(false);
    SyncEvent event;
    RT::System::getInstance()->postEvent(&event);
    return active;
}

void Oscilloscope::Panel::flushFifo(void)
{
    char yogi;
    while (fifo.read(&yogi, sizeof(yogi), false))
        ;
}

void Oscilloscope::Panel::adjustDataSize(void)
{
    double period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
    size_t size = ceil(scopeWindow->getDivT() * scopeWindow->getDivX() / period) + 1;
    scopeWindow->setDataSize(size);
    sizesEdit->setText(QString::number(scopeWindow->getDataSize()));
}

void Oscilloscope::Panel::timeoutEvent(void)
{
    size_t size;
    while (fifo.read(&size, sizeof(size), false))
        {
            double data[size];
            if (fifo.read(data, sizeof(data)))
                scopeWindow->setData(data, size);
        }
}

void Oscilloscope::Panel::doDeferred(const Settings::Object::State &s)
{
    bool active = setInactiveSync();

    for (size_t i = 0, nchans = s.loadInteger("Num Channels"); i < nchans; ++i)
        {
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

            QwtPlotCurve *curve = new QwtPlotCurve(info->name);

            std::list<Scope::Channel>::iterator chan = scopeWindow->insertChannel(info->name, s.loadDouble(str.str() + " scale"),
                    s.loadDouble(str.str() + " offset"), QPen(QColor(QString::fromStdString(s.loadString(str.str() + " pen color"))),
                            s.loadInteger(str.str() + " pen width"), Qt::PenStyle(s.loadInteger(str.str() + " pen style"))), curve, info);

            scopeWindow->setChannelLabel(chan, info->name + " - " + scalesList->itemText(static_cast<int> (round(4 * (log10(1/chan->getScale()) + 1)))).simplified());
        }

    flushFifo();
    setActive(active);
}

void Oscilloscope::Panel::doLoad(const Settings::Object::State &s)
{
    scopeWindow->setDataSize(s.loadInteger("Size"));
    scopeWindow->setDivT(s.loadDouble("DivT"));

    if (s.loadInteger("Maximized"))
        scopeWindow->showMaximized();
    else if (s.loadInteger("Minimized"))
        scopeWindow->showMinimized();

    if (scopeWindow->paused() != s.loadInteger("Paused"))
        togglePause();

    scopeWindow->setRefresh(s.loadInteger("Refresh"));

    subWindow->resize(s.loadInteger("W"), s.loadInteger("H"));
    parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
}

void Oscilloscope::Panel::doSave(Settings::Object::State &s) const
{
    s.saveInteger("Size", scopeWindow->getDataSize());
    s.saveInteger("DivX", scopeWindow->getDivX());
    s.saveInteger("DivY", scopeWindow->getDivY());
    s.saveDouble("DivT", scopeWindow->getDivT());

    if (isMaximized())
        s.saveInteger("Maximized", 1);
    else if (isMinimized())
        s.saveInteger("Minimized", 1);

    s.saveInteger("Paused", scopeWindow->paused());
    s.saveInteger("Refresh", scopeWindow->getRefresh());

    s.saveInteger("X", parentWidget()->pos().x());
    s.saveInteger("Y", parentWidget()->pos().y());
    s.saveInteger("W", width());
    s.saveInteger("H", height());

    s.saveInteger("Num Channels", scopeWindow->getChannelCount());
    size_t n = 0;
    for (std::list<Scope::Channel>::const_iterator i = scopeWindow->getChannelsBegin(), end = scopeWindow->getChannelsEnd(); i != end; ++i)
        {
            std::ostringstream str;
            str << n++;

            const struct channel_info *info = reinterpret_cast<const struct channel_info *> (i->getInfo());
            s.saveInteger(str.str() + " ID", info->block->getID());
            s.saveInteger(str.str() + " type", info->type);
            s.saveInteger(str.str() + " index", info->index);

            s.saveDouble(str.str() + " scale", i->getScale());
            s.saveDouble(str.str() + " offset", i->getOffset());

            s.saveString(str.str() + " pen color", i->getPen().color().name().toStdString());
            s.saveInteger(str.str() + " pen style", i->getPen().style());
            s.saveInteger(str.str() + " pen width", i->getPen().width());
        }
}

static Mutex mutex;
Oscilloscope::Plugin *Oscilloscope::Plugin::instance = 0;

Oscilloscope::Plugin * Oscilloscope::Plugin::getInstance(void)
{
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
