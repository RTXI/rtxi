/*
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Weill Cornell Medical College

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

#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

#include <unistd.h>

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

#include "debug.hpp"
#include "main_window.hpp"
#include "data_recorder.h"

struct param_hdf_t
{
  long long index;
  double value;
};

struct find_daq_t
{
  int index;
  DAQ::Device* device;
};

DataRecorder::Panel::Panel(MainWindow* mwindow, Event::Manager* ev_manager) 
    : Modules::Panel(std::string(DataRecorder::MODULE_NAME), mwindow, ev_manager)
{
  setWhatsThis(
      "<p><b>Data Recorder:</b><br>The Data Recorder writes data to an HDF5 "
      "file format "
      "All available signals for saving to file are automatically detected. "
      "Currently "
      "loaded user modules are listed in the \"Block\" drop-down box. "
      "Available DAQ cards "
      "are listed here as /proc/analogy/devices. Use the \"Type\" and "
      "\"Channel\" drop-down boxes "
      "to select the signals that you want to save. Use the left and right "
      "arrow buttons to "
      "add these signals to the file. You may select a downsampling rate that "
      "is applied "
      "to the real-time period for execution (set in the System Control "
      "Panel). The real-time "
      "period and the data downsampling rate are both saved as metadata in the "
      "HDF5 file "
      "so that you can reconstruct your data correctly. The current recording "
      "status of "
      "the Data Recorder is shown at the bottom.</p>");

  // Make Mdi
  subWindow = new QMdiSubWindow;
  subWindow->setWindowIcon(QIcon("/usr/local/share/rtxi/RTXI-widget-icon.png"));
  subWindow->setAttribute(Qt::WA_DeleteOnClose);
  subWindow->setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowCloseButtonHint
                            | Qt::WindowMinimizeButtonHint);
  mwindow->createMdi(subWindow);

  // Create main layout
  QGridLayout* layout = new QGridLayout;

  // Create child widget and layout for channel selection
  channelGroup = new QGroupBox(tr("Channel Selection"));
  QVBoxLayout* channelLayout = new QVBoxLayout;

  // Create elements for channel box
  channelLayout->addWidget(new QLabel(tr("Block:")));
  blockList = new QComboBox;
  channelLayout->addWidget(blockList);
  QObject::connect(
      blockList, SIGNAL(activated(int)), this, SLOT(buildChannelList()));

  channelLayout->addWidget(new QLabel(tr("Type:")));
  typeList = new QComboBox;
  channelLayout->addWidget(typeList);
  typeList->addItem("Input");
  typeList->addItem("Output");
  typeList->addItem("Parameter");
  typeList->addItem("State");
  typeList->addItem("Event");
  QObject::connect(
      typeList, SIGNAL(activated(int)), this, SLOT(buildChannelList()));

  channelLayout->addWidget(new QLabel(tr("Channel:")));
  channelList = new QComboBox;
  channelLayout->addWidget(channelList);

  // Attach layout to child widget
  channelGroup->setLayout(channelLayout);

  // Create elements for arrow
  rButton = new QPushButton("Add");
  channelLayout->addWidget(rButton);
  QObject::connect(
      rButton, SIGNAL(released()), this, SLOT(insertChannel()));
  rButton->setEnabled(false);
  lButton = new QPushButton("Remove");
  channelLayout->addWidget(lButton);
  QObject::connect(
      lButton, SIGNAL(released()), this, SLOT(removeChannel()));
  lButton->setEnabled(false);

  // Timestamp
  stampGroup = new QGroupBox(tr("Tag Data"));
  QHBoxLayout* stampLayout = new QHBoxLayout;

  // Add timestamp elements
  timeStampEdit = new QLineEdit;
  stampLayout->addWidget(timeStampEdit);
  addTag = new QPushButton(tr("Tag"));
  stampLayout->addWidget(addTag);
  QObject::connect(addTag, SIGNAL(released()), this, SLOT(addNewTag()));

  // Attach layout to child widget
  stampGroup->setLayout(stampLayout);

  // Create child widget and layout
  sampleGroup = new QGroupBox(tr("Trial Metadata"));
  QHBoxLayout* sampleLayout = new QHBoxLayout;

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
  QHBoxLayout* fileLayout = new QHBoxLayout;

  // Create elements for file control
  fileLayout->addWidget(new QLabel(tr("File Name:")));
  fileNameEdit = new QLineEdit;
  fileNameEdit->setReadOnly(true);
  fileLayout->addWidget(fileNameEdit);
  QPushButton* fileChangeButton = new QPushButton("Choose File");
  fileLayout->addWidget(fileChangeButton);
  QObject::connect(fileChangeButton,
                   SIGNAL(released()),
                   this,
                   SLOT(changeDataFile()));

  fileLayout->addWidget(new QLabel(tr("Downsample \nRate:")));
  downsampleSpin = new QSpinBox(this);
  downsampleSpin->setMinimum(1);
  downsampleSpin->setMaximum(500);
  fileLayout->addWidget(downsampleSpin);
  QObject::connect(downsampleSpin,
                   SIGNAL(valueChanged(int)),
                   this,
                   SLOT(updateDownsampleRate(int)));

  // Attach layout to child
  fileGroup->setLayout(fileLayout);

  // Create child widget and layout
  listGroup = new QGroupBox(tr("Currently Recording"));
  QGridLayout* listLayout = new QGridLayout;

  // Create elements for box
  selectionBox = new QListWidget;
  listLayout->addWidget(selectionBox, 1, 1, 4, 5);

  // Attach layout to child
  listGroup->setLayout(listLayout);

  // Creat child widget and layout for buttons
  buttonGroup = new QGroupBox;
  QHBoxLayout* buttonLayout = new QHBoxLayout;

  // Create elements for box
  startRecordButton = new QPushButton("Start Recording");
  QObject::connect(startRecordButton,
                   SIGNAL(released()),
                   this,
                   SLOT(startRecordClicked()));
  buttonLayout->addWidget(startRecordButton);
  startRecordButton->setEnabled(false);
  stopRecordButton = new QPushButton("Stop Recording");
  QObject::connect(stopRecordButton,
                   SIGNAL(released()),
                   this,
                   SLOT(stopRecordClicked()));
  buttonLayout->addWidget(stopRecordButton);
  stopRecordButton->setEnabled(false);
  closeButton = new QPushButton("Close");
  QObject::connect(
      closeButton, SIGNAL(released()), subWindow, SLOT(close()));
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
  setWindowTitle(tr(std::string(DataRecorder::MODULE_NAME).c_str()));

  // Set layout to Mdi
  subWindow->setWidget(this);
  subWindow->setFixedSize(subWindow->minimumSizeHint());
  show();

  // Build initial block list
  Event::Object event(Event::Type::IO_BLOCK_QUERY_EVENT);
  this->getRTXIEventManager()->postEvent(&event); 
  auto blockPtrList = std::any_cast<std::vector<IO::Block*>>(event.getParam("blockList"));
  for (auto blockptr : blockPtrList){
    blockList->addItem(QString::fromStdString(blockptr->getName()) + " "
                       + QString::number(blockptr->getID()));
  }

  // Build initial channel list
  buildChannelList();

  // Launch Recording Thread
  counter = 0;
  downsample_rate = 1;
  prev_input = 0.0;
  count = 0;
}

// Destructor for Panel
DataRecorder::Panel::~Panel()
{
  this->stopRecording();
}

// Execute loop
void DataRecorder::Component::execute()
{

}

// Event handler
void DataRecorder::Plugin::receiveEvent(Event::Object* event)
{

}

// Populate list of blocks and channels
void DataRecorder::Panel::buildChannelList()
{
  channelList->clear();
  if (!blockList->count())
    return;

  // Get block
  IO::Block* block = blockPtrList.at(static_cast<size_t>(blockList->currentIndex()));

  // Get type
  IO::flags_t type = static_cast<IO::flags_t>(this->typeList->currentIndex());
  for (size_t i = 0; i < block->getCount(type); ++i){
    channelList->addItem(QString::fromStdString(block->getChannelName(type, i)));
  }

  if (channelList->count())
    rButton->setEnabled(true);
  else
    rButton->setEnabled(false);
}

// Slot for changing data file
void DataRecorder::Panel::changeDataFile()
{
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.setWindowTitle("Select Data File");

  QSettings userprefs;
  userprefs.setPath(QSettings::NativeFormat,
                    QSettings::SystemScope,
                    "/usr/local/share/rtxi/");
  fileDialog.setDirectory(
      userprefs.value("/dirs/data", getenv("HOME")).toString());

  QStringList filterList;
  filterList.push_back("HDF5 files (*.h5)");
  filterList.push_back("All files (*.*)");
  fileDialog.setNameFilters(filterList);
  fileDialog.selectNameFilter("HDF5 files (*.h5)");

  QStringList files;
  if (fileDialog.exec()){
    files = fileDialog.selectedFiles();
  }

  QString filename;
  if (files.isEmpty() || files[0] == NULL || files[0] == "/")
    return;
  else
    filename = files[0];

  if (!filename.toLower().endsWith(QString(".h5")))
    filename += ".h5";

  // Write this directory to the user prefs as most recently used
  userprefs.setValue("/dirs/data", fileDialog.directory().path());

  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->change_file(filename.toStdString());
}

// Insert channel to record into list
void DataRecorder::Panel::insertChannel()
{
  if (!blockList->count() || !channelList->count())
    return;

  IO::endpoint endpoint;
  IO::Block* block = blockPtrList.at(static_cast<size_t>(blockList->currentIndex()));
  IO::flags_t direction = direction = static_cast<IO::flags_t>(typeList->currentIndex());
  size_t port = static_cast<size_t>(channelList->currentIndex());

  endpoint = {block, port, direction};
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  int result = hplugin->insertChannel(endpoint);

  if(result != 0){
    ERROR_MSG("DataRecorder::Panel::insertChannel : Unable to create recording component");
    return;
  }

  this->channels = hplugin->syncChannels();
  //channel.name += block->getName();
  //channel.name += " ";
  //channel.name += std::to_string(block->getID());
  //channel.name += " : ";
  //channel.name += block->getChannelName(direction, port);

  //this->channels.push_back(std::move(channel));
  if (selectionBox->count()) {
    lButton->setEnabled(true);
    if (!fileNameEdit->text().isEmpty()) {
      startRecordButton->setEnabled(true);
    }
  } else {
    startRecordButton->setEnabled(false);
    lButton->setEnabled(false);
  }
}

// Remove channel from recorder list
void DataRecorder::Panel::removeChannel()
{
  if (!selectionBox->count() || selectionBox->selectedItems().isEmpty()) { return; }

  size_t indx = static_cast<size_t>(this->selectionBox->currentRow());

  auto iter = std::find(this->channels.begin(),
                        this->channels.end(),
                        this->channels.at(indx).endpoint);
  if(iter == this->channels.end()) { return; }
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->removeChannel(iter->endpoint);
  this->channels = hplugin->syncChannels();
  if (selectionBox->count()) {
    startRecordButton->setEnabled(true);
    lButton->setEnabled(true);
  } else {
    startRecordButton->setEnabled(false);
    lButton->setEnabled(false);
  }
}

// Register new data tag/stamp
void DataRecorder::Panel::addNewTag()
{
  //std::string newTag(std::to_string(RT::OS::getTime()));
  auto* hplugin = static_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  std::string tag = this->timeStampEdit->text().toStdString();
  if(hplugin->apply_tag(tag) != 0){
    ERROR_MSG("DataRecorder::Panel::addNewTag : could not tag data with tag {}",
              tag);
    timeStampEdit->clear();
    recordStatus->setText("Tagging Failed!");
    return;
  }
  timeStampEdit->clear();
  recordStatus->setText("Tagged");
}

// Start recording slot
void DataRecorder::Panel::startRecordClicked()
{
  if (fileNameEdit->text().isEmpty()) {
    QMessageBox::critical(this,
                          "Data file not specified.",
                          "Please specify a file to write data to.",
                          QMessageBox::Ok,
                          QMessageBox::NoButton);
    return;
  }

  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->startRecording();
}

// Stop recording slot
void DataRecorder::Panel::stopRecordClicked()
{
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->stopRecording();
}

// Update downsample rate
void DataRecorder::Panel::updateDownsampleRate(size_t rate)
{
  this->downsample_rate = rate;
}

// Custom event handler
//void DataRecorder::Panel::customEvent(QEvent* e)
//{
//  if (e->type() == QFileExistsEvent) {
//    mutex.lock();
//    CustomEvent* event = static_cast<CustomEvent*>(e);
//    FileExistsEventData* data =
//        reinterpret_cast<FileExistsEventData*>(event->getData());
//    data->response = QMessageBox::question(
//        this,
//        "File exists",
//        "The file already exists. What would you like to do?",
//        "Append",
//        "Overwrite",
//        "Cancel",
//        0,
//        2);
//    recordStatus->setText("Not Recording");
//    data->done.wakeAll();
//    mutex.unlock();
//  } else if (e->type() == QSetFileNameEditEvent) {
//    mutex.lock();
//    CustomEvent* event = static_cast<CustomEvent*>(e);
//    SetFileNameEditEventData* data =
//        reinterpret_cast<SetFileNameEditEventData*>(event->getData());
//    fileNameEdit->setText(data->filename);
//    recordStatus->setText("Ready.");
//    if (selectionBox->count()) {
//      startRecordButton->setEnabled(true);
//    }
//    data->done.wakeAll();
//    mutex.unlock();
//  } else if (e->type() == QDisableGroupsEvent) {
//    startRecordButton->setEnabled(false);
//    stopRecordButton->setEnabled(true);
//    closeButton->setEnabled(false);
//    channelGroup->setEnabled(false);
//    sampleGroup->setEnabled(false);
//    recordStatus->setText("Recording...");
//  } else if (e->type() == QEnableGroupsEvent) {
//    startRecordButton->setEnabled(true);
//    stopRecordButton->setEnabled(false);
//    closeButton->setEnabled(true);
//    channelGroup->setEnabled(true);
//    sampleGroup->setEnabled(true);
//    recordStatus->setText("Ready.");
//    fileSize->setNum(int(QFile(fileNameEdit->text()).size()) / 1024.0 / 1024.0);
//    trialLength->setNum(
//        double(RT::System::getInstance()->getPeriod() * 1e-9 * fixedcount));
//    count = 0;
//  }
//}

//void* DataRecorder::Panel::bounce(void* param)
//{
//  Panel* that = reinterpret_cast<Panel*>(param);
//  if (that) {
//    that->processData();
//  }
//  return 0;
//}

void DataRecorder::Panel::processData()
{
  tokenRetrieved = false;
  for (;;) {
    if (!tokenRetrieved) {
      // Returns true if data was available and retrieved
      if (fifo.read(&_token, sizeof(_token)))
        tokenRetrieved = true;
      else {
        // Sleep loop then restart if no token was retrieved
        nanosleep(&sleep, NULL);
        continue;
      }
    }
    if (_token.type == SYNC) {
      if (state == RECORD) {
        double data[_token.size / sizeof(double)];
        if (!fifo.read(data, _token.size))
          continue;  // Restart loop if data is not available
        H5PTappend(file.cdata, 1, data);
        ++file.idx;
      }
    } else if (_token.type == ASYNC) {
      if (state == RECORD) {
        double data[_token.size / sizeof(double)];
        if (!fifo.read(data, _token.size))
          continue;  // Restart loop if data is not available
        if (data) {
          hsize_t array_size[] = {_token.size / sizeof(double)};
          hid_t array_space = H5Screate_simple(1, array_size, array_size);
          hid_t array_type = H5Tarray_create(H5T_IEEE_F64LE, 1, array_size);

          QString data_name =
              QString::number(static_cast<unsigned long long>(_token.time));
          hid_t adata = H5Dcreate(file.adata,
                                  data_name.toLatin1().constData(),
                                  array_type,
                                  array_space,
                                  H5P_DEFAULT,
                                  H5P_DEFAULT,
                                  H5P_DEFAULT);
          H5Dwrite(adata, array_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, data);

          H5Dclose(adata);
          H5Tclose(array_type);
          H5Sclose(array_space);
        }
      }
    } else if (_token.type == OPEN) {
      if (state == RECORD)
        stopRecording(_token.time);
      if (state != CLOSED)
        closeFile();
      char filename_string[_token.size];
      if (!fifo.read(filename_string, _token.size))
        continue;  // Restart loop if data is not available
      QString filename = filename_string;
      if (openFile(filename))
        state = CLOSED;
      else
        state = OPENED;
    } else if (_token.type == CLOSE) {
      if (state == RECORD)
        stopRecording(RT::OS::getTime());
      if (state != CLOSED)
        closeFile();
      state = CLOSED;
    } else if (_token.type == START) {
      if (state == OPENED) {
        count = 0;
        startRecording(_token.time);
        state = RECORD;
        QEvent* event =
            new QEvent(static_cast<QEvent::Type> QDisableGroupsEvent);
        QApplication::postEvent(this, event);
      }
    } else if (_token.type == STOP) {
      if (state == RECORD) {
        stopRecording(_token.time);
        state = OPENED;
        fixedcount = count;
        QEvent* event =
            new QEvent(static_cast<QEvent::Type> QEnableGroupsEvent);
        QApplication::postEvent(this, event);
      }
    } else if (_token.type == DONE) {
      if (state == RECORD)
        stopRecording(_token.time);
      if (state != CLOSED)
        closeFile(true);
      break;
    } else if (_token.type == PARAM) {
      param_change_t data;
      if (!fifo.read(&data, sizeof(data)))
        continue;  // Restart loop if data is not available

      IO::Block* block = dynamic_cast<IO::Block*>(
          Settings::Manager::getInstance()->getObject(data.id));

      if (block && state == RECORD) {
        param_hdf_t param = {
            data.step,
            data.value,
        };

        hid_t param_type;
        param_type = H5Tcreate(H5T_COMPOUND, sizeof(param_hdf_t));
        H5Tinsert(
            param_type, "index", HOFFSET(param_hdf_t, index), H5T_STD_I64LE);
        H5Tinsert(
            param_type, "value", HOFFSET(param_hdf_t, value), H5T_IEEE_F64LE);

        QString parameter_name = QString::number(block->getID()) + " "
            + QString::fromStdString(block->getName()) + " : "
            + QString::fromStdString(block->getName(Workspace::PARAMETER,
                                                    data.index));

        hid_t data =
            H5PTopen(file.pdata, parameter_name.toLatin1().constData());
        H5PTappend(data, 1, &param);
        H5PTclose(data);
        H5Tclose(param_type);
      }
    }
    tokenRetrieved = false;
  }
}

int DataRecorder::Panel::openFile(QString& filename)
{
#ifdef DEBUG
  if (!pthread_equal(pthread_self(), thread)) {
    std::cout << "DataRecorder::Panel::openFile : called by invalid thread\n";
    PRINT_BACKTRACE();
  }
#endif

  if (QFile::exists(filename)) {
    mutex.lock();
    CustomEvent* event =
        new CustomEvent(static_cast<QEvent::Type> QFileExistsEvent);
    FileExistsEventData data;
    event->setData(static_cast<void*>(&data));
    data.filename = filename;
    QApplication::postEvent(this, event);
    data.done.wait(&mutex);
    mutex.unlock();

    if (data.response == 0)  // append
    {
      file.id =
          H5Fopen(filename.toLatin1().constData(), H5F_ACC_RDWR, H5P_DEFAULT);
      size_t trial_num;
      QString trial_name;
      H5Eset_auto(H5E_DEFAULT, NULL, NULL);
      for (trial_num = 1;; ++trial_num) {
        trial_name = "/Trial" + QString::number(trial_num);
        file.trial =
            H5Gopen(file.id, trial_name.toLatin1().constData(), H5P_DEFAULT);
        if (file.trial < 0) {
          H5Eclear(H5E_DEFAULT);
          break;
        } else {
          H5Gclose(file.trial);
        }
      }
      trialNum->setNum(int(trial_num) - 1);
    } else if (data.response == 1)  // overwrite
    {
      file.id = H5Fcreate(filename.toLatin1().constData(),
                          H5F_ACC_TRUNC,
                          H5P_DEFAULT,
                          H5P_DEFAULT);
      trialNum->setText("0");
    } else {
      return -1;
    }
  } else {
    file.id = H5Fcreate(filename.toLatin1().constData(),
                        H5F_ACC_TRUNC,
                        H5P_DEFAULT,
                        H5P_DEFAULT);
    trialNum->setText("0");
  }
  if (file.id < 0) {
    H5E_type_t error_type;
    size_t error_size;
    error_size = H5Eget_msg(file.id, &error_type, NULL, 0);
    char error_msg[error_size + 1];
    H5Eget_msg(file.id, &error_type, error_msg, error_size);
    error_msg[error_size] = 0;
    H5Eclear(file.id);

    ERROR_MSG(
        "DataRecorder::Panel::processData : failed to open \"%s\" for writing "
        "with error : %s\n",
        filename.toStdString().c_str(),
        error_msg);
    return -1;
  }

  mutex.lock();
  CustomEvent* event =
      new CustomEvent(static_cast<QEvent::Type> QSetFileNameEditEvent);
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
  if (!pthread_equal(pthread_self(), thread)) {
    ERROR_MSG("DataRecorder::Panel::closeFile : called by invalid thread\n");
    PRINT_BACKTRACE();
  }
#endif

  if (!dataTags.empty()) {
    // Write tags to data file
    hid_t tag_type, tag_space, data;
    // herr_t status;
    hsize_t dims[1] = {1};
    tag_type = H5Tcreate(H5T_STRING, TAG_SIZE);
    tag_space = H5Screate_simple(1, dims, NULL);

    // Create group for tags
    file.tdata =
        H5Gcreate(file.id, "Tags", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

    // Iterate over vector (buffer) and put into data file
    size_t i = 0;
    for (std::vector<std::string>::iterator it = dataTags.begin();
         it != dataTags.end();
         ++it)
    {
      data = H5Dcreate(file.tdata,
                       std::string("Tag " + std::to_string(i++)).c_str(),
                       tag_type,
                       tag_space,
                       H5P_DEFAULT,
                       H5P_DEFAULT,
                       H5P_DEFAULT);
      // status = H5Dwrite(data, tag_type, H5S_ALL, H5S_ALL, H5P_DEFAULT,
      // it->c_str());
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

  if (!shutdown) {
    mutex.lock();
    CustomEvent* event =
        new CustomEvent(static_cast<QEvent::Type> QSetFileNameEditEvent);
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
  size_t trial_num;
  QString trial_name;

  H5Eset_auto(H5E_DEFAULT, NULL, NULL);

  for (trial_num = 1;; ++trial_num) {
    trial_name = "/Trial" + QString::number(trial_num);
    file.trial =
        H5Gopen(file.id, trial_name.toLatin1().constData(), H5P_DEFAULT);

    if (file.trial < 0) {
      H5Eclear(H5E_DEFAULT);
      break;
    } else
      H5Gclose(file.trial);
  }

  trialNum->setNum(int(trial_num));
  file.trial = H5Gcreate(file.id,
                         trial_name.toLatin1().constData(),
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);
  file.pdata = H5Gcreate(
      file.trial, "Parameters", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  file.adata = H5Gcreate(
      file.trial, "Asynchronous Data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  file.sdata = H5Gcreate(
      file.trial, "Synchronous Data", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
  file.sysdata = H5Gcreate(
      file.trial, "System Settings", H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

  hid_t scalar_space = H5Screate(H5S_SCALAR);
  hid_t string_type = H5Tcopy(H5T_C_S1);
  size_t string_size = 1024;
  H5Tset_size(string_type, string_size);
  hid_t data;

  data = H5Dcreate(file.trial,
                   "Version",
                   string_type,
                   scalar_space,
                   H5P_DEFAULT,
                   H5P_DEFAULT,
                   H5P_DEFAULT);
  std::string version_string = QString(VERSION).toStdString();
  char* version_c_string = new char[version_string.length() + 1];
  std::strcpy(version_c_string, version_string.c_str());
  H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, version_c_string);
  delete[] version_c_string;
  H5Dclose(data);

  long long period = RT::System::getInstance()->getPeriod();
  data = H5Dcreate(file.trial,
                   "Period (ns)",
                   H5T_STD_U64LE,
                   scalar_space,
                   H5P_DEFAULT,
                   H5P_DEFAULT,
                   H5P_DEFAULT);
  H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &period);
  H5Dclose(data);

  long long downsample = downsample_rate;
  data = H5Dcreate(file.trial,
                   "Downsampling Rate",
                   H5T_STD_U64LE,
                   scalar_space,
                   H5P_DEFAULT,
                   H5P_DEFAULT,
                   H5P_DEFAULT);
  H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &downsample);
  H5Dclose(data);

  data = H5Dcreate(file.trial,
                   "Timestamp Start (ns)",
                   H5T_STD_U64LE,
                   scalar_space,
                   H5P_DEFAULT,
                   H5P_DEFAULT,
                   H5P_DEFAULT);
  H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &timestamp);
  H5Dclose(data);

  data = H5Dcreate(file.trial,
                   "Date",
                   string_type,
                   scalar_space,
                   H5P_DEFAULT,
                   H5P_DEFAULT,
                   H5P_DEFAULT);
  std::string date_string =
      QDateTime::currentDateTime().toString(Qt::ISODate).toStdString();
  char* date_c_string = new char[date_string.length() + 1];
  std::strcpy(date_c_string, date_string.c_str());
  H5Dwrite(data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, date_c_string);
  delete[] date_c_string;
  H5Dclose(data);

  hid_t param_type;
  param_type = H5Tcreate(H5T_COMPOUND, sizeof(param_hdf_t));
  H5Tinsert(param_type, "index", HOFFSET(param_hdf_t, index), H5T_STD_I64LE);
  H5Tinsert(param_type, "value", HOFFSET(param_hdf_t, value), H5T_IEEE_F64LE);

  for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end();
       i != end;
       ++i)
  {
    IO::Block* block = i->block;
    for (size_t j = 0; j < block->getCount(Workspace::PARAMETER); ++j) {
      QString parameter_name = QString::number(block->getID()) + " "
          + QString::fromStdString(block->getName()) + " : "
          + QString::fromStdString(block->getName(Workspace::PARAMETER, j));
      data = H5PTcreate_fl(file.pdata,
                           parameter_name.toLatin1().constData(),
                           param_type,
                           sizeof(param_hdf_t),
                           -1);
      struct param_hdf_t value = {
          0,
          block->getValue(Workspace::PARAMETER, j),
      };
      H5PTappend(data, 1, &value);
      H5PTclose(data);
    }
    for (size_t j = 0; j < block->getCount(Workspace::COMMENT); ++j) {
      QString comment_name = QString::number(block->getID()) + " "
          + QString::fromStdString(block->getName()) + " : "
          + QString::fromStdString(block->getName(Workspace::COMMENT, j));
      hsize_t dims = dynamic_cast<Workspace::Instance*>(block)
                         ->getValueString(Workspace::COMMENT, j)
                         .size()
          + 1;
      hid_t comment_space = H5Screate_simple(1, &dims, &dims);
      data = H5Dcreate(file.pdata,
                       comment_name.toLatin1().constData(),
                       H5T_C_S1,
                       comment_space,
                       H5P_DEFAULT,
                       H5P_DEFAULT,
                       H5P_DEFAULT);
      H5Dwrite(data,
               H5T_C_S1,
               H5S_ALL,
               H5S_ALL,
               H5P_DEFAULT,
               dynamic_cast<Workspace::Instance*>(block)
                   ->getValueString(Workspace::COMMENT, j)
                   .c_str());
      H5Dclose(data);
    }
  }

  H5Tclose(param_type);

  size_t count = 0;
  for (RT::List<Channel>::iterator i = channels.begin(), end = channels.end();
       i != end;
       ++i)
  {
    std::string rec_chan_name =
        std::to_string(++count) + " " + i->name.toStdString();
    rec_chan_name.erase(
        std::remove_if(rec_chan_name.begin(), rec_chan_name.end(), &ispunct),
        rec_chan_name.end());
    hid_t data = H5Dcreate(file.sdata,
                           rec_chan_name.c_str(),
                           string_type,
                           scalar_space,
                           H5P_DEFAULT,
                           H5P_DEFAULT,
                           H5P_DEFAULT);
    H5Dwrite(data,
             string_type,
             H5S_ALL,
             H5S_ALL,
             H5P_DEFAULT,
             rec_chan_name.c_str());
    H5Dclose(data);
  }

  DAQ::Device* dev;
  {
    struct find_daq_t info = {
        0,
        0,
    };
    DAQ::Manager::getInstance()->foreachDevice(findDAQDevice, &info);
    dev = info.device;
  }

  // Save channel configurations
  if (dev)
    for (size_t i = 0; i < dev->getChannelCount(DAQ::AI); ++i)
      if (dev->getChannelActive(DAQ::AI, static_cast<DAQ::index_t>(i))) {
        std::string chan_name = "Analog Channel " + std::to_string(i);
        file.chandata = H5Gcreate(file.sysdata,
                                  chan_name.c_str(),
                                  H5P_DEFAULT,
                                  H5P_DEFAULT,
                                  H5P_DEFAULT);

        hid_t data = H5Dcreate(file.chandata,
                               "Range",
                               string_type,
                               scalar_space,
                               H5P_DEFAULT,
                               H5P_DEFAULT,
                               H5P_DEFAULT);
        std::string range_string = dev->getAnalogRangeString(
            DAQ::AI,
            static_cast<DAQ::index_t>(i),
            dev->getAnalogRange(DAQ::AI, static_cast<DAQ::index_t>(i)));
        char* range_c_string = new char[range_string.length() + 1];
        std::strcpy(range_c_string, range_string.c_str());
        H5Dwrite(
            data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, range_c_string);
        delete[] range_c_string;

        data = H5Dcreate(file.chandata,
                         "Reference",
                         string_type,
                         scalar_space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);
        std::string ref_string = dev->getAnalogReferenceString(
            DAQ::AI,
            static_cast<DAQ::index_t>(i),
            dev->getAnalogReference(DAQ::AI, static_cast<DAQ::index_t>(i)));
        char* ref_c_string = new char[ref_string.length() + 1];
        std::strcpy(ref_c_string, ref_string.c_str());
        H5Dwrite(
            data, string_type, H5S_ALL, H5S_ALL, H5P_DEFAULT, ref_c_string);
        delete[] ref_c_string;

        double scale =
            dev->getAnalogGain(DAQ::AI, static_cast<DAQ::index_t>(i));
        data = H5Dcreate(file.chandata,
                         "Gain",
                         H5T_IEEE_F64LE,
                         scalar_space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);
        H5Dwrite(data, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &scale);

        double offset =
            dev->getAnalogZeroOffset(DAQ::AI, static_cast<DAQ::index_t>(i));
        data = H5Dcreate(file.chandata,
                         "Offset",
                         H5T_IEEE_F64LE,
                         scalar_space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);
        H5Dwrite(data, H5T_IEEE_F64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &offset);

        int downsample =
            dev->getAnalogDownsample(DAQ::AI, static_cast<DAQ::index_t>(i));
        data = H5Dcreate(file.chandata,
                         "Downsample",
                         H5T_STD_I16LE,
                         scalar_space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);
        H5Dwrite(
            data, H5T_STD_I16LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &downsample);
        H5Dclose(data);
      }

  H5Tclose(string_type);
  H5Sclose(scalar_space);

  if (channels.size()) {
    hsize_t array_size[] = {channels.size()};
    hid_t array_type = H5Tarray_create(H5T_IEEE_F64LE, 1, array_size);
    file.cdata =
        H5PTcreate_fl(file.sdata, "Channel Data", array_type, (hsize_t)64, 1);
    H5Tclose(array_type);
  }

  file.idx = 0;

  return 0;
}

void DataRecorder::Panel::stopRecording(long long timestamp)
{
#ifdef DEBUG
  if (!pthread_equal(pthread_self(), thread)) {
    ERROR_MSG(
        "DataRecorder::Panel::stopRecording : called by invalid thread\n");
    PRINT_BACKTRACE();
  }
#endif

  // Write stop time to data file
  hid_t scalar_space = H5Screate(H5S_SCALAR);
  hid_t data = H5Dcreate(file.trial,
                         "Timestamp Stop (ns)",
                         H5T_STD_U64LE,
                         scalar_space,
                         H5P_DEFAULT,
                         H5P_DEFAULT,
                         H5P_DEFAULT);
  H5Dwrite(data, H5T_STD_U64LE, H5S_ALL, H5S_ALL, H5P_DEFAULT, &timestamp);
  H5Dclose(data);

  // Write trial length to data file
  fixedcount = count;
  long long period = RT::System::getInstance()->getPeriod();
  long long datalength = period * fixedcount;
  data = H5Dcreate(file.trial,
                   "Trial Length (ns)",
                   H5T_STD_U64LE,
                   scalar_space,
                   H5P_DEFAULT,
                   H5P_DEFAULT,
                   H5P_DEFAULT);
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
  void* file_handle;
  H5Fget_vfd_handle(file.id, H5P_DEFAULT, &file_handle);
  if (fsync(*static_cast<int*>(file_handle))) {
    std::cout
        << "DataRecorder::Panel::stopRecording : fsync failed, running sync\n";
    sync();
  }
}

DataRecorder::Plugin::Plugin()
{
  // get the HDF data recorder buffer size from user preference
  QSettings userprefs;
  userprefs.setPath(QSettings::NativeFormat,
                    QSettings::SystemScope,
                    "/usr/local/share/rtxi/");
  buffersize = (userprefs.value("/system/HDFbuffer", 10).toInt()) * 1048576;
}

DataRecorder::Plugin::~Plugin()
{
}

void DataRecorder::Plugin::removeDataRecorderPanel(DataRecorder::Panel* panel)
{
  panelList.remove(panel);
}

//void DataRecorder::Plugin::doDeferred(const Settings::Object::State& s)
//{
//  size_t i = 0;
//  for (std::list<Panel*>::iterator j = panelList.begin(), end = panelList.end();
//       j != end;
//       ++j)
//    (*j)->deferred(s.loadState(QString::number(i++).toStdString()));
//}
//
//void DataRecorder::Plugin::doLoad(const Settings::Object::State& s)
//{
//  for (size_t i = 0; i < static_cast<size_t>(s.loadInteger("Num Panels")); ++i)
//  {
//    Panel* panel =
//        new Panel(MainWindow::getInstance()->centralWidget(), buffersize);
//    panelList.push_back(panel);
//    panel->load(s.loadState(QString::number(i).toStdString()));
//  }
//}
//
//void DataRecorder::Plugin::doSave(Settings::Object::State& s) const
//{
//  s.saveInteger("Num Panels", panelList.size());
//  size_t n = 0;
//  for (std::list<Panel*>::const_iterator i = panelList.begin(),
//                                         end = panelList.end();
//       i != end;
//       ++i)
//    s.saveState(QString::number(n++).toStdString(), (*i)->save());
//}

