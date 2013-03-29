#include <axon_multiclamp700_commander.h>
#include <main_window.h>
#include <mutex.h>
#include <iostream>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qfont.h>
#include <qlabel.h>
#include <qspinbox.h>

static IO::channel_t chan[] = {
    {
        "Mode Output",
        "",
        IO::OUTPUT,
    },
};

static size_t num_chan = sizeof( chan ) / sizeof( chan[0] );

static void getDevice( DAQ::Device *d,void *p ) {
    DAQ::Device **device = reinterpret_cast<DAQ::Device **>( p );

    if( !*device )
        *device = d;
}

Multiclamp700Commander::Panel::Panel( QWidget *parent )
    : QWidget( parent, "Axon Multiclamp 700 Series Commander", Qt::WStyle_NormalBorder | Qt::WDestructiveClose ), RT::Thread( 0 ) {
    QBoxLayout *mainLayout = new QVBoxLayout( this );

    setCaption( QString::number(getID())+" Axon Multiclamp 700 Series Commander" );

    ui = new Multiclamp700_UI( this );

    QObject::connect( ui->vcigComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( ui->vcecsComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( ui->icecsComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( ui->icigComboBox, SIGNAL(activated(int)), this, SLOT(updateGains(void)) );
    QObject::connect( ui->buttonGroup, SIGNAL(pressed(int)), this, SLOT(updateMode(int)) );
    QObject::connect( ui->inputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );
    QObject::connect( ui->outputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );

    ui->buttonGroup->setButton( 1 );
    updateMode( 1 );
    
    mainLayout->addWidget( ui );
    
    show();
}

Multiclamp700Commander::Panel::~Panel(void) {
    Plugin::getInstance()->removeMulticlamp700CommanderPanel(this);
}

void Multiclamp700Commander::Panel::updateChannels( void ) { // Update the DAQ channels whose gains will be modified
    inputChan = ui->inputChannelSpinBox->value();
    outputChan = ui->outputChannelSpinBox->value();
}

void Multiclamp700Commander::Panel::updateGains( void ) { // Update gains whenever selection in combo boxes are changed
    double vclamp_out_gain, vclamp_ao_gain, iclamp_out_gain, iclamp_ao_gain;
    
    switch( ui->vcigComboBox->currentItem() ) {
    default: vclamp_out_gain = 1;
        break;
    case 1: vclamp_out_gain = 2;
        break;
    case 2: vclamp_out_gain = 5;
        break;
    case 3: vclamp_out_gain = 10;
        break;
    case 4: vclamp_out_gain = 20;
        break;
    case 5: vclamp_out_gain = 50;
        break;
    case 6: vclamp_out_gain = 100;
        break;
    case 7: vclamp_out_gain = 200;
        break;
    case 8: vclamp_out_gain = 600;
        break;
    case 9: vclamp_out_gain = 1000;
        break;
     case 10: vclamp_out_gain = 2000;
        break;
    }

    switch( ui->vcecsComboBox->currentItem() ) {
    case 0: vclamp_ao_gain = 1.0 / 20e-3; // 20mV / V
        break;
    default: vclamp_ao_gain = 1.0 / 100e-3; // 100mV / V
        break;
    }

    switch( ui->icigComboBox->currentItem() ) {
    default: iclamp_out_gain = 1;
        break;
    case 1: iclamp_out_gain = 2;
        break;
    case 2: iclamp_out_gain = 5;
        break;
    case 3: iclamp_out_gain = 10;
        break;
    case 4: iclamp_out_gain = 20;
        break;
    case 5: iclamp_out_gain = 50;
        break;
    case 6: iclamp_out_gain = 100;
        break;
    case 7: iclamp_out_gain = 200;
        break;
    case 8: iclamp_out_gain = 600;
        break;
    case 9: iclamp_out_gain = 1000;
        break;
     case 10: iclamp_out_gain = 2000;
        break;
    }
    
    switch( ui->icecsComboBox->currentItem() ) {
    case 0: iclamp_ao_gain = 1.0 / 400e-12; // 400 pA / V
        break;
    default: iclamp_ao_gain = 1.0 / 2e-9; // 2 nA / V
    }

    amp.setGains( vclamp_out_gain, vclamp_ao_gain, iclamp_out_gain, iclamp_ao_gain );
}

void Multiclamp700Commander::Panel::updateMode(int mode) {
    switch(mode) {
    case 0: amp.setMode(Multiclamp700Commander::Amplifier::Vclamp);
        break;
    case 1: amp.setMode(Multiclamp700Commander::Amplifier::Izero);
        break;
    case 2: amp.setMode(Multiclamp700Commander::Amplifier::Iclamp);
        break;
    default: std::cout << "Error: Multiclamp700Commander::Plugin::updateMode() default case called" << std::endl;
        break;
    }
}

void Multiclamp700Commander::Panel::doSave(Settings::Object::State &s) const {
    s.saveInteger( "Input Channel",  ui->inputChannelSpinBox->value() );
    s.saveInteger( "Output Channel",  ui->outputChannelSpinBox->value() );
    s.saveInteger( "Mode", ui->buttonGroup->selectedId() );
    s.saveInteger( "VClamp Gain", ui->vcigComboBox->currentItem() );
    s.saveInteger( "VClamp Command", ui->vcecsComboBox->currentItem() );
    s.saveInteger( "IClamp Gain", ui->icigComboBox->currentItem() );
    s.saveInteger( "IClamp Command", ui->icecsComboBox->currentItem() );
    
	if (isMaximized())
		s.saveInteger("Maximized", 1);
	else if (isMinimized())
		s.saveInteger("Minimized", 1);

	QPoint pos = parentWidget()->pos();
	s.saveInteger("X", pos.x());
	s.saveInteger("Y", pos.y());
	s.saveInteger("W", width());
	s.saveInteger("H", height());
}

void Multiclamp700Commander::Panel::doLoad(const Settings::Object::State &s) {
	if (s.loadInteger("Maximized"))
		showMaximized();
	else if (s.loadInteger("Minimized"))
		showMinimized();
	if (s.loadInteger("W") != NULL) {
		resize(s.loadInteger("W"), s.loadInteger("H"));
		parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
	}

    ui->inputChannelSpinBox->setValue( s.loadInteger( "Input Channel" ) );
    ui->outputChannelSpinBox->setValue( s.loadInteger( "Output Channel" ) );
    ui->buttonGroup->setButton( s.loadInteger( "Mode" ) );
    ui->vcigComboBox->setCurrentItem( s.loadInteger( "VClamp Gain" ) );
    ui->vcecsComboBox->setCurrentItem( s.loadInteger( "VClamp Command" ) );
    ui->icigComboBox->setCurrentItem( s.loadInteger( "IClamp Gain" ) );
    ui->icecsComboBox->setCurrentItem( s.loadInteger( "IClamp Command" ) );

    updateGains();
    updateChannels();
    updateMode( s.loadInteger("Mode") );
}

Multiclamp700Commander::Amplifier::Amplifier(void)
    : IO::Block("Axon 700A Commander",::chan,::num_chan), device(0) {
    DAQ::Manager::getInstance()->foreachDevice(getDevice,&device);

    iclamp_ai_gain = 1; // 1 V / V
    iclamp_ao_gain = 1.0 / 400e-12; // 400 pA / V
    izero_ai_gain = 1; // 1 V / V
    izero_ao_gain = 0; // No output
    vclamp_ai_gain = 2e-9 / 1.0; // 0.5V / nA or 2 nA / V
    vclamp_ao_gain = 1.0 / 20e-3; // 20mV / V

    iclamp_out_gain = 1;
    vclamp_out_gain = 1;

    setMode(Izero);
}

Multiclamp700Commander::Amplifier::~Amplifier(void) {}

void Multiclamp700Commander::Amplifier::setMode(mode_t mode) {

    this->mode = mode;

    switch(mode) {
    case Iclamp:
        if(device) {
            device->setAnalogRange(DAQ::AI,0,0);
            device->setAnalogGain(DAQ::AI,0,iclamp_ai_gain);
            device->setAnalogGain(DAQ::AO,0,iclamp_ao_gain * iclamp_out_gain);
        }
        output(0) = 5.0;
        break;
    case Vclamp:
        if(device) {
            device->setAnalogRange(DAQ::AI,0,0);
            device->setAnalogGain(DAQ::AI,0,vclamp_ai_gain);
            device->setAnalogGain(DAQ::AO,0,vclamp_ao_gain * vclamp_out_gain);
        }
        output(0) = 0.0;
        break;
    case Izero:
        if(device) {
            device->setAnalogRange(DAQ::AI,0,0);
            device->setAnalogGain(DAQ::AI,0,izero_ai_gain);
            device->setAnalogGain(DAQ::AO,0,izero_ao_gain);
        }
        output(0) = 5.0;
        break;
    }
}

void Multiclamp700Commander::Amplifier::setGains( double vc_out, double vc_ao, double ic_out, double ic_ao ) {
    vclamp_out_gain = vc_out;
    vclamp_ao_gain = vc_ao;
    iclamp_out_gain = ic_out;
    iclamp_ao_gain = ic_ao;

    setMode( mode ); // Update gains by setting mode to current mode
}

Multiclamp700Commander::Amplifier::mode_t Multiclamp700Commander::Amplifier::getMode() {
	return mode;
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return Multiclamp700Commander::Plugin::getInstance();
}

Multiclamp700Commander::Plugin::Plugin(void) {
    menuID = MainWindow::getInstance()->createPatchClampMenuItem("Axon Multiclamp 700 Commander",this,SLOT(createMulticlamp700CommanderPanel(void)));
}

Multiclamp700Commander::Plugin::~Plugin(void) {
    MainWindow::getInstance()->removeUtilMenuItem(menuID);
    while(panelList.size())
        delete panelList.front();
    instance = 0;
}

void Multiclamp700Commander::Plugin::createMulticlamp700CommanderPanel(void) {
    Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
    panelList.push_back(panel);
}

void Multiclamp700Commander::Plugin::removeMulticlamp700CommanderPanel(Multiclamp700Commander::Panel *panel) {
    panelList.remove(panel);
}

static Mutex mutex;
Multiclamp700Commander::Plugin *Multiclamp700Commander::Plugin::instance = 0;

Multiclamp700Commander::Plugin *Multiclamp700Commander::Plugin::getInstance(void) {
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

void Multiclamp700Commander::Plugin::doSave(Settings::Object::State &s) const {    
    s.saveInteger("Num Panels",panelList.size());
    size_t n = 0;
    for(std::list<Panel *>::const_iterator i = panelList.begin(),end = panelList.end();i != end;++i)
        s.saveState(QString::number(n++),(*i)->save());
}

void Multiclamp700Commander::Plugin::doLoad(const Settings::Object::State &s) {    
    for(size_t i = 0;i < static_cast<size_t>(s.loadInteger("Num Panels"));++i) {
        Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
        panelList.push_back(panel);
        panel->load(s.loadState(QString::number(i)));
    }
}
