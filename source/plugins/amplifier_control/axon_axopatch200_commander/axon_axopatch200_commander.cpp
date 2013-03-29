#include <axon_axopatch200_commander.h>
#include <main_window.h>
#include <mutex.h>
#include <iostream>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qfont.h>
#include <qlabel.h>
#include <qspinbox.h>

static IO::channel_t chan[] = {
    {
        "Mode Telegraph",
        "",
        IO::INPUT,
    },
    {
        "Gain Telegraph",
        "",
        IO::INPUT,
    },
};

static size_t num_chan = sizeof(chan)/sizeof(chan[0]);

static void getDevice(DAQ::Device *d,void *p) {
    DAQ::Device **device = reinterpret_cast<DAQ::Device **>(p);

    if(!*device)
        *device = d;
}

Axon200Commander::Panel::Panel(QWidget *parent)
    : QWidget(parent,"Axon 200 Amp Commander",Qt::WStyle_NormalBorder|Qt::WDestructiveClose),
      RT::Thread(0), IO::Block("Axon 200 Commander",chan,num_chan), currentGain(1), oldGain(1) {
    mainUI = new Axon_200_Amp_CommanderUI( this );
    
    QBoxLayout *mainLayout = new QVBoxLayout( this );
    mainLayout->addWidget( mainUI );

     setCaption(QString::number(getID())+" Axon 200 Series Amp Commander");

     // Set refresh
    refreshTimer = new QTimer( this );
     
    QObject::connect( mainUI->gainComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( mainUI->configComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( mainUI->ampModeButtonGroup, SIGNAL(pressed(int)), this, SLOT(updateMode(int)) );
    QObject::connect( mainUI->autoModeButton, SIGNAL(toggled(bool)), this, SLOT(toggleAutoMode(bool)) );
    QObject::connect( mainUI->inputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );
    QObject::connect( mainUI->outputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );
    QObject::connect( refreshTimer, SIGNAL(timeout(void)), this, SLOT(refresh(void)) );

    mainUI->ampModeButtonGroup->setButton( 0 );
    
    setActive(true);
    
    show();
}

Axon200Commander::Panel::~Panel(void) {
    Plugin::getInstance()->removeAxon200CommanderPanel(this);
}

void Axon200Commander::Panel::execute() { // Only run during auto mode
    // Check current mode
    if( input(0) <  3 )
        currentMode = Amplifier::Iclamp;
    else
        currentMode = Amplifier::Vclamp;

    // Check current gain
    if( input(1) <= 0.75 )
        currentGain = 0.05;
    else if ( input(1) <= 1.25 )
        currentGain = 0.1;
    else if( input(1) <= 1.75 )
        currentGain = 0.2;
    else if ( input(1) <= 2.25 )
        currentGain = 0.5;
    else if( input(1) <= 2.75 )
        currentGain = 1.0;
    else if( input(1) <= 3.25 )
        currentGain = 2.0;
    else if( input(1) <= 3.75 )
        currentGain = 5.0;
    else if ( input(1) <= 4.25 )
        currentGain = 10.0;
    else if( input(1) <= 4.75 )
        currentGain = 20.0;
    else if ( input(1) <= 5.25 )
        currentGain = 50.0;
    else if( input(1) <= 5.75 )
        currentGain = 100.0;
    else if( input(1) <= 6.25 )
        currentGain = 200.0;
    else if( input(1) <= 6.75 )
        currentGain = 500.0;
}

void Axon200Commander::Panel::updateChannels( void ) {
    inputChan = mainUI->inputChannelSpinBox->value();
    outputChan = mainUI->outputChannelSpinBox->value();
}

void Axon200Commander::Panel::updateGains( void ) { // Update gains whenever selection in combo boxes are changed
    double scaled_gain, headstage_gain;

    // Alpha
    switch( mainUI->gainComboBox->currentItem() ) {
    default: scaled_gain = 0.5;
        break;
    case 1: scaled_gain = 1;
        break;
    case 2: scaled_gain = 2;
        break;
    case 3: scaled_gain = 5;
        break;
    case 4: scaled_gain = 10;
        break;
    case 5: scaled_gain = 20;
        break;
    case 6: scaled_gain = 50;
        break;
    case 7: scaled_gain = 100;
        break;
    case 8: scaled_gain = 200;
        break;
    case 9: scaled_gain = 500;
        break;
    }

    switch( mainUI->configComboBox->currentItem() ) {
    case 2: headstage_gain = 0.1; // Beta = 0.1
        break;
    default: headstage_gain = 1.0; // Beta = 1.0
        break;
    }

    amp.setGains( (1 / scaled_gain), headstage_gain );
    amp.setMode( amp.getMode(), inputChan, outputChan );
}

void Axon200Commander::Panel::updateMode( int mode ) {
    switch(mode) {
    case 0: amp.setMode(Axon200Commander::Amplifier::Vclamp,inputChan,outputChan);
        break;
    case 1: amp.setMode(Axon200Commander::Amplifier::Iclamp,inputChan,outputChan);
        break;
    default: std::cout << "Error: Axon200Commander::Plugin::updateMode() default case called" << std::endl;
        break;
    }
}

void Axon200Commander::Panel::refresh( void ) { // Only run during auto mode
    // Update gains if it has changed
    if( currentGain != oldGain ) {
        oldGain = currentGain;
        double headstage_gain;
        switch( mainUI->configComboBox->currentItem() ) {
        case 2: headstage_gain = 0.1; // Beta = 0.1
            break;
        default: headstage_gain = 1.0; // Beta = 1.0
            break;
        }
    
        // Divide currentGain by headstage_gain due to the fact telegraph takes Beta into account    
        amp.setGains( (1 / currentGain) / headstage_gain , headstage_gain );
        amp.setMode( amp.getMode(), inputChan, outputChan );
    }
    
    // Update mode if it has changed
    if( currentMode != amp.getMode() ) {
        if( currentMode == Amplifier::Vclamp )
            mainUI->ampModeLabel->setText( "V Clamp" );
        else
            mainUI->ampModeLabel->setText( "I Clamp" );
        
        amp.setMode( currentMode, inputChan, outputChan );
    }
}

void Axon200Commander::Panel::toggleAutoMode( bool on ) {
    if( on ) {
        mainUI->gainComboBox->setEnabled( false );
        mainUI->ampModeButtonGroup->setEnabled( false );
        refreshTimer->start(100);

        if( amp.getMode() == Amplifier::Vclamp )
            mainUI->ampModeLabel->setText( "V Clamp" );
        else
            mainUI->ampModeLabel->setText( "I Clamp" );
    }
    else {        
        mainUI->gainComboBox->setEnabled( true );
        mainUI->ampModeButtonGroup->setEnabled( true );
        mainUI->ampModeButtonGroup->setButton( 0 );
        refreshTimer->stop();
        mainUI->ampModeLabel->setText("Amp Mode");
    }
    setActive( on );
}

void Axon200Commander::Panel::doSave( Settings::Object::State &s ) const {
	if (isMaximized())
		s.saveInteger("Maximized", 1);
	else if (isMinimized())
		s.saveInteger("Minimized", 1);

	QPoint pos = parentWidget()->pos();
	s.saveInteger("X", pos.x());
	s.saveInteger("Y", pos.y());
	s.saveInteger("W", width());
	s.saveInteger("H", height());

    s.saveInteger("Input Channel", mainUI->inputChannelSpinBox->value() );
    s.saveInteger("Output Channel", mainUI->outputChannelSpinBox->value() );
    s.saveInteger("Gain", mainUI->gainComboBox->currentItem() );
    s.saveInteger("Headstage Config", mainUI->configComboBox->currentItem() );
    s.saveInteger("Mode", mainUI->ampModeButtonGroup->selectedId() );
    s.saveInteger("Auto Mode", mainUI->autoModeButton->isOn() );
}

void Axon200Commander::Panel::doLoad( const Settings::Object::State &s ) {
	if (s.loadInteger("Maximized"))
		showMaximized();
	else if (s.loadInteger("Minimized"))
		showMinimized();
	if (s.loadInteger("W") != NULL) {
		resize(s.loadInteger("W"), s.loadInteger("H"));
		parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
	}

    mainUI->inputChannelSpinBox->setValue( s.loadInteger("Input Channel") );
    mainUI->outputChannelSpinBox->setValue( s.loadInteger("Output Channel") );
    mainUI->gainComboBox->setCurrentItem( s.loadInteger("Gain") );
    mainUI->configComboBox->setCurrentItem( s.loadInteger("Headstage Config") );
    mainUI->ampModeButtonGroup->setButton( s.loadInteger("Mode") );
    mainUI->autoModeButton->setOn( s.loadInteger("Auto Mode") );

    updateChannels();
    updateGains();
    updateMode( s.loadInteger("Mode") );
}


Axon200Commander::Amplifier::Amplifier( void )
    : device(0) {
    DAQ::Manager::getInstance()->foreachDevice(getDevice,&device);

    iclamp_ai_gain = 1; // 1 V / V
    iclamp_ao_gain = 1.0 / 2e-9; // 2 nA / V
    izero_ai_gain = 1; // 1 V / V
    izero_ao_gain = 0; // No output
    vclamp_ai_gain = 1e-12 / 1e-3; // 1mv / pA
    vclamp_ao_gain = 1.0 / 20e-3; // 20mV / V

    headstage_gain = 1;
    scaled_gain = 1;
}

Axon200Commander::Amplifier::~Amplifier( void ) {}

void Axon200Commander::Amplifier::setMode( mode_t mode, int inputChan, int outputChan ) {

    this->mode = mode;

    switch(mode) {
    case Iclamp:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,0);
            device->setAnalogGain(DAQ::AI,inputChan,iclamp_ai_gain * scaled_gain * headstage_gain );
            device->setAnalogGain(DAQ::AO,outputChan,iclamp_ao_gain * headstage_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
            device->setAnalogCalibration(DAQ::AI,inputChan);
        }
        break;
    default: // Vclamp
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,0);
            device->setAnalogGain(DAQ::AI,inputChan,vclamp_ai_gain * scaled_gain * headstage_gain );
            device->setAnalogGain(DAQ::AO,outputChan,vclamp_ao_gain );
            device->setAnalogCalibration(DAQ::AO,outputChan);
            device->setAnalogCalibration(DAQ::AI,inputChan);
        }
        break;
    }
}

void Axon200Commander::Amplifier::setGains( double oc, double hg ) {
    scaled_gain = oc;
    headstage_gain = hg;    
}

Axon200Commander::Amplifier::mode_t Axon200Commander::Amplifier::getMode() {
	return mode;
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return Axon200Commander::Plugin::getInstance();
}

Axon200Commander::Plugin::Plugin(void) {
    menuID = MainWindow::getInstance()->createPatchClampMenuItem("Axon 200 Commander",this,SLOT(createAxon200CommanderPanel(void)));
}

Axon200Commander::Plugin::~Plugin(void) {
    MainWindow::getInstance()->removeUtilMenuItem(menuID);
    while(panelList.size())
        delete panelList.front();
    instance = 0;
}

void Axon200Commander::Plugin::createAxon200CommanderPanel(void) {
    Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
    panelList.push_back(panel);
}

void Axon200Commander::Plugin::removeAxon200CommanderPanel(Axon200Commander::Panel *panel) {
    panelList.remove(panel);
}

static Mutex mutex;
Axon200Commander::Plugin *Axon200Commander::Plugin::instance = 0;

Axon200Commander::Plugin *Axon200Commander::Plugin::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but allocation isn't        *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance)
        instance = new Plugin();

    return instance;
}

void Axon200Commander::Plugin::doSave(Settings::Object::State &s) const {    
    s.saveInteger("Num Panels",panelList.size());
    size_t n = 0;
    for(std::list<Panel *>::const_iterator i = panelList.begin(),end = panelList.end();i != end;++i)
        s.saveState(QString::number(n++),(*i)->save());
}

void Axon200Commander::Plugin::doLoad(const Settings::Object::State &s) {    
    for(size_t i = 0;i < static_cast<size_t>(s.loadInteger("Num Panels"));++i) {
        Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
        panelList.push_back(panel);
        panel->load(s.loadState(QString::number(i)));
    }
}
