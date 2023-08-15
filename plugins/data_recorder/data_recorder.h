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

#ifndef DATA_RECORDER_H
#define DATA_RECORDER_H

#include <QComboBox>
#include <QListWidget>
#include <QMutex>
#include <QSpinBox>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

#include <hdf5_hl.h>
#include <time.h>

#include "event.hpp"
#include "io.hpp"
#include "module.hpp"

namespace DataRecorder
{

typedef struct data_token_t
{
  int64_t time;
  double value;
} data_token_t;

constexpr size_t DEFAULT_BUFFER_SIZE = 10000 * sizeof(data_token_t);
constexpr std::string_view MODULE_NAME = "Data Recorder";

inline std::vector<Modules::Variable::Info> get_default_vars()
{
  return {};
}

inline std::vector<IO::channel_t> get_default_channels()
{
  return {{"Recording Channel",
           "This is the channel used by the Data Recorder to record on other "
           "inputs and "
           "output ports",
           IO::INPUT,
           0}};
}

typedef struct record_channel
{
  std::string name;
  IO::endpoint endpoint;
  RT::OS::Fifo* data_source;
  bool operator==(const record_channel& rhs) const
  {
    return (this->endpoint == rhs.endpoint)
        && (this->data_source == rhs.data_source);
  }
  bool operator!=(const record_channel& rhs) const { return !operator==(rhs); }
} record_channel;

class Component : public Modules::Component
{
public:
  Component(Modules::Plugin* hplugin, const std::string& probe_name);
  void execute() override;
  RT::OS::Fifo* get_fifo();

private:
  std::unique_ptr<RT::OS::Fifo> m_fifo;
};

class Panel : public Modules::Panel
{
  Q_OBJECT

public:
  Panel(const Panel&) = delete;
  Panel(Panel&&) = delete;
  Panel& operator=(const Panel&) = delete;
  Panel& operator=(Panel&&) = delete;
  Panel(QMainWindow* mwindow, Event::Manager* ev_manager);
  ~Panel() override = default;

signals:
  void updateBlockInfo();

public slots:
  void startRecordClicked();
  void stopRecordClicked();
  void updateDownsampleRate(size_t rate);
  void removeRecorders(IO::Block* block);

private slots:
  void buildBlockList();
  void buildChannelList();
  void changeDataFile();
  void insertChannel();
  void removeChannel();
  void addNewTag();
  void processData();

private:
  int openFile(QString& filename);
  size_t m_buffer_size = DEFAULT_BUFFER_SIZE;
  void closeFile();
  void startRecording();
  void stopRecording();
  size_t downsample_rate;
  std::vector<std::string> dataTags;

  bool recording;

  QGroupBox* channelGroup = nullptr;
  QGroupBox* stampGroup = nullptr;
  QGroupBox* sampleGroup = nullptr;
  QGroupBox* fileGroup = nullptr;
  QGroupBox* buttonGroup = nullptr;
  QGroupBox* listGroup = nullptr;

  QComboBox* blockList = nullptr;
  QComboBox* channelList = nullptr;
  QComboBox* typeList = nullptr;
  QListWidget* selectionBox = nullptr;
  QLabel* recordStatus = nullptr;
  QPushButton* rButton = nullptr;
  QPushButton* lButton = nullptr;
  QPushButton* addTag = nullptr;

  QSpinBox* downsampleSpin = nullptr;

  QLineEdit* fileNameEdit = nullptr;
  QLineEdit* timeStampEdit = nullptr;
  QLineEdit* fileFormatEdit = nullptr;
  QLabel* fileSizeLbl = nullptr;
  QLabel* fileSize = nullptr;
  QLabel* trialLengthLbl = nullptr;
  QLabel* trialLength = nullptr;
  QLabel* trialNumLbl = nullptr;
  QLabel* trialNum = nullptr;
  QTimer* recording_timer = nullptr;

  QPushButton* startRecordButton = nullptr;
  QPushButton* stopRecordButton = nullptr;
  QPushButton* closeButton = nullptr;
};  // class Panel

class Plugin : public Modules::Plugin
{
public:
  Plugin(const Plugin&) = delete;
  Plugin(Plugin&&) = delete;
  Plugin& operator=(const Plugin&) = delete;
  Plugin& operator=(Plugin&&) = delete;
  explicit Plugin(Event::Manager* ev_manager);
  ~Plugin() override;

  void receiveEvent(Event::Object* event) override;
  void startRecording();
  void stopRecording();
  void openFile(const std::string& file_name);
  void closeFile();
  void change_file(const std::string& file_name);
  int create_component(IO::endpoint endpoint);
  void destroy_component(IO::endpoint endpoint);
  std::string getRecorderName(IO::endpoint endpoint);
  DataRecorder::Component* getRecorderPtr(IO::endpoint endpoint);
  RT::OS::Fifo* getFifo(IO::endpoint endpoint);
  std::vector<record_channel> get_recording_channels();
  int apply_tag(const std::string& tag);
  void process_data_worker();

private:
  void append_new_trial();
  void close_trial_group(const std::unique_lock<std::shared_mutex>& lock);
  void open_trial_group(const std::unique_lock<std::shared_mutex>& lock);
  void save_data(hid_t data_id,
                 const std::vector<data_token_t>& data,
                 size_t packet_count);
  hsize_t m_data_chunk_size = static_cast<hsize_t>(1000);
  int m_compression_factor = 5;
  struct hdf5_handles
  {
    hid_t file_handle;
    hid_t trial_group_handle;
    hid_t attribute_handle;
    hid_t sync_group_handle;
    hid_t async_group_handle;
    hid_t sys_data_group_handle;
    hid_t channel_datatype_handle;
  } hdf5_handles;

  struct recorder
  {
    recorder(record_channel chan, std::unique_ptr<DataRecorder::Component> comp)
        : channel(std::move(chan))
        , component(std::move(comp))
    {
    }
    record_channel channel;
    std::unique_ptr<DataRecorder::Component> component;
    hid_t hdf5_data_handle;
  };

  int trial_count = 0;
  std::string hdf5_filename;
  std::vector<recorder> m_recording_channels_list;
  std::shared_mutex m_channels_list_mut;
  std::shared_mutex m_file_mut;
  std::mutex m_hdf5_file_mut;
  std::atomic<bool> open_file = false;
};  // class Plugin

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager);

Modules::Panel* createRTXIPanel(QMainWindow* main_window,
                                Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(
    Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();

};  // namespace DataRecorder

#endif /* DATA_RECORDER_H */
