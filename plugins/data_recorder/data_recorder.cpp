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

#include <qapplication.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qhbox.h>
#include <qhgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qsettings.h>
#include <qspinbox.h>
#include <qstringlist.h>
#include <qvbox.h>
#include <qvgroupbox.h>
#include <qwaitcondition.h>
#include <qsettings.h>
#include <qwhatsthis.h>

#include <compiler.h>
#include <debug.h>
#include <main_window.h>
#include <sstream>
#include <workspace.h>
#include <data_recorder.h>

#define QFileExistsEvent            (QEvent::User+0)
#define QNoFileOpenEvent            (QEvent::User+1)
#define QSetFileNameEditEvent       (QEvent::User+2)
#define QDisableGroupsEvent         (QEvent::User+3)
#define QEnableGroupsEvent          (QEvent::User+4)

struct param_hdf_t {
	long long index;
	double value;
};

namespace {

void buildBlockPtrList(IO::Block *block, void *arg) {
	std::vector<IO::Block *> *list =
			reinterpret_cast<std::vector<IO::Block *> *> (arg);
	list->push_back(block);
}
;

struct FileExistsEventData {
	QString filename;
	int response;
	QWaitCondition done;
};

struct SetFileNameEditEventData {
	QString filename;
	QWaitCondition done;
};

class InsertChannelEvent: public RT::Event {

public:

	InsertChannelEvent(bool &, RT::List<DataRecorder::Channel> &, RT::List<
			DataRecorder::Channel>::iterator, DataRecorder::Channel &);
	~InsertChannelEvent(void);

	int callback(void);

private:

	bool &recording;
	RT::List<DataRecorder::Channel> &channels;
	RT::List<DataRecorder::Channel>::iterator end;
	DataRecorder::Channel &channel;

}; // class InsertChannelEvent

class RemoveChannelEvent: public RT::Event {

public:

	RemoveChannelEvent(bool &, RT::List<DataRecorder::Channel> &,
			DataRecorder::Channel &);
	~RemoveChannelEvent(void);

	int callback(void);

private:

	bool &recording;
	RT::List<DataRecorder::Channel> &channels;
	DataRecorder::Channel &channel;

}; // class RemoveChannelEvent

class OpenFileEvent: public RT::Event {

public:

	OpenFileEvent(QString &, Fifo &);
	~OpenFileEvent(void);

	int callback(void);

private:

	QString &filename;
	Fifo &fifo;

}; // class OpenFileEvent

class StartRecordingEvent: public RT::Event {

public:

	StartRecordingEvent(bool &, Fifo &);
	~StartRecordingEvent(void);

	int callback(void);

private:

	bool &recording;
	Fifo &fifo;

}; // class StartRecordingEvent

class StopRecordingEvent: public RT::Event {

public:

	StopRecordingEvent(bool &, Fifo &);
	~StopRecordingEvent(void);

	int callback(void);

private:

	bool &recording;
	Fifo &fifo;

}; //class StopRecordingEvent

class AsyncDataEvent: public RT::Event {

public:

	AsyncDataEvent(const double *, size_t, Fifo &);
	~AsyncDataEvent(void);

	int callback(void);

private:

	const double *data;
	size_t size;
	Fifo &fifo;

}; // class AsyncDataEvent

class DoneEvent: public RT::Event {

public:

	DoneEvent(Fifo &);
	~DoneEvent(void);

	int callback(void);

private:

	Fifo &fifo;

}; // class DoneEvent

}
; // namespace

InsertChannelEvent::InsertChannelEvent(bool &r,
		RT::List<DataRecorder::Channel> & l,
		RT::List<DataRecorder::Channel>::iterator e, DataRecorder::Channel &c) :
	recording(r), channels(l), end(e), channel(c) {
}

InsertChannelEvent::~InsertChannelEvent(void) {
}

int InsertChannelEvent::callback(void) {
	if (recording)
		return -1;

	channels.insertRT(end, channel);
	return 0;
}

RemoveChannelEvent::RemoveChannelEvent(bool &r,
		RT::List<DataRecorder::Channel> & l, DataRecorder::Channel &c) :
	recording(r), channels(l), channel(c) {
}

RemoveChannelEvent::~RemoveChannelEvent(void) {
}

int RemoveChannelEvent::callback(void) {
	if (recording)
		return -1;

	channels.removeRT(channel);
	return 0;
}

OpenFileEvent::OpenFileEvent(QString &n, Fifo &f) :
	filename(n), fifo(f) {
}

OpenFileEvent::~OpenFileEvent(void) {
}

int OpenFileEvent::callback(void) {
	DataRecorder::data_token_t token;

	token.type = DataRecorder::OPEN;
	token.size = filename.length() + 1;
	token.time = RT::OS::getTime();

	fifo.write(&token, sizeof(token));
	fifo.write(filename.latin1(), token.size);

	return 0;
}

StartRecordingEvent::StartRecordingEvent(bool &r, Fifo &f) :
	recording(r), fifo(f) {
}

StartRecordingEvent::~StartRecordingEvent(void) {
}

int StartRecordingEvent::callback(void) {
	DataRecorder::data_token_t token;

	recording = true;

	token.type = DataRecorder::START;
	token.size = 0;
	token.time = RT::OS::getTime();

	fifo.write(&token, sizeof(token));

	return 0;
}

StopRecordingEvent::StopRecordingEvent(bool &r, Fifo &f) :
	recording(r), fifo(f) {
}

StopRecordingEvent::~StopRecordingEvent(void) {
}

int StopRecordingEvent::callback(void) {
	DataRecorder::data_token_t token;

	recording = false;

	token.type = DataRecorder::STOP;
	token.size = 0;
	token.time = RT::OS::getTime();

	fifo.write(&token, sizeof(token));

	return 0;
}

AsyncDataEvent::AsyncDataEvent(const double *d, size_t s, Fifo &f) :
	data(d), size(s), fifo(f) {
}

AsyncDataEvent::~AsyncDataEvent(void) {
}

int AsyncDataEvent::callback(void) {
	DataRecorder::data_token_t token;

	token.type = DataRecorder::ASYNC;
	token.size = size * sizeof(double);
	token.time = RT::OS::getTime();

	fifo.write(&token, sizeof(token));
	fifo.write(data, token.size);

	return 1;
}

DoneEvent::DoneEvent(Fifo &f) :
	fifo(f) {
}

DoneEvent::~DoneEvent(void) {
}

int DoneEvent::callback(void) {
	DataRecorder::data_token_t token;

	token.type = DataRecorder::DONE;
	token.size = 0;
	token.time = RT::OS::getTime();

	fifo.write(&token, sizeof(token));

	return 0;
}

void DataRecorder::startRecording(void) {
	Event::Object event(Event::START_RECORDING_EVENT);

	if (RT::OS::isRealtime())
		Event::Manager::getInstance()->postEventRT(&event);
	else
		Event::Manager::getInstance()->postEvent(&event);
}

void DataRecorder::stopRecording(void) {
	Event::Object event(Event::STOP_RECORDING_EVENT);

	if (RT::OS::isRealtime())
		Event::Manager::getInstance()->postEventRT(&event);
	else
		Event::Manager::getInstance()->postEvent(&event);
}

void DataRecorder::openFile(const QString& filename) {
	Event::Object event(Event::OPEN_FILE_EVENT);
	event.setParam("filename", const_cast<char *> (filename.latin1()));

	if (RT::OS::isRealtime())
		Event::Manager::getInstance()->postEventRT(&event);
	else
		Event::Manager::getInstance()->postEvent(&event);
}

void DataRecorder::postAsyncData(const double *data, size_t size) {
	Event::Object event(Event::ASYNC_DATA_EVENT);
	event.setParam("data", const_cast<double *> (data));
	event.setParam("size", &size);

	if (RT::OS::isRealtime())
		Event::Manager::getInstance()->postEventRT(&event);
	else
		Event::Manager::getInstance()->postEvent(&event);
}

DataRecorder::Channel::Channel(void) {
}

DataRecorder::Channel::~Channel(void) {
}

DataRecorder::Panel::Panel(QWidget *parent, size_t buffersize) :
	QWidget(parent, 0, Qt::WStyle_NormalBorder | Qt::WDestructiveClose),
			RT::Thread(RT::Thread::MinimumPriority), fifo(buffersize), recording(
					false) {
        setCaption(QString::number(getID()) + " Data Recorder");
	QWhatsThis::add(
			this,
			"<p><b>Data Recorder:</b><br>The Data Recorder writes data to an HDF5 file format "
			"All available signals for saving to file are automatically detected. Currently "
			"loaded user modules are listed in the \"Block\" drop-down box. Available DAQ cards "
			"are listed here as /dev/comedi[#]. Use the \"Type\" and \"Channel\" drop-down boxes "
			"to select the signals that you want to save. Use the left and right arrow buttons to "
			"add these signals to the file. You may select a downsampling rate that is applied "
            "to the real-time period for execution (set in the System Control Panel). The real-time "
            "period and the data downsampling rate are both saved as metadata in the HDF5 file "
            "so that you can reconstruct your data correctly. The current recording status of "
            "the Data Recorder is shown at the bottom.</p>");

	QHBox *hbox;
	QVBox *vbox;

	QBoxLayout *layout = new QVBoxLayout(this);
	layout->setMargin(5);

	channelBox = new QHGroupBox("Channel Selection", this);
	channelBox->setInsideMargin(5);
	layout->addWidget(channelBox);

	vbox = new QVBox(channelBox);
	vbox->setMaximumHeight(125);

	hbox = new QHBox(vbox);
	(new QLabel("Block:", hbox))->setFixedWidth(75);
	blockList = new QComboBox(hbox);
	blockList->setFixedWidth(150);
	QObject::connect(blockList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));

	hbox = new QHBox(vbox);
	(new QLabel("Type:", hbox))->setFixedWidth(75);
	typeList = new QComboBox(hbox);
	typeList->setFixedWidth(150);
	typeList->insertItem("Input");
	typeList->insertItem("Output");
	typeList->insertItem("Parameter");
	typeList->insertItem("State");
	typeList->insertItem("Event");
	QObject::connect(typeList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));

	hbox = new QHBox(vbox);
	(new QLabel("Channel:", hbox))->setFixedWidth(75);
	channelList = new QComboBox(hbox);
	channelList->setFixedWidth(150);

	vbox = new QVBox(channelBox);
	vbox->setMaximumHeight(100);

	QPushButton *rButton = new QPushButton(">", vbox);
	rButton->setFixedWidth(rButton->height());
	QObject::connect(rButton,SIGNAL(pressed(void)),this,SLOT(insertChannel(void)));
	QPushButton *lButton = new QPushButton("<", vbox);
	lButton->setFixedWidth(lButton->height());
	QObject::connect(lButton,SIGNAL(pressed(void)),this,SLOT(removeChannel(void)));

	selectionBox = new QListBox(channelBox);

	sampleBox = new QVGroupBox("Sample Control", this);
	sampleBox->setInsideMargin(5);
	layout->addWidget(sampleBox);

	hbox = new QHBox(sampleBox);
	(new QLabel("Downsampling Rate:", hbox))->setFixedWidth(150);
	downsampleSpin = new QSpinBox(1, 1000, 1, hbox);
	QObject::connect(downsampleSpin,SIGNAL(valueChanged(int)),this,SLOT(updateDownsampleRate(int)));

	fileBox = new QVGroupBox("File Control", this);
	fileBox->setInsideMargin(5);
	layout->addWidget(fileBox);

	vbox = new QVBox(fileBox);
	hbox = new QHBox(vbox);
	hbox->setSpacing(2);
	(new QLabel("Filename:", hbox))->setFixedWidth(60);
	fileNameEdit = new QLineEdit(hbox);
	fileNameEdit->setReadOnly(true);
	QPushButton *fileChangeButton = new QPushButton("Choose File", hbox);
	QObject::connect(fileChangeButton,SIGNAL(clicked(void)),this,SLOT(changeDataFile(void)));
	hbox = new QHBox(vbox);
	fileSizeLbl = new QLabel(hbox);
	fileSizeLbl->setText("File Size (kb):");
	fileSize = new QLabel(hbox);
	fileSize->setText("No data recorded.");
	fileSize->setAlignment(AlignLeft | AlignVCenter);

	trialNumLbl = new QLabel(hbox);
	trialNumLbl->setText("Trial:");
	trialNumLbl->setAlignment(AlignRight | AlignVCenter);
	trialNum = new QLabel(hbox);
	trialNum->setText("0");
	trialNum->setAlignment(AlignLeft | AlignVCenter);

	trialLengthLbl = new QLabel(hbox);
	trialLengthLbl->setText("Trial Length (s):");
	trialLength = new QLabel(hbox);
	trialLength->setText("No data recorded.");
	trialLength->setAlignment(AlignLeft | AlignVCenter);

	hbox = new QHBox(this);
	layout->addWidget(hbox);

	startRecordButton = new QPushButton("Start Recording", hbox);
	QObject::connect(startRecordButton,SIGNAL(clicked(void)),this,SLOT(startRecordClicked(void)));
	stopRecordButton = new QPushButton("Stop Recording", hbox);
	QObject::connect(stopRecordButton,SIGNAL(clicked(void)),this,SLOT(stopRecordClicked(void)));
	recordStatus = new QLabel(hbox);
	recordStatus->setText("Waiting...");
	recordStatus->setFrameStyle(QFrame::Panel | QFrame::Sunken);
	recordStatus->setAlignment(AlignHCenter | AlignVCenter);

	QPushButton *closeButton = new QPushButton("Close", hbox);
	QObject::connect(closeButton,SIGNAL(clicked(void)),this,SLOT(close(void)));

	resize(550, 260);
	show();

	// Build initial block list
	IO::Connector::getInstance()->foreachBlock(buildBlockPtrList, &blockPtrList);
	for (std::vector<IO::Block *>::const_iterator i = blockPtrList.begin(),
			end = blockPtrList.end(); i != end; ++i)
		blockList->insertItem((*i)->getName() + " " + QString::number(
				(*i)->getID()));

	// Build initial channel list
	buildChannelList();

	// Launch Recording Thread
	pthread_create(&thread, 0, bounce, this);

	counter = 0;
	downsample_rate = 1;
	prev_input = 0.0;
	count = 0;
	setActive(true);
}

DataRecorder::Panel::~Panel(void) {
	Plugin::getInstance()->removeDataRecorderPanel(this);

	setActive(false);

	DoneEvent event(fifo);
	while (RT::System::getInstance()->postEvent(&event))
		;

	pthread_join(thread, 0);

	for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i
			!= end;)
		delete &*(i++);
}

void DataRecorder::Panel::execute(void) {
	if (recording && !counter++) {
		data_token_t token;
		double data[channels.size()];

		size_t n = 0;
		token.type = SYNC;
		token.size = channels.size() * sizeof(double);
		for (RT::List<Channel>::iterator i = channels.begin(), end =
				channels.end(); i != end; ++i)
			if (i->block)
				data[n++] = i->block->getValue(i->type, i->index);

		fifo.write(&token, sizeof(token));
		fifo.write(data, sizeof(data));
	}
	count++;
	counter %= downsample_rate;
}

void DataRecorder::Panel::receiveEvent(const Event::Object *event) {
	if (event->getName() == Event::IO_BLOCK_INSERT_EVENT) {

		IO::Block *block = reinterpret_cast<IO::Block *> (event->getParam(
				"block"));

		blockPtrList.push_back(block);
		blockList->insertItem(block->getName() + " " + QString::number(
				block->getID()));
		if (blockList->count() == 1)
			buildChannelList();

	} else if (event->getName() == Event::IO_BLOCK_REMOVE_EVENT) {

		IO::Block *block = reinterpret_cast<IO::Block *> (event->getParam(
				"block"));
		QString name = block->getName() + " " + QString::number(block->getID());

		int n = 0;
		for (; n < blockList->count() && blockList->text(n) != name; ++n)
			;
		if (n < blockList->count())
			blockList->removeItem(n);
		blockPtrList.erase(blockPtrList.begin() + n);

		for (RT::List<Channel>::iterator i = channels.begin(), end =
				channels.end(); i != end; ++i)
			if (i->block == block)
				if (recording)
					i->block = 0;

	} else if (event->getName() == Event::OPEN_FILE_EVENT) {
            
		QString filename(reinterpret_cast<char*> (event->getParam("filename")));
		OpenFileEvent e(filename, fifo);
		RT::System::getInstance()->postEvent(&e);

	} else if (event->getName() == Event::START_RECORDING_EVENT) {

		StartRecordingEvent e(recording, fifo);
		RT::System::getInstance()->postEvent(&e);

	} else if (event->getName() == Event::STOP_RECORDING_EVENT) {

		StopRecordingEvent e(recording, fifo);
		RT::System::getInstance()->postEvent(&e);

	} else if (event->getName() == Event::ASYNC_DATA_EVENT) {

		AsyncDataEvent e(reinterpret_cast<double *> (event->getParam("data")),
				*reinterpret_cast<size_t *> (event->getParam("size")), fifo);
		RT::System::getInstance()->postEvent(&e);

	}
}

void DataRecorder::Panel::receiveEventRT(const Event::Object *event) {
	if (event->getName() == Event::OPEN_FILE_EVENT) {
		QString filename = QString(reinterpret_cast<char*> (event->getParam("filename")));
            
		data_token_t token;
            
		token.type = DataRecorder::OPEN;
		token.size = filename.length() + 1;
		token.time = RT::OS::getTime();

		fifo.write(&token, sizeof(token));
		fifo.write(filename.latin1(), token.size);
	} else if (event->getName() == Event::START_RECORDING_EVENT) {
		data_token_t token;

		recording = true;

		token.type = DataRecorder::START;
		token.size = 0;
		token.time = RT::OS::getTime();

		fifo.write(&token, sizeof(token));
	} else if (event->getName() == Event::STOP_RECORDING_EVENT) {
		data_token_t token;

		recording = false;

		token.type = DataRecorder::STOP;
		token.size = 0;
		token.time = RT::OS::getTime();

		fifo.write(&token, sizeof(token));
	} else if (event->getName() == Event::ASYNC_DATA_EVENT) {
		size_t size = *reinterpret_cast<size_t *> (event->getParam("size"));

		data_token_t token;

		token.type = DataRecorder::ASYNC;
		token.size = size * sizeof(double);
		token.time = RT::OS::getTime();

		fifo.write(&token, sizeof(token));
		fifo.write(event->getParam("data"), token.size);
	} else if (event->getName() == Event::WORKSPACE_PARAMETER_CHANGE_EVENT) {
		data_token_t token;

		token.type = DataRecorder::PARAM;
		token.size = sizeof(param_change_t);
		token.time = RT::OS::getTime();

		param_change_t data;
		data.id = reinterpret_cast<Settings::Object::ID> (event->getParam(
				"object"));
		data.index = reinterpret_cast<size_t> (event->getParam("index"));
		data.step = file.idx;
		data.value = *reinterpret_cast<double *> (event->getParam("value"));

		fifo.write(&token, sizeof(token));
		fifo.write(&data, sizeof(data));
	}
}

void DataRecorder::Panel::buildChannelList(void) {
	channelList->clear();

	if (!blockList->count())
		return;

	IO::Block *block = blockPtrList[blockList->currentItem()];
	IO::flags_t type;
	switch (typeList->currentItem()) {
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
	case 4:
		type = Workspace::EVENT;
		break;
	default:
		ERROR_MSG("DataRecorder::Panel::buildChannelList : invalid type selection\n");
		typeList->setCurrentItem(0);
		type = Workspace::INPUT;
	}

	for (size_t i = 0; i < block->getCount(type); ++i)
		channelList->insertItem(block->getName(type, i));
}

void DataRecorder::Panel::changeDataFile(void) {
	QFileDialog fileDialog(this, NULL, true);
	fileDialog.setCaption("Select Data File");
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);
    fileDialog.setDir(userprefs.readEntry("/dirs/data", getenv("HOME")));
	fileDialog.setMode(QFileDialog::AnyFile);

	QStringList filterList;
	filterList.push_back("HDF5 files (*.h5)");
	filterList.push_back("All files (*.*)");
	fileDialog.setFilters(filterList);

	fileDialog.exec();

	if (fileDialog.selectedFile() == "/" || fileDialog.selectedFile().isNull()
			|| fileDialog.selectedFile().isEmpty())
		return;

	QString filename = fileDialog.selectedFile();

	if (!filename.lower().endsWith(QString(".h5")))
		filename += ".h5";

	// write this directory to the user prefs as most recently used
	userprefs.writeEntry("/dirs/data", fileDialog.dirPath());

	OpenFileEvent event(filename, fifo);
	RT::System::getInstance()->postEvent(&event);
}

void DataRecorder::Panel::insertChannel(void) {
	if (!blockList->count() || !channelList->count())
		return;

	Channel *channel = new Channel();
	channel->block = blockPtrList[blockList->currentItem()];
	switch (typeList->currentItem()) {
	case 0:
		channel->type = Workspace::INPUT;
		break;
	case 1:
		channel->type = Workspace::OUTPUT;
		break;
	case 2:
		channel->type = Workspace::PARAMETER;
		break;
	case 3:
		channel->type = Workspace::STATE;
		break;
	case 4:
		channel->type = Workspace::EVENT;
		break;
	default:
		ERROR_MSG("DataRecorder::Panel::insertChannel : invalid type selection\n");
		typeList->setCurrentItem(0);
		channel->type = Workspace::INPUT;
	}
	channel->index = channelList->currentItem();

	channel->name.sprintf("%s %ld : %s", channel->block->getName().c_str(),
			channel->block->getID(), channel->block->getName(channel->type,
					channel->index).c_str());

	InsertChannelEvent event(recording, channels, channels.end(), *channel);
	if (!RT::System::getInstance()->postEvent(&event))
		selectionBox->insertItem(channel->name);
}

void DataRecorder::Panel::removeChannel(void) {
	if (!selectionBox->count())
		return;

	for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i
			!= end; ++i)
		if (i->name == selectionBox->currentText()) {
			RemoveChannelEvent event(recording, channels, *i);
			if (!RT::System::getInstance()->postEvent(&event))
				selectionBox->removeItem(selectionBox->currentItem());
			break;
		}
}

void DataRecorder::Panel::startRecordClicked(void) {
	count = 0;
	StartRecordingEvent event(recording, fifo);
	RT::System::getInstance()->postEvent(&event);
}

void DataRecorder::Panel::stopRecordClicked(void) {
	fixedcount = count;
	StopRecordingEvent event(recording, fifo);
	RT::System::getInstance()->postEvent(&event);
}

void DataRecorder::Panel::updateDownsampleRate(int r) {
	downsample_rate = r;
}

void DataRecorder::Panel::customEvent(QCustomEvent *e) {
	if (e->type() == QFileExistsEvent) {
		FileExistsEventData *data =
				reinterpret_cast<FileExistsEventData *> (e->data());
		data->response = QMessageBox::information(this, "File exists",
				"The file \"" + data->filename + "\" already exists.",
				"Append", "Overwrite", "Cancel", 0, 2);
		data->done.wakeAll();
		recordStatus->setText("Waiting...");
	} else if (e->type() == QNoFileOpenEvent) {
		QMessageBox::critical(
				this,
				"Failed to start recording.",
				"No file has been opened for writing so recording could not be started.",
				QMessageBox::Ok, QMessageBox::NoButton);
		recordStatus->setText("Waiting...");
	} else if (e->type() == QSetFileNameEditEvent) {
		SetFileNameEditEventData *data =
				reinterpret_cast<SetFileNameEditEventData *> (e->data());
		fileNameEdit->setText(data->filename);
		recordStatus->setText("Ready.");
		data->done.wakeAll();
	} else if (e->type() == QDisableGroupsEvent) {
		channelBox->setEnabled(false);
		sampleBox->setEnabled(false);
		recordStatus->setText("Recording...");
	} else if (e->type() == QEnableGroupsEvent) {
		channelBox->setEnabled(true);
		sampleBox->setEnabled(true);
		recordStatus->setText("Done.");
		fileSize->setNum(int(QFile(fileNameEdit->text()).size()) / 1024);
		trialLength->setNum(double(RT::System::getInstance()->getPeriod()
				* 1e-9 * fixedcount));
		count = 0;
	}
}

void DataRecorder::Panel::doDeferred(const Settings::Object::State &s) {
	for (int i = 0; i < s.loadInteger("Num Channels"); ++i) {
		Channel *channel;
		IO::Block *block;
		std::ostringstream str;
		str << i;

		block
				= dynamic_cast<IO::Block *> (Settings::Manager::getInstance()->getObject(
						s.loadInteger(str.str() + " ID")));
		if (!block)
			continue;

		channel = new Channel();

		channel->block = block;
		channel->type = s.loadInteger(str.str() + " type");
		channel->index = s.loadInteger(str.str() + " index");
		channel->name.sprintf("%s %ld : %s", channel->block->getName().c_str(),
				channel->block->getID(), channel->block->getName(channel->type,
						channel->index).c_str());

		channels.insert(channels.end(), *channel);
		selectionBox->insertItem(channel->name);
	}
}

void DataRecorder::Panel::doLoad(const Settings::Object::State &s) {
	if (s.loadInteger("Maximized"))
		showMaximized();
	else if (s.loadInteger("Minimized"))
		showMinimized();

	downsampleSpin->setValue(s.loadInteger("Downsample"));

	resize(s.loadInteger("W"), s.loadInteger("H"));
	parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
}

void DataRecorder::Panel::doSave(Settings::Object::State &s) const {
	if (isMaximized())
		s.saveInteger("Maximized", 1);
	else if (isMinimized())
		s.saveInteger("Minimized", 1);

	QPoint pos = parentWidget()->pos();
	s.saveInteger("X", pos.x());
	s.saveInteger("Y", pos.y());
	s.saveInteger("W", width());
	s.saveInteger("H", height());

	s.saveInteger("Downsample", downsampleSpin->value());

	s.saveInteger("Num Channels", channels.size());
	size_t n = 0;
	for (RT::List<Channel>::const_iterator i = channels.begin(), end =
			channels.end(); i != end; ++i) {
		std::ostringstream str;
		str << n++;

		s.saveInteger(str.str() + " ID", i->block->getID());
		s.saveInteger(str.str() + " type", i->type);
		s.saveInteger(str.str() + " index", i->index);
	}
}

void *DataRecorder::Panel::bounce(void *param) {
	Panel *that = reinterpret_cast<Panel *> (param);
	if (that)
		that->processData();
	return 0;
}

void DataRecorder::Panel::processData(void) {
	enum {
		CLOSED, OPENED, RECORD,
	} state = CLOSED;

	data_token_t token;

	for (;;) {

		fifo.read(&token, sizeof(token));

		if (token.type == SYNC) {

			if (state == RECORD) {
				double data[token.size / sizeof(double)];
				fifo.read(data, token.size);
				H5PTappend(file.cdata, 1, data);

				++file.idx;
			}

		} else if (token.type == ASYNC) {

			if (state == RECORD) {

				double data[token.size / sizeof(double)];
				fifo.read(data, token.size);

				if (data) {
					hsize_t array_size[] = { token.size / sizeof(double) };
					hid_t array_space = H5Screate_simple(1, array_size,
							array_size);
					hid_t array_type = H5Tarray_create(H5T_IEEE_F64LE, 1,
							array_size);

					QString data_name = QString::number(
							static_cast<unsigned long long> (token.time));

					hid_t adata = H5Dcreate(file.adata, data_name.latin1(),
							array_type, array_space, H5P_DEFAULT, H5P_DEFAULT,
							H5P_DEFAULT);
					H5Dwrite(adata, array_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
							data);

					H5Dclose(adata);
					H5Tclose(array_type);
					H5Sclose(array_space);
				}
			}

		} else if (token.type == OPEN) {

			if (state == RECORD)
				stopRecording(token.time);

			if (state != CLOSED)
				closeFile();

			char filename_string[token.size];
			fifo.read(filename_string, token.size);
			QString filename = filename_string;

			if (openFile(filename))
				state = CLOSED;
			else
				state = OPENED;

		} else if (token.type == CLOSE) {

			if (state == RECORD)
				stopRecording(RT::OS::getTime());

			if (state != CLOSED)
				closeFile();

			state = CLOSED;

		} else if (token.type == START) {

			if (state == CLOSED) {
				QCustomEvent *event = new QCustomEvent(QNoFileOpenEvent);
				QApplication::postEvent(this, event);
			} else if (state == OPENED) {
				startRecording(token.time);
				state = RECORD;
			}

		} else if (token.type == STOP) {

			if (state == RECORD) {
				stopRecording(token.time);
				state = OPENED;
			}

		} else if (token.type == DONE) {

			if (state == RECORD)
				stopRecording(token.time, true);

			if (state != CLOSED)
				closeFile(true);

			break;
		} else if (token.type == PARAM) {
			param_change_t data;
			fifo.read(&data, sizeof(data));

			IO::Block
					*block =
							dynamic_cast<IO::Block *> (Settings::Manager::getInstance()->getObject(
									data.id));

			if (block && state == RECORD) {
				param_hdf_t param = { data.step, data.value, };

				hid_t param_type;
				param_type = H5Tcreate(H5T_COMPOUND, sizeof(param_hdf_t));
				H5Tinsert(param_type, "index", HOFFSET(param_hdf_t,index),
						H5T_STD_I64LE);
				H5Tinsert(param_type, "value", HOFFSET(param_hdf_t,value),
						H5T_IEEE_F64LE);

				QString parameter_name = QString::number(block->getID()) + " "
						+ block->getName() + " : " + block->getName(
						Workspace::PARAMETER, data.index);

				hid_t data = H5PTopen(file.pdata, parameter_name.latin1());
				H5PTappend(data, 1, &param);
				H5PTclose(data);

				H5Tclose(param_type);
			}
		}

	}
}

int DataRecorder::Panel::openFile(QString &filename) {

#ifdef DEBUG
	if(!pthread_equal(pthread_self(),thread)) {
		ERROR_MSG("DataRecorder::Panel::openFile : called by invalid thread\n");
		PRINT_BACKTRACE();
	}
#endif

	if (QFile::exists(filename)) {
		QCustomEvent *event = new QCustomEvent(QFileExistsEvent);
		FileExistsEventData data;

		event->setData(&data);
		data.filename = filename;

		QApplication::postEvent(this, event);
		data.done.wait();

		if (data.response == 0) { // append
			file.id = H5Fopen(filename.latin1(), H5F_ACC_RDWR, H5P_DEFAULT);
			size_t trial_num;
			QString trial_name;
			H5Eset_auto(H5E_DEFAULT, NULL, NULL);
			for (trial_num = 1;; ++trial_num) {
				trial_name = "/Trial" + QString::number(trial_num);
				file.trial = H5Gopen(file.id, trial_name.latin1(), H5P_DEFAULT);
				if (file.trial < 0) {
					H5Eclear(H5E_DEFAULT);
					break;
				} else
				H5Gclose(file.trial);
			}
      			trialNum->setNum(int(trial_num)-1);
		} else if (data.response == 1) { //overwrite
			file.id = H5Fcreate(filename.latin1(), H5F_ACC_TRUNC, H5P_DEFAULT,
					H5P_DEFAULT);
			trialNum->setText("0");
		} else {
			return -1;
                }
	} else {
		file.id = H5Fcreate(filename.latin1(), H5F_ACC_TRUNC, H5P_DEFAULT,
				H5P_DEFAULT);
                trialNum->setText("0");
        }
	if (file.id < 0) {
		H5E_type_t error_type;
		size_t error_size;
		error_size = H5Eget_msg(file.id, &error_type, NULL, 0);

		{
			char error_msg[error_size + 1];
			H5Eget_msg(file.id, &error_type, error_msg, error_size);
			error_msg[error_size] = 0;
			H5Eclear(file.id);

			ERROR_MSG("DataRecorder::Panel::processData : failed to open \"%s\" for writing with error : %s\n",filename.latin1(),error_msg);
			return -1;
		}
	}

	QCustomEvent *event = new QCustomEvent(QSetFileNameEditEvent);
	SetFileNameEditEventData data;

	event->setData(&data);
	data.filename = filename;

	QApplication::postEvent(this, event);
	data.done.wait();

	return 0;
}

void DataRecorder::Panel::closeFile(bool shutdown) {

#ifdef DEBUG
	if(!pthread_equal(pthread_self(),thread)) {
		ERROR_MSG("DataRecorder::Panel::closeFile : called by invalid thread\n");
		PRINT_BACKTRACE();
	}
#endif

	H5Fclose(file.id);

	if (!shutdown) {
		QCustomEvent *event = new QCustomEvent(QSetFileNameEditEvent);
		SetFileNameEditEventData data;

		event->setData(&data);
		data.filename = "";

		QApplication::postEvent(this, event);
		data.done.wait();
	}
}

int DataRecorder::Panel::startRecording(long long timestamp) {

#ifdef DEBUG
	if(!pthread_equal(pthread_self(),thread)) {
		ERROR_MSG("DataRecorder::Panel::startRecording : called by invalid thread\n");
		PRINT_BACKTRACE();
	}
#endif

	size_t trial_num;
	QString trial_name;

	H5Eset_auto(H5E_DEFAULT, NULL, NULL);

	for (trial_num = 1;; ++trial_num) {
		trial_name = "/Trial" + QString::number(trial_num);
		file.trial = H5Gopen(file.id, trial_name.latin1(), H5P_DEFAULT);

		if (file.trial < 0) {
			H5Eclear(H5E_DEFAULT);
			break;
		} else
			H5Gclose(file.trial);
	}

        trialNum->setNum(int(trial_num));
	file.trial = H5Gcreate(file.id, trial_name.latin1(), H5P_DEFAULT,
			H5P_DEFAULT, H5P_DEFAULT);
	file.pdata = H5Gcreate(file.trial, "Parameters", H5P_DEFAULT, H5P_DEFAULT,
			H5P_DEFAULT);
	file.adata = H5Gcreate(file.trial, "Asynchronous Data", H5P_DEFAULT,
			H5P_DEFAULT, H5P_DEFAULT);
	file.sdata = H5Gcreate(file.trial, "Synchronous Data", H5P_DEFAULT,
			H5P_DEFAULT, H5P_DEFAULT);

	hid_t scalar_space = H5Screate(H5S_SCALAR);
	hid_t string_type = H5Tcopy(H5T_C_S1);
	size_t string_size = 512;
	H5Tset_size(string_type, string_size);
	hid_t data;

	long long period = RT::System::getInstance()->getPeriod();
	data = H5Dcreate(file.trial, "Period (ns)", H5T_STD_U64LE, scalar_space,
			H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &period);
	H5Dclose(data);

	long long downsample = downsample_rate;
	data = H5Dcreate(file.trial, "Downsampling Rate", H5T_STD_U64LE,
			scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &downsample);
	H5Dclose(data);

	data = H5Dcreate(file.trial, "Timestamp Start (ns)", H5T_STD_U64LE,
			scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &timestamp);
	H5Dclose(data);

	data = H5Dcreate(file.trial, "Date", string_type, scalar_space,
			H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
			QDateTime::currentDateTime().toString(Qt::ISODate).latin1());
	H5Dclose(data);

	hid_t param_type;
	param_type = H5Tcreate(H5T_COMPOUND, sizeof(param_hdf_t));
	H5Tinsert(param_type, "index", HOFFSET(param_hdf_t,index), H5T_STD_I64LE);
	H5Tinsert(param_type, "value", HOFFSET(param_hdf_t,value), H5T_IEEE_F64LE);

	for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i
			!= end; ++i) {
		IO::Block *block = i->block;
		for (size_t j = 0; j < block->getCount(Workspace::PARAMETER); ++j) {
			QString parameter_name = QString::number(block->getID()) + " "
					+ block->getName() + " : " + block->getName(
					Workspace::PARAMETER, j);
			data = H5PTcreate_fl(file.pdata, parameter_name.latin1(),
					param_type, sizeof(param_hdf_t), -1);
			struct param_hdf_t value = { 0, block->getValue(
					Workspace::PARAMETER, j), };
			H5PTappend(data, 1, &value);
			H5PTclose(data);
		}
		for (size_t j = 0; j < block->getCount(Workspace::COMMENT); ++j) {
			QString comment_name = QString::number(block->getID()) + " "
					+ block->getName() + " : " + block->getName(
					Workspace::COMMENT, j);
			hsize_t
					dims =
							dynamic_cast<Workspace::Instance *> (block)->getValueString(
									Workspace::COMMENT, j).size() + 1;
			hid_t comment_space = H5Screate_simple(1, &dims, &dims);
			data = H5Dcreate(file.pdata, comment_name.latin1(), H5T_C_S1,
					comment_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
			H5Dwrite(
					data,
					H5T_C_S1,
					H5S_ALL,
					H5S_ALL,
					H5P_DEFAULT,
					dynamic_cast<Workspace::Instance *> (block)->getValueString(
							Workspace::COMMENT, j).c_str());
			H5Dclose(data);
		}
	}

	H5Tclose(param_type);

	size_t count = 0;
	for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i
			!= end; ++i) {
		QString channel_name = "Channel " + QString::number(++count) + " Name";
		hid_t data = H5Dcreate(file.sdata, channel_name.latin1(), string_type,
				scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
				i->name.latin1());
		H5Dclose(data);
	}

	H5Tclose(string_type);
	H5Sclose(scalar_space);

	if (channels.size()) {
		hsize_t array_size[] = { channels.size() };
		hid_t array_type = H5Tarray_create(H5T_IEEE_F64LE, 1, array_size);
		file.cdata = H5PTcreate_fl(file.sdata, "Channel Data", array_type,
				(hsize_t) 64, 1);
		H5Tclose(array_type);
	}

	file.idx = 0;

	QCustomEvent *event = new QCustomEvent(QDisableGroupsEvent);
	QApplication::postEvent(this, event);

	return 0;
}

void DataRecorder::Panel::stopRecording(long long timestamp, bool shutdown) {

#ifdef DEBUG
	if(!pthread_equal(pthread_self(),thread)) {
		ERROR_MSG("DataRecorder::Panel::stopRecording : called by invalid thread\n");
		PRINT_BACKTRACE();
	}
#endif
        fixedcount = count;
	hid_t scalar_space = H5Screate(H5S_SCALAR);
	hid_t data = H5Dcreate(file.trial, "Timestamp Stop (ns)", H5T_STD_U64LE,
			scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &timestamp);
	H5Dclose(data);

	long long period = RT::System::getInstance()->getPeriod();
	long long datalength = period * fixedcount;
	data = H5Dcreate(file.trial, "Trial Length (ns)", H5T_STD_U64LE,
			scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
	H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &datalength);
	H5Dclose(data);
	H5Sclose(scalar_space);

	H5PTclose(file.cdata);
	H5Gclose(file.sdata);
	H5Gclose(file.pdata);
	H5Gclose(file.adata);
	H5Gclose(file.trial);

	H5Fflush(file.id, H5F_SCOPE_LOCAL);
	void *file_handle;
	H5Fget_vfd_handle(file.id, H5P_DEFAULT, &file_handle);
	if (fsync(*static_cast<int *> (file_handle))) {
		DEBUG_MSG("DataRecorder::Panel::stopRecording : fsync failed, running sync\n");
		sync();
	}

	if (!shutdown) {
		QCustomEvent *event = new QCustomEvent(QEnableGroupsEvent);
		QApplication::postEvent(this, event);
	}
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
	return DataRecorder::Plugin::getInstance();
}

DataRecorder::Plugin::Plugin(void) {
        // get the HDF data recorder buffer size from user preference
        QSettings userprefs;
        userprefs.setPath("RTXI.org", "RTXI", QSettings::User);

        buffersize = userprefs.readNumEntry("/system/HDFbuffer", 10)*1048576;
        menuID = MainWindow::getInstance()->createSystemMenuItem("HDF Data Recorder",this,SLOT(createDataRecorderPanel(void)));
}

DataRecorder::Plugin::~Plugin(void) {
	MainWindow::getInstance()->removeSystemMenuItem(menuID);
	while (panelList.size())
		delete panelList.front();
	instance = 0;
}

DataRecorder::Panel *DataRecorder::Plugin::createDataRecorderPanel(void) {
	Panel *panel = new Panel(MainWindow::getInstance()->centralWidget(), buffersize);
	panelList.push_back(panel);
	return panel;
}

void DataRecorder::Plugin::removeDataRecorderPanel(DataRecorder::Panel *panel) {
	panelList.remove(panel);
}

void DataRecorder::Plugin::doDeferred(const Settings::Object::State &s) {
	size_t i = 0;
	for (std::list<Panel *>::iterator j = panelList.begin(), end =
			panelList.end(); j != end; ++j)
		(*j)->deferred(s.loadState(QString::number(i++)));
}

void DataRecorder::Plugin::doLoad(const Settings::Object::State &s) {
	for (size_t i = 0; i < static_cast<size_t> (s.loadInteger("Num Panels")); ++i) {

		Panel *panel = new Panel(MainWindow::getInstance()->centralWidget(), buffersize);
		panelList.push_back(panel);
		panel->load(s.loadState(QString::number(i)));
	}
}

void DataRecorder::Plugin::doSave(Settings::Object::State &s) const {
	s.saveInteger("Num Panels", panelList.size());
	size_t n = 0;
	for (std::list<Panel *>::const_iterator i = panelList.begin(), end =
			panelList.end(); i != end; ++i)
		s.saveState(QString::number(n++), (*i)->save());
}

static Mutex mutex;
DataRecorder::Plugin *DataRecorder::Plugin::instance = 0;

DataRecorder::Plugin *DataRecorder::Plugin::getInstance(void) {
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
