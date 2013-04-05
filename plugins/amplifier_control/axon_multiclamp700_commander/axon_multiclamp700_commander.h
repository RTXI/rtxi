#ifndef MULTICLAMP_700_COMMANDER_H
#define MULTICLAMP_700_COMMANDER_H

#include <axon_multiclamp700_commanderUI.h>

#include <rt.h>
#include <settings.h>
#include <daq.h>
#include <io.h>
#include <plugin.h>

#include <qvbuttongroup.h>
#include <qwidget.h>
#include <qcombobox.h>

namespace Multiclamp700Commander {

    class Amplifier : public IO::Block {
        
    public:

        enum mode_t {
            Vclamp,
            Izero,
            Iclamp,
        };

        Amplifier( void );
        ~Amplifier( void );

        void setMode( mode_t );
        void setGains( double, double, double, double );
        mode_t getMode();

    private:

        DAQ::Device *device;

        double iclamp_ai_gain;
        double iclamp_ao_gain;
        double izero_ai_gain;
        double izero_ao_gain;
        double vclamp_ai_gain;
        double vclamp_ao_gain;
        double iclamp_out_gain;
        double vclamp_out_gain;
        mode_t mode;

    }; // class Amplifier
    
    class Panel : public QWidget, public RT::Thread, virtual public Settings::Object {
        Q_OBJECT

    public:

        Panel( QWidget * );
        ~Panel( void );

        Multiclamp700_UI *ui;
        int inputChan;
        int outputChan;

        Amplifier amp;

    public slots:
        void updateMode( int );
        void updateGains( void );
        void updateChannels( void );

    private:
        virtual void doLoad( const Settings::Object::State & );
        virtual void doSave( Settings::Object::State & ) const;

    }; // class Panel
    
    class Plugin : public QObject, public ::Plugin::Object {

        Q_OBJECT

        public:

        static Plugin *getInstance( void );

    public slots:

        void createMulticlamp700CommanderPanel( void );
        void removeMulticlamp700CommanderPanel( Panel * );

    protected:

        virtual void doLoad( const Settings::Object::State & );
        virtual void doSave( Settings::Object::State & ) const;
        
    private:

        Plugin( void );
        ~Plugin( void );
        Plugin( const Plugin & ) {};
        Plugin &operator=( const Plugin & ) { return *getInstance(); };

        static Plugin *instance;        

        int menuID;
        std::list< Panel * > panelList;

    }; // class Plugin

}; // namespace Multiclamp700Commander

#endif // MULTICLAMP_700_COMMANDER_H
