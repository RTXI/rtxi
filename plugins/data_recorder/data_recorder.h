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

#ifndef DATA_RECORDER_H
#define DATA_RECORDER_H

#include <atomic_fifo.h>
#include <event.h>
#include <io.h>
#include <mutex.h>
#include <plugin.h>
#include <workspace.h>
#include <vector>
#include <time.h>
#include <daq.h>

#include <hdf5.h>
#include <hdf5_hl.h>

namespace DataRecorder
{
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
    long long time;
};

struct param_change_t
{
    Settings::Object::ID id;
    size_t index;
    long long step;
    double value;
};

void startRecording(void);
void stopRecording(void);
void openFile(const QString &);
void postAsyncData(const double *,size_t);

class CustomEvent : public QEvent
{
public:
    CustomEvent(QEvent::Type);
    virtual ~CustomEvent(void) {};
    void setData(void *data);
    void *getData(void);

private:
    void *data;
};

class Channel : public RT::List<Channel>::Node
{

    friend class Panel;

private:
    Channel(void);
    ~Channel(void);
    QString name;
    IO::Block *block;
    IO::flags_t type;
    size_t index;
}; // class Channel

class Panel : public QWidget, virtual public Settings::Object, public Event::Handler, public Event::RTHandler, public RT::Thread
{
    Q_OBJECT

public:
    Panel(QWidget *, size_t);
    ~Panel(void);

    void execute(void);
    void receiveEvent(const Event::Object *);
    void receiveEventRT(const Event::Object *);

public slots:
    void startRecordClicked(void);
    void stopRecordClicked(void);
    void updateDownsampleRate(int);

private slots:
    void buildChannelList(void);
    void changeDataFile(void);
    void insertChannel(void);
    void removeChannel(void);
    void addNewTag(void);

protected:
    void customEvent(QEvent *);
    virtual void doDeferred(const Settings::Object::State &);
    virtual void doLoad(const Settings::Object::State &);
    virtual void doSave(Settings::Object::State &) const;

private:
    static void *bounce(void *);
    void processData(void);
    int openFile(QString &);
    void closeFile(bool =false);
    int startRecording(long long);
    void stopRecording(long long);
    double prev_input;
    size_t counter;
    size_t downsample_rate;
    long long count;
    long long fixedcount;
    std::vector<std::string> dataTags;

    QMutex mutex;

    pthread_t thread;
    AtomicFifo fifo;
    data_token_t _token;
    bool tokenRetrieved;
    struct timespec sleep;

    struct file_t
    {
        hid_t id;
        hid_t trial;
        hid_t adata, cdata, pdata, sdata, tdata, sysdata;
        hid_t chandata;
        long long idx;
    } file;

    bool recording;

    QMdiSubWindow *subWindow;

    QGroupBox *channelGroup;
    QGroupBox *stampGroup;
    QGroupBox *sampleGroup;
    QGroupBox *fileGroup;
    QGroupBox *buttonGroup;
    QGroupBox *listGroup;

    QComboBox *blockList;
    QComboBox *channelList;
    QComboBox *typeList;
    QListWidget *selectionBox;
    QLabel *recordStatus;
    QPushButton *rButton;
    QPushButton *lButton;
    QPushButton *addTag;

    QSpinBox *downsampleSpin;

    QLineEdit *fileNameEdit;
    QLineEdit *timeStampEdit;
    QLineEdit *fileFormatEdit;
    QLabel *fileSizeLbl;
    QLabel *fileSize;
    QLabel *trialLengthLbl;
    QLabel *trialLength;
    QLabel *trialNumLbl;
    QLabel *trialNum;

    QPushButton *startRecordButton;
    QPushButton *stopRecordButton;
    QPushButton *closeButton;

    RT::List<Channel> channels;
    std::vector<IO::Block *> blockPtrList;
}; // class Panel

class Plugin : public QObject, public ::Plugin::Object
{
    Q_OBJECT

    friend class Panel;

public:
    static Plugin *getInstance(void);
    std::list<Panel *> panelList;

public slots:
    Panel *createDataRecorderPanel(void);

protected:
    virtual void doDeferred(const Settings::Object::State &);
    virtual void doLoad(const Settings::Object::State &);
    virtual void doSave(Settings::Object::State &) const;

private:

    Plugin(void);
    ~Plugin(void);
    Plugin(const Plugin &) {};
    Plugin &operator=(const Plugin &)
    {
        return *getInstance();
    };
    static Plugin *instance;

    void removeDataRecorderPanel(Panel *);

    size_t buffersize;
}; // class Plugin
}; // namespace DataRecorder

#endif /* DATA_RECORDER_H */
