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

#include <rtxi_config.h>
#include <cstring>
#include <string>
#include <unistd.h>
#include <compiler.h>
#include <debug.h>
#include <main_window.h>
#include <sstream>
#include <workspace.h>
#include <data_recorder.h>
#include <iostream>
#include <pthread.h>

#define QFileExistsEvent            (QEvent::User+0)
#define QSetFileNameEditEvent       (QEvent::User+1)
#define QDisableGroupsEvent         (QEvent::User+2)
#define QEnableGroupsEvent          (QEvent::User+3)

#define TAG_SIZE 1024

struct param_hdf_t
{
    long long index;
    double value;
};

// Debug for event handling
QDebug operator<<(QDebug str, const QEvent * ev)
{
    static int eventEnumIndex = QEvent::staticMetaObject.indexOfEnumerator("Type");
    str << "QEvent";
    if (ev)
        {
            QString name = QEvent::staticMetaObject.enumerator(eventEnumIndex).valueToKey(ev->type());
            if (!name.isEmpty())
                str << name;
            else
                str << ev->type();
        }
    else
        str << (void*)ev;
    return str.maybeSpace();
}

struct find_daq_t
{
    int index;
    DAQ::Device *device;
};

static void findDAQDevice(DAQ::Device *dev,void *arg)
{
    struct find_daq_t *info = static_cast<struct find_daq_t *>(arg);
    if(!info->index)
        info->device = dev;
    info->index--;
}

namespace
{
void buildBlockPtrList(IO::Block *block, void *arg)
{
    std::vector<IO::Block *> *list = reinterpret_cast<std::vector<IO::Block *> *> (arg);
    list->push_back(block);
};

struct FileExistsEventData
{
    QString filename;
    int response;
    QWaitCondition done;
};

struct SetFileNameEditEventData
{
    QString filename;
    QWaitCondition done;
};

class InsertChannelEvent: public RT::Event
{
public:
    InsertChannelEvent(bool &, RT::List<DataRecorder::Channel> &,
                       RT::List<DataRecorder::Channel>::iterator, DataRecorder::Channel &);
    ~InsertChannelEvent(void);
    int callback(void);

private:
    bool &recording;
    RT::List<DataRecorder::Channel> &channels;
    RT::List<DataRecorder::Channel>::iterator end;
    DataRecorder::Channel &channel;
}; // class InsertChannelEvent

class RemoveChannelEvent: public RT::Event
{
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

class OpenFileEvent: public RT::Event
{
public:
    OpenFileEvent(QString &, AtomicFifo &);
    ~OpenFileEvent(void);
    int callback(void);

private:
    QString &filename;
    AtomicFifo &fifo;
}; // class OpenFileEvent

class StartRecordingEvent: public RT::Event
{
public:
    StartRecordingEvent(bool &, AtomicFifo &);
    ~StartRecordingEvent(void);
    int callback(void);

private:
    bool &recording;
    AtomicFifo &fifo;
}; // class StartRecordingEvent

class StopRecordingEvent: public RT::Event
{
public:
    StopRecordingEvent(bool &, AtomicFifo &);
    ~StopRecordingEvent(void);
    int callback(void);

private:
    bool &recording;
    AtomicFifo &fifo;
}; //class StopRecordingEvent

class AsyncDataEvent: public RT::Event
{
public:
    AsyncDataEvent(const double *, size_t, AtomicFifo &);
    ~AsyncDataEvent(void);
    int callback(void);

private:
    const double *data;
    size_t size;
    AtomicFifo &fifo;
}; // class AsyncDataEvent

class DoneEvent: public RT::Event
{
public:
    DoneEvent(AtomicFifo &);
    ~DoneEvent(void);
    int callback(void);

private:
    AtomicFifo &fifo;
}; // class DoneEvent
}; // namespace

InsertChannelEvent::InsertChannelEvent(bool &r, RT::List<DataRecorder::Channel> & l,
                                       RT::List<DataRecorder::Channel>::iterator e, DataRecorder::Channel &c) :
    recording(r), channels(l), end(e), channel(c)
{
}

InsertChannelEvent::~InsertChannelEvent(void)
{
}

int InsertChannelEvent::callback(void)
{
    if(recording)
        return -1;
    channels.insertRT(end, channel);
    return 0;
}

RemoveChannelEvent::RemoveChannelEvent(bool &r,	RT::List<DataRecorder::Channel> & l,
                                       DataRecorder::Channel &c) :
    recording(r), channels(l), channel(c)
{
}

RemoveChannelEvent::~RemoveChannelEvent(void)
{
}

int RemoveChannelEvent::callback(void)
{
    if (recording)
        return -1;
    channels.removeRT(channel);
    return 0;
}

OpenFileEvent::OpenFileEvent(QString &n, AtomicFifo &f) :
    filename(n), fifo(f)
{
}

OpenFileEvent::~OpenFileEvent(void)
{
}

int OpenFileEvent::callback(void)
{
    DataRecorder::data_token_t token;
    token.type = DataRecorder::OPEN;
    token.size = filename.length() + 1;
    token.time = RT::OS::getTime();
    fifo.write(&token, sizeof(token));
    fifo.write(filename.toLatin1().constData(), token.size);
    return 0;
}

StartRecordingEvent::StartRecordingEvent(bool &r, AtomicFifo &f) :
    recording(r), fifo(f)
{
}

StartRecordingEvent::~StartRecordingEvent(void)
{
}

int StartRecordingEvent::callback(void)
{
    DataRecorder::data_token_t token;
    recording = true;
    token.type = DataRecorder::START;
    token.size = 0;
    token.time = RT::OS::getTime();
    fifo.write(&token, sizeof(token));
    return 0;
}

StopRecordingEvent::StopRecordingEvent(bool &r, AtomicFifo &f) :
    recording(r), fifo(f)
{
}

StopRecordingEvent::~StopRecordingEvent(void)
{
}

int StopRecordingEvent::callback(void)
{
    DataRecorder::data_token_t token;
    recording = false;
    token.type = DataRecorder::STOP;
    token.size = 0;
    token.time = RT::OS::getTime();
    fifo.write(&token, sizeof(token));
    return 0;
}

AsyncDataEvent::AsyncDataEvent(const double *d, size_t s, AtomicFifo &f) :
    data(d), size(s), fifo(f)
{
}

AsyncDataEvent::~AsyncDataEvent(void)
{
}

int AsyncDataEvent::callback(void)
{
    DataRecorder::data_token_t token;
    token.type = DataRecorder::ASYNC;
    token.size = size * sizeof(double);
    token.time = RT::OS::getTime();
    fifo.write(&token, sizeof(token));
    fifo.write(data, token.size);
    return 1;
}

DoneEvent::DoneEvent(AtomicFifo &f) :
    fifo(f)
{
}

DoneEvent::~DoneEvent(void)
{
}

int DoneEvent::callback(void)
{
    DataRecorder::data_token_t token;
    token.type = DataRecorder::DONE;
    token.size = 0;
    token.time = RT::OS::getTime();
    fifo.write(&token, sizeof(token));
    return 0;
}

DataRecorder::CustomEvent::CustomEvent(QEvent::Type type) : QEvent(type)
{
    data = 0;
}

void DataRecorder::CustomEvent::setData(void *ptr)
{
    data = ptr;
}

void * DataRecorder::CustomEvent::getData(void)
{
    return data;
}

void DataRecorder::startRecording(void)
{
    Event::Object event(Event::START_RECORDING_EVENT);
    if (RT::OS::isRealtime())
        Event::Manager::getInstance()->postEventRT(&event);
    else
        Event::Manager::getInstance()->postEvent(&event);
}

void DataRecorder::stopRecording(void)
{
    Event::Object event(Event::STOP_RECORDING_EVENT);
    if (RT::OS::isRealtime())
        Event::Manager::getInstance()->postEventRT(&event);
    else
        Event::Manager::getInstance()->postEvent(&event);
}

void DataRecorder::openFile(const QString& filename)
{
    Event::Object event(Event::OPEN_FILE_EVENT);
    event.setParam("filename", const_cast<char *> (filename.toLatin1().constData()));
    if (RT::OS::isRealtime())
        Event::Manager::getInstance()->postEventRT(&event);
    else
        Event::Manager::getInstance()->postEvent(&event);
}

void DataRecorder::postAsyncData(const double *data, size_t size)
{
    Event::Object event(Event::ASYNC_DATA_EVENT);
    event.setParam("data", const_cast<double *> (data));
    event.setParam("size", &size);
    if (RT::OS::isRealtime())
        Event::Manager::getInstance()->postEventRT(&event);
    else
        Event::Manager::getInstance()->postEvent(&event);
}

DataRecorder::Channel::Channel(void)
{
}

DataRecorder::Channel::~Channel(void)
{
}

DataRecorder::Panel::Panel(QWidget *parent, size_t buffersize) :
    QWidget(parent), RT::Thread(RT::Thread::MinimumPriority), fifo(buffersize), recording(false)
{
    setWhatsThis(
        "<p><b>Data Recorder:</b><br>The Data Recorder writes data to an HDF5 file format "
        "All available signals for saving to file are automatically detected. Currently "
        "loaded user modules are listed in the \"Block\" drop-down box. Available DAQ cards "
        "are listed here as /proc/analogy/devices. Use the \"Type\" and \"Channel\" drop-down boxes "
        "to select the signals that you want to save. Use the left and right arrow buttons to "
        "add these signals to the file. You may select a downsampling rate that is applied "
        "to the real-time period for execution (set in the System Control Panel). The real-time "
        "period and the data downsampling rate are both saved as metadata in the HDF5 file "
        "so that you can reconstruct your data correctly. The current recording status of "
        "the Data Recorder is shown at the bottom.</p>");

    // Make Mdi
    subWindow = new QMdiSubWindow;
    subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
    subWindow->setAttribute(Qt::WA_DeleteOnClose);
    subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint |
                              Qt::WindowMinimizeButtonHint);
    MainWindow::getInstance()->createMdi(subWindow);

    // Create main layout
    QGridLayout *layout = new QGridLayout;

    // Create child widget and layout for channel selection
    channelGroup = new QGroupBox(tr("Channel Selection"));
    QVBoxLayout *channelLayout = new QVBoxLayout;

    // Create elements for channel box
    channelLayout->addWidget(new QLabel(tr("Block:")));
    blockList = new QComboBox;
    channelLayout->addWidget(blockList);
    QObject::connect(blockList,SIGNAL(activated(int)), this, SLOT(buildChannelList(void)));

    channelLayout->addWidget(new QLabel(tr("Type:")));
    typeList = new QComboBox;
    channelLayout->addWidget(typeList);
    typeList->addItem("Input");
    typeList->addItem("Output");
    typeList->addItem("Parameter");
    typeList->addItem("State");
    typeList->addItem("Event");
    QObject::connect(typeList,SIGNAL(activated(int)),this,SLOT(buildChannelList(void)));

    channelLayout->addWidget(new QLabel(tr("Channel:")));
    channelList = new QComboBox;
    channelLayout->addWidget(channelList);

    // Attach layout to child widget
    channelGroup->setLayout(channelLayout);

    // Create elements for arrow
    rButton = new QPushButton("Add");
    channelLayout->addWidget(rButton);
    QObject::connect(rButton,SIGNAL(released(void)),this,SLOT(insertChannel(void)));
    rButton->setEnabled(false);
    lButton = new QPushButton("Remove");
    channelLayout->addWidget(lButton);
    QObject::connect(lButton,SIGNAL(released(void)),this,SLOT(removeChannel(void)));
    lButton->setEnabled(false);

    // Timestamp
    stampGroup = new QGroupBox(tr("Tag Data"));
    QHBoxLayout *stampLayout = new QHBoxLayout;

    // Add timestamp elements
    timeStampEdit = new QLineEdit;
    stampLayout->addWidget(timeStampEdit);
    addTag = new QPushButton(tr("Tag"));
    stampLayout->addWidget(addTag);
    QObject::connect(addTag,SIGNAL(released(void)),this,SLOT(addNewTag(void)));

    // Attach layout to child widget
    stampGroup->setLayout(stampLayout);

    // Create child widget and layout
    sampleGroup = new QGroupBox(tr("Trial Metadata"));
    QHBoxLayout *sampleLayout = new QHBoxLayout;

    // create elements for sample box
    trialNumLbl = new QLabel;
    trialNumLbl->setText("Trial #:");
    trialNumLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    sampleLayout->addWidget(trialNumLbl);
    trialNum = new QLabel;
    trialNum->setText("0");
    trialNum->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    sampleLayout->addWidget(trialNum);

    trialLengthLbl = new QLabel;
    trialLengthLbl->setText("Trial Length (s):");
    sampleLayout->addWidget(trialLengthLbl);
    trialLength = new QLabel;
    trialLength->setText("No data recorded");
    trialLength->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    sampleLayout->addWidget(trialLength);

    fileSizeLbl = new QLabel;
    fileSizeLbl->setText("File Size (MB):");
    sampleLayout->addWidget(fileSizeLbl);
    fileSize = new QLabel;
    fileSize->setText("No data recorded");
    sampleLayout->addWidget(fileSize);
    fileSize->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    // Attach layout to child widget
    sampleGroup->setLayout(sampleLayout);

    // Create child widget and layout for file control
    fileGroup = new QGroupBox(tr("File Control"));
    QHBoxLayout *fileLayout = new QHBoxLayout;

    // Create elements for file control
    fileLayout->addWidget(new QLabel(tr("File Name:")));
    fileNameEdit = new QLineEdit;
    fileNameEdit->setReadOnly(true);
    fileLayout->addWidget(fileNameEdit);
    QPushButton *fileChangeButton = new QPushButton("Choose File");
    fileLayout->addWidget(fileChangeButton);
    QObject::connect(fileChangeButton,SIGNAL(released(void)),this,SLOT(changeDataFile(void)));

    fileLayout->addWidget(new QLabel(tr("Downsample \nRate:")));
    downsampleSpin = new QSpinBox(this);
    downsampleSpin->setMinimum(1);
    downsampleSpin->setMaximum(500);
    fileLayout->addWidget(downsampleSpin);
    QObject::connect(downsampleSpin,SIGNAL(valueChanged(int)),this,SLOT(updateDownsampleRate(int)));

    // Attach layout to child
    fileGroup->setLayout(fileLayout);

    // Create child widget and layout
    listGroup = new QGroupBox(tr("Currently Recording"));
    QGridLayout *listLayout = new QGridLayout;

    // Create elements for box
    selectionBox = new QListWidget;
    listLayout->addWidget(selectionBox,1,1,4,5);

    // Attach layout to child
    listGroup->setLayout(listLayout);

    // Creat child widget and layout for buttons
    buttonGroup = new QGroupBox;
    QHBoxLayout *buttonLayout = new QHBoxLayout;

    // Create elements for box
    startRecordButton = new QPushButton("Start Recording");
    QObject::connect(startRecordButton,SIGNAL(released(void)),this,SLOT(startRecordClicked(void)));
    buttonLayout->addWidget(startRecordButton);
    startRecordButton->setEnabled(false);
    stopRecordButton = new QPushButton("Stop Recording");
    QObject::connect(stopRecordButton,SIGNAL(released(void)),this,SLOT(stopRecordClicked(void)));
    buttonLayout->addWidget(stopRecordButton);
    stopRecordButton->setEnabled(false);
    closeButton = new QPushButton("Close");
    QObject::connect(closeButton,SIGNAL(released(void)),subWindow,SLOT(close()));
    buttonLayout->addWidget(closeButton);
    recordStatus = new QLabel;
    buttonLayout->addWidget(recordStatus);
    recordStatus->setText("Not ready.");
    recordStatus->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    recordStatus->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    // Attach layout to group
    buttonGroup->setLayout(buttonLayout);

    // Attach child widgets to parent
    layout->addWidget(channelGroup, 0, 0, 1, 2);
    layout->addWidget(listGroup, 0, 2, 1, 4);
    layout->addWidget(stampGroup, 2, 0, 2, 6);
    layout->addWidget(fileGroup, 4, 0, 1, 6);
    layout->addWidget(sampleGroup, 5, 0, 1, 6);
    layout->addWidget(buttonGroup, 6, 0, 1, 6);

    setLayout(layout);
    setWindowTitle(QString::number(getID()) + " Data Recorder");

    // Set layout to Mdi
    subWindow->setWidget(this);
    subWindow->setFixedSize(subWindow->minimumSizeHint());
    show();

    // Register custom QEvents
    QEvent::registerEventType(QFileExistsEvent);
    QEvent::registerEventType(QSetFileNameEditEvent);
    QEvent::registerEventType(QDisableGroupsEvent);
    QEvent::registerEventType(QEnableGroupsEvent);

    // Build initial block list
    IO::Connector::getInstance()->foreachBlock(buildBlockPtrList, &blockPtrList);
    for (std::vector<IO::Block *>::const_iterator i = blockPtrList.begin(), end = blockPtrList.end(); i != end; ++i)
        blockList->addItem(QString::fromStdString((*i)->getName()) + " " + QString::number((*i)->getID()));

    // Setup thread sleep
    sleep.tv_sec = 0;
    sleep.tv_nsec = RT::System::getInstance()->getPeriod();

    // Check if FIFO is truly atomic for hardware arcstartecture
    if(!fifo.isLockFree())
        ERROR_MSG("DataRecorder::Panel: WARNING: Atomic FIFO is not lock free\n");

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

// Destructor for Panel
DataRecorder::Panel::~Panel(void)
{
    Plugin::getInstance()->removeDataRecorderPanel(this);
    setActive(false);
    DoneEvent RTevent(fifo);
    while (RT::System::getInstance()->postEvent(&RTevent));
    pthread_join(thread, 0);
    for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i!= end;)
        delete &*(i++);
}

// Execute loop
void DataRecorder::Panel::execute(void)
{
    if (recording && !counter++)
        {
            data_token_t token;
            double data[channels.size()];

            size_t n = 0;
            token.type = SYNC;
            token.size = channels.size() * sizeof(double);
            for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
                if (i->block)
                    data[n++] = i->block->getValue(i->type, i->index);

            fifo.write(&token, sizeof(token));
            fifo.write(data, sizeof(data));
        }
    count++;
    counter %= downsample_rate;
}

// Event handler
void DataRecorder::Panel::receiveEvent(const Event::Object *event)
{
    if (event->getName() == Event::IO_BLOCK_INSERT_EVENT)
        {
            IO::Block *block = reinterpret_cast<IO::Block *> (event->getParam("block"));
            blockPtrList.push_back(block);
            blockList->addItem(QString::fromStdString(block->getName()) + " " + QString::number(block->getID()));
            buildChannelList();
        }
    else if (event->getName() == Event::IO_BLOCK_REMOVE_EVENT)
        {
            IO::Block *block = reinterpret_cast<IO::Block *> (event->getParam("block"));
            QString name = QString::fromStdString(block->getName()) + " " + QString::number(block->getID());
            int n = 0;
            for (; n < blockList->count() && blockList->itemText(n) != name; ++n) ;
            if (n < blockList->count()) 
                blockList->removeItem(n);
            blockPtrList.erase(blockPtrList.begin() + n);

            for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
                if (i->block == block) {
                    if (recording) i->block = 0;
                    RemoveChannelEvent RTevent(recording, channels, *i);
                    if (!RT::System::getInstance()->postEvent(&RTevent)) {
                        QList<QListWidgetItem*> channelItems = selectionBox->findItems(i->name, Qt::MatchExactly);
                        if (!channelItems.isEmpty()) {
                            /* Use takeItem(row) to remove the channel item. */
                            selectionBox->takeItem(selectionBox->row(channelItems.takeFirst()));
                        } 
                    }
                }
            buildChannelList();
        }
    else if (event->getName() == Event::OPEN_FILE_EVENT)
        {
            QString filename(reinterpret_cast<char*> (event->getParam("filename")));
            OpenFileEvent RTevent(filename, fifo);
            RT::System::getInstance()->postEvent(&RTevent);
        }
    else if (event->getName() == Event::START_RECORDING_EVENT)
        {
            StartRecordingEvent RTevent(recording, fifo);
            RT::System::getInstance()->postEvent(&RTevent);
        }
    else if (event->getName() == Event::STOP_RECORDING_EVENT)
        {
            StopRecordingEvent RTevent(recording, fifo);
            RT::System::getInstance()->postEvent(&RTevent);
        }
    else if (event->getName() == Event::ASYNC_DATA_EVENT)
        {
            AsyncDataEvent RTevent(reinterpret_cast<double *> (event->getParam("data")),*reinterpret_cast<size_t *> (event->getParam("size")), fifo);
            RT::System::getInstance()->postEvent(&RTevent);
        }
    else if( event->getName() == Event::RT_POSTPERIOD_EVENT )
        {
            sleep.tv_nsec = RT::System::getInstance()->getPeriod(); // Update recording thread sleep time
            buildChannelList();
        }
}

// RT Event Handler
void DataRecorder::Panel::receiveEventRT(const Event::Object *event)
{
    if (event->getName() == Event::OPEN_FILE_EVENT)
        {
            QString filename = QString(reinterpret_cast<char*> (event->getParam("filename")));
            data_token_t token;
            token.type = DataRecorder::OPEN;
            token.size = filename.length() + 1;
            token.time = RT::OS::getTime();
            fifo.write(&token, sizeof(token));
            fifo.write(filename.toLatin1().constData(), token.size);
        }
    else if (event->getName() == Event::START_RECORDING_EVENT)
        {
            data_token_t token;
            recording = true;
            token.type = DataRecorder::START;
            token.size = 0;
            token.time = RT::OS::getTime();
            fifo.write(&token, sizeof(token));
        }
    else if (event->getName() == Event::STOP_RECORDING_EVENT)
        {
            data_token_t token;
            recording = false;
            token.type = DataRecorder::STOP;
            token.size = 0;
            token.time = RT::OS::getTime();
            fifo.write(&token, sizeof(token));
        }
    else if (event->getName() == Event::ASYNC_DATA_EVENT)
        {
            size_t size = *reinterpret_cast<size_t *> (event->getParam("size"));
            data_token_t token;
            token.type = DataRecorder::ASYNC;
            token.size = size * sizeof(double);
            token.time = RT::OS::getTime();
            fifo.write(&token, sizeof(token));
            fifo.write(event->getParam("data"), token.size);
        }
    else if (event->getName() == Event::WORKSPACE_PARAMETER_CHANGE_EVENT)
        {
            data_token_t token;
            token.type = DataRecorder::PARAM;
            token.size = sizeof(param_change_t);
            token.time = RT::OS::getTime();
            param_change_t data;
            data.id = reinterpret_cast<Settings::Object::ID> (event->getParam("object"));
            data.index = reinterpret_cast<size_t> (event->getParam("index"));
            data.step = file.idx;
            data.value = *reinterpret_cast<double *> (event->getParam("value"));
            fifo.write(&token, sizeof(token));
            fifo.write(&data, sizeof(data));
        }
    else if( event->getName() == Event::RT_POSTPERIOD_EVENT )
        {
            sleep.tv_nsec = RT::System::getInstance()->getPeriod(); // Update recording thread sleep time
        }
}

// Populate list of blocks and channels
void DataRecorder::Panel::buildChannelList(void)
{
    channelList->clear();
    if (!blockList->count())
        return;

    // Get block
    IO::Block *block = blockPtrList[blockList->currentIndex()];

    // Get type
    IO::flags_t type;
    switch (typeList->currentIndex())
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
        case 4:
            type = Workspace::EVENT;
            break;
        default:
            ERROR_MSG("DataRecorder::Panel::buildChannelList : invalid type selection\n");
            typeList->setCurrentIndex(0);
            type = Workspace::INPUT;
        }

    for (size_t i = 0; i < block->getCount(type); ++i)
        channelList->addItem(QString::fromStdString(block->getName(type, i)));

    if(channelList->count())
        rButton->setEnabled(true);
    else
        rButton->setEnabled(false);
}

// Slot for changing data file
void DataRecorder::Panel::changeDataFile(void)
{
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::AnyFile);
    fileDialog.setWindowTitle("Select Data File");

    QSettings userprefs;
    userprefs.setPath(QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
    fileDialog.setDirectory(userprefs.value("/dirs/data", getenv("HOME")).toString());

    QStringList filterList;
    filterList.push_back("HDF5 files (*.h5)");
    filterList.push_back("All files (*.*)");
    fileDialog.setNameFilters(filterList);
    fileDialog.selectNameFilter("HDF5 files (*.h5)");

    QStringList files;
    if(fileDialog.exec())
        files = fileDialog.selectedFiles();

    QString filename;
    if(files.isEmpty() || files[0] == NULL || files[0] == "/" )
        return;
    else
        filename = files[0];

    if (!filename.toLower().endsWith(QString(".h5")))
        filename += ".h5";

    // Write this directory to the user prefs as most recently used
    userprefs.setValue("/dirs/data", fileDialog.directory().path());

    // Post to event queue
    OpenFileEvent RTevent(filename, fifo);
    RT::System::getInstance()->postEvent(&RTevent);
}

// Insert channel to record into list
void DataRecorder::Panel::insertChannel(void)
{
    if (!blockList->count() || !channelList->count())
        return;

    Channel *channel = new Channel();
    channel->block = blockPtrList[blockList->currentIndex()];
    switch (typeList->currentIndex())
        {
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
            typeList->setCurrentIndex(0);
            channel->type = Workspace::INPUT;
        }
    channel->index = channelList->currentIndex();

    channel->name.sprintf("%s %ld : %s", channel->block->getName().c_str(),
                          channel->block->getID(), channel->block->getName(channel->type, channel->index).c_str());

    if(selectionBox->findItems(QString(channel->name), Qt::MatchExactly).isEmpty())
        {
            InsertChannelEvent RTevent(recording, channels, channels.end(), *channel);
            if (!RT::System::getInstance()->postEvent(&RTevent))
                selectionBox->addItem(channel->name);
        }

    if(selectionBox->count())
        {
            lButton->setEnabled(true);
            if(!fileNameEdit->text().isEmpty())
                {
                    startRecordButton->setEnabled(true);
                }
        }
    else
        {
            startRecordButton->setEnabled(false);
            lButton->setEnabled(false);
        }
}

// Remove channel from recorder list
void DataRecorder::Panel::removeChannel(void)
{
    if(!selectionBox->count() || selectionBox->selectedItems().isEmpty())
        return;

    for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
        if (i->name == selectionBox->selectedItems().first()->text())
            {
                RemoveChannelEvent RTevent(recording, channels, *i);
                if (!RT::System::getInstance()->postEvent(&RTevent))
                    selectionBox->takeItem(selectionBox->row(selectionBox->selectedItems().first()));
                break;
            }

    if(selectionBox->count())
        {
            startRecordButton->setEnabled(true);
            lButton->setEnabled(true);
        }
    else
        {
            startRecordButton->setEnabled(false);
            lButton->setEnabled(false);
        }
}

// Register new data tag/stamp
void DataRecorder::Panel::addNewTag(void)
{
    std::string newTag(std::to_string(RT::OS::getTime()));
    newTag += ",";
    newTag += timeStampEdit->text().toStdString();
    dataTags.push_back(newTag);
    timeStampEdit->clear();
    recordStatus->setText("Tagged");
}

// Start recording slot
void DataRecorder::Panel::startRecordClicked(void)
{
    if(fileNameEdit->text().isEmpty())
        {
            QMessageBox::critical(
                this, "Data file not specified.",
                "Please specify a file to write data to.",
                QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }

    StartRecordingEvent RTevent(recording, fifo);
    RT::System::getInstance()->postEvent(&RTevent);
}

// Stop recording slot
void DataRecorder::Panel::stopRecordClicked(void)
{
    StopRecordingEvent RTevent(recording, fifo);
    RT::System::getInstance()->postEvent(&RTevent);
}

// Update downsample rate
void DataRecorder::Panel::updateDownsampleRate(int r)
{
    downsample_rate = r;
}

// Custom event handler
void DataRecorder::Panel::customEvent(QEvent *e)
{
    if (e->type() == QFileExistsEvent)
        {
            mutex.lock();
            CustomEvent * event = static_cast<CustomEvent *>(e);
            FileExistsEventData *data = reinterpret_cast<FileExistsEventData *> (event->getData());
            data->response = QMessageBox::question(this, "File exists",
                                                   "The file already exists. What would you like to do?",
                                                   "Append", "Overwrite", "Cancel", 0, 2);
            recordStatus->setText("Not Recording");
            data->done.wakeAll();
            mutex.unlock();
        }
    else if (e->type() == QSetFileNameEditEvent)
        {
            mutex.lock();
            CustomEvent * event = static_cast<CustomEvent *>(e);
            SetFileNameEditEventData *data = reinterpret_cast<SetFileNameEditEventData *> (event->getData());
            fileNameEdit->setText(data->filename);
            recordStatus->setText("Ready.");
            if(selectionBox->count())
                {
                    startRecordButton->setEnabled(true);
                }
            data->done.wakeAll();
            mutex.unlock();
        }
    else if (e->type() == QDisableGroupsEvent)
        {
            startRecordButton->setEnabled(false);
            stopRecordButton->setEnabled(true);
            closeButton->setEnabled(false);
            channelGroup->setEnabled(false);
            sampleGroup->setEnabled(false);
            recordStatus->setText("Recording...");
        }
    else if (e->type() == QEnableGroupsEvent)
        {
            startRecordButton->setEnabled(true);
            stopRecordButton->setEnabled(false);
            closeButton->setEnabled(true);
            channelGroup->setEnabled(true);
            sampleGroup->setEnabled(true);
            recordStatus->setText("Ready.");
            fileSize->setNum(int(QFile(fileNameEdit->text()).size())/1024.0/1024.0);
            trialLength->setNum(double(RT::System::getInstance()->getPeriod()*1e-9* fixedcount));
            count = 0;
        }
}

void DataRecorder::Panel::doDeferred(const Settings::Object::State &s)
{
    for (int i = 0; i < s.loadInteger("Num Channels"); ++i)
        {
            Channel *channel;
            IO::Block *block;
            std::ostringstream str;
            str << i;

            block	= dynamic_cast<IO::Block *> (Settings::Manager::getInstance()->getObject(s.loadInteger(str.str() + " ID")));
            if (!block)
                continue;

            channel = new Channel();
            channel->block = block;
            channel->type = s.loadInteger(str.str() + " type");
            channel->index = s.loadInteger(str.str() + " index");
            channel->name.sprintf("%s %ld : %s", channel->block->getName().c_str(),
                                  channel->block->getID(), channel->block->getName(channel->type,	channel->index).c_str());

            channels.insert(channels.end(), *channel);
            selectionBox->addItem(channel->name);
            if(selectionBox->count())
                lButton->setEnabled(true);
        }
}

void DataRecorder::Panel::doLoad(const Settings::Object::State &s)
{
    if (s.loadInteger("Maximized"))
        showMaximized();
    else if (s.loadInteger("Minimized"))
        showMinimized();

    downsampleSpin->setValue(s.loadInteger("Downsample"));
    parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
}

void DataRecorder::Panel::doSave(Settings::Object::State &s) const
{
    if (isMaximized())
        s.saveInteger("Maximized", 1);
    else if (isMinimized())
        s.saveInteger("Minimized", 1);

    QPoint pos = parentWidget()->pos();
    s.saveInteger("X", pos.x());
    s.saveInteger("Y", pos.y());

    s.saveInteger("Downsample", downsampleSpin->value());
    s.saveInteger("Num Channels", channels.size());
    size_t n = 0;
    for (RT::List<Channel>::const_iterator i = channels.begin(), end = channels.end(); i != end; ++i)
        {
            std::ostringstream str;
            str << n++;

            s.saveInteger(str.str() + " ID", i->block->getID());
            s.saveInteger(str.str() + " type", i->type);
            s.saveInteger(str.str() + " index", i->index);
        }
}

void *DataRecorder::Panel::bounce(void *param)
{
    Panel *that = reinterpret_cast<Panel *> (param);
    if (that)
        {
            that->processData();
        }
    return 0;
}

void DataRecorder::Panel::processData(void)
{
    enum
    {
        CLOSED, OPENED, RECORD,
    } state = CLOSED;

    tokenRetrieved = false;
    for (;;)
        {
            if(!tokenRetrieved)
                {
                    // Returns true if data was available and retrieved
                    if(fifo.read(&_token, sizeof(_token)))
                        tokenRetrieved = true;
                    else
                        {
                            // Sleep loop then restart if no token was retrieved
                            nanosleep(&sleep, NULL);
                            continue;
                        }
                }
            if (_token.type == SYNC)
                {
                    if (state == RECORD)
                        {
                            double data[_token.size / sizeof(double)];
                            if(!fifo.read(data, _token.size))
                                continue; // Restart loop if data is not available
                            H5PTappend(file.cdata, 1, data);
                            ++file.idx;
                        }
                }
            else if (_token.type == ASYNC)
                {
                    if (state == RECORD)
                        {
                            double data[_token.size / sizeof(double)];
                            if(!fifo.read(data, _token.size))
                                continue; // Restart loop if data is not available
                            if (data)
                                {
                                    hsize_t array_size[] = { _token.size / sizeof(double) };
                                    hid_t array_space = H5Screate_simple(1, array_size,	array_size);
                                    hid_t array_type = H5Tarray_create(H5T_IEEE_F64LE, 1,	array_size);

                                    QString data_name = QString::number(static_cast<unsigned long long> (_token.time));
                                    hid_t adata = H5Dcreate(file.adata, data_name.toLatin1().constData(),
                                                            array_type, array_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                                    H5Dwrite(adata, array_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

                                    H5Dclose(adata);
                                    H5Tclose(array_type);
                                    H5Sclose(array_space);
                                }
                        }
                }
            else if (_token.type == OPEN)
                {
                    if (state == RECORD)
                        stopRecording(_token.time);
                    if (state != CLOSED)
                        closeFile();
                    char filename_string[_token.size];
                    if(!fifo.read(filename_string, _token.size))
                        continue; // Restart loop if data is not available
                    QString filename = filename_string;
                    if (openFile(filename))
                        state = CLOSED;
                    else
                        state = OPENED;
                }
            else if (_token.type == CLOSE)
                {
                    if (state == RECORD)
                        stopRecording(RT::OS::getTime());
                    if (state != CLOSED)
                        closeFile();
                    state = CLOSED;
                }
            else if (_token.type == START)
                {
                    if (state == OPENED)
                        {
                            count = 0;
                            startRecording(_token.time);
                            state = RECORD;
                            QEvent *event = new QEvent(static_cast<QEvent::Type>QDisableGroupsEvent);
                            QApplication::postEvent(this, event);
                        }
                }
            else if (_token.type == STOP)
                {
                    if (state == RECORD)
                        {
                            stopRecording(_token.time);
                            state = OPENED;
                            fixedcount = count;
                            QEvent *event = new QEvent(static_cast<QEvent::Type>QEnableGroupsEvent);
                            QApplication::postEvent(this, event);
                        }
                }
            else if (_token.type == DONE)
                {
                    if (state == RECORD)
                        stopRecording(_token.time);
                    if (state != CLOSED)
                        closeFile(true);
                    break;
                }
            else if (_token.type == PARAM)
                {
                    param_change_t data;
                    if(!fifo.read(&data, sizeof(data)))
                        continue; // Restart loop if data is not available

                    IO::Block	*block = dynamic_cast<IO::Block *> (Settings::Manager::getInstance()->getObject(data.id));

                    if (block && state == RECORD)
                        {
                            param_hdf_t param = { data.step, data.value, };

                            hid_t param_type;
                            param_type = H5Tcreate(H5T_COMPOUND, sizeof(param_hdf_t));
                            H5Tinsert(param_type, "index", HOFFSET(param_hdf_t,index), H5T_STD_I64LE);
                            H5Tinsert(param_type, "value", HOFFSET(param_hdf_t,value), H5T_IEEE_F64LE);

                            QString parameter_name = QString::number(block->getID()) + " "
                                                     + QString::fromStdString(block->getName()) + " : "
                                                     + QString::fromStdString(block->getName(Workspace::PARAMETER, data.index));

                            hid_t data = H5PTopen(file.pdata, parameter_name.toLatin1().constData());
                            H5PTappend(data, 1, &param);
                            H5PTclose(data);
                            H5Tclose(param_type);
                        }
                }
            tokenRetrieved = false;
        }
}

int DataRecorder::Panel::openFile(QString &filename)
{
#ifdef DEBUG
    if(!pthread_equal(pthread_self(),thread))
        {
            ERROR_MSG("DataRecorder::Panel::openFile : called by invalid thread\n");
            PRINT_BACKTRACE();
        }
#endif

    if (QFile::exists(filename))
        {
            mutex.lock();
            CustomEvent *event = new CustomEvent(static_cast<QEvent::Type>QFileExistsEvent);
            FileExistsEventData data;
            event->setData(static_cast<void *>(&data));
            data.filename = filename;
            QApplication::postEvent(this, event);
            data.done.wait(&mutex);
            mutex.unlock();

            if (data.response == 0)   // append
                {
                    file.id = H5Fopen(filename.toLatin1().constData(), H5F_ACC_RDWR, H5P_DEFAULT);
                    size_t trial_num;
                    QString trial_name;
                    H5Eset_auto(H5E_DEFAULT, NULL, NULL);
                    for (trial_num = 1;; ++trial_num)
                        {
                            trial_name = "/Trial" + QString::number(trial_num);
                            file.trial = H5Gopen(file.id, trial_name.toLatin1().constData(), H5P_DEFAULT);
                            if (file.trial < 0)
                                {
                                    H5Eclear(H5E_DEFAULT);
                                    break;
                                }
                            else
                                {
                                    H5Gclose(file.trial);
                                }
                        }
                    trialNum->setNum(int(trial_num)-1);
                }
            else if (data.response == 1)     //overwrite
                {
                    file.id = H5Fcreate(filename.toLatin1().constData(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
                    trialNum->setText("0");
                }
            else
                {
                    return -1;
                }
        }
    else
        {
            file.id = H5Fcreate(filename.toLatin1().constData(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
            trialNum->setText("0");
        }
    if (file.id < 0)
        {
            H5E_type_t error_type;
            size_t error_size;
            error_size = H5Eget_msg(file.id, &error_type, NULL, 0);
            char error_msg[error_size + 1];
            H5Eget_msg(file.id, &error_type, error_msg, error_size);
            error_msg[error_size] = 0;
            H5Eclear(file.id);

            ERROR_MSG("DataRecorder::Panel::processData : failed to open \"%s\" for writing with error : %s\n", filename.toStdString().c_str(),error_msg);
            return -1;
        }

    mutex.lock();
    CustomEvent *event = new CustomEvent(static_cast<QEvent::Type>QSetFileNameEditEvent);
    SetFileNameEditEventData data;
    data.filename = filename;
    event->setData(static_cast<void*>(&data));
    QApplication::postEvent(this, event);
    data.done.wait(&mutex);
    mutex.unlock();

    return 0;
}

void DataRecorder::Panel::closeFile(bool shutdown)
{
#ifdef DEBUG
    if(!pthread_equal(pthread_self(),thread))
        {
            ERROR_MSG("DataRecorder::Panel::closeFile : called by invalid thread\n");
            PRINT_BACKTRACE();
        }
#endif

				if(!dataTags.empty())
				{
    // Write tags to data file
    hid_t tag_type, tag_space, data;
    herr_t status;
    hsize_t dims[1] = {1};
    tag_type = H5Tcreate(H5T_STRING, TAG_SIZE);
    tag_space = H5Screate_simple(1, dims, NULL);

    // Create group for tags
    file.tdata = H5Gcreate(file.id, "Tags", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Iterate over vector (buffer) and put into data file
    size_t i = 0;
    for(std::vector<std::string>::iterator it = dataTags.begin(); it != dataTags.end(); ++it)
        {
            data = H5Dcreate(file.tdata, std::string("Tag " + std::to_string(i++)).c_str(), tag_type, tag_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            status = H5Dwrite(data, tag_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, it->c_str());
        }
    dataTags.clear();

    // Close all open structs
    H5Dclose(data);
    H5Sclose(tag_space);
    H5Tclose(tag_type);
    H5Gclose(file.tdata);
				}

    // Close file
    H5Fclose(file.id);

    if (!shutdown)
        {
            mutex.lock();
            CustomEvent *event = new CustomEvent(static_cast<QEvent::Type>QSetFileNameEditEvent);
            SetFileNameEditEventData data;
            data.filename = "";
            event->setData(static_cast<void*>(&data));
            QApplication::postEvent(this, event);
            data.done.wait(&mutex);
            mutex.unlock();
        }
}

int DataRecorder::Panel::startRecording(long long timestamp)
{
#ifdef DEBUG
    if(!pthread_equal(pthread_self(),thread))
        {
            ERROR_MSG("DataRecorder::Panel::startRecording : called by invalid thread\n");
            PRINT_BACKTRACE();
        }
#endif

    size_t trial_num;
    QString trial_name;

    H5Eset_auto(H5E_DEFAULT, NULL, NULL);

    for (trial_num = 1;; ++trial_num)
        {
            trial_name = "/Trial" + QString::number(trial_num);
            file.trial = H5Gopen(file.id, trial_name.toLatin1().constData(), H5P_DEFAULT);

            if (file.trial < 0)
                {
                    H5Eclear(H5E_DEFAULT);
                    break;
                }
            else
                H5Gclose(file.trial);
        }

    trialNum->setNum(int(trial_num));
    file.trial = H5Gcreate(file.id, trial_name.toLatin1().constData(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    file.pdata = H5Gcreate(file.trial, "Parameters", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    file.adata = H5Gcreate(file.trial, "Asynchronous Data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    file.sdata = H5Gcreate(file.trial, "Synchronous Data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    file.sysdata = H5Gcreate(file.trial, "System Settings", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    hid_t scalar_space = H5Screate(H5S_SCALAR);
    hid_t string_type = H5Tcopy(H5T_C_S1);
    size_t string_size = 1024;
    H5Tset_size(string_type, string_size);
    hid_t data;

    data = H5Dcreate(file.trial, "Version", string_type,
                     scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		std::string version_string = QString(VERSION).toStdString();
		char * version_c_string = new char[version_string.length()+1];
		std::strcpy(version_c_string, version_string.c_str());
    H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, version_c_string);
    delete[] version_c_string;
    H5Dclose(data);

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

    data = H5Dcreate(file.trial, "Date", string_type,
                     scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
		std::string date_string = QDateTime::currentDateTime().toString(Qt::ISODate).toStdString();
		char * date_c_string = new char[date_string.length()+1];
		std::strcpy(date_c_string, date_string.c_str());
    H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, date_c_string);
    delete[] date_c_string;
    H5Dclose(data);

    hid_t param_type;
    param_type = H5Tcreate(H5T_COMPOUND, sizeof(param_hdf_t));
    H5Tinsert(param_type, "index", HOFFSET(param_hdf_t,index), H5T_STD_I64LE);
    H5Tinsert(param_type, "value", HOFFSET(param_hdf_t,value), H5T_IEEE_F64LE);

    for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
        {
            IO::Block *block = i->block;
            for (size_t j = 0; j < block->getCount(Workspace::PARAMETER); ++j)
                {
                    QString parameter_name = QString::number(block->getID()) + " "
                                             + QString::fromStdString(block->getName()) + " : " + QString::fromStdString(block->getName(Workspace::PARAMETER, j));
                    data = H5PTcreate_fl(file.pdata, parameter_name.toLatin1().constData(),	param_type, sizeof(param_hdf_t), -1);
                    struct param_hdf_t value = { 0, block->getValue(Workspace::PARAMETER, j),};
                    H5PTappend(data, 1, &value);
                    H5PTclose(data);
                }
            for (size_t j = 0; j < block->getCount(Workspace::COMMENT); ++j)
                {
                    QString comment_name = QString::number(block->getID()) + " "
                                           + QString::fromStdString(block->getName()) + " : " + QString::fromStdString(block->getName(Workspace::COMMENT, j));
                    hsize_t	dims = dynamic_cast<Workspace::Instance *> (block)->getValueString(Workspace::COMMENT, j).size() + 1;
                    hid_t comment_space = H5Screate_simple(1, &dims, &dims);
                    data = H5Dcreate(file.pdata, comment_name.toLatin1().constData(), H5T_C_S1,	comment_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
                    H5Dwrite(data, H5T_C_S1, H5S_ALL, H5S_ALL, H5P_DEFAULT,	dynamic_cast<Workspace::Instance *> (block)->getValueString(Workspace::COMMENT, j).c_str());
                    H5Dclose(data);
                }
        }

    H5Tclose(param_type);

    size_t count = 0;
    for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end(); i != end; ++i)
        {
            std::string rec_chan_name = std::to_string(++count) + " " + i->name.toStdString();
            rec_chan_name.erase(std::remove_if(rec_chan_name.begin(), rec_chan_name.end(), &ispunct), rec_chan_name.end());
            hid_t data = H5Dcreate(file.sdata, rec_chan_name.c_str(), string_type, scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
            H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, rec_chan_name.c_str());
            H5Dclose(data);
        }

    DAQ::Device *dev;
    {
        struct find_daq_t info = { 0, 0, };
        DAQ::Manager::getInstance()->foreachDevice(findDAQDevice, &info);
        dev = info.device;
    }

				// Save channel configurations
				if(dev)
					for(size_t i=0; i<dev->getChannelCount(DAQ::AI); ++i)
						if(dev->getChannelActive(DAQ::AI,static_cast<DAQ::index_t>(i)))
						{
							std::string chan_name = "Analog Channel " + std::to_string(i);
							file.chandata = H5Gcreate(file.sysdata, chan_name.c_str(), H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

							hid_t data = H5Dcreate(file.chandata, "Range", string_type, scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
							std::string range_string = dev->getAnalogRangeString(DAQ::AI,static_cast<DAQ::index_t>(i),dev->getAnalogRange(DAQ::AI,static_cast<DAQ::index_t>(i)));
							char * range_c_string = new char[range_string.length()+1];
							std::strcpy(range_c_string, range_string.c_str());
							H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, range_c_string);
							delete[] range_c_string;

							data = H5Dcreate(file.chandata, "Reference", string_type, scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
							std::string ref_string = dev->getAnalogReferenceString(DAQ::AI,static_cast<DAQ::index_t>(i),dev->getAnalogReference(DAQ::AI,static_cast<DAQ::index_t>(i)));
							char * ref_c_string = new char[ref_string.length()+1];
							std::strcpy(ref_c_string, ref_string.c_str());
							H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, ref_c_string);
							delete[] ref_c_string;

							double scale = dev->getAnalogGain(DAQ::AI,static_cast<DAQ::index_t>(i));
							data = H5Dcreate(file.chandata, "Gain", H5T_IEEE_F64LE, scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
							H5Dwrite(data, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &scale); 

							double offset = dev->getAnalogZeroOffset(DAQ::AI,static_cast<DAQ::index_t>(i));
							data = H5Dcreate(file.chandata, "Offset", H5T_IEEE_F64LE, scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
							H5Dwrite(data, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &offset); 

							int downsample = dev->getAnalogDownsample(DAQ::AI,static_cast<DAQ::index_t>(i));
							data = H5Dcreate(file.chandata, "Downsample", H5T_STD_I16LE, scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
							H5Dwrite(data, H5T_STD_I16LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &downsample);
							H5Dclose(data);
						}

    H5Tclose(string_type);
    H5Sclose(scalar_space);

    if (channels.size())
        {
            hsize_t array_size[] = { channels.size() };
            hid_t array_type = H5Tarray_create(H5T_IEEE_F64LE, 1, array_size);
            file.cdata = H5PTcreate_fl(file.sdata, "Channel Data", array_type, (hsize_t) 64, 1);
            H5Tclose(array_type);
        }

    file.idx = 0;

    return 0;
}

void DataRecorder::Panel::stopRecording(long long timestamp)
{
#ifdef DEBUG
    if(!pthread_equal(pthread_self(),thread))
        {
            ERROR_MSG("DataRecorder::Panel::stopRecording : called by invalid thread\n");
            PRINT_BACKTRACE();
        }
#endif

    // Write stop time to data file
    hid_t scalar_space = H5Screate(H5S_SCALAR);
    hid_t data = H5Dcreate(file.trial, "Timestamp Stop (ns)", H5T_STD_U64LE,
                           scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &timestamp);
    H5Dclose(data);

    // Write trial length to data file
    fixedcount = count;
    long long period = RT::System::getInstance()->getPeriod();
    long long datalength = period * fixedcount;
    data = H5Dcreate(file.trial, "Trial Length (ns)", H5T_STD_U64LE,
                     scalar_space, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
    H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &datalength);
    H5Dclose(data);

    // Close all open structs
    H5Sclose(scalar_space);
    H5PTclose(file.cdata);
    H5Gclose(file.sdata);
    H5Gclose(file.pdata);
    H5Gclose(file.adata);
    H5Gclose(file.sysdata);
    H5Gclose(file.chandata);
    H5Gclose(file.trial);

    H5Fflush(file.id, H5F_SCOPE_LOCAL);
    void *file_handle;
    H5Fget_vfd_handle(file.id, H5P_DEFAULT, &file_handle);
    if (fsync(*static_cast<int *> (file_handle)))
        {
            DEBUG_MSG("DataRecorder::Panel::stopRecording : fsync failed, running sync\n");
            sync();
        }
}

extern "C" Plugin::Object *createRTXIPlugin(void *)
{
    return DataRecorder::Plugin::getInstance();
}

DataRecorder::Plugin::Plugin(void)
{
    // get the HDF data recorder buffer size from user preference
    QSettings userprefs;
    userprefs.setPath(QSettings::NativeFormat, QSettings::SystemScope, "/usr/local/share/rtxi/");
    buffersize = (userprefs.value("/system/HDFbuffer", 10).toInt())*1048576;
    MainWindow::getInstance()->createSystemMenuItem("Data Recorder",this,SLOT(createDataRecorderPanel(void)));
}

DataRecorder::Plugin::~Plugin(void)
{
    while (panelList.size())
        delete panelList.front();
    instance = 0;
}

DataRecorder::Panel *DataRecorder::Plugin::createDataRecorderPanel(void)
{
    Panel *panel = new Panel(MainWindow::getInstance()->centralWidget(), buffersize);
    panelList.push_back(panel);
    return panel;
}

void DataRecorder::Plugin::removeDataRecorderPanel(DataRecorder::Panel *panel)
{
    panelList.remove(panel);
}

void DataRecorder::Plugin::doDeferred(const Settings::Object::State &s)
{
    size_t i = 0;
    for (std::list<Panel *>::iterator j = panelList.begin(), end = panelList.end(); j != end; ++j)
        (*j)->deferred(s.loadState(QString::number(i++).toStdString()));
}

void DataRecorder::Plugin::doLoad(const Settings::Object::State &s)
{
    for (size_t i = 0; i < static_cast<size_t> (s.loadInteger("Num Panels")); ++i)
        {
            Panel *panel = new Panel(MainWindow::getInstance()->centralWidget(), buffersize);
            panelList.push_back(panel);
            panel->load(s.loadState(QString::number(i).toStdString()));
        }
}

void DataRecorder::Plugin::doSave(Settings::Object::State &s) const
{
    s.saveInteger("Num Panels", panelList.size());
    size_t n = 0;
    for (std::list<Panel *>::const_iterator i = panelList.begin(), end = panelList.end(); i != end; ++i)
        s.saveState(QString::number(n++).toStdString(), (*i)->save());
}

static Mutex mutex;
DataRecorder::Plugin *DataRecorder::Plugin::instance = 0;

DataRecorder::Plugin *DataRecorder::Plugin::getInstance(void)
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
