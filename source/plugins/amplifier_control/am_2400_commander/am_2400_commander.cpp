#include <am_2400_commander.h>
#include <main_window.h>
#include <mutex.h>
#include <iostream>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qfont.h>
#include <qspinbox.h>

static IO::channel_t chan[] = {
    {
        "Mode Bit 1",
        "",
        IO::OUTPUT,
    },
    {
        "Mode Bit 2",
        "",
        IO::OUTPUT,
    },
    {
        "Mode Bit 4",
        "",
        IO::OUTPUT,
    },
};

static size_t num_chan = sizeof(chan)/sizeof(chan[0]);

static void getDevice(DAQ::Device *d,void *p) {
    DAQ::Device **device = reinterpret_cast<DAQ::Device **>(p);

    if(!*device)
        *device = d;
}

AM2400Commander::Panel::Panel(QWidget *parent)
    : QWidget(parent,"AM Amp Commander",Qt::WStyle_NormalBorder|Qt::WDestructiveClose), RT::Thread(0) {
    QBoxLayout *mainLayout = new QVBoxLayout( this );

    ui = new AM_Amp_CommanderUI( this );
    setCaption(QString::number(getID())+" AM 2400 Amp Commander");

    QObject::connect( ui->ampModeButtonGroup, SIGNAL(pressed(int)), this, SLOT(updateMode(int)) );   
    QObject::connect( ui->inputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );
    QObject::connect( ui->outputChannelSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateChannels(void)) );

    ui->ampModeButtonGroup->setButton( 1 ); // Set to IZero as default
    mainLayout->addWidget( ui );
    
    show();
}

AM2400Commander::Panel::~Panel(void) {
    Plugin::getInstance()->removeAM2400CommanderPanel( this );
}

void AM2400Commander::Panel::updateMode(int mode) {
    switch(mode) {
    case 0: amp.setMode(AM2400Commander::Amplifier::Vclamp, inputChan, outputChan);
        break;
    case 1: amp.setMode(AM2400Commander::Amplifier::Izero, inputChan, outputChan);
        break;
    case 2: amp.setMode(AM2400Commander::Amplifier::Iclamp, inputChan, outputChan);
        break;
    case 3: amp.setMode(AM2400Commander::Amplifier::Vcomp, inputChan, outputChan);
        break;
    case 4: amp.setMode(AM2400Commander::Amplifier::Vtest, inputChan, outputChan);
        break;
    case 5: amp.setMode(AM2400Commander::Amplifier::Iresist, inputChan, outputChan);
        break;
    case 6: amp.setMode(AM2400Commander::Amplifier::Ifollow, inputChan, outputChan);
        break;
    default: std::cout << "Error: AM2400Commander::Plugin::updateMode() default case called" << std::endl;
        break;
    }
}

void AM2400Commander::Panel::updateChannels( void ) {
    inputChan = ui->inputChannelSpinBox->value();
    outputChan = ui->outputChannelSpinBox->value();
}

void AM2400Commander::Panel::doSave(Settings::Object::State &s) const {
    
    s.saveInteger( "Input Channel",  ui->inputChannelSpinBox->value() );
    s.saveInteger( "Output Channel",  ui->outputChannelSpinBox->value() );
    s.saveInteger("Mode", ui->ampModeButtonGroup->selectedId() );
    
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

void AM2400Commander::Panel::doLoad(const Settings::Object::State &s) {    
	if (s.loadInteger("Maximized"))
		showMaximized();
	else if (s.loadInteger("Minimized"))
		showMinimized();
	if (s.loadInteger("W") != NULL) {
		resize(s.loadInteger("W"), s.loadInteger("H"));
		parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
	}
    
    ui->inputChannelSpinBox->setValue( s.loadInteger("Input Channel") );
    ui->outputChannelSpinBox->setValue( s.loadInteger("Output Channel") );
    ui->ampModeButtonGroup->setButton( s.loadInteger("Mode") );
    
    updateChannels();
    updateMode( s.loadInteger("Mode") );
}


AM2400Commander::Amplifier::Amplifier(void)
    : IO::Block("AM Amplifier Commander",::chan,::num_chan), device(0) {
    DAQ::Manager::getInstance()->foreachDevice(getDevice,&device);

    iclamp_ai_gain = 200e-3;
    iclamp_ao_gain = 500e6;
    izero_ai_gain = 200e-3;
    izero_ao_gain = 1;
    vclamp_ai_gain = 2e-9;
    vclamp_ao_gain = 50;
    setMode(Izero, 0, 0);
}

AM2400Commander::Amplifier::~Amplifier(void) {}

void AM2400Commander::Amplifier::setMode(mode_t mode, int inputChan, int outputChan) {

    this->mode = mode;

    switch(mode) {
    case Iclamp:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,3);
            device->setAnalogGain(DAQ::AI,inputChan,iclamp_ai_gain);
            device->setAnalogCalibration(DAQ::AI,inputChan);
            device->setAnalogGain(DAQ::AO,outputChan,iclamp_ao_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
        }
        output(0) = 0.0;
        output(1) = 0.0;
        output(2) = 5.0;
        break;
    case Vclamp:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,0);
            device->setAnalogGain(DAQ::AI,inputChan,vclamp_ai_gain);
            device->setAnalogCalibration(DAQ::AI,inputChan);
            device->setAnalogGain(DAQ::AO,outputChan,vclamp_ao_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
        }
        output(0) = 0.0;
        output(1) = 5.0;
        output(2) = 0.0;
        break;
    case Izero:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,3);
            device->setAnalogGain(DAQ::AI,inputChan,izero_ai_gain);
            device->setAnalogCalibration(DAQ::AI,inputChan);
            device->setAnalogGain(DAQ::AO,outputChan,izero_ao_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
        }
        output(0) = 5.0;
        output(1) = 5.0;
        output(2) = 0.0;
        break;
    case Vcomp:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,0);
            device->setAnalogGain(DAQ::AI,inputChan,vclamp_ai_gain);
            device->setAnalogCalibration(DAQ::AI,inputChan);
            device->setAnalogGain(DAQ::AO,outputChan,vclamp_ao_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
        }
        output(0) = 5.0;
        output(1) = 0.0;
        output(2) = 0.0;
        break;
    case Vtest:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,0);
            device->setAnalogGain(DAQ::AI,inputChan,vclamp_ai_gain);
            device->setAnalogCalibration(DAQ::AI,inputChan);
            device->setAnalogGain(DAQ::AO,outputChan,vclamp_ao_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
        }
        output(0) = 0.0;
        output(1) = 0.0;
        output(2) = 0.0;
        break;
    case Iresist:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,3);
            device->setAnalogGain(DAQ::AI,inputChan,iclamp_ai_gain);
            device->setAnalogCalibration(DAQ::AI,inputChan);
            device->setAnalogGain(DAQ::AO,outputChan,iclamp_ao_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
        }
        output(0) = 5.0;
        output(1) = 0.0;
        output(2) = 5.0;
        break;
    case Ifollow:
        if(device) {
            device->setAnalogRange(DAQ::AI,inputChan,3);
            device->setAnalogGain(DAQ::AI,inputChan,iclamp_ai_gain);
            device->setAnalogCalibration(DAQ::AI,inputChan);
            device->setAnalogGain(DAQ::AO,outputChan,iclamp_ao_gain);
            device->setAnalogCalibration(DAQ::AO,outputChan);
        }
        output(0) = 0.0;
        output(1) = 5.0;
        output(2) = 5.0;
        break;
    }
}

AM2400Commander::Amplifier::mode_t AM2400Commander::Amplifier::getMode() {
	return mode;
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return AM2400Commander::Plugin::getInstance();
}

AM2400Commander::Plugin::Plugin(void) {
    menuID = MainWindow::getInstance()->createPatchClampMenuItem("AM 2400 Commander",this,SLOT(createAM2400CommanderPanel(void)));
}

AM2400Commander::Plugin::~Plugin(void) {
    MainWindow::getInstance()->removeUtilMenuItem(menuID);
    while(panelList.size())
        delete panelList.front();
    instance = 0;
}
/*
AM2400Commander::Amplifier *AM2400Commander::Plugin::getAmplifier(size_t idx) {
    if(idx)
        return 0;
    else
        return &amp;
}*/

void AM2400Commander::Plugin::createAM2400CommanderPanel(void) {
    Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
    panelList.push_back(panel);
}

void AM2400Commander::Plugin::removeAM2400CommanderPanel(AM2400Commander::Panel *panel) {
    panelList.remove(panel);
}

static Mutex mutex;
AM2400Commander::Plugin *AM2400Commander::Plugin::instance = 0;

AM2400Commander::Plugin *AM2400Commander::Plugin::getInstance(void) {
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

void AM2400Commander::Plugin::doSave(Settings::Object::State &s) const {    
    s.saveInteger("Num Panels",panelList.size());
    size_t n = 0;
    for(std::list<Panel *>::const_iterator i = panelList.begin(),end = panelList.end();i != end;++i)
        s.saveState(QString::number(n++),(*i)->save());
}

void AM2400Commander::Plugin::doLoad(const Settings::Object::State &s) {
    for(size_t i = 0;i < static_cast<size_t>(s.loadInteger("Num Panels"));++i) {
        Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
        panelList.push_back(panel);
        panel->load(s.loadState(QString::number(i)));
    }
}
