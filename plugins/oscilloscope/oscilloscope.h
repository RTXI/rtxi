#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include "scope.h"

#include <event.h>
#include <fifo.h>
#include <io.h>
#include <mutex.h>
#include <plugin.h>
#include <qdialog.h>
#include <rt.h>
#include <settings.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLineEdit;
class QPushButton;
class QSpinBox;
class QTabWidget;

namespace Oscilloscope {

    class Panel;

    class Properties : public QDialog, public Event::Handler {

        Q_OBJECT

        friend class Panel;

    public:

        Properties(Panel *);
        ~Properties(void);

        void receiveEvent(const ::Event::Object *);

    protected:

        void closeEvent(QCloseEvent *);

    private slots:

        void activateChannel(bool);
        void apply(void);
        void buildChannelList(void);
        void okay(void);
        void showTab(void);

    private:

        void applyAdvancedTab(void);
        void applyChannelTab(void);
        void applyDisplayTab(void);
        void createAdvancedTab(void);
        void createChannelTab(void);
        void createDisplayTab(void);
        void showAdvancedTab(void);
        void showChannelTab(void);
        void showDisplayTab(void);

        Panel *panel;

        QTabWidget *tabWidget;

        QSpinBox *divXSpin;
        QSpinBox *divYSpin;
        QSpinBox *rateSpin;
        QSpinBox *sizeSpin;

        QComboBox *blockList;
        QComboBox *channelList;
        QComboBox *colorList;
        QComboBox *offsetList;
        QComboBox *scaleList;
        QComboBox *styleList;
        QComboBox *typeList;
        QComboBox *widthList;
        QGroupBox *displayBox;
        QGroupBox *lineBox;
        QLineEdit *offsetEdit;
        QPushButton *activateButton;

        QButtonGroup *trigGroup;
        QComboBox *timeList;
        QComboBox *trigChanList;
        QComboBox *trigThreshList;
        QSpinBox *refreshSpin;
        QLineEdit *trigThreshEdit;
        QCheckBox *trigHoldingCheck;
        QLineEdit *trigHoldoffEdit;
        QComboBox *trigHoldoffList;

    }; // class Properties

    class Plugin : public QObject, public ::Plugin::Object, public RT::Thread {

        Q_OBJECT

        friend class Panel;

    public:

        static Plugin *getInstance(void);

    public slots:

        void createOscilloscopePanel(void);

    protected:

        virtual void doDeferred(const Settings::Object::State &);
        virtual void doLoad(const Settings::Object::State &);
        virtual void doSave(Settings::Object::State &) const;

    private:

        Plugin(void);
        ~Plugin(void);
        Plugin(const Plugin &) {};
        Plugin &operator=(const Plugin &) { return *getInstance(); };

        static Plugin *instance;

        void removeOscilloscopePanel(Panel *);

        int menuID;
        std::list<Panel *> panelList;

    }; // Plugin

    class Panel : public Scope, public RT::Thread, public virtual Settings::Object {

        Q_OBJECT

        friend class Properties;

    public:

        Panel(QWidget *);
        virtual ~Panel(void);

        void execute(void);

        bool setInactiveSync(void);
        void flushFifo(void);
        void adjustDataSize(void);

        void doDeferred(const Settings::Object::State &);
        void doLoad(const Settings::Object::State &);
        void doSave(Settings::Object::State &) const;

    public slots:

        void showProperties(void);
        void timeoutEvent(void);

    protected:

        void mouseDoubleClickEvent(QMouseEvent *);
        void mousePressEvent(QMouseEvent *);

    private:

        Fifo fifo;
        Properties *properties;
        std::vector<IO::Block *> blocks;

    }; // class Panel

}; // class Oscilloscope

#endif // OSCILLOSCOPE_H
