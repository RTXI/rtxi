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

#include <vector>
#include <mutex>
#include <thread>

#include <time.h>

#include <hdf5.h>
#include <hdf5_hl.h>

#include <QComboBox>
#include <QMutex>
#include <QListWidget>
#include <QSpinBox>

#include "io.hpp"
#include "event.hpp"
#include "module.hpp"

namespace DataRecorder
{

constexpr std::string_view MODULE_NAME = "data_recorder";
constexpr size_t TAG_SIZE = 1024;
enum data_type_t
{
  OPEN,
  CLOSE,
  START,
  STOP,
  SYNC,
  ASYNC,
  DONE,
  PARAM,
};

struct data_token_t
{
  data_type_t type;
  size_t size;
  int64_t time;
};

//struct param_change_t
//{
//  Settings::Object::ID id;
//  size_t index;
//  long long step;
//  double value;
//};

typedef struct Channel 
{
  std::string name;
  std::unique_ptr<RT::OS::Fifo> fifo;
  IO::endpoint endpoint; 
}Channel;  // class Channel

class Component : public Modules::Component
{
public:
  void execute() override;
private:
  RT::OS::Fifo* m_fifo;
};

class Panel: public Modules::Panel 
{
  Q_OBJECT

public:
  Panel(const Panel&) = delete;
  Panel(Panel&&) = delete;
  Panel& operator=(const Panel&) = delete;
  Panel& operator=(Panel&&) = delete;
  Panel(MainWindow* mwindow, Event::Manager* ev_manager);
  ~Panel() override;

 public slots:
  void startRecordClicked();
  void stopRecordClicked();
  void updateDownsampleRate(int);

private slots:
  void buildChannelList();
  void changeDataFile();
  void insertChannel();
  void removeChannel();
  void addNewTag();

private:
  int openFile(QString&);
  void closeFile(bool = false);
  int startRecording(int64_t);
  void stopRecording(int64_t);
  double prev_input;
  size_t counter;
  size_t downsample_rate;
  int64_t count;
  int64_t fixedcount;
  std::vector<std::string> dataTags;

  data_token_t _token;
  bool tokenRetrieved;
  struct timespec sleep;

  struct file_t
  {
    hid_t id;
    hid_t trial;
    hid_t adata, cdata, pdata, sdata, tdata, sysdata;
    hid_t chandata;
    int64_t idx;
  } file;

  bool recording;

  QMdiSubWindow* subWindow=nullptr;

  QGroupBox* channelGroup=nullptr;
  QGroupBox* stampGroup=nullptr;
  QGroupBox* sampleGroup=nullptr;
  QGroupBox* fileGroup=nullptr;
  QGroupBox* buttonGroup=nullptr;
  QGroupBox* listGroup=nullptr;

  QComboBox* blockList=nullptr;
  QComboBox* channelList=nullptr;
  QComboBox* typeList=nullptr;
  QListWidget* selectionBox=nullptr;
  QLabel* recordStatus=nullptr;
  QPushButton* rButton=nullptr;
  QPushButton* lButton=nullptr;
  QPushButton* addTag=nullptr;

  QSpinBox* downsampleSpin=nullptr;

  QLineEdit* fileNameEdit=nullptr;
  QLineEdit* timeStampEdit=nullptr;
  QLineEdit* fileFormatEdit=nullptr;
  QLabel* fileSizeLbl=nullptr;
  QLabel* fileSize=nullptr;
  QLabel* trialLengthLbl=nullptr;
  QLabel* trialLength=nullptr;
  QLabel* trialNumLbl=nullptr;
  QLabel* trialNum=nullptr;

  QPushButton* startRecordButton=nullptr;
  QPushButton* stopRecordButton=nullptr;
  QPushButton* closeButton=nullptr;

  std::list<Channel> channels;
  std::vector<IO::Block*> blockPtrList;
};  // class Panel

class Plugin : public Modules::Plugin
{
  Q_OBJECT
public:
  void receiveEvent(Event::Object* event) override;
  void startRecording();
  void stopRecording();
  void openFile(const std::string& file_name);
  void change_file(const std::string& file_name);
  int create_component(IO::endpoint& chan); 

public slots:

private:
  void processData();
  std::thread m_processdata_thread;
  std::list<DataRecorder::Component> m_components_list;
  std::vector<DataRecorder::Channel> m_channels_list;
  std::mutex m_channels_list_mut;
  std::mutex m_hdf5_file_mut;
};  // class Plugin

std::unique_ptr<Modules::Plugin> createRTXIPlugin(Event::Manager* ev_manager,
                                                  MainWindow* main_window);

Modules::Panel* createRTXIPanel(MainWindow* main_window,
                                Event::Manager* ev_manager);

std::unique_ptr<Modules::Component> createRTXIComponent(
    Modules::Plugin* host_plugin);

Modules::FactoryMethods getFactories();

};  // namespace DataRecorder

#endif /* DATA_RECORDER_H */
