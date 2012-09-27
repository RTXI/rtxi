#include "clamp_protocol.h"
#include "CP_protocol_editor.h"

#include <cmath>
#include <iostream>
#include <debug.h>
#include <main_window.h>
#include <qfiledialog.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qstring.h>
#include <qtooltip.h>
#include <qcheckbox.h>
#include <qspinbox.h>
#include <qvalidator.h>
#include <qwt-qt3/qwt_legend.h>

static Workspace::variable_t vars[] = { // Variables
    {
        "Current Input (A)", "", Workspace::INPUT, },
    {
        "V Output (V) (w/ LJP)", "", Workspace::OUTPUT, },
    {
        "Protocol Name", "", Workspace::COMMENT, }, // Name of loaded protocol
    {
        "Trial", "", Workspace::STATE, }, // Number of current trial
    {
        "Segment", "", Workspace::STATE, }, // Number of current segment in protocol
    {
        "Sweep", "", Workspace::STATE, }, // Sweep number of current segment
    {
        "Time (ms)", "", Workspace::STATE, }, // Time elapsed for current trial
    {
        "V Output (mV) (w/o LJP)", "", Workspace::STATE, }, // Voltage output without liquid junction potential
    {
        "Interval Time", "", Workspace::PARAMETER }, // Time allocated between trials
    {
        "Number of Trials", "", Workspace::PARAMETER }, // Number of trials to be run
    {
        "Liquid Junction Potential (mv)", "", Workspace::PARAMETER }, // Number of trials to be run
};

static size_t num_vars = sizeof( vars ) / sizeof( Workspace::variable_t ); // Required variable (number of variables)

// Class Panel - GUI and Execution
ClampProtocol::Panel::Panel( QWidget *parent )
 : QWidget( parent, 0, Qt::WStyle_NormalBorder | Qt::WDestructiveClose ),
   RT::Thread( 0 ),
   Workspace::Instance( "Clamp Protocol", vars, num_vars ),
   trial( 1 ), time( 0 ), segmentNumber( 1 ), sweep( 1 ),  voltage( 0 ), intervalTime( 1000 ),
   numTrials( 1 ), junctionPotential( 0 ), executeMode( IDLE ), segmentIdx( 0 ), sweepIdx( 0 ),
   stepIdx( 0 ), trialIdx( 0 ), fifo( 10 * 1048576 ), recordData( false ), recording( false ), plotting( false )
   {
    
    mainWindow = new CP_main_windowUI( this ); // GUI built using Qt Desinger
    setCaption( QString::number( getID() ) + " Clamp Protocol" );

    // Construct main layout
    QBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( mainWindow );
    
    // Set refresh and plot timer
    QTimer *refreshTimer = new QTimer( this );
	refreshTimer->start(100);
    plotTimer = new QTimer( this ); // Only started when a plot window is open

    // Set validator for line edit
    mainWindow->junctionPotentialEdit->setValidator( new QDoubleValidator(mainWindow->junctionPotentialEdit) );
    
    // GUI connection
    QObject::connect( mainWindow->pauseButton, SIGNAL(toggled(bool)), this, SLOT( pause(bool)) );
    QObject::connect( mainWindow->runProtocolButton, SIGNAL(clicked(void)), this, SLOT( toggleProtocol(void)) );
    QObject::connect( mainWindow->modifyButton, SIGNAL(clicked(void)), this, SLOT(updateOptions(void)) );
    QObject::connect( mainWindow->intervalTimeEdit, SIGNAL(valueChanged(int)), this, SLOT(updateOptions(void)) );
    QObject::connect( mainWindow->trialsEdit, SIGNAL(valueChanged(int)), this, SLOT(updateOptions(void)) );
    QObject::connect( mainWindow->junctionPotentialEdit, SIGNAL(returnPressed(void)), this, SLOT(updateOptions(void)) );
    QObject::connect( mainWindow->loadProtocolButton, SIGNAL(clicked(void)), this, SLOT( loadProtocol(void)) );
    QObject::connect( mainWindow->protocolEditorButton, SIGNAL(clicked(void)), this, SLOT( openProtocolEditor(void)) );
    QObject::connect( mainWindow->plotWindowButton, SIGNAL(clicked(void)), this, SLOT( openPlotWindow(void)) );
    QObject::connect( mainWindow->dataRecordCheckBox, SIGNAL(clicked(void)), this, SLOT( updateOptions(void)) );
    QObject::connect( refreshTimer, SIGNAL(timeout(void)), this, SLOT(updateDisplay(void)) );
    QObject::connect( plotTimer, SIGNAL(timeout(void)), this, SLOT(updatePlot(void)) );

    // Set parameter inputs to default values
    mainWindow->intervalTimeEdit->setValue( intervalTime );
    mainWindow->trialsEdit->setValue( numTrials );
    mainWindow->junctionPotentialEdit->setText( QString::number(junctionPotential) );
    mainWindow->protocolNameEdit->setText( "No protocol loaded" );
    QToolTip::add( mainWindow->protocolNameEdit, "No protocol loaded" );

    Workspace::Instance::setComment( 0, "No protocol loaded" );
    
    // Connect states to workspace
    setData( Workspace::STATE, 0, &trial );
    setData( Workspace::STATE, 1, &segmentNumber );
    setData( Workspace::STATE, 2, &sweep );
    setData( Workspace::STATE, 3, &time );
    setData( Workspace::STATE, 4, &voltage );
    
    resize( minimumSize() ); // Set window size to minimum
    show();
    setActive( true );
}

ClampProtocol::Panel::~Panel( void ) {
    setActive( false ); // Stop RT thread
    
    Plugin::getInstance()->removeClampProtocolPanel( this );

    while( plotWindowList.size() )
        delete plotWindowList.front();
}

// Settings Functions
void ClampProtocol::Panel::doDeferred( const Settings::Object::State & ) {}
 
void ClampProtocol::Panel::doLoad( const Settings::Object::State &s ) {
    if ( s.loadInteger("Maximized") )
        showMaximized();
    else if ( s.loadInteger("Minimized") )
        showMinimized();

    // Window Position
    if ( s.loadInteger( "W" ) != NULL ) {
        resize( s.loadInteger("W"), s.loadInteger("H") );
        parentWidget()->move( s.loadInteger("X"), s.loadInteger("Y") );
    }

    // Load saved parameter values
    intervalTime = s.loadDouble( "Interval Time" );
    numTrials = s.loadInteger( "Num Trials" );
    junctionPotential = s.loadDouble( "Junction Potential" );
    recordData = s.loadInteger( "Record Data" );
    mainWindow->intervalTimeEdit->setValue( intervalTime );
    mainWindow->trialsEdit->setValue( numTrials );
    mainWindow->junctionPotentialEdit->setText( QString::number(junctionPotential) );
    mainWindow->dataRecordCheckBox->setChecked( recordData );

    // Load File
    QString fileName = s.loadString( "Protocol" );
    if( QString::compare( fileName, "No protocol loaded" ) != 0 ) {
        QDomDocument doc( "protocol" );
        QFile file( fileName );

        if( !file.open( IO_ReadOnly ) ) { // Make sure file can be opened, if not, warn user
            QMessageBox::warning(this,
                                 "Error",
                                 "Unable to open protocol file saved in settings" );
            return ;
        }
        if( !doc.setContent( &file ) ) { // Make sure file contents are loaded into document        
            QMessageBox::warning(this,
                                 "Error",
                                 "Unable to set file contents to document" );
            file.close();
            return ;
        }
        file.close();

        protocol.fromDoc( doc ); // Translate document into protocol
    
        if( protocol.numSegments() <= 0 ) { // If no segments exist
            QMessageBox::warning(this,
                                 "Error",
                                 "Protocol did not contain any segments" );
        }
        QString text = fileName.right( abs( int( fileName.findRev("/") - fileName.length() ) ) - 1 ); // Typecast to int since length() returns unsigned int
        mainWindow->protocolNameEdit->setText( text );
        QToolTip::add( mainWindow->protocolNameEdit, fileName );
    }
    
    // Load plot windows
    for( size_t i = 0 ; i < static_cast<size_t>( s.loadInteger("Num Plots") ) ; ++i ) {
        PlotWindow *plotWindow = new PlotWindow( MainWindow::getInstance()->centralWidget(), this );
        QObject::connect( this, SIGNAL(plotCurve(double * , curve_token_t)), plotWindow, SLOT(addCurve(double *, curve_token_t)) );
        plotWindowList.push_back ( plotWindow );;
        plotWindow->show();
        plotWindow->setCaption( QString::number( getID() ) + " Clamp Protocol: Plot Window (" + QString::number( i + 1 ) + ")" );
        plotWindow->load( s.loadState( QString::number(i) ) );
    }

    if( s.loadInteger( "Num Plots" ) > 0 ) {
        plotting = true;
        plotTimer->start(25); // 25ms refresh rate for plotting
    }
}

void ClampProtocol::Panel::doSave( Settings::Object::State &s ) const {
    if ( isMaximized() )
        s.saveInteger( "Maximized", 1 );
    else if ( isMinimized() )
        s.saveInteger( "Minimized", 1 );

    // Window Position
    QPoint pos = parentWidget()->pos();
    s.saveInteger( "X", pos.x() );
    s.saveInteger( "Y", pos.y() );
    s.saveInteger( "W", width() );
    s.saveInteger( "H", height() );
    
    // Save parameter values directly from GUI
    s.saveInteger( "Interval Time", mainWindow->intervalTimeEdit->value() );
    s.saveInteger( "Num Trials", mainWindow->trialsEdit->value() );
    s.saveDouble( "Junction Potential", mainWindow->junctionPotentialEdit->text().toDouble() );
    s.saveInteger( "Record Data", mainWindow->dataRecordCheckBox->isChecked() );
    s.saveString( "Protocol", Workspace::Instance::getValueString(Workspace::COMMENT, 0) );

    // Save states for each plot window
    s.saveInteger( "Num Plots", plotWindowList.size() );
    size_t n = 0;
    for( std::list<PlotWindow *>::const_iterator i = plotWindowList.begin(), end = plotWindowList.end() ; i != end ; ++i )
        s.saveState( QString::number(n++), (*i)->save() );
}

void ClampProtocol::Panel::execute( void ) { // RT thread execution
    switch( executeMode ) {
        
    case IDLE:
        break;

    case PROTOCOL:               

        if( protocolMode == END ) { // End of protocol
            if( recordData && recording ) { // Data record checkbox is ticked and data recorder is on
                // Stop data recorder
                Event::Object event(Event::STOP_RECORDING_EVENT);
                Event::Manager::getInstance()->postEventRT(&event);
            }

            if( trialIdx < ( numTrials - 1 ) ) { // Restart protocol if additional trials are needed
                trialIdx++; // Advance trial
                trial++;
                protocolEndTime = RT::OS::getTime() * 1e-6; // Time at end of protocol (ms)
                protocolMode = WAIT; // Wait for interval time to be finished
            }
            else { // All trials finished
                executeMode = IDLE;
            }
        } // end ( protocolMode == END )

        if( protocolMode == SEGMENT ) { // Segment initialization
            numSweeps = protocol.numSweeps( segmentIdx );
            numSteps = protocol.numSteps( segmentIdx );            
            protocolMode = STEP; // Move on to step initialization            
        } // end ( protocolMode == SEGMENT )

        if( protocolMode == STEP ) { // Step initialization
            step = protocol.getStep( segmentIdx, stepIdx ); // Retrieve step pointer
            stepType = step->stepType; // Retrieve step type
            stepTime = 0;
            
            // Initialize step variables
            stepEndTime = ( ( step->stepDuration + ( step->deltaStepDuration * (sweepIdx) ) ) / period ) - 1; // Unitless to prevent rounding errors
            stepOutput = step->holdingLevel1 + ( step->deltaHoldingLevel1 * (sweepIdx) );

            if( stepType == ProtocolStep::RAMP ) {
                double h2 = step->holdingLevel2 + ( step->deltaHoldingLevel2 * (sweepIdx) ); // End of ramp value
                rampIncrement = ( h2 - stepOutput ) / stepEndTime; // Slope of ramp
            }
            else if ( stepType == ProtocolStep::TRAIN ) {
                pulseWidth = step->pulseWidth / period; // Unitless to prevent rounding errors
                pulseRate = step->pulseRate / ( period * 1000 ); // Unitless to prevent rounding errors
            }
 
            // Factors will help if switching modes
            outputFactor = 1e-3;; // stepOutput in mV, must convert to V
            inputFactor = 1e9; // input in A, conver to nA

            if( plotting )
                stepStart = time / period;
            
            protocolMode = EXECUTE; // Move on to tep execution    
        } // end ( protocolMode == STEP )
        
        if( protocolMode == EXECUTE ) { // Assuming whole-cell configuration, Vcmd = Vm + LJP
            switch( stepType ) {
            case ProtocolStep::STEP:
                voltage = stepOutput;
                output( 0 ) = ( voltage + junctionPotential ) * outputFactor;
                break;
                
            case ProtocolStep::RAMP:
                voltage = ( stepOutput + (stepTime * rampIncrement) );
                output( 0 ) = ( voltage + junctionPotential ) * outputFactor;
                break;
                
            case ProtocolStep::TRAIN:
                if( stepTime % pulseRate < pulseWidth ) {
                    voltage =  stepOutput;
                    output( 0 ) = ( voltage + junctionPotential ) * outputFactor;
                }
                else {
                    voltage = 0;
                    output( 0 ) = ( voltage + junctionPotential ) * outputFactor;
                }
                break;
                
            default:
                std::cout << "Error - ClampProtocol::execute(): switch( stepType ) default case reached" << std::endl;
                break;
            } // end switch( stepType)

            stepTime++;

            if( plotting ) // Track data if plot window is open
            data.push_back( input(0) * inputFactor );
            
            if( stepTime > stepEndTime ) { // If step is finished

                if( plotting ) { // Add data to fifo, GUI thread will plot data to all plot windows after a timeout
                    int stepStartSweep = 0;

                    // Start point is sum of all previous segment lengths plus previous steps, used in sweep overlay
                    for( int i = 0; i < segmentIdx; i++ )
                        stepStartSweep += protocol.segmentLength( segmentIdx - 1, period, false );

                    for( int i = 0; i < stepIdx; i++ )
                        stepStartSweep += protocol.getStep( segmentIdx, i )->stepDuration / period;

                    // Offset by 1 since an additional step is added to connect curves
                    token.trial = trialIdx;
                    token.sweep = sweepIdx;
                    token.stepStartSweep = stepStartSweep;
                    token.stepStart = stepStart - 1; // stepStart has no units, period is used to convert to time
                    token.period = period;
                    token.points = data.size();
                    token.lastStep = false;
                }

                stepIdx++;
                protocolMode = STEP;
                
                if( stepIdx == numSteps ) { // If done with all steps
                    sweepIdx++;
                    sweep++;
                    stepIdx = 0;
                    
                    if( sweepIdx == numSweeps ) { // If done with all sweeps
                        segmentIdx++;
                        segmentNumber++;
                        sweepIdx = 0;
                        sweep = 1;                        
                        
                        protocolMode = SEGMENT; // Move on to next segment

                        if( segmentIdx >= protocol.numSegments() ) {// If finished with all segments
                            protocolMode = END; // End protocol
                            token.lastStep = true;
                        }
                    }
                }            

                if( plotting ) { // If plotting, write to fifo
                    fifo.write( &token, sizeof(token) );
                    fifo.write( &data[0], token.points * sizeof(double) );

                    data.clear(); // Clear data vector in preparation for next step

                    // Re-add previous point in order to connect curves
                    data.push_back( input(0) * inputFactor );
                }                
            } // end stepTime > stepEndTime                            
        } // end ( protocolMode == EXECUTE )        

        if( protocolMode == WAIT ) { // Wait between trials
            if( ( (RT::OS::getTime() * 1e-6) - protocolEndTime ) > intervalTime ) { // If interval time has been reached
                time = 0; // -period due to time increment before new trial starts
                segmentIdx = 0;
                 if( recordData && !recording ) {                     
                    Event::Object event(Event::START_RECORDING_EVENT);
                    Event::Manager::getInstance()->postEventRT(&event);
                }
                protocolMode = SEGMENT;
                executeMode = PROTOCOL;
            }
            return ;
        } // end ( protocolMode == WAIT )

        // Update States
        time += period;        
        
       break; // end PROTOCOL
        
    default:
        break;        
        
    } // end switch(executeMode)
} // end execute


// Event handling
void ClampProtocol::Panel::receiveEvent( const ::Event::Object *event ) {
    if( event->getName() == Event::RT_POSTPERIOD_EVENT )
        period = RT::System::getInstance()->getPeriod()*1e-6; // Grabs RTXI thread period and converts to ms (from ns)    
    if( event->getName() == Event::START_RECORDING_EVENT )
        recording = true;
    if( event->getName() == Event::STOP_RECORDING_EVENT )
        recording = false;
}

void ClampProtocol::Panel::receiveEventRT( const ::Event::Object *event ) {
    if( event->getName() == Event::RT_POSTPERIOD_EVENT )
        period = RT::System::getInstance()->getPeriod()*1e-6; // Grabs RTXI thread period and converts to ms (from ns)
    if( event->getName() == Event::START_RECORDING_EVENT )
        recording = true;
    if( event->getName() == Event::STOP_RECORDING_EVENT )
        recording = false;
}

// Panel Slot Functions
void ClampProtocol::Panel::pause( bool paused ) { // Outputs set to 0, RT thread halted
    if( paused ) {
        setActive( false );
        output( 0 ) = 0;
    }
    else
        setActive( true );    
}

void ClampProtocol::Panel::updateDisplay( void ) { // GUI refresh
    mainWindow->trialNumberEdit->setText( QString::number(trial) );
    mainWindow->segmentEdit->setText( QString::number(segmentNumber) );
    mainWindow->sweepEdit->setText( QString::number(sweep) );
    mainWindow->timeEdit->setText( QString::number(time) );

    if( mainWindow->runProtocolButton->isOn() ) // If protocol button is down / protocol running
        if( executeMode == IDLE ) { // If protocol finished
            mainWindow->runProtocolButton->setOn( false ); // Untoggle run button
        }
}

void ClampProtocol::Panel::updatePlot( void ) { // Plot refresh    
    curve_token_t token;

    // Read from FIFO every refresh and emit plot signals if necessary
    while( fifo.read( &token, sizeof(token), false ) ) { // Will return 0 if fifo is empty        
        double data[token.points];
        if( fifo.read( &data, token.points * sizeof(double) ) )
            emit plotCurve( data, token );        
    }
}

void ClampProtocol::Panel::updateOptions( void ) { // Update changes to parameter values
    int it = mainWindow->intervalTimeEdit->value();
    int nt = mainWindow->trialsEdit->value();
    double jp = mainWindow->junctionPotentialEdit->text().toDouble();
    bool rd = mainWindow->dataRecordCheckBox->isChecked();    
    
    if( it == intervalTime && nt == numTrials && jp == junctionPotential
        && rd == recordData ) // If nothing has changed
        return ;

    // Set parameters, posts parameter change event
    // Since function posts event, must be called from within GUI thread
    setValue( 0, it ); 
    setValue( 1, nt );
    setValue( 2, jp );
    
    UpdateOptionsEvent event( this, it, nt, jp, rd );
    RT::System::getInstance()->postEvent( &event );
}

void ClampProtocol::Panel::loadProtocol( void ) {
     // Save dialog to retrieve desired filename and location
    QString fileName = QFileDialog::getOpenFileName(
                                                    "~/",
                                                    "Clamp Protocol Files (*.csp)",
                                                    this,
                                                    "open file dialog",
                                                    "Open a protocol" );
    if( fileName == NULL ) // Null if user cancels dialog
        return ;
    
    QDomDocument doc( "protocol" );
    QFile file( fileName );

    if( !file.open( IO_ReadOnly ) ) { // Make sure file can be opened, if not, warn user
        QMessageBox::warning(this,
                             "Error",
                             "Unable to open protocol file" );
        return ;
    }
    if( !doc.setContent( &file ) ) { // Make sure file contents are loaded into document        
            QMessageBox::warning(this,
                                 "Error",
                                 "Unable to set file contents to document" );
            file.close();
            return ;
        }
    file.close();

    protocol.fromDoc( doc ); // Translate document into protocol
    
    if( protocol.numSegments() <= 0 ) { // If no segments exist
        QMessageBox::warning(this,
                             "Error",
                             "Protocol did not contain any segments" );
    }

    Workspace::Instance::setComment( 0, fileName );
    QString text = fileName.right( abs( int( fileName.findRev("/") - fileName.length() ) ) - 1 ); // Typecast to int since length() returns unsigned int
    mainWindow->protocolNameEdit->setText( text );
    QToolTip::add( mainWindow->protocolNameEdit, fileName );
}

void ClampProtocol::Panel::openProtocolEditor( void ) {
    ProtocolEditor *protocolEditor = new ProtocolEditor( this );
    protocolEditor->show();
}

void ClampProtocol::Panel::openPlotWindow( void ) {
    PlotWindow *plotWindow = new PlotWindow( MainWindow::getInstance()->centralWidget(), this );
    QObject::connect( this, SIGNAL(plotCurve(double * , curve_token_t)), plotWindow, SLOT(addCurve(double *, curve_token_t)) );
    plotWindowList.push_back( plotWindow );
    plotWindow->show();
    plotWindow->setCaption( QString::number( getID() ) + " Clamp Protocol: Plot Window (" + QString::number( plotWindowList.size() ) + ")" );
    plotting = true;
    plotTimer->start(25); // 25ms refresh rate for plotting
}

void ClampProtocol::Panel::removePlotWindow( PlotWindow *plotWindow ) {
    plotWindowList.remove( plotWindow );
    if( plotWindowList.empty() ) {
        plotting = false;
        plotTimer->stop();
    }
}

void ClampProtocol::Panel::toggleProtocol( void ) { // Starts and stops protocol
    bool on = mainWindow->runProtocolButton->isOn();
    if( on ) {
        if( protocol.numSegments() == 0 ) { // If no protocol has been loaded
            QMessageBox::warning(this,
                                 "Error",
                                 "No protocol has been loaded.");
            mainWindow->runProtocolButton->setOn(false);
            return ;
        }
    }
    
    ToggleProtocolEvent event( this, on, recordData );
    RT::System::getInstance()->postEvent( &event );
}

// RT::Events - Called from GUI thread, handled by RT thread
ClampProtocol::Panel::UpdateOptionsEvent::UpdateOptionsEvent( Panel *p, int it, int nt, double jp, bool rd )
    : panel( p ), intervalTimeValue( it ), numTrialsValue( nt ), junctionPotentialValue( jp ), recordDataValue( rd ) {
}

int ClampProtocol::Panel::UpdateOptionsEvent::callback( void ) {
    // Set workspace parameter values
    panel->intervalTime = intervalTimeValue;
    panel->numTrials = numTrialsValue;
    panel->junctionPotential = junctionPotentialValue;
    panel->recordData = recordDataValue;
    return 0;
}

ClampProtocol::Panel::ToggleProtocolEvent::ToggleProtocolEvent( Panel *p, bool po, bool rd )
 : panel( p ), protocolOn( po ), recordData( rd ) {
}

int ClampProtocol::Panel::ToggleProtocolEvent::callback( void ) {
    if( protocolOn ) { // Start protocol, reinitialize parameters to start values
        panel->period = RT::System::getInstance()->getPeriod()*1e-6; // Grabs RTXI thread period and converts to ms (from ns)
        panel->trial = 1;
        panel->trialIdx = 0;
        panel->sweep = 1;
        panel->sweepIdx = 0;
        panel->time = 0;
        panel->segmentIdx = 0;
        panel->segmentNumber = 1;
        panel->stepIdx = 0;
        panel->segmentNumber = 1;
        if( recordData && !panel->recording ) { // Start protocol if record data is checked and data recorder is not recording
            ::Event::Object event(::Event::START_RECORDING_EVENT);
            ::Event::Manager::getInstance()->postEventRT(&event);            
        }
        panel->data.clear(); // Used in plotting
        panel->data.push_back( 0 );
        panel->protocolMode = SEGMENT;
        panel->executeMode = PROTOCOL;
    }
    else { // Stop protocol, only called when protocol button is unclicked in the middle of a run
        if( panel->recording ) { // Stop data recorder if recording
            ::Event::Object event(::Event::STOP_RECORDING_EVENT);
            ::Event::Manager::getInstance()->postEventRT(&event);
        }
        panel->executeMode = IDLE;
    }
    
    return 0;
}

// Class Plugin
extern "C" Plugin::Object *createRTXIPlugin( void * ) {
    return ClampProtocol::Plugin::getInstance();
}

ClampProtocol::Plugin::Plugin( void ) {
    menuID = MainWindow::getInstance()->createPatchClampMenuItem( "Clamp Protocol", this, SLOT(createClampProtocolPanel(void)) );
}

ClampProtocol::Plugin::~Plugin( void ) {
    MainWindow::getInstance()->removeUtilMenuItem( menuID );
    while( panelList.size() )
        delete panelList.front();
    instance = 0;
}

void ClampProtocol::Plugin::createClampProtocolPanel( void ) {
    Panel *panel = new Panel( MainWindow::getInstance()->centralWidget() );
    panelList.push_back( panel );
}

void ClampProtocol::Plugin::removeClampProtocolPanel( Panel *p ) {
    panelList.remove( p );
}

void ClampProtocol::Plugin::doDeferred( const Settings::Object::State & ) { }

void ClampProtocol::Plugin::doLoad( const Settings::Object::State &s ) {
    for( size_t i = 0 ; i < static_cast<size_t>( s.loadInteger("Num Panels") ) ; ++i ) {
        Panel *panel = new Panel( MainWindow::getInstance()->centralWidget() );
        panelList.push_back ( panel );
        panel->load( s.loadState( QString::number(i) ) );
    }
}

void ClampProtocol::Plugin::doSave( Settings::Object::State &s ) const {
    s.saveInteger( "Num Panels",panelList.size() );
    size_t n = 0;
    for( std::list<Panel *>::const_iterator i = panelList.begin(), end = panelList.end() ; i != end ; ++i )
        s.saveState( QString::number(n++), (*i)->save() );
}

static Mutex mutex;
ClampProtocol::Plugin *ClampProtocol::Plugin::instance = 0;

ClampProtocol::Plugin *ClampProtocol::Plugin::getInstance( void ) {
    if( instance )
        return instance;

    Mutex::Locker lock( &::mutex );
    if( !instance )
        instance = new Plugin();

    return instance;
}
