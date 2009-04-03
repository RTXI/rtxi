#include <cmath>
#include <electrode_resistance_measurement.h>
#include <main_window.h>
#include <rt.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qvalidator.h>

static IO::channel_t chans[] = {
    {
        "I",
        "",
        IO::INPUT,
    },
    {
        "V",
        "",
        IO::OUTPUT,
    },
};
static size_t num_chans = sizeof(chans)/sizeof(chans[0]);

ElectrodeResistanceMeasurement::Panel::Panel(QWidget *parent)
    : QWidget(parent,0,Qt::WStyle_NormalBorder | Qt::WDestructiveClose), RT::Thread(0), IO::Block("Electrode Resistance",chans,num_chans),
      V0(-65e-3), dV(10e-3), T(17e-3), I1(0.0), I2(0.0), dI(0.0), cnt(round(2*T/(RT::System::getInstance()->getPeriod()*1e-9))), idx(0) {
    DEBUG_MSG("ElectrodeResistanceMeasurement::Panel::Panel : starting\n");

    if(!cnt) {
        DEBUG_MSG("cnt == 0\n");
        return;
    }

    setCaption(QString::number(getID())+" Electrode Resistance");

    QGridLayout *layout = new QGridLayout(this,6,4);

    RLabel = new QLabel(this);
    RLabel->setFont(QFont("Courier",32));
    RLabel->setAlignment(Qt::AlignCenter);
    RLabel->setScaledContents(true);

    layout->addMultiCellWidget(RLabel,0,1,0,3);

    V0Edit = new QLineEdit(this);
    V0Edit->setValidator(new QDoubleValidator(V0Edit));
    V0Edit->setAlignment(Qt::AlignRight);
    V0Edit->setText(QString::number(V0*1e3));

    layout->addMultiCellWidget(V0Edit,2,2,0,2);

    QLabel *V0Label = new QLabel("Holding Potential (mV)",this);

    layout->addWidget(V0Label,2,3);

    dVEdit = new QLineEdit(this);
    dVEdit->setValidator(new QDoubleValidator(dVEdit));
    dVEdit->setAlignment(Qt::AlignRight);
    dVEdit->setText(QString::number(dV*1e3));

    layout->addMultiCellWidget(dVEdit,3,3,0,2);

    QLabel *dVLabel = new QLabel("Step Size (mV)",this);

    layout->addWidget(dVLabel,3,3);

    TEdit = new QLineEdit(this);
    TEdit->setValidator(new QDoubleValidator(TEdit));
    TEdit->setAlignment(Qt::AlignRight);
    TEdit->setText(QString::number(T*1e3));

    layout->addMultiCellWidget(TEdit,4,4,0,2);

    QLabel *TLabel = new QLabel("Pulse Width (ms)",this);

    layout->addWidget(TLabel,4,3);

    QPushButton *applyButton = new QPushButton("Apply",this);
    pauseButton = new QPushButton("Pause",this);
    pauseButton->setToggleButton(true);

    layout->addMultiCellWidget(applyButton,5,5,0,1);
    layout->addMultiCellWidget(pauseButton,5,5,2,3);

    QTimer *timer = new QTimer(this);
    timer->start(100);

    QObject::connect(applyButton,SIGNAL(clicked(void)),this,SLOT(updateParameters(void)));
    QObject::connect(pauseButton,SIGNAL(toggled(bool)),this,SLOT(togglePause(void)));
    QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(updateDisplay(void)));

    show();
    setActive(true);

    DEBUG_MSG("ElectrodeResistanceMeasurement::Panel::Panel : finished\n");
}

ElectrodeResistanceMeasurement::Panel::~Panel(void) {
    Plugin::getInstance()->removeElectrodeResistanceMeasurementPanel(this);
}

void ElectrodeResistanceMeasurement::Panel::doDeferred(const Settings::Object::State &) {}

void ElectrodeResistanceMeasurement::Panel::doLoad(const Settings::Object::State &s) {
    V0Edit->setText(QString::number(s.loadDouble("Holding Potential")));
    dVEdit->setText(QString::number(s.loadDouble("Step Size")));
    TEdit->setText(QString::number(s.loadDouble("Pulse Width")));

    updateParameters();
}

void ElectrodeResistanceMeasurement::Panel::doSave(Settings::Object::State &s) const {
    s.saveDouble("Holding Potential",V0*1e3);
    s.saveDouble("Step Size",dV*1e3);
    s.saveDouble("Pulse Width",T*1e3);
}

void ElectrodeResistanceMeasurement::Panel::execute(void) {
    if(idx < cnt/2) {
        if(idx > cnt/4)
            I1 += input(0);
        output(0) = V0+dV;
    } else {
        if(idx > (3*cnt)/4)
            I2 += input(0);
        output(0) = V0;
    }

    if(!(++idx %= cnt)) {
        dI = (I1-I2)/(cnt/4);
        I1 = I2 = 0.0;
    }
}

void ElectrodeResistanceMeasurement::Panel::updateDisplay(void) {
    if(!getActive())
        return;

    double R = fabs(dV/dI);
    size_t exp = 0;

    if(R != INFINITY)
        while(R >= 1e3) {
            R *= 1e-3;
            ++exp;
        }

    QString RString;
    RString.sprintf("%7.3f",R);

    if(exp) {
        if(exp == 1)
            RString.append(" K");
        else if(exp == 2)
            RString.append(" M");
        else if(exp == 3)
            RString.append(" G");
        else {
            QString suffix;
            suffix.sprintf(" * 1e%lu",3*exp);
        }
    }

    RLabel->setText(RString);
}

void ElectrodeResistanceMeasurement::Panel::updateParameters(void) {
    double v0 = V0Edit->text().toDouble()*1e-3;
    double dv = dVEdit->text().toDouble()*1e-3;
    double t = TEdit->text().toDouble()*1e-3;

    if(V0 == v0 && dv == dV && T == t)
        return;

    UpdateParametersEvent event (this,v0,dv,t);
    RT::System::getInstance()->postEvent(&event);
}

void ElectrodeResistanceMeasurement::Panel::togglePause(void) {
    setActive(!pauseButton->isOn());
    if(!getActive())
        output(0) = 0.0;
}

ElectrodeResistanceMeasurement::Panel::UpdateParametersEvent::UpdateParametersEvent(ElectrodeResistanceMeasurement::Panel *p,double v0,double dv,double t)
    : panel(p), V0(v0), dV(dv), T(t) {}

ElectrodeResistanceMeasurement::Panel::UpdateParametersEvent::~UpdateParametersEvent(void) {}

int ElectrodeResistanceMeasurement::Panel::UpdateParametersEvent::callback(void) {
    panel->V0 = V0;
    panel->dV = dV;
    panel->T = T;

    panel->cnt = 2.0*T/(RT::System::getInstance()->getPeriod()*1e-9);
    if(!panel->cnt)
        panel->cnt = 1;

    panel->idx = 0;

    panel->I1 = panel->I2 = 0.0;

    return 0;
}

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return ElectrodeResistanceMeasurement::Plugin::getInstance();
}

ElectrodeResistanceMeasurement::Plugin::Plugin(void) {
    menuID = MainWindow::getInstance()->createControlMenuItem("Electrode Resistance Measurement",this,SLOT(createElectrodeResistanceMeasurementPanel(void)));
}

ElectrodeResistanceMeasurement::Plugin::~Plugin(void) {
    MainWindow::getInstance()->removeControlMenuItem(menuID);
    while(panelList.size())
        delete panelList.front();
    instance = 0;
}

void ElectrodeResistanceMeasurement::Plugin::createElectrodeResistanceMeasurementPanel(void) {
    Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
    panelList.push_back(panel);
}

void ElectrodeResistanceMeasurement::Plugin::removeElectrodeResistanceMeasurementPanel(ElectrodeResistanceMeasurement::Panel *panel) {
    panelList.remove(panel);
}

void ElectrodeResistanceMeasurement::Plugin::doDeferred(const Settings::Object::State &s) {
    size_t i = 0;
    for(std::list<Panel *>::iterator j = panelList.begin(),end = panelList.end();j != end;++j)
        (*j)->deferred(s.loadState(QString::number(i++)));
}

void ElectrodeResistanceMeasurement::Plugin::doLoad(const Settings::Object::State &s) {
    for(size_t i = 0;i < static_cast<size_t>(s.loadInteger("Num Panels"));++i) {
        Panel *panel = new Panel(MainWindow::getInstance()->centralWidget());
        panelList.push_back(panel);
        panel->load(s.loadState(QString::number(i)));
    }
}

void ElectrodeResistanceMeasurement::Plugin::doSave(Settings::Object::State &s) const {
    s.saveInteger("Num Panels",panelList.size());
    size_t n = 0;
    for(std::list<Panel *>::const_iterator i = panelList.begin(),end = panelList.end();i != end;++i)
        s.saveState(QString::number(n++),(*i)->save());
}

static Mutex mutex;
ElectrodeResistanceMeasurement::Plugin *ElectrodeResistanceMeasurement::Plugin::instance = 0;

ElectrodeResistanceMeasurement::Plugin *ElectrodeResistanceMeasurement::Plugin::getInstance(void) {
    if(instance)
        return instance;

    Mutex::Locker lock(&::mutex);
    if(!instance)
        instance = new Plugin();

    return instance;
}
