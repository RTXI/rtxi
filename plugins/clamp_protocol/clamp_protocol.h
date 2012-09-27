#ifndef CLAMP_PROTOCOL_H
#define CLAMP_PROTOCOL_H

#include "ui/CP_main_windowUI.h"
#include "CP_protocol.h"
#include "CP_plot_window.h"

#include <vector>

#include <rt.h>
#include <settings.h>
#include <fifo.h>
#include <workspace.h>
#include <event.h>
#include <plugin.h>

#include <qtimer.h>
#include <qobject.h>
#include <qwidget.h>

namespace ClampProtocol {

    class Panel;

    class Plugin : public QObject, public ::Plugin::Object {

        Q_OBJECT

        friend class Panel;

    public:
        static Plugin *getInstance( void );

    public slots:
        void createClampProtocolPanel( void ); // Create panel GUI through menu item

    protected:
        void doDeferred( const Settings::Object::State & );
        void doLoad( const Settings::Object::State & );
        void doSave( Settings::Object::State & ) const;

    private:
        Plugin( void );
        ~Plugin( void );
        Plugin( const Plugin & ) { };
        Plugin &operator=( const Plugin & ) { return *getInstance(); };

        void removeClampProtocolPanel( Panel * ); // Remove panel GUI

        static Plugin *instance;

        int menuID;
        std::list< Panel * > panelList;
    }; // class Plugin

    class Panel : public QWidget, public RT::Thread, public Workspace::Instance, public Event::Handler, public Event::RTHandler {
        Q_OBJECT

        // Bitmask flags
        static const IO::flags_t INPUT = Workspace::INPUT;
        static const IO::flags_t OUTPUT = Workspace::OUTPUT;
        static const IO::flags_t PARAMETER = Workspace::PARAMETER;
        static const IO::flags_t STATE = Workspace::STATE;
        static const IO::flags_t EVENT = Workspace::EVENT;
        static const IO::flags_t COMMENT = Workspace::COMMENT;

    public:
        Panel( QWidget * );
        ~Panel( void );

        void execute( void );
        void receiveEvent( const ::Event::Object * );
        void receiveEventRT( const ::Event::Object * );
        void removePlotWindow( PlotWindow * ); // Remove plot window
                                                      
    public slots:
        void pause( bool ); // Sets RT thread to inactive and outputs to 0
        void updateDisplay( void ); // Update display of states
        void updatePlot( void ); // Update plot windows
        void updateOptions( void ); // Update trial options, through refresh button or return key while editting option
        void loadProtocol( void );
        void openProtocolEditor( void );
        void openPlotWindow( void );        
        void toggleProtocol( void ); // Toggles protocol run

    protected:
        void doDeferred( const Settings::Object::State & );
        void doLoad( const Settings::Object::State & );
        void doSave( Settings::Object::State & ) const;

    private:
        friend class UpdateOptionsEvent;
        friend class ToggleProtocolEvent;

        class UpdateOptionsEvent : public RT::Event {

        public:
            UpdateOptionsEvent( Panel *, int, int, double, bool ); // Updates interval time, number of trials, and data recording
            ~UpdateOptionsEvent( void ) { };

            int callback( void );

        private:
            Panel *panel;
            int intervalTimeValue;
            int numTrialsValue;
            double junctionPotentialValue;
            bool recordDataValue;

        }; // class UpdateOptionsEvent

        class ToggleProtocolEvent;

        class ToggleProtocolEvent : public RT::Event {

        public:
            ToggleProtocolEvent( Panel *, bool, bool ); // Toggles running of protocol and whether data recording starts/stops
            ~ToggleProtocolEvent( void ) { };

            int callback( void );

        private:
            Panel *panel;
            bool protocolOn;
            bool recordData;

        }; // class ToggleProtocolEvent

        // GUI
        CP_main_windowUI *mainWindow;
        std::list< PlotWindow * > plotWindowList;
        QTimer *plotTimer;

        // State Variables - Workspace only takes doubles, but calculations use int to avoid equality issues
        double trial;
        double time;
        double segmentNumber;
        double sweep;
        double voltage; // Voltage output without LJP
        
        // Parameters
        double intervalTime; // Wait time between trials
        int numTrials;
        double junctionPotential;
        
        // Protocol
        Protocol protocol;
        enum executeMode_t { IDLE, PROTOCOL } executeMode;
        enum protocolMode_t { SEGMENT, STEP, EXECUTE, END, WAIT } protocolMode;
        Step step;
        ProtocolStep::stepType_t stepType;
        double period;
        int segmentIdx; // Current segment
        int sweepIdx; // Current sweep
        int stepIdx; // Current step
        int trialIdx; // Current trial
        int numSweeps; // Number of sweeps for the current segment
        int numSteps; // Number of steps in the current segment
        int stepTime, stepEndTime; // Time elapsed during the current step
        double protocolEndTime; // Used to determine time elapsed between trials
        double stepOutput;
        double outputFactor;
        double inputFactor;
        double rampIncrement;
        double pulseWidth;
        int pulseRate;
        Fifo fifo;
        std::vector<double> data;
        

        // Plotting variables                
        double prevSegmentEnd; // Time segment ends after its first sweep        
        int stepStart; // Time when step starts divided by period
        curve_token_t token;
        
        // Flags
        bool recordData; // If record data checkbox is ticked
        bool recording; // If data is being recorded
        bool plotting; // If a plot window is opened

        signals:
        void plotCurve( double *, curve_token_t );
        
    }; // class Panel
    
}; // namespace ClampProtocol

#endif // CLAMP_PROTOCOL_H
