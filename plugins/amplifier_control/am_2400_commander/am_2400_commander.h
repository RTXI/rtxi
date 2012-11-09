#ifndef AM_2400_COMMANDER_H
#define AM_2400_COMMANDER_H


#include <am_2400_commanderUI.h>

#include <daq.h>
#include <io.h>
#include <plugin.h>
#include <qvbuttongroup.h>
#include <qwidget.h>
#include <rt.h>
#include <settings.h>

namespace AM2400Commander {

    class Amplifier : public IO::Block {
        
    public:

        enum mode_t {
            Vclamp,
            Izero,
            Iclamp,
            Vcomp,
            Vtest,            
            Iresist,
            Ifollow
        };

        Amplifier(void);
        ~Amplifier(void);

        void setMode(mode_t, int, int);
        mode_t getMode();

    private:

        void setIclamp(void);
        void setVclamp(void);

        DAQ::Device *device;

        double iclamp_ai_gain;
        double iclamp_ao_gain;
        double izero_ai_gain;
        double izero_ao_gain;
        double vclamp_ai_gain;
        double vclamp_ao_gain;
        double iclamp_ao_offset;
        mode_t mode;

    }; // class Amplifier
    
    class Panel : public QWidget, public RT::Thread, virtual public Settings::Object {
        Q_OBJECT

    public:

        Panel(QWidget *);
        ~Panel(void);

        QVButtonGroup *buttonGroup;

        Amplifier amp;

    public slots:
        void updateMode(int);
        void updateChannels(void);

    private:        
        virtual void doLoad(const Settings::Object::State &);
        virtual void doSave(Settings::Object::State &) const;

        AM_Amp_CommanderUI *ui;
        int inputChan;
        int outputChan;

    }; // class Panel
    
    class Plugin : public QObject, public ::Plugin::Object {

        Q_OBJECT

        public:

        static Plugin *getInstance(void);

        //   Amplifier *getAmplifier(size_t =0);

    public slots:

        void createAM2400CommanderPanel(void);
        void removeAM2400CommanderPanel(Panel *);
        //   void updateMode(int);

    protected:

        virtual void doLoad(const Settings::Object::State &);
        virtual void doSave(Settings::Object::State &) const;

    private:

        Plugin(void);
        ~Plugin(void);
        Plugin(const Plugin &) {};
        Plugin &operator=(const Plugin &) { return *getInstance(); };

        static Plugin *instance;        

        int menuID;
        std::list<Panel *> panelList;

    }; // class Plugin

}; // namespace AM2400Commander

#endif // AM_2400_COMMANDER_H
