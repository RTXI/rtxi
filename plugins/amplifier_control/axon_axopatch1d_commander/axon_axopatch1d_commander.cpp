#include <axon_axopatch1d_commander.h>
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

Axon1dCommander::Panel::Panel(QWidget *parent)
    : QWidget(parent,"Axon 1D Amp Commander",Qt::WStyle_NormalBorder|Qt::WDestructiveClose),
      RT::Thread(0), IO::Block("Axon 1D Commander",chan,num_chan), currentGain(1), oldGain(1) {

    mainUI = new Axon_1D_Amp_CommanderUI( this );
    
    QBoxLayout *mainLayout = new QVBoxLayout( this );
    mainLayout->addWidget( mainUI );

     setCaption(QString::number(getID())+" Axon Axopatch 1D Commander");

     // Set refresh
    refreshTimer = new QTimer( this );
     
    QObject::connect( mainUI->gainComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( mainUI->configComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( mainUI->commandComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( mainUI->ampModeButtonGroup, SIGNAL(pressed(int)), this, SLOT(updateMode(int)) );
    QObject::connect( mainUI->autoModeButton, SIGNAL(toggled(bool)), this, SLOT(toggleAutoMode(bool)) );
    QObject::connect( mainUI->inputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );
    QObject::connect( mainUI->outputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );
    QObject::connect( refreshTimer, SIGNAL(timeout(void)), this, SLOT(refresh(void)) );

    mainUI->ampModeButtonGroup->setButton( 0 );
    
    setActive(true);
    
    show();
}

Axon1dCommander::Panel::~Panel(void) {
    Plugin::getInstance()->removeAxon1dCommanderPanel(this);
}

void Axon1dCommander::Panel::execute() { // Only run during auto mode

    // Check current gain
    if ( input(0) <= 0.6 )
        currentGain = 0.5;
    else if( input(0) <= 1.0 )
        currentGain = 1.0;
    else if( input(0) <= 1.4 )
        currentGain = 2.0;
    else if( input(0) <= 1.8 )
        currentGain = 5.0;
    else if ( input(0) <= 2.2 )
        currentGain = 10.0;
    else if( input(0) <= 2.6 )
        currentGain = 20.0;
    else if ( input(0) <= 3.0 )
        currentGain = 50.0;
    else
        currentGain = 100.0;
}

void Axon1dCommander::Panel::updateChannels( void ) {
    inputChan = mainUI->inputChannelSpinBox->value();
    outputChan = mainUI->outputChannelSpinBox->value();
    updateGains();
}

void Axon1dCommander::Panel::updateGains( void ) { // Update gains whenever selection in combo boxes are changed
    double scaled_gain, headstage_gain, command_gain;

    // Alpha
    switch( mainUI->gainComboBox->currentItem() ) {
    case 0: scaled_gain = 0.5;
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
    default: scaled_gain = 100;
        break;
    }

    switch( mainUI->configComboBox->currentItem() ) {
    case 0: headstage_gain = 100; // Beta = 100
        break;
    case 1: headstage_gain = 1.0; // Beta = 0.1
        break;
    default: headstage_gain = 0.1; // Beta = 1.0
        break;
    }

    switch( mainUI->commandComboBox->currentItem() ) {
    case 0: command_gain = 1 / 20e-3; // 20 mV / V
        break;
    default: command_gain = 1 / 1e-3; // 1 mv / V
        break;
    }

    amp.setGains( (1 / scaled_gain), headstage_gain, command_gain );
    amp.setMode( amp.getMode(), inputChan, outputChan );
}

void Axon1dCommander::Panel::updateMode( int mode ) {
    switch(mode) {
    case 0: amp.setMode(Axon1dCommander::Amplifier::Vclamp,inputChan,outputChan);
        break;
    case 1: amp.setMode(Axon1dCommander::Amplifier::Izero,inputChan,outputChan);
        break;
    case 2: amp.setMode(Axon1dCommander::Amplifier::Iclamp,inputChan,outputChan);
        break;
    default: std::cout << "Error: Axon1dCommander::Plugin::updateMode() default case called" << std::endl;
        break;
    }
}

void Axon1dCommander::Panel::refresh( void ) { // Only run during auto mode
    // Update gains if it has changed
    if( currentGain != oldGain ) {
      oldGain = currentGain;

      if( currentGain < 1 )
	mainUI->gainComboBox->setCurrentItem( 0 );
      else if( currentGain < 2 )
	mainUI->gainComboBox->setCurrentItem( 1 );
      else if( currentGain < 5 )
	mainUI->gainComboBox->setCurrentItem( 2 );
      else if( currentGain < 10 )
	mainUI->gainComboBox->setCurrentItem( 3 );
      else if( currentGain < 20 )
	mainUI->gainComboBox->setCurrentItem( 4 );
      else if( currentGain < 50 )
	mainUI->gainComboBox->setCurrentItem( 5 );
      else if( currentGain < 100 )
	mainUI->gainComboBox->setCurrentItem( 6 );
      else
	mainUI->gainComboBox->setCurrentItem( 7 );
    

      double headstage_gain;
      switch( mainUI->configComboBox->currentItem() ) {
      case 0: headstage_gain = 100; // Beta = 100
	break;
      case 1: headstage_gain = 1.0; // Beta = 0.1
            break;
        default: headstage_gain = 0.1; // Beta = 1.0
            break;
        }
        double command_gain;
        switch( mainUI->commandComboBox->currentItem() ) {
        case 0: command_gain = 1 / 20e-3; // 20 mV / V
            break;
        default: command_gain = 1 / 1e-3; // 1 mv / V
            break;
        }
    
        // Divide currentGain by headstage_gain due to the fact telegraph takes Beta into account    
        amp.setGains( (1 / currentGain) / headstage_gain , headstage_gain, command_gain );
        amp.setMode( amp.getMode(), inputChan, outputChan );
    }
}

void Axon1dCommander::Panel::toggleAutoMode( bool on ) {
    if( on ) {
        mainUI->gainComboBox->setEnabled( false );
        refreshTimer->start(100);
    }
    else {
        mainUI->gainComboBox->setEnabled( true );
        refreshTimer->stop();
    }
    setActive( on );
}

void Axon1dCommander::Panel::doSave( Settings::Object::State &s ) const {
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
    s.saveInteger("Command Config", mainUI->commandComboBox->currentItem() );
    s.saveInteger("Mode", mainUI->ampModeButtonGroup->selectedId() );
    s.saveInteger("Auto Mode", mainUI->autoModeButton->isOn() );
}

void Axon1dCommander::Panel::doLoad( const Settings::Object::State &s ) {
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
    mainUI->commandComboBox->setCurrentItem( s.loadInteger("Command Config") );
    mainUI->ampModeButtonGroup->setButton( s.loadInteger("Mode") );
    mainUI->autoModeButton->setOn( s.loadInteger("Auto Mode") );

    updateChannels();
    updateGains();
    updateMode( s.loadInteger("Mode") );
}


Axon1dCommander::Amplifier::Amplifier( void )
    : device(0) {
    DAQ::Manager::getInstance()->foreachDevice(getDevice,&device);

    iclamp_ai_gain = 1.0 / 10.0; //  1 V / 10 V
    iclamp_ao_gain = 1e-3 / 10e-12; // 10 pA /mV (reciprocal due to it being an output)
    izero_ai_gain = 1.0 / 10.0; // 1 V / 10 V
    izero_ao_gain = 0; // No output
    vclamp_ai_gain = 1e-12 / 1e-3; // 1mv / pA
    vclamp_ao_gain = 1.0 / 1.0; // 1 V / 1 V (reciprocal due to it being an output)

    command_gain = 1 / 20e-3; // 20mv / V (reciprocal due to it dealing with output)
    headstage_gain = 1;
    scaled_gain = 1;
}

Axon1dCommander::Amplifier::~Amplifier( void ) {}

void Axon1dCommander::Amplifier::setMode( mode_t mode, int inputChan, int outputChan ) {

    this->mode = mode;

    switch(mode) {
    case Iclamp:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,0);
            device->setAnalogGain(DAQ::AI,inputChan,iclamp_ai_gain * scaled_gain * headstage_gain );
            device->setAnalogGain(DAQ::AO,outputChan,iclamp_ao_gain * headstage_gain * command_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
            device->setAnalogCalibration(DAQ::AI,inputChan);
        }
        break;
    case Izero:
      if(device) {
	device->setAnalogRange(DAQ::AI,inputChan,0);
	device->setAnalogGain(DAQ::AI,inputChan,iclamp_ai_gain * scaled_gain * headstage_gain );
	device->setAnalogGain(DAQ::AO,outputChan,0);
	device->setAnalogCalibration(DAQ::AO,outputChan);
	device->setAnalogCalibration(DAQ::AI,inputChan);
      }
      break;
    default: // Vclamp
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,0);
            device->setAnalogGain(DAQ::AI,inputChan,vclamp_ai_gain * scaled_gain * headstage_gain );
            device->setAnalogGain(DAQ::AO,outputChan,vclamp_ao_gain * command_gain );
            device->setAnalogCalibration(DAQ::AO,outputChan);
            device->setAnalogCalibration(DAQ::AI,inputChan);
        }
        break;
    }
}

void Axon1dCommander::Amplifier::setGains( double oc, double hg, double cg ) {
    scaled_gain = oc;
    headstage_gain = hg;
    command_gain = cg;
}

Axon1dCommander::Amplifier::mode_t Axon1dCommander::Amplifier::getMode() {
	return mode;
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return Axon1dCommander::Plugin::getInstance();
}

Axon1dCommander::Plugin::Plugin(void) {
    menuID = MainWindow::getInstance()->createPatchClampMenuItem("Axon Axopatch 1D Commander",this,SLOT(createAxon1dCommanderPanel(void)));
}

Axon1dCommander::Plugin::~Plugin(void) {
    MainWindow::getInstance()->removeUtilMenuItem(menuID);
    while(panelList.size())
        delete panelList.front();
    instance = 0;
}

void Axon1dCommander::Plugin::createAxon1dCommanderPanel(void) {
    Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
    panelList.push_back(panel);
}

void Axon1dCommander::Plugin::removeAxon1dCommanderPanel(Axon1dCommander::Panel *panel) {
    panelList.remove(panel);
}

static Mutex mutex;
Axon1dCommander::Plugin *Axon1dCommander::Plugin::instance = 0;

Axon1dCommander::Plugin *Axon1dCommander::Plugin::getInstance(void) {
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

void Axon1dCommander::Plugin::doSave(Settings::Object::State &s) const {    
    s.saveInteger("Num Panels",panelList.size());
    size_t n = 0;
    for(std::list<Panel *>::const_iterator i = panelList.begin(),end = panelList.end();i != end;++i)
        s.saveState(QString::number(n++),(*i)->save());
}

void Axon1dCommander::Plugin::doLoad(const Settings::Object::State &s) {    
    for(size_t i = 0;i < static_cast<size_t>(s.loadInteger("Num Panels"));++i) {
        Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
        panelList.push_back(panel);
        panel->load(s.loadState(QString::number(i)));
    }
}
