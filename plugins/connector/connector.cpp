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

#include <connector.h>
#include <main_window.h>
#include <mutex.h>

struct block_list_info_t {
	QComboBox *blockList0;
	QComboBox *blockList1;
	std::vector<IO::Block *> *blocks;
};

static void buildBlockList(IO::Block *block,void *arg) {
	block_list_info_t *info = static_cast<block_list_info_t *>(arg);
	info->blockList0->addItem(QString::fromStdString(block->getName())+" "+QString::number(block->getID()));
	info->blockList1->addItem(QString::fromStdString(block->getName())+" "+QString::number(block->getID()));
	info->blocks->push_back(block);
}

Connector::Panel::Panel(QWidget *parent) : QWidget(parent) {

	setAttribute(Qt::WA_DeleteOnClose);
	setWhatsThis(
			"<p><b>Connector:</b><br>The Connector panel allows you to make connections between "
			"signals and slots in your workspace. Signals are generated by the DAQ card (associated "
			"with input channels) and by user modules. Available signals are listed in the \"Output "
			"Block\" drop-down box and available slots are listed in the \"Input Block\" drop-down box. "
			"The arrow button is a toggle button that turns connections on and off. Clicking the toggle "
			"button immediately makes a connection active or inactive in real-time. Current connections "
			"are listed in the \"Connections\" box.</p>");

	// Create main layout
	QGridLayout *layout = new QGridLayout;

	// Create child widget and layout for output block
	outputGroup = new QGroupBox(tr("Ouput Block"));
	QVBoxLayout *outputLayout = new QVBoxLayout;

	// Create elements for output
	outputLayout->addWidget(new QLabel(tr("Block:")), 1, 0);
	outputBlock = new QComboBox;
	outputLayout->addWidget(outputBlock);
	QObject::connect(outputBlock,SIGNAL(activated(int)),this,SLOT(buildOutputChannelList(void)));

	outputLayout->addWidget(new QLabel(tr("Channel:")), 2, 0);
	outputChannel = new QComboBox;
	outputLayout->addWidget(outputChannel);
	QObject::connect(outputChannel,SIGNAL(activated(int)),this,SLOT(updateConnectionButton(void)));

	// Assign layout to child widget
	outputGroup->setLayout(outputLayout);

	// Create child widget and layout for connection button
	buttonGroup = new QGroupBox;
	QVBoxLayout *buttonLayout = new QVBoxLayout;

	// Create elements for button
	connectionButton = new QPushButton("Connect");
	connectionButton->isCheckable();
	buttonLayout->addWidget(connectionButton);
	QObject::connect(connectionButton,SIGNAL(toggled(bool)),this,SLOT(toggleConnection(bool)));

	// Assign layout to child widget
	buttonGroup->setLayout(buttonLayout);

	// Create child widget and layout for input block
	inputGroup = new QGroupBox(tr("Input Block"));
	QVBoxLayout *inputLayout = new QVBoxLayout;

	// Create elements for output
	inputLayout->addWidget(new QLabel(tr("Block:")), 1, 0);
	inputBlock = new QComboBox;
	inputLayout->addWidget(inputBlock);
	QObject::connect(inputBlock,SIGNAL(activated(int)),this,SLOT(buildInputChannelList(void)));

	inputLayout->addWidget(new QLabel(tr("Channel:")), 2, 0);
	inputChannel = new QComboBox;
	inputLayout->addWidget(inputChannel);
	QObject::connect(inputChannel,SIGNAL(activated(int)),this,SLOT(updateConnectionButton(void)));

	// Assign layout to child widget
	inputGroup->setLayout(inputLayout);

	// Create child widget and layout for connections box
	connectionGroup = new QGroupBox(tr("Connections"));
	QVBoxLayout *connectionLayout = new QVBoxLayout;

	// Create elements for connection box
	connectionBox = new QListWidget;
	connectionLayout->addWidget(connectionBox);
	QObject::connect(connectionBox,SIGNAL(itemClicked(QListWidgetItem *)),this,SLOT(highlightConnectionBox(QListWidgetItem *)));

	// Assign layout to child widget
	connectionGroup->setLayout(connectionLayout);

	// Attach child widget to parent widget
	layout->addWidget(outputGroup, 1, 0, 1, 2);
	layout->addWidget(buttonGroup, 2, 0, 1, 4);
	layout->addWidget(inputGroup, 1, 2, 1, 2);
	layout->addWidget(connectionGroup, 3, 0, 1, 4);

	// Attach layout to widget
	setLayout(layout);
	setWindowTitle("Connector Panel");
	//show();

	block_list_info_t info = {inputBlock, outputBlock, &blocks};
	IO::Connector::getInstance()->foreachBlock(::buildBlockList,&info);

	if(blocks.size() >= 1) {
		buildInputChannelList();
		buildOutputChannelList();
	}

	IO::Connector::getInstance()->foreachConnection(&buildConnectionList,&links);
	for(size_t i = 0, iend = links.size();i < iend;++i) {
		connectionBox->addItem(QString::number(links[i].src->getID())+" "+QString::fromStdString(links[i].src->getName())+" : "+QString::number(links[i].src_idx)+" "+QString::fromStdString(links[i].src->getName(IO::OUTPUT,links[i].src_idx))+" ==> "+
				QString::number(links[i].dest->getID())+" "+QString::fromStdString(links[i].dest->getName())+" : "+QString::number(links[i].dest_idx)+" "+QString::fromStdString(links[i].dest->getName(IO::INPUT,links[i].dest_idx)));
	}
}

Connector::Panel::~Panel(void) {
	Plugin::getInstance()->removeConnectorPanel(this);
}

void Connector::Panel::receiveEvent(const Event::Object *event) {
	if(event->getName() == Event::IO_BLOCK_INSERT_EVENT) {
		IO::Block *block = reinterpret_cast<IO::Block *>(event->getParam("block"));

		inputBlock->addItem(QString::fromStdString(block->getName())+QString(" ")+QString::number(block->getID()));
		outputBlock->addItem(QString::fromStdString(block->getName())+QString(" ")+QString::number(block->getID()));
		blocks.push_back(block);

		if(blocks.size() == 1) {
			buildInputChannelList();
			buildOutputChannelList();
		}
	} else if(event->getName() == Event::IO_BLOCK_REMOVE_EVENT) {
		IO::Block *block = reinterpret_cast<IO::Block *>(event->getParam("block"));

		size_t index;
		for(index = 0;index < blocks.size() && blocks[index] != block;++index);
		if(index >= blocks.size())
			return;

		size_t current0 = inputBlock->currentIndex();
		size_t current1 = outputBlock->currentIndex();

		inputBlock->removeItem(index);
		outputBlock->removeItem(index);
		blocks.erase(blocks.begin()+index);

		if(current0 == index) {
			inputBlock->setCurrentIndex(0);
			buildInputChannelList();
		}
		if(current1 == index) {
			outputBlock->setCurrentIndex(0);
			buildOutputChannelList();
		}
	} else if(event->getName() == Event::IO_LINK_INSERT_EVENT) {
		IO::Block *src = reinterpret_cast<IO::Block *>(event->getParam("src"));
		size_t src_idx = *reinterpret_cast<size_t *>(event->getParam("src_num"));
		IO::Block *dest = reinterpret_cast<IO::Block *>(event->getParam("dest"));
		size_t dest_idx = *reinterpret_cast<size_t *>(event->getParam("dest_num"));

		connectionBox->addItem(QString::number(src->getID())+" "+QString::fromStdString(src->getName())+" : "+
				QString::number(src_idx)+" "+QString::fromStdString(src->getName(IO::OUTPUT,src_idx))+" ==> "+
				QString::number(dest->getID())+" "+QString::fromStdString(dest->getName())+" : "+
				QString::number(dest_idx)+" "+QString::fromStdString(dest->getName(IO::INPUT,dest_idx)));
	} else if(event->getName() == Event::IO_LINK_REMOVE_EVENT) {
		IO::Block *src = reinterpret_cast<IO::Block *>(event->getParam("src"));
		size_t src_idx = *reinterpret_cast<size_t *>(event->getParam("src_num"));
		IO::Block *dest = reinterpret_cast<IO::Block *>(event->getParam("dest"));
		size_t dest_idx = *reinterpret_cast<size_t *>(event->getParam("dest_num"));

		QString link_name = QString::number(src->getID())+" "+QString::fromStdString(src->getName())+" : "+	
			QString::number(src_idx)+" "+QString::fromStdString(src->getName(IO::OUTPUT,src_idx))+" ==> "+
			QString::number(dest->getID())+" "+QString::fromStdString(dest->getName())+" : "+
			QString::number(dest_idx)+" "+QString::fromStdString(dest->getName(IO::INPUT,dest_idx));

		size_t index;
		for(index=0;index < connectionBox->count() && connectionBox->currentItem()->text() != link_name;++index);
		if(index >= connectionBox->count())
			ERROR_MSG("Connector::Panel::receiveEvent : removing non-existant link.\n");
		else
			connectionBox->takeItem(index);
	}
}

void Connector::Panel::buildInputChannelList(void) {
	inputChannel->clear();
	if(!inputBlock->count())
		return;

	IO::Block *block = blocks[inputBlock->currentIndex()];

	for(size_t i = 0;i < block->getCount(IO::INPUT);++i)
		inputChannel->addItem(QString::fromStdString(block->getName(IO::INPUT,i)));

	updateConnectionButton();
}

void Connector::Panel::buildOutputChannelList(void) {
	outputChannel->clear();
	if(!outputBlock->count())
		return;

	IO::Block *block = blocks[outputBlock->currentIndex()];

	for(size_t i = 0;i < block->getCount(IO::OUTPUT);++i)
		outputChannel->addItem(QString::fromStdString(block->getName(IO::OUTPUT,i)));

	updateConnectionButton();
}

void Connector::Panel::highlightConnectionBox(QListWidgetItem * item) {

	QString selection = item->text();
	QString substr;
	int sep;

	sep = selection.indexOf(' ');
	substr = selection.left(sep);
	selection = selection.right(selection.length()-sep-1);
	Settings::Object::ID src_id = substr.toULong();

	sep = selection.indexOf(':');
	selection = selection.right(selection.length()-sep-2);

	sep = selection.indexOf(' ');
	substr = selection.left(sep);
	selection = selection.right(selection.length()-sep-1);
	size_t src_idx = substr.toULong();

	sep = selection.indexOf("==>");
	selection = selection.right(selection.length()-sep-4);

	sep = selection.indexOf(' ');
	substr = selection.left(sep);
	selection = selection.right(selection.length()-sep-1);
	Settings::Object::ID dest_id = substr.toULong();

	sep = selection.indexOf(':');
	selection = selection.right(selection.length()-sep-2);

	sep = selection.indexOf(' ');
	substr = selection.left(sep);
	selection = selection.right(selection.length()-sep-1);
	size_t dest_idx = substr.toULong();

	IO::Block *src = dynamic_cast<IO::Block *>(Settings::Manager::getInstance()->getObject(src_id));

	size_t index;
	for(index = 0;index < blocks.size() && blocks[index] != src;++index);
	if(index >= blocks.size())
		ERROR_MSG("Connector::Panel::highlightConnectionBox : highlighted source does not exist.\n");

	outputBlock->setCurrentIndex(index);
	buildOutputChannelList();
	outputChannel->setCurrentIndex(src_idx);

	IO::Block *dest = dynamic_cast<IO::Block *>(Settings::Manager::getInstance()->getObject(dest_id));
	for(index = 0;index < blocks.size() && blocks[index] != dest;++index);
	if(index >= blocks.size())
		ERROR_MSG("Connector::Panel::highlightConnectionBox : highlighted destination does not exist.\n");

	inputBlock->setCurrentIndex(index);
	buildInputChannelList();
	inputChannel->setCurrentIndex(dest_idx);
}

void Connector::Panel::toggleConnection(bool on) {
	IO::Block *src = blocks[outputBlock->currentIndex()];
	IO::Block *dest = blocks[inputBlock->currentIndex()];
	size_t src_num = outputChannel->currentIndex();
	size_t dest_num = inputChannel->currentIndex();

	if(IO::Connector::getInstance()->connected(src,src_num,dest,dest_num) == on)
		return;

	if(on)
		IO::Connector::getInstance()->connect(src,src_num,dest,dest_num);
	else
		IO::Connector::getInstance()->disconnect(src,src_num,dest,dest_num);
}

void Connector::Panel::updateConnectionButton(void) {
	IO::Block *src = blocks[outputBlock->currentIndex()];
	IO::Block *dest = blocks[inputBlock->currentIndex()];
	size_t src_num = outputChannel->currentIndex();
	size_t dest_num = inputChannel->currentIndex();

	connectionButton->setChecked(IO::Connector::getInstance()->connected(src,src_num,dest,dest_num));
}

void Connector::Panel::buildConnectionList(IO::Block *src,size_t src_num,IO::Block *dest,size_t dest_num,void *arg) {
	std::vector<link_t> &list = *reinterpret_cast<std::vector<link_t> *>(arg);

	link_t link = {
		src,
		src_num,
		dest,
		dest_num,
	};

	list.push_back(link);
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
	return Connector::Plugin::getInstance();
}

Connector::Plugin::Plugin(void) : panel(0) {
	MainWindow::getInstance()->createSystemMenuItem("Connector",this,SLOT(showConnectorPanel(void)));
}

Connector::Plugin::~Plugin(void) {
	MainWindow::getInstance()->removeSystemMenuItem(menuID);
	if(panel)
		delete panel;
	instance = 0;
}

void Connector::Plugin::showConnectorPanel(void) {
	if(!panel)
		panel = new Panel(MainWindow::getInstance()->centralWidget());
	panel->show();
}

void Connector::Plugin::removeConnectorPanel(Connector::Panel *p) {
	if(p == panel)
		panel = NULL;
}

static Mutex mutex;
Connector::Plugin *Connector::Plugin::instance = 0;

Connector::Plugin *Connector::Plugin::getInstance(void) {
	if(instance)
		return instance;

	/*************************************************************************
	 * Seems like alot of hoops to jump through, but allocation isn't        *
	 *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
	 *************************************************************************/

	Mutex::Locker lock(&::mutex);
	if(!instance)
		instance = new Plugin();

	return instance;
}
