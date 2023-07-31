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

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <cstring>
#include <iostream>
#include <mutex>
#include <sstream>
#include <string>

#include "data_recorder.h"

#include <unistd.h>

#include "debug.hpp"
#include "main_window.hpp"

DataRecorder::Panel::Panel(QMainWindow* mwindow, Event::Manager* ev_manager)
    : Modules::Panel(
        std::string(DataRecorder::MODULE_NAME), mwindow, ev_manager)
    , downsample_rate(1)
    , recording(false)
    , buttonGroup(new QGroupBox)
    , blockList(new QComboBox)
    , channelList(new QComboBox)
    , typeList(new QComboBox)
    , selectionBox(new QListWidget)
    , recordStatus(new QLabel)
    , downsampleSpin(new QSpinBox(this))
    , fileNameEdit(new QLineEdit)
    , timeStampEdit(new QLineEdit)
    , fileSizeLbl(new QLabel)
    , fileSize(new QLabel)
    , trialLengthLbl(new QLabel)
    , trialLength(new QLabel)
    , trialNumLbl(new QLabel)
    , trialNum(new QLabel)
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
  auto* layout = new QGridLayout;

  channelGroup = new QGroupBox(tr("Channel Selection"));
  auto* channelLayout = new QVBoxLayout;

  channelLayout->addWidget(new QLabel(tr("Block:")));
  channelLayout->addWidget(blockList);
  QObject::connect(
      blockList, SIGNAL(activated(int)), this, SLOT(buildChannelList()));

  channelLayout->addWidget(new QLabel(tr("Type:")));

  channelLayout->addWidget(typeList);
  typeList->addItem("Input");
  typeList->addItem("Output");
  QObject::connect(
      typeList, SIGNAL(activated(int)), this, SLOT(buildChannelList()));

  channelLayout->addWidget(new QLabel(tr("Channel:")));

  channelLayout->addWidget(channelList);

  // Attach layout to child widget
  channelGroup->setLayout(channelLayout);

  // Create elements for arrow
  rButton = new QPushButton("Add");
  channelLayout->addWidget(rButton);
  QObject::connect(rButton, SIGNAL(released()), this, SLOT(insertChannel()));
  rButton->setEnabled(false);
  lButton = new QPushButton("Remove");
  channelLayout->addWidget(lButton);
  QObject::connect(lButton, SIGNAL(released()), this, SLOT(removeChannel()));
  lButton->setEnabled(false);

  // Timestamp
  stampGroup = new QGroupBox(tr("Tag Data"));
  auto* stampLayout = new QHBoxLayout;

  // Add timestamp elements

  stampLayout->addWidget(timeStampEdit);
  addTag = new QPushButton(tr("Tag"));
  stampLayout->addWidget(addTag);
  // QObject::connect(addTag, SIGNAL(released()), this, SLOT(addNewTag()));

  // Attach layout to child widget
  stampGroup->setLayout(stampLayout);

  // Create child widget and layout
  sampleGroup = new QGroupBox(tr("Trial Metadata"));
  auto* sampleLayout = new QHBoxLayout;

  // create elements for sample box

  trialNumLbl->setText("Trial #:");
  trialNumLbl->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  sampleLayout->addWidget(trialNumLbl);

  trialNum->setText("0");
  trialNum->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  sampleLayout->addWidget(trialNum);

  trialLengthLbl->setText("Trial Length (s):");
  sampleLayout->addWidget(trialLengthLbl);

  trialLength->setText("No data recorded");
  trialLength->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
  sampleLayout->addWidget(trialLength);

  fileSizeLbl->setText("File Size (MB):");
  sampleLayout->addWidget(fileSizeLbl);

  fileSize->setText("No data recorded");
  sampleLayout->addWidget(fileSize);
  fileSize->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

  // Attach layout to child widget
  sampleGroup->setLayout(sampleLayout);

  // Create child widget and layout for file control
  fileGroup = new QGroupBox(tr("File Control"));
  auto* fileLayout = new QHBoxLayout;

  // Create elements for file control
  fileLayout->addWidget(new QLabel(tr("File Name:")));

  fileNameEdit->setReadOnly(true);
  fileLayout->addWidget(fileNameEdit);
  auto* fileChangeButton = new QPushButton("Choose File");
  fileLayout->addWidget(fileChangeButton);
  // QObject::connect(fileChangeButton,
  //                  SIGNAL(released()),
  //                  this,
  //                  SLOT(changeDataFile()));

  fileLayout->addWidget(new QLabel(tr("Downsample \nRate:")));

  downsampleSpin->setMinimum(1);
  downsampleSpin->setMaximum(500);
  fileLayout->addWidget(downsampleSpin);
  // QObject::connect(downsampleSpin,
  //                  SIGNAL(valueChanged(int)),
  //                  this,
  //                  SLOT(updateDownsampleRate(int)));

  // Attach layout to child
  fileGroup->setLayout(fileLayout);

  // Create child widget and layout
  listGroup = new QGroupBox(tr("Currently Recording"));
  auto* listLayout = new QGridLayout;

  // Create elements for box

  listLayout->addWidget(selectionBox, 1, 1, 4, 5);

  // Attach layout to child
  listGroup->setLayout(listLayout);

  // Creat child widget and layout for buttons

  auto* buttonLayout = new QHBoxLayout;

  // Create elements for box
  startRecordButton = new QPushButton("Start Recording");
  // QObject::connect(startRecordButton,
  //                  SIGNAL(released()),
  //                  this,
  //                  SLOT(startRecordClicked()));
  buttonLayout->addWidget(startRecordButton);
  startRecordButton->setEnabled(false);
  stopRecordButton = new QPushButton("Stop Recording");
  // QObject::connect(stopRecordButton,
  //                  SIGNAL(released()),
  //                  this,
  //                  SLOT(stopRecordClicked()));
  buttonLayout->addWidget(stopRecordButton);
  stopRecordButton->setEnabled(false);
  closeButton = new QPushButton("Close");
  // QObject::connect(
  //     closeButton, SIGNAL(released()), this, SLOT(close()));
  buttonLayout->addWidget(closeButton);

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
  this->getMdiWindow()->setFixedSize(this->minimumSizeHint());

  // Build initial block list
  Event::Object event(Event::Type::IO_BLOCK_QUERY_EVENT);
  this->getRTXIEventManager()->postEvent(&event);
  blockPtrList =
      std::any_cast<std::vector<IO::Block*>>(event.getParam("blockList"));
  for (auto* blockptr : blockPtrList) {
    blockList->addItem(QString::fromStdString(blockptr->getName()) + " "
                       + QString::number(blockptr->getID()));
  }

  // Build initial channel list
  buildChannelList();
}

DataRecorder::Panel::~Panel() = default;

void DataRecorder::Panel::buildChannelList()
{
  channelList->clear();
  if (blockList->count() == 0) {
    return;
  }

  IO::Block* block =
      blockPtrList.at(static_cast<size_t>(blockList->currentIndex()));

  auto type = static_cast<IO::flags_t>(this->typeList->currentIndex());
  for (size_t i = 0; i < block->getCount(type); ++i) {
    channelList->addItem(
        QString::fromStdString(block->getChannelName(type, i)));
  }

  rButton->setEnabled(channelList->count() != 0);
}

void DataRecorder::Panel::changeDataFile()
{
  QFileDialog fileDialog(this);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.setWindowTitle("Select Data File");

  QSettings userprefs;
  QSettings::setPath(QSettings::NativeFormat,
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
  if (fileDialog.exec() != 0) {
    files = fileDialog.selectedFiles();
  }

  QString filename;
  if (files.isEmpty() || files[0] == nullptr || files[0] == "/") {
    return;
  }
  filename = files[0];
  if (!filename.toLower().endsWith(QString(".h5"))) {
    filename += ".h5";
  }

  // Write this directory to the user prefs as most recently used
  userprefs.setValue("/dirs/data", fileDialog.directory().path());

  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->change_file(filename.toStdString());
}

// Insert channel to record into list
void DataRecorder::Panel::insertChannel()
{
  if ((blockList->count() == 0) || (channelList->count() == 0)) {
    return;
  }

  IO::endpoint endpoint;
  IO::Block* block =
      blockPtrList.at(static_cast<size_t>(blockList->currentIndex()));
  IO::flags_t direction = direction =
      static_cast<IO::flags_t>(typeList->currentIndex());
  auto port = static_cast<size_t>(channelList->currentIndex());

  endpoint = {block, port, direction};
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  int result = hplugin->create_component(endpoint);

  if (result != 0) {
    ERROR_MSG(
        "DataRecorder::Panel::insertChannel : Unable to create recording "
        "component");
    return;
  }

  this->m_recording_channels = hplugin->get_recording_channels();
  // channel.name += block->getName();
  // channel.name += " ";
  // channel.name += std::to_string(block->getID());
  // channel.name += " : ";
  // channel.name += block->getChannelName(direction, port);

  // this->channels.push_back(std::move(channel));
  if (selectionBox->count() != 0) {
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
  if ((selectionBox->count() == 0) || selectionBox->selectedItems().isEmpty()) {
    return;
  }

  auto indx = static_cast<size_t>(this->selectionBox->currentRow());
  DataRecorder::record_channel chan = this->m_recording_channels.at(indx);
  IO::endpoint endpoint = chan.endpoint;
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->destroy_component(endpoint);
  this->m_recording_channels = hplugin->get_recording_channels();
  if (selectionBox->count() != 0) {
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
  // std::string newTag(std::to_string(RT::OS::getTime()));
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  std::string tag = this->timeStampEdit->text().toStdString();
  if (hplugin->apply_tag(tag) != 0) {
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

void DataRecorder::Panel::processData() {}

int DataRecorder::Panel::openFile(QString& filename)
{
  if (QFile::exists(filename)) {
    QMessageBox::StandardButton response =
        QMessageBox::question(this,
                              "File exists",
                              "The file already exists. Overwrite?",
                              QMessageBox::Yes | QMessageBox::No,
                              QMessageBox::Cancel);

    switch (response) {
      case QMessageBox::Yes:
        break;
      case QMessageBox::No:
      default:
        return -1;
    }
  }
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->openFile(filename.toStdString());
  return 0;
}

void DataRecorder::Panel::closeFile()
{
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->closeFile();
}

void DataRecorder::Panel::startRecording()
{
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->startRecording();
}

void DataRecorder::Panel::stopRecording()
{
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->stopRecording();
}

DataRecorder::Plugin::Plugin(Event::Manager* ev_manager)
    : Modules::Plugin(ev_manager, std::string(DataRecorder::MODULE_NAME))
{
}

DataRecorder::Plugin::~Plugin()
{
  std::vector<Event::Object> unloadEvents;
  for (auto& rec_channel : this->m_recording_channels_list) {
    unloadEvents.emplace_back(Event::Type::RT_THREAD_REMOVE_EVENT);
    unloadEvents.back().setParam(
        "thread",
        std::any(static_cast<RT::Thread*>(rec_channel.component.get())));
  }
  this->getEventManager()->postEvent(unloadEvents);
}

void DataRecorder::Plugin::receiveEvent(Event::Object* event) {}

void DataRecorder::Plugin::startRecording()
{
  Event::Type event_type = Event::Type::RT_THREAD_UNPAUSE_EVENT;
  std::vector<Event::Object> start_recording_event;
  for (auto& rec_channel : this->m_recording_channels_list) {
    start_recording_event.emplace_back(event_type);
    start_recording_event.back().setParam(
        "thread", static_cast<RT::Thread*>(rec_channel.component.get()));
  }
  this->getEventManager()->postEvent(start_recording_event);
}

void DataRecorder::Plugin::stopRecording()
{
  Event::Type event_type = Event::Type::RT_THREAD_PAUSE_EVENT;
  std::vector<Event::Object> stop_recording_event;
  for (auto& rec_chan : this->m_recording_channels_list) {
    stop_recording_event.emplace_back(event_type);
    stop_recording_event.back().setParam(
        "thread", static_cast<RT::Thread*>(rec_chan.component.get()));
  }
  this->getEventManager()->postEvent(stop_recording_event);
}

void DataRecorder::Plugin::close_trial_group(
    const std::unique_lock<std::shared_mutex>& /*lock*/)
{
  for (auto& channel : this->m_recording_channels_list) {
    H5PTclose(channel.hdf5_data_handle);
  }
  H5Gclose(this->hdf5_handles.sync_group_handle);
  H5Gclose(this->hdf5_handles.async_group_handle);
  H5Gclose(this->hdf5_handles.sys_data_group_handle);
  H5Gclose(this->hdf5_handles.trial_group_handle);
}

void DataRecorder::Plugin::open_trial_group(
    const std::unique_lock<std::shared_mutex>& /*lock*/)
{
  this->trial_count += 1;
  std::string trial_name = "/Trial";
  trial_name += std::to_string(this->trial_count);
  this->hdf5_handles.trial_group_handle =
      H5Gcreate(this->hdf5_handles.file_handle,
                trial_name.c_str(),
                H5P_DEFAULT,
                H5P_DEFAULT,
                H5P_DEFAULT);
  this->hdf5_handles.sync_group_handle =
      H5Gcreate(this->hdf5_handles.trial_group_handle,
                "Synchronous Data",
                H5P_DEFAULT,
                H5P_DEFAULT,
                H5P_DEFAULT);
  this->hdf5_handles.async_group_handle =
      H5Gcreate(this->hdf5_handles.trial_group_handle,
                "Aynchronous Data",
                H5P_DEFAULT,
                H5P_DEFAULT,
                H5P_DEFAULT);
  this->hdf5_handles.sys_data_group_handle =
      H5Gcreate(this->hdf5_handles.trial_group_handle,
                "System Settings",
                H5P_DEFAULT,
                H5P_DEFAULT,
                H5P_DEFAULT);
  for (auto& channel : this->m_recording_channels_list) {
    channel.hdf5_data_handle =
        H5PTcreate_fl(this->hdf5_handles.sync_group_handle,
                      channel.channel.name.c_str(),
                      this->hdf5_handles.channel_datatype_handle,
                      this->m_data_chunk_size,
                      this->m_compression_factor);
  }
}

void DataRecorder::Plugin::append_new_trial()
{
  std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  this->close_trial_group(lk);
  this->open_trial_group(lk);
}

void DataRecorder::Plugin::openFile(const std::string& file_name)
{
  std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  this->hdf5_handles.file_handle =
      H5Fcreate(file_name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (this->hdf5_handles.file_handle == H5I_INVALID_HID) {
    ERROR_MSG("DataRecorder::Plugin::openFile : Unable to open file {}",
              file_name);
    H5Fclose(this->hdf5_handles.file_handle);
    return;
  }
  this->hdf5_filename = file_name;
  this->trial_count = 0;
  this->hdf5_handles.channel_datatype_handle =
      H5Tcreate(H5T_COMPOUND, sizeof(DataRecorder::data_token_t));
  this->open_trial_group(lk);
  this->open_file = true;
}

void DataRecorder::Plugin::closeFile()
{
  std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  this->close_trial_group(lk);
  if (H5Fclose(this->hdf5_handles.file_handle) != 0) {
    ERROR_MSG("DataRecorder::Plugin::closeFile : Unable to close file {}",
              this->hdf5_filename);
  }
  this->open_file = false;
}

void DataRecorder::Plugin::change_file(const std::string& file_name)
{
  this->closeFile();
  this->openFile(file_name);
}

int DataRecorder::Plugin::create_component(IO::endpoint endpoint)
{
  if (endpoint.block == nullptr) {
    return -1;
  }
  auto iter = std::find_if(this->m_recording_channels_list.begin(),
                           this->m_recording_channels_list.end(),
                           [endpoint](const recorder& rec)
                           { return rec.channel.endpoint == endpoint; });
  if (iter != this->m_recording_channels_list.end()) {
    return 0;
  }
  DataRecorder::record_channel chan;
  chan.name = endpoint.block->getName();
  chan.name += " ";
  chan.name += std::to_string(endpoint.block->getID());
  chan.name += " ";
  chan.name += "Recording Component";
  chan.endpoint = endpoint;

  chan.data_source =
      this->m_recording_channels_list.back().component->get_fifo();
  Event::Object event(Event::Type::RT_THREAD_INSERT_EVENT);
  event.setParam("thread",
                 static_cast<RT::Thread*>(
                     m_recording_channels_list.back().component.get()));
  this->getEventManager()->postEvent(&event);
  std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  this->m_recording_channels_list.emplace_back(
      chan, std::make_unique<DataRecorder::Component>(this, chan.name));
  return 0;
}

void DataRecorder::Plugin::destroy_component(IO::endpoint endpoint)
{
  DataRecorder::Component* component = this->getRecorderPtr(endpoint);
  Event::Object pause_event(Event::Type::RT_THREAD_PAUSE_EVENT);
  pause_event.setParam("thread", static_cast<RT::Thread*>(component));
  this->getEventManager()->postEvent(&pause_event);
  Event::Object unplug_event(Event::Type::RT_THREAD_REMOVE_EVENT);
  unplug_event.setParam("thread", static_cast<RT::Thread*>(component));
  this->getEventManager()->postEvent(&unplug_event);
  std::unique_lock<std::shared_mutex> recorder_lock(this->m_channels_list_mut);
  auto iter = std::find_if(this->m_recording_channels_list.begin(),
                           this->m_recording_channels_list.end(),
                           [component](const recorder& rec_chan)
                           { return rec_chan.component.get() == component; });
  if (iter == this->m_recording_channels_list.end()) {
    return;
  }
  this->m_recording_channels_list.erase(iter);
}

std::string DataRecorder::Plugin::getRecorderName(IO::endpoint endpoint)
{
  std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  auto iter = std::find_if(this->m_recording_channels_list.begin(),
                           this->m_recording_channels_list.end(),
                           [endpoint](const recorder& rec_chan)
                           { return rec_chan.channel.endpoint == endpoint; });
  if (iter == this->m_recording_channels_list.end()) {
    return "";
  }
  return iter->channel.name;
}

DataRecorder::Component* DataRecorder::Plugin::getRecorderPtr(
    IO::endpoint endpoint)
{
  std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  auto iter = std::find_if(this->m_recording_channels_list.begin(),
                           this->m_recording_channels_list.end(),
                           [endpoint](const recorder& rec_chan)
                           { return rec_chan.channel.endpoint == endpoint; });
  if (iter == this->m_recording_channels_list.end()) {
    return nullptr;
  }
  return iter->component.get();
}

std::vector<DataRecorder::record_channel>
DataRecorder::Plugin::get_recording_channels()
{
  std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  std::vector<DataRecorder::record_channel> result(
      this->m_recording_channels_list.size());
  for (size_t i = 0; i < result.size(); i++) {
    result[i] = this->m_recording_channels_list[i].channel;
  }
  return result;
}

int DataRecorder::Plugin::apply_tag(const std::string& tag)
{
  return 0;
}

void DataRecorder::Plugin::process_data_worker()
{
  if (!this->open_file) {
    return;
  }
  std::vector<DataRecorder::data_token_t> data_buffer(this->m_data_chunk_size);
  size_t packet_byte_size = sizeof(DataRecorder::data_token_t);
  ssize_t read_bytes = 0;
  size_t packet_count = 0;
  std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  for (auto& channel : this->m_recording_channels_list) {
    while (read_bytes = channel.channel.data_source->read(
               data_buffer.data(), packet_byte_size * data_buffer.size()),
           read_bytes > 0)
    {
      packet_count = static_cast<size_t>(read_bytes) / packet_byte_size;
      this->save_data(channel.hdf5_data_handle, data_buffer, packet_count);
    }
  }
}

void DataRecorder::Plugin::save_data(
    hid_t data_id,
    const std::vector<DataRecorder::data_token_t>& data,
    size_t packet_count)
{
  herr_t err =
      H5PTappend(data_id, static_cast<hsize_t>(packet_count), data.data());
  if (err < 0) {
    ERROR_MSG("Unable to write data into hdf5 file!");
  }
}

DataRecorder::Component::Component(Modules::Plugin* hplugin,
                                   const std::string& probe_name)
    : Modules::Component(hplugin,
                         probe_name,
                         DataRecorder::get_default_channels(),
                         DataRecorder::get_default_vars())
{
}

void DataRecorder::Component::execute() {}

RT::OS::Fifo* DataRecorder::Component::get_fifo()
{
  return this->m_fifo.get();
}

std::unique_ptr<Modules::Plugin> DataRecorder::createRTXIPlugin(
    Event::Manager* ev_manager)
{
  return std::make_unique<DataRecorder::Plugin>(ev_manager);
}

Modules::Panel* DataRecorder::createRTXIPanel(QMainWindow* main_window,
                                              Event::Manager* ev_manager)
{
  return static_cast<Modules::Panel*>(
      new DataRecorder::Panel(main_window, ev_manager));
}

std::unique_ptr<Modules::Component> DataRecorder::createRTXIComponent(
    Modules::Plugin* /*host_plugin*/)
{
  return std::unique_ptr<DataRecorder::Component>(nullptr);
}

Modules::FactoryMethods DataRecorder::getFactories()
{
  Modules::FactoryMethods fact;
  fact.createPanel = &DataRecorder::createRTXIPanel;
  fact.createComponent = &DataRecorder::createRTXIComponent;
  fact.createPlugin = &DataRecorder::createRTXIPlugin;
  return fact;
}
