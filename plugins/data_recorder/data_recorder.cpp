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

#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTimer>
#include <mutex>
#include <string>

#include "data_recorder.hpp"

#include <H5Dpublic.h>
#include <H5Spublic.h>
#include <hdf5.h>

#include "debug.hpp"
#include "fifo.hpp"
#include "rtos.hpp"
#include "userprefs/userprefs.hpp"
#include "widgets.hpp"

DataRecorder::Panel::Panel(QMainWindow* mwindow, Event::Manager* ev_manager)
    : Widgets::Panel(
          std::string(DataRecorder::MODULE_NAME), mwindow, ev_manager)
    , buttonGroup(new QGroupBox)
    , blockList(new QComboBox)
    , channelList(new QComboBox)
    , typeList(new QComboBox)
    , timeTagType(new QComboBox())
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
    , recording_timer(new QTimer(this))
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
  QObject::connect(blockList,
                   QOverload<int>::of(&QComboBox::activated),
                   this,
                   &DataRecorder::Panel::buildChannelList);

  channelLayout->addWidget(new QLabel(tr("Type:")));

  channelLayout->addWidget(typeList);
  typeList->addItem("Output", QVariant::fromValue(IO::OUTPUT));
  typeList->addItem("Input", QVariant::fromValue(IO::INPUT));
  QObject::connect(typeList,
                   QOverload<int>::of(&QComboBox::activated),
                   this,
                   &DataRecorder::Panel::buildChannelList);

  channelLayout->addWidget(new QLabel(tr("Channel:")));

  channelLayout->addWidget(channelList);

  // Attach layout to child widget
  channelGroup->setLayout(channelLayout);

  // Create elements for arrow
  addRecorderButton = new QPushButton("Add");
  channelLayout->addWidget(addRecorderButton);
  QObject::connect(addRecorderButton,
                   &QPushButton::released,
                   this,
                   &DataRecorder::Panel::insertChannel);
  addRecorderButton->setEnabled(false);
  removeRecorderButton = new QPushButton("Remove");
  channelLayout->addWidget(removeRecorderButton);
  QObject::connect(removeRecorderButton,
                   &QPushButton::released,
                   this,
                   &DataRecorder::Panel::removeChannel);
  removeRecorderButton->setEnabled(false);

  // Timestamp
  stampGroup = new QGroupBox(tr("Tag Data"));
  auto* stampLayout = new QHBoxLayout;

  // Add timestamp elements
  stampLayout->addWidget(timeStampEdit);
  addTag = new QPushButton(tr("Tag"));
  stampLayout->addWidget(addTag);

  auto* timeTagLabel = new QLabel(tr("Time Tag Type:"));
  stampLayout->addWidget(timeTagLabel);
  timeTagType->addItem("Index", TIME_TAG_TYPE::INDEX);
  timeTagType->addItem("Time Stamp", TIME_TAG_TYPE::TIME);
  timeTagType->addItem("No Tag", TIME_TAG_TYPE::NONE);
  stampLayout->addWidget(timeTagType);
  QObject::connect(timeTagType,
                   QOverload<int>::of(&QComboBox::currentIndexChanged),
                   this,
                   &DataRecorder::Panel::setTimeTagType);
  QObject::connect(
      addTag, &QPushButton::released, this, &DataRecorder::Panel::addNewTag);

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
  QObject::connect(fileChangeButton,
                   &QPushButton::released,
                   this,
                   &DataRecorder::Panel::changeDataFile);

  fileLayout->addWidget(new QLabel(tr("Downsample \nRate:")));

  downsampleSpin->setMinimum(1);
  downsampleSpin->setMaximum(500);
  fileLayout->addWidget(downsampleSpin);
  QObject::connect(downsampleSpin,
                   QOverload<int>::of(&QSpinBox::valueChanged),
                   this,
                   &DataRecorder::Panel::updateDownsampleRate);

  // Attach layout to child
  fileGroup->setLayout(fileLayout);

  // Create child widget and layout
  listGroup = new QGroupBox(tr("Currently Recording"));
  auto* listLayout = new QGridLayout;

  // Create elements for box

  listLayout->addWidget(selectionBox, 1, 1, 4, 5);

  // Attach layout to child
  listGroup->setLayout(listLayout);

  // Create child widget and layout for buttons

  auto* buttonLayout = new QHBoxLayout;

  // Create elements for box
  startRecordButton = new QPushButton("Start Recording");
  QObject::connect(startRecordButton,
                   &QPushButton::released,
                   this,
                   &DataRecorder::Panel::startRecordClicked);
  buttonLayout->addWidget(startRecordButton);
  startRecordButton->setEnabled(false);
  stopRecordButton = new QPushButton("Stop Recording");
  QObject::connect(stopRecordButton,
                   &QPushButton::released,
                   this,
                   &DataRecorder::Panel::stopRecordClicked);
  buttonLayout->addWidget(stopRecordButton);
  stopRecordButton->setEnabled(false);
  closeButton = new QPushButton("Close");
  QObject::connect(
      closeButton, &QPushButton::released, parentWidget(), &QWidget::close);
  buttonLayout->addWidget(closeButton);

  buttonLayout->addWidget(recordStatus);
  recordStatus->setText("Not ready.");
  recordStatus->setFrameStyle(QFrame::Panel);
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

  this->buildBlockList();
  this->buildChannelList();

  this->recording_timer->setInterval(1000);
  QObject::connect(recording_timer,
                   &QTimer::timeout,
                   this,
                   &DataRecorder::Panel::processData);
  QObject::connect(this,
                   &DataRecorder::Panel::updateBlockInfo,
                   this,
                   &DataRecorder::Panel::buildBlockList);
  QObject::connect(fileNameEdit,
                   &QLineEdit::textChanged,
                   this,
                   &DataRecorder::Panel::syncEnableRecordingButtons);
  QObject::connect(this,
                   &DataRecorder::Panel::record_signal,
                   this,
                   &DataRecorder::Panel::record_slot);
  QObject::connect(this,
                   &DataRecorder::Panel::record_signal,
                   timeTagType,
                   &QComboBox::setDisabled);
  recording_timer->start();
}

void DataRecorder::Panel::record_slot(bool record)
{
  record ? startRecordClicked() : stopRecordClicked();
}

void DataRecorder::Panel::buildBlockList()
{
  // Build initial block list
  Event::Object event(Event::Type::IO_BLOCK_QUERY_EVENT);
  this->getRTXIEventManager()->postEvent(&event);
  auto blockPtrList =
      std::any_cast<std::vector<IO::Block*>>(event.getParam("blockList"));
  auto prev_selected_block = blockList->currentData();
  blockList->clear();
  for (auto* blockptr : blockPtrList) {
    if (blockptr->getName().find("Probe") != std::string::npos
        || blockptr->getName().find("Recording") != std::string::npos)
    {
      continue;
    }
    blockList->addItem(QString(blockptr->getName().c_str()) + " "
                           + QString::number(blockptr->getID()),
                       QVariant::fromValue(blockptr));
  }
  blockList->setCurrentIndex(blockList->findData(prev_selected_block));
}

void DataRecorder::Panel::buildChannelList()
{
  channelList->clear();
  if (blockList->count() == 0 || blockList->currentIndex() < 0) {
    return;
  }

  auto* block = blockList->currentData().value<IO::Block*>();

  auto type = this->typeList->currentData().value<IO::flags_t>();
  for (size_t i = 0; i < block->getCount(type); ++i) {
    channelList->addItem(QString(block->getChannelName(type, i).c_str()),
                         QVariant::fromValue(i));
  }
  addRecorderButton->setEnabled(channelList->count() != 0);
}

void DataRecorder::Panel::changeDataFile()
{
  QFileDialog fileDialog(this);
  // Accept save mode will automatically confirm whether to "overwrite"
  fileDialog.setAcceptMode(QFileDialog::AcceptSave);
  fileDialog.setFileMode(QFileDialog::AnyFile);
  fileDialog.setWindowTitle("Select Data File");

  QSettings userprefs;

  userprefs.beginGroup("settings");
  fileDialog.setDirectory(userprefs
                              .value(QString::fromStdString(std::string(
                                  UserPrefs::HDF5_SAVE_LOCATION_KEY)))
                              .toString());  // NOLINT
  userprefs.endGroup();
  QStringList filterList;
  filterList.push_back("HDF5 files (*.h5)");
  filterList.push_back("All files (*.*)");
  fileDialog.setNameFilters(filterList);
  fileDialog.selectNameFilter("HDF5 files (*.h5)");

  fileDialog.selectFile(
      QString("rtxi_datafile_") + QDate::currentDate().toString("yyyyMMdd")
      + QString("_") + QTime::currentTime().toString("hhmmss"));
  QStringList files;
  if (fileDialog.exec() != QDialog::Accepted) {
    return;
  }
  files = fileDialog.selectedFiles();
  QString filename;
  if (files.isEmpty() || files[0] == nullptr || files[0] == "/") {
    return;
  }
  filename = files[0];
  if (!filename.toLower().endsWith(QString(".h5"))) {
    filename += ".h5";
  }

  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->change_file(filename.toStdString());
  this->fileNameEdit->setText(QString(hplugin->getOpenFilename().c_str()));
}

// Insert channel to record into list
void DataRecorder::Panel::insertChannel()
{
  if ((blockList->count() == 0) || (channelList->count() == 0)) {
    return;
  }

  IO::endpoint endpoint;
  endpoint.block = blockList->currentData().value<IO::Block*>();
  endpoint.direction = typeList->currentData().value<IO::flags_t>();
  endpoint.port = channelList->currentData().value<size_t>();

  for (int row = 0; row < this->selectionBox->count(); row++) {
    if (this->selectionBox->item(row)->data(Qt::UserRole).value<IO::endpoint>()
        == endpoint)
    {
      this->selectionBox->setCurrentRow(row);
      return;
    }
  }

  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  const int result = hplugin->create_component(endpoint);

  if (result != 0) {
    ERROR_MSG(
        "DataRecorder::Panel::insertChannel : Unable to create recording "
        "component");
    return;
  }

  QListWidgetItem* temp_item = nullptr;
  const std::string formatting = "{} {} direction: {} port: {}";
  const std::string temp_name =
      fmt::format(formatting,
                  endpoint.block->getName(),
                  endpoint.block->getID(),
                  endpoint.direction == IO::INPUT ? "INPUT" : "OUTPUT",
                  endpoint.port);
  temp_item = new QListWidgetItem(QString(temp_name.c_str()));
  temp_item->setData(Qt::UserRole, QVariant::fromValue(endpoint));
  selectionBox->addItem(temp_item);

  removeRecorderButton->setEnabled(selectionBox->count() != 0);
}

void DataRecorder::Panel::removeChannel()
{
  if ((selectionBox->count() == 0) || selectionBox->selectedItems().isEmpty()) {
    return;
  }
  QListWidgetItem* currentItem =
      this->selectionBox->takeItem(this->selectionBox->currentRow());
  auto endpoint = currentItem->data(Qt::UserRole).value<IO::endpoint>();
  this->selectionBox->setCurrentRow(-1);
  this->selectionBox->removeItemWidget(currentItem);
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->destroy_component(endpoint);
  // Taking an item out means we have to handle deletion ourselves
  delete currentItem;

  removeRecorderButton->setEnabled(selectionBox->count() != 0);
}

void DataRecorder::Panel::addNewTag()
{
  // std::string newTag(std::to_string(RT::OS::getTime()));
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  const std::string tag = this->timeStampEdit->text().toStdString();
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
  this->recordStatus->setText(hplugin->isRecording() ? "Recording..."
                                                     : "Not ready");
  if (hplugin->isRecording()) {
    this->starting_record_time = QTime::currentTime();
    this->trialNum->setNum(hplugin->getTrialCount());
    this->trialLength->setText("Recording...");
    this->timeTagType->setDisabled(true);
  }
}

// Stop recording slot
void DataRecorder::Panel::stopRecordClicked()
{
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->stopRecording();
  this->recordStatus->setText(!hplugin->isRecording() ? "Ready"
                                                      : "Recording...");
  if (!hplugin->isRecording()) {
    this->trialNum->setNum(hplugin->getTrialCount());
    this->trialLength->setNum(
        this->starting_record_time.secsTo(QTime::currentTime()));
    this->fileSize->setNum(
        static_cast<double>(QFile(fileNameEdit->text()).size())
        / (1024.0 * 1024.0));
    this->timeTagType->setDisabled(false);
  }
}

// Update downsample rate
void DataRecorder::Panel::updateDownsampleRate(size_t rate)
{
  this->downsample_rate = rate;
}

void DataRecorder::Panel::removeRecorders(IO::Block* block)
{
  std::vector<QListWidgetItem*> all_recorders;
  IO::endpoint temp_endpoint;
  for (int row = 0; row < this->selectionBox->count(); row++) {
    temp_endpoint =
        this->selectionBox->item(row)->data(Qt::UserRole).value<IO::endpoint>();
    if (temp_endpoint.block == block) {
      all_recorders.push_back(this->selectionBox->item(row));
    }
  }
  for (auto* item : all_recorders) {
    this->selectionBox->removeItemWidget(item);
  }
}

void DataRecorder::Panel::processData()
{
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin());
  hplugin->process_data_worker();
}

void DataRecorder::Panel::syncEnableRecordingButtons(const QString& /*unused*/)
{
  auto* hplugin = dynamic_cast<DataRecorder::Plugin*>(getHostPlugin());
  const bool ready = hplugin->isFileOpen();
  startRecordButton->setEnabled(ready);
  stopRecordButton->setEnabled(ready);
  this->recordStatus->setText(ready ? "Ready" : "Not ready");
}

void DataRecorder::Panel::setTimeTagType(int tag_type)
{
  // Update our local copy of tag type if successfully changed in RT thread
  if (dynamic_cast<DataRecorder::Plugin*>(this->getHostPlugin())
          ->changeIndexingType(tag_type))
  {
    this->time_type = static_cast<TIME_TAG_TYPE>(tag_type);
  };
}

DataRecorder::TIME_TAG_TYPE DataRecorder::Panel::getTimeTagType() const
{
  return this->time_type;
}

DataRecorder::Plugin::Plugin(Event::Manager* ev_manager)
    : Widgets::Plugin(ev_manager, std::string(DataRecorder::MODULE_NAME))
    , recording(false)
{
}

DataRecorder::Plugin::~Plugin()
{
  this->stopRecording();
  this->closeFile();
  const Event::Type event_type = Event::Type::RT_THREAD_REMOVE_EVENT;
  std::vector<Event::Object> unload_events;
  for (auto& entry : this->m_recording_channels_list) {
    unload_events.emplace_back(event_type);
    unload_events.back().setParam(
        "thread", std::any(static_cast<RT::Thread*>(entry.component.get())));
  }
  this->getEventManager()->postEvent(unload_events);
}

void DataRecorder::Plugin::receiveEvent(Event::Object* event)
{
  IO::Block* block = nullptr;
  std::vector<IO::endpoint> endpoints;
  switch (event->getType()) {
    case Event::Type::RT_THREAD_REMOVE_EVENT:
    case Event::Type::RT_DEVICE_REMOVE_EVENT:
      dynamic_cast<DataRecorder::Panel*>(this->getPanel())
          ->removeRecorders(block);
      for (const auto& entry : this->m_recording_channels_list) {
        if (entry.channel.endpoint.block == block) {
          endpoints.push_back(entry.channel.endpoint);
        }
      }
      for (const auto& endpoint : endpoints) {
        this->destroy_component(endpoint);
      }
      dynamic_cast<DataRecorder::Panel*>(this->getPanel())->updateBlockInfo();
      break;
    case Event::Type::RT_THREAD_INSERT_EVENT:
    case Event::Type::RT_DEVICE_INSERT_EVENT:
      dynamic_cast<DataRecorder::Panel*>(this->getPanel())->updateBlockInfo();
      break;
    case Event::Type::START_RECORDING_EVENT:
      dynamic_cast<DataRecorder::Panel*>(this->getPanel())
          ->record_signal(/*record=*/true);
      break;
    case Event::Type::STOP_RECORDING_EVENT:
      dynamic_cast<DataRecorder::Panel*>(this->getPanel())
          ->record_signal(/*record=*/false);
      break;
    default:
      break;
  }
}

void DataRecorder::Plugin::startRecording()
{
  if (this->recording.load() || this->m_recording_channels_list.empty()) {
    return;
  }
  const std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  this->append_new_trial();
  const Event::Type event_type = Event::Type::RT_THREAD_UNPAUSE_EVENT;
  const Event::Type unpause_event_type =
      Event::Type::RT_WIDGET_STATE_CHANGE_EVENT;
  std::vector<Event::Object> start_recording_event;
  for (auto& rec_channel : this->m_recording_channels_list) {
    start_recording_event.emplace_back(unpause_event_type);
    start_recording_event.back().setParam(
        "component",
        static_cast<Widgets::Component*>(rec_channel.component.get()));
    start_recording_event.back().setParam("state", RT::State::UNPAUSE);
    start_recording_event.emplace_back(event_type);
    start_recording_event.back().setParam(
        "thread", static_cast<RT::Thread*>(rec_channel.component.get()));
  }
  this->getEventManager()->postEvent(start_recording_event);
  this->recording.store(true);
}

void DataRecorder::Plugin::stopRecording()
{
  if (!this->recording.load()) {
    return;
  }
  const std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  const Event::Type event_type = Event::Type::RT_THREAD_PAUSE_EVENT;
  const Event::Type pause_event_type =
      Event::Type::RT_WIDGET_STATE_CHANGE_EVENT;
  std::vector<Event::Object> stop_recording_event;
  for (auto& rec_chan : this->m_recording_channels_list) {
    stop_recording_event.emplace_back(pause_event_type);
    stop_recording_event.back().setParam(
        "component",
        static_cast<Widgets::Component*>(rec_chan.component.get()));
    stop_recording_event.back().setParam("state", RT::State::PAUSE);
    stop_recording_event.emplace_back(event_type);
    stop_recording_event.back().setParam(
        "thread", static_cast<RT::Thread*>(rec_chan.component.get()));
  }
  this->getEventManager()->postEvent(stop_recording_event);
  this->recording.store(false);
}

bool DataRecorder::Plugin::changeIndexingType(int tag_type)
{
  // We only change the index type while not recording!
  if (this->recording.load()) {
    ERROR_MSG(
        "DataRecorder::Plugin::changeIndexingType : Unable to change index "
        "type while recording.");
    return false;
  }
  if (tag_type < 0) {
    ERROR_MSG(
        "DataRecorderr::Plugin::changeIndexingType : Invalid index provided. "
        "Unable to change index type");
    return false;
  }
  std::vector<Event::Object> change_index_type_events;
  change_index_type_events.reserve(this->m_recording_channels_list.size());
  for (auto& rec_chan : this->m_recording_channels_list) {
    change_index_type_events.emplace_back(
        Event::Type::RT_WIDGET_PARAMETER_CHANGE_EVENT);
    change_index_type_events.back().setParam(
        "paramID", std::any(static_cast<size_t>(PARAMETER::INDEXING)));
    change_index_type_events.back().setParam(
        "paramType", std::any(Widgets::Variable::UINT_PARAMETER));
    change_index_type_events.back().setParam(
        "paramValue", std::any(static_cast<uint64_t>(tag_type)));
    change_index_type_events.back().setParam(
        "paramWidget",
        std::any(static_cast<Widgets::Component*>(rec_chan.component.get())));
  }
  this->getEventManager()->postEvent(change_index_type_events);

  return true;
}

void DataRecorder::Plugin::close_trial_group()
{
  if (!open_file.load()) {
    return;
  }
  for (auto& channel : this->m_recording_channels_list) {
    if (channel.hdf5_data_handle != H5I_INVALID_HID) {
      H5PTclose(channel.hdf5_data_handle);
      channel.hdf5_data_handle = H5I_INVALID_HID;
    }
  }
  if (this->hdf5_handles.sync_group_handle != H5I_INVALID_HID) {
    H5Gclose(this->hdf5_handles.sync_group_handle);
    this->hdf5_handles.sync_group_handle = H5I_INVALID_HID;
  }
  if (this->hdf5_handles.async_group_handle != H5I_INVALID_HID) {
    H5Gclose(this->hdf5_handles.async_group_handle);
    this->hdf5_handles.async_group_handle = H5I_INVALID_HID;
  }
  if (this->hdf5_handles.sys_data_group_handle != H5I_INVALID_HID) {
    H5Gclose(this->hdf5_handles.sys_data_group_handle);
    this->hdf5_handles.sys_data_group_handle = H5I_INVALID_HID;
  }
  if (this->hdf5_handles.trial_group_handle != H5I_INVALID_HID) {
    H5Gclose(this->hdf5_handles.trial_group_handle);
    this->hdf5_handles.trial_group_handle = H5I_INVALID_HID;
  }
}

void DataRecorder::Plugin::open_trial_group()
{
  this->trial_count += 1;
  hid_t compression_property = H5I_INVALID_HID;
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
                (trial_name + "/Synchronous Data").c_str(),
                H5P_DEFAULT,
                H5P_DEFAULT,
                H5P_DEFAULT);
  this->hdf5_handles.async_group_handle =
      H5Gcreate(this->hdf5_handles.trial_group_handle,
                (trial_name + "/Asynchronous Data").c_str(),
                H5P_DEFAULT,
                H5P_DEFAULT,
                H5P_DEFAULT);
  this->hdf5_handles.sys_data_group_handle =
      H5Gcreate(this->hdf5_handles.trial_group_handle,
                (trial_name + "/System Settings").c_str(),
                H5P_DEFAULT,
                H5P_DEFAULT,
                H5P_DEFAULT);
  const TIME_TAG_TYPE data_type =
      dynamic_cast<DataRecorder::Panel*>(this->getPanel())->getTimeTagType();
  if (data_type == NONE) {
    for (auto& channel : this->m_recording_channels_list) {
      compression_property = H5Pcreate(H5P_DATASET_CREATE);
      H5Pset_deflate(compression_property, 7);
      channel.hdf5_data_handle =
          H5PTcreate(this->hdf5_handles.sync_group_handle,
                     channel.channel.name.c_str(),
                     H5T_IEEE_F64LE,
                     this->m_data_chunk_size,
                     compression_property);
    }
  } else {
    for (auto& channel : this->m_recording_channels_list) {
      compression_property = H5Pcreate(H5P_DATASET_CREATE);
      H5Pset_deflate(compression_property, 7);
      channel.hdf5_data_handle =
          H5PTcreate(this->hdf5_handles.sync_group_handle,
                     channel.channel.name.c_str(),
                     this->hdf5_handles.channel_index_datatype_handle,
                     this->m_data_chunk_size,
                     compression_property);
    }
  }
}

void DataRecorder::Plugin::append_new_trial()
{
  this->close_trial_group();
  // We have to flush all of the data from the buffers that did not make it
  std::array<data_token_t, 100> tempbuffer {};
  for (auto& recorder : this->m_recording_channels_list) {
    while (recorder.channel.data_source->read(
               tempbuffer.data(),
               sizeof(DataRecorder::data_token_t) * tempbuffer.size())
           > 0)
    {
    }
  }
  this->open_trial_group();
}

void DataRecorder::Plugin::openFile(const std::string& file_name)
{
  if (this->open_file.load()) {
    return;
  }
  const std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  this->hdf5_handles.file_handle =
      H5Fcreate(file_name.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
  if (this->hdf5_handles.file_handle == H5I_INVALID_HID) {
    ERROR_MSG("DataRecorder::Plugin::openFile : Unable to open file {}",
              file_name);
    H5Fclose(this->hdf5_handles.file_handle);
    this->hdf5_filename = "";
    return;
  }
  this->hdf5_filename = file_name;
  this->trial_count = 0;
  this->hdf5_handles.channel_index_datatype_handle =
      H5Tcreate(H5T_COMPOUND, sizeof(DataRecorder::data_token_t));
  H5Tinsert(this->hdf5_handles.channel_index_datatype_handle,
            "time",
            HOFFSET(DataRecorder::data_token_t, time),
            H5T_STD_I64LE);
  H5Tinsert(this->hdf5_handles.channel_index_datatype_handle,
            "value",
            HOFFSET(DataRecorder::data_token_t, value),
            H5T_IEEE_F64LE);
  this->open_file.store(true);
}

void DataRecorder::Plugin::closeFile()
{
  if (!this->open_file.load()) {
    return;
  }
  const std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  // Attempt to close all group and dataset handles in hdf5 before closing
  // file
  close_trial_group();
  H5Tclose(this->hdf5_handles.channel_index_datatype_handle);
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
                           [endpoint](const recorder_t& rec)
                           { return rec.channel.endpoint == endpoint; });
  if (iter != this->m_recording_channels_list.end()) {
    return 0;
  }
  DataRecorder::record_channel chan;
  chan.name = std::to_string(endpoint.block->getID());
  chan.name += " ";
  chan.name += endpoint.block->getName();
  chan.name += " ";
  chan.name += endpoint.direction == IO::INPUT ? "INPUT" : "OUTPUT";
  chan.name += " ";
  chan.name += std::to_string(endpoint.port);
  chan.name += " ";
  chan.name += "Recording Component";
  chan.endpoint = endpoint;
  std::unique_ptr<DataRecorder::Component> component =
      std::make_unique<DataRecorder::Component>(this, chan.name);
  chan.data_source = component->get_fifo();
  Event::Object event(Event::Type::RT_THREAD_INSERT_EVENT);
  component->setActive(this->recording.load());
  event.setParam("thread", static_cast<RT::Thread*>(component.get()));
  this->getEventManager()->postEvent(&event);
  RT::block_connection_t connection;
  connection.src = endpoint.block;
  connection.src_port_type = endpoint.direction;
  connection.src_port = endpoint.port;
  connection.dest = component.get();
  connection.dest_port = 0;  // Recording components only have one input
  Event::Object connect_event(Event::Type::IO_LINK_INSERT_EVENT);
  connect_event.setParam("connection", std::any(connection));
  this->getEventManager()->postEvent(&connect_event);
  hid_t data_handle = H5I_INVALID_HID;
  const std::unique_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  // if (this->hdf5_handles.file_handle != H5I_INVALID_HID
  //     && this->hdf5_handles.sync_group_handle != H5I_INVALID_HID)
  // {
  //   const hid_t compression_property = H5Pcreate(H5P_DATASET_CREATE);
  //   H5Pset_deflate(compression_property, 7);
  //   data_handle = H5PTcreate(this->hdf5_handles.sync_group_handle,
  //                            chan.name.c_str(),
  //                            this->hdf5_handles.channel_index_datatype_handle,
  //                            this->m_data_chunk_size,
  //                            compression_property);
  // }
  this->m_recording_channels_list.emplace_back(
      chan, std::move(component), data_handle);
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
  const std::unique_lock<std::shared_mutex> recorder_lock(
      this->m_channels_list_mut);
  auto iter = std::find_if(this->m_recording_channels_list.begin(),
                           this->m_recording_channels_list.end(),
                           [component](const recorder_t& rec_chan)
                           { return rec_chan.component.get() == component; });
  if (iter == this->m_recording_channels_list.end()) {
    return;
  }
  H5PTclose(iter->hdf5_data_handle);
  this->m_recording_channels_list.erase(iter);
}

std::string DataRecorder::Plugin::getRecorderName(IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  auto iter = std::find_if(this->m_recording_channels_list.begin(),
                           this->m_recording_channels_list.end(),
                           [endpoint](const recorder_t& rec_chan)
                           { return rec_chan.channel.endpoint == endpoint; });
  if (iter == this->m_recording_channels_list.end()) {
    return "";
  }
  return iter->channel.name;
}

DataRecorder::Component* DataRecorder::Plugin::getRecorderPtr(
    IO::endpoint endpoint)
{
  const std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  auto iter = std::find_if(this->m_recording_channels_list.begin(),
                           this->m_recording_channels_list.end(),
                           [endpoint](const recorder_t& rec_chan)
                           { return rec_chan.channel.endpoint == endpoint; });
  if (iter == this->m_recording_channels_list.end()) {
    return nullptr;
  }
  return iter->component.get();
}

std::vector<DataRecorder::record_channel>
DataRecorder::Plugin::get_recording_channels()
{
  const std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  std::vector<DataRecorder::record_channel> result(
      this->m_recording_channels_list.size());
  for (size_t i = 0; i < result.size(); i++) {
    result[i] = this->m_recording_channels_list[i].channel;
  }
  return result;
}

int DataRecorder::Plugin::apply_tag(const std::string& tag)
{
  // const std::string tag_key =
  //     std::string("TAG") + std::to_string(this->tag_count++);
  // std::array<hsize_t, 1> string_dims = {tag.length()};
  // const hid_t tag_group_handle =
  // H5Gcreate(this->hdf5_handles.sync_group_handle,
  //                                          tag_key.c_str(),
  //                                          H5P_DEFAULT,
  //                                          H5P_DEFAULT,
  //                                          H5P_DEFAULT);
  // const hid_t dataspace_id = H5Screate_simple(1, string_dims.data(),
  // nullptr); const hid_t tag_name_handle = H5Dcreate(tag_group_handle,
  //                                         tag.c_str(),
  //                                         H5T_C_S1,
  //                                         dataspace_id,
  //                                         H5P_DEFAULT,
  //                                         H5P_DEFAULT,
  //                                         H5P_DEFAULT);
  return 0;
}

void DataRecorder::Plugin::process_data_worker()
{
  if (!this->open_file) {
    return;
  }
  std::vector<DataRecorder::data_token_t> data_buffer(this->m_data_chunk_size);
  std::vector<double> data_buffer_doubles;
  data_buffer_doubles.reserve(this->m_data_chunk_size);
  const size_t packet_byte_size = sizeof(DataRecorder::data_token_t);
  int64_t read_bytes = 0;
  size_t packet_count = 0;
  const TIME_TAG_TYPE time_type =
      dynamic_cast<DataRecorder::Panel*>(this->getPanel())->getTimeTagType();
  const std::shared_lock<std::shared_mutex> lk(this->m_channels_list_mut);
  switch (time_type) {
    case INDEX:
    case TIME:
      for (auto& channel : this->m_recording_channels_list) {
        while (read_bytes = channel.channel.data_source->read(
                   data_buffer.data(), packet_byte_size * data_buffer.size()),
               read_bytes > 0)
        {
          packet_count = static_cast<size_t>(read_bytes) / packet_byte_size;
          DataRecorder::Plugin::save_data(
              channel.hdf5_data_handle, data_buffer, packet_count);
        }
      }
      break;
    case NONE:
      for (auto& channel : this->m_recording_channels_list) {
        while (read_bytes = channel.channel.data_source->read(
                   data_buffer.data(), packet_byte_size * data_buffer.size()),
               read_bytes > 0)
        {
          packet_count = static_cast<size_t>(read_bytes) / packet_byte_size;
          if (packet_count > data_buffer_doubles.size()) {
            data_buffer_doubles.resize(packet_count);
          }
          for (size_t i = 0; i < packet_count; i++) {
            data_buffer_doubles.at(i) = data_buffer.at(i).value;
          }
          DataRecorder::Plugin::save_data(
              channel.hdf5_data_handle, data_buffer_doubles, packet_count);
          data_buffer_doubles.clear();
        }
      }
      break;
    default:
      ERROR_MSG(
          "DataRecorder::Plugin::process_data_worker : Bad time tagging type "
          "detected. Unable to save data to hdf5 file");
      break;
  }
}

void DataRecorder::Plugin::save_data(
    hid_t data_id,
    const std::vector<DataRecorder::data_token_t>& data,
    size_t packet_count)
{
  const herr_t err =
      H5PTappend(data_id, static_cast<hsize_t>(packet_count), data.data());
  if (err < 0) {
    ERROR_MSG("Unable to write data into hdf5 file!");
  }
}

void DataRecorder::Plugin::save_data(hid_t data_id,
                                     const std::vector<double>& data,
                                     size_t packet_count)
{
  const herr_t err =
      H5PTappend(data_id, static_cast<hsize_t>(packet_count), data.data());
  if (err < 0) {
    ERROR_MSG("Unable to write data into hdf5 file!");
  }
}

DataRecorder::Component::Component(Widgets::Plugin* hplugin,
                                   const std::string& probe_name)
    : Widgets::Component(hplugin,
                         probe_name,
                         DataRecorder::get_default_channels(),
                         DataRecorder::get_default_vars())
{
  if (RT::OS::getFifo(this->m_fifo, DataRecorder::DEFAULT_BUFFER_SIZE) != 0) {
    ERROR_MSG("Unable to create xfifo for Data Recorder Component {}",
              probe_name);
  }
}

void DataRecorder::Component::execute()
{
  DataRecorder::data_token_t data_sample;
  const double value = readinput(0);
  switch (this->getState()) {
    case RT::State::EXEC:
      switch (this->getValue<uint64_t>(PARAMETER::INDEXING)) {
        case INDEXING:
          data_sample.time = index++;
          break;
        case TIME:
          data_sample.time = RT::OS::getTime();
          break;
        case NONE:
        default:
          break;
      }
      data_sample.value = value;
      this->m_fifo->writeRT(&data_sample, sizeof(DataRecorder::data_token_t));
      break;
    case RT::State::UNPAUSE:
    case RT::State::INIT:
      index = 0;
      this->setState(RT::State::EXEC);
      break;
    case RT::State::PAUSE:
    case RT::State::EXIT:
    case RT::State::UNDEFINED:
    case RT::State::MODIFY:
    default:
      break;
  }
}

RT::OS::Fifo* DataRecorder::Component::get_fifo()
{
  return this->m_fifo.get();
}

std::unique_ptr<Widgets::Plugin> DataRecorder::createRTXIPlugin(
    Event::Manager* ev_manager)
{
  return std::make_unique<DataRecorder::Plugin>(ev_manager);
}

Widgets::Panel* DataRecorder::createRTXIPanel(QMainWindow* mwindow,
                                              Event::Manager* ev_manager)
{
  return static_cast<Widgets::Panel*>(
      new DataRecorder::Panel(mwindow, ev_manager));
}

std::unique_ptr<Widgets::Component> DataRecorder::createRTXIComponent(
    Widgets::Plugin* /*host_plugin*/)
{
  return std::unique_ptr<DataRecorder::Component>(nullptr);
}

Widgets::FactoryMethods DataRecorder::getFactories()
{
  Widgets::FactoryMethods fact;
  fact.createPanel = &DataRecorder::createRTXIPanel;
  fact.createComponent = &DataRecorder::createRTXIComponent;
  fact.createPlugin = &DataRecorder::createRTXIPlugin;
  return fact;
}
