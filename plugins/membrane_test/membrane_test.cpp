/*
 * Copyright (C) 2012 Weill Medical College of Cornell University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*** INTRO
 * Membrane Test
 * 
 * membrane_test.cpp, v1.1
 *
 * Author: Francis A. Ortega (2012)
 *
 * Notes in header
 *
 ***/

#include "membrane_test.h"
#include <main_window.h>
#include <iostream>
#include <cmath>
#include <gsl/gsl_linalg.h>

#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qvalidator.h>
#include <qlayout.h>
#include <qstring.h>
#include <qbuttongroup.h>
#include <qcombobox.h>

// Input and output channels
static IO::channel_t chans[] = {
    {
        "Current Input (A)", "", IO::INPUT, },
    {
        "Voltage Output (V)", "", IO::OUTPUT, },
};

static size_t num_chans = sizeof( chans ) / sizeof( chans[0] ); // Required variable (number of input and output channels)

// Class Panel - GUI and execution
MembraneTest::Panel::Panel( QWidget *parent )
  : QWidget( parent, 0, Qt::WStyle_NormalBorder | Qt::WDestructiveClose ),  RT::Thread( 0 ),
    IO::Block( "Membrane Test", chans ,num_chans ), zapOn( false ), holdingSelection(1), holdingOptionValue(0),
    holdingOption1( 0 ), holdingOption2( -80 ), holdingOption3( -40 ),  pulseSize( 10 ), pulseWidth( 10 ), zapSize( 1000 ),
    zapWidth( 10 ), updateRate( 1 ), numStepsAvg( 5 ), memTestMode( SINGLE ), collectMemTestData( false ), memTestOn( false ),
    memTestDone( false ), idx( 0 ), cnt( round( (2*10*1e-3) / (RT::System::getInstance()->getPeriod()*1e-9)) ), zidx( 0 ),
    stepsSaved( 1 ), zcnt( round( ( 10*1e-3 ) / (RT::System::getInstance()->getPeriod()*1e-9)) ), midx( 0 ) {
    
    QBoxLayout *layout = new QVBoxLayout( this );
    ui = new MembraneTestUI( this ); // UI created with Qt Designer
    layout->addWidget( ui );

    setCaption( QString::number( getID() ) + " Membrane Test" );
    
    // Set validators and set text to defaults
    ui->holdingOption1Edit->setValidator( new QDoubleValidator( ui->holdingOption1Edit ) );
    ui->holdingOption1Edit->setText( QString::number( holdingOption1 ) );
    
    ui->holdingOption2Edit->setValidator( new QDoubleValidator( ui->holdingOption2Edit ) );
    ui->holdingOption2Edit->setText( QString::number( holdingOption2 ) );
    
    ui->holdingOption3Edit->setValidator( new QDoubleValidator( ui->holdingOption3Edit ) );
    ui->holdingOption3Edit->setText( QString::number( holdingOption3 ) );
    
    ui->pulseSizeEdit->setValidator( new QDoubleValidator( ui->pulseSizeEdit ) );
    ui->pulseSizeEdit->setText( QString::number( pulseSize ) );
    
    ui->pulseWidthEdit->setValidator( new QDoubleValidator( ui->pulseWidthEdit ) );
    ui->pulseWidthEdit->setText( QString::number( pulseWidth ) );
    
    ui->zapSizeEdit->setValidator( new QDoubleValidator( ui->zapSizeEdit ) );
    ui->zapSizeEdit->setText( QString::number( zapSize ) );
    
    ui->zapWidthEdit->setValidator( new QDoubleValidator( ui->zapWidthEdit ) );                              
    ui->zapWidthEdit->setText( QString::number( zapWidth ) );
        
    ui->memRateSpinBox->setValidator( new QIntValidator( ui->memRateSpinBox ) );
    ui->memRateSpinBox->setValue( updateRate );
    
    omega = QChar(0x3A9); // Greek letter omega for resistance
    
    ui->accessResistOutput->setText( QString( "--- M" ).append(omega) );
    ui->membraneCapOutput->setText( QString( "--- pF" ) );
    ui->membraneResistOutput->setText(  QString( "--- M" ).append(omega) );
    ui->pulseOutputLabel->setText( "-----" );

    QTimer *timer = new QTimer( this );
    timer->start( 100 );

    memTestTimer = new QTimer( this );
    
    // Qt signal and slot connections
    QObject::connect( ui->pulseButton, SIGNAL(toggled(bool)), this, SLOT(togglePulse(bool)) );
    QObject::connect( ui->zapButton, SIGNAL(clicked(void)), this, SLOT(toggleZap(void)) );
    QObject::connect( ui->acquireMemPropButton, SIGNAL(toggled(bool)), this, SLOT(toggleMemTest(bool)) );
    QObject::connect( ui->pulseButton, SIGNAL(toggled(bool)), ui->zapButton, SLOT(setEnabled(bool)) ); // Enable zap only when pulse is toggled
    QObject::connect( ui->pulseButton, SIGNAL(toggled(bool)), ui->acquireMemPropButton, SLOT(setEnabled(bool)) ); // Enable acquire only when pulse is toggled
    // Pause and Modify/Refresh Button
    // Two pause buttons for each tab, cannot use toggled() since setOn() function retriggers slot
    QObject::connect( ui->pauseButton1, SIGNAL(clicked(void)), this, SLOT(pause1(void)) );
    QObject::connect( ui->modifyButton1, SIGNAL(clicked(void)), this, SLOT(updateAll(void)) );
    QObject::connect( ui->pauseButton2, SIGNAL(clicked(void)), this, SLOT(pause2(void)) );
    QObject::connect( ui->modifyButton2, SIGNAL(clicked(void)), this, SLOT(updateAll(void)) );
    // Update parameters when enter is pressed
    QObject::connect( ui->holdingGroup, SIGNAL(pressed(int)), this, SLOT(updateHoldingGroup(int)) );
    QObject::connect( ui->holdingOption1Edit, SIGNAL(returnPressed(void)), this, SLOT(updateHoldingOption(void)) );
    QObject::connect( ui->holdingOption2Edit, SIGNAL(returnPressed(void)), this, SLOT(updateHoldingOption(void)) );
    QObject::connect( ui->holdingOption3Edit, SIGNAL(returnPressed(void)), this, SLOT(updateHoldingOption(void)) );
    QObject::connect( ui->pulseSizeEdit, SIGNAL(returnPressed(void)), this, SLOT(updatePulse(void)) );
    QObject::connect( ui->pulseWidthEdit, SIGNAL(returnPressed(void)), this, SLOT(updatePulse(void)) );
    QObject::connect( ui->zapSizeEdit, SIGNAL(returnPressed(void)), this, SLOT(updateZap(void)) );
    QObject::connect( ui->zapWidthEdit, SIGNAL(returnPressed(void)), this, SLOT(updateZap(void)) );    
    QObject::connect( ui->memRateSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateMemTest(void)) );
    QObject::connect( ui->numStepAvgSpinBox, SIGNAL(valueChanged(int)), this, SLOT(updateMemTest(void)) );
    QObject::connect( ui->memModeComboBox, SIGNAL(activated(int)), this, SLOT(updateMemTest(void)) );
    // Display updates
    QObject::connect( timer, SIGNAL(timeout(void)), this, SLOT(updateRDisplay(void)) ); // Display update rate dependent on timer
    QObject::connect( memTestTimer, SIGNAL(timeout(void)), this, SLOT(updateMemTestDisplay(void)) );
    
    show();
    setActive( false );
}

MembraneTest::Panel::~Panel( void ) {
    Plugin::getInstance()->removeMembraneTestPanel(this);
}

void MembraneTest::Panel::doDeferred( const Settings::Object::State & ) {}

// Settings Functions
void MembraneTest::Panel::doLoad( const Settings::Object::State &s ) {
    if (s.loadInteger("Maximized"))
        showMaximized();
    else if (s.loadInteger("Minimized"))
        showMinimized();

    // Window Position
    if (s.loadInteger("W") != NULL) {
        resize(s.loadInteger("W"), s.loadInteger("H"));
        parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
    }

    // Parameters
    ui->holdingGroup->setButton( s.loadInteger( "Holding Selection" ) );
    ui->holdingOption1Edit->setText( QString::number( s.loadDouble( "Holding Option 1" ) ) );
    ui->holdingOption2Edit->setText( QString::number( s.loadDouble( "Holding Option 2" ) ) );
    ui->holdingOption3Edit->setText( QString::number( s.loadDouble( "Holding Option 3" ) ) );
    updateHoldingGroup( s.loadInteger( "Holding Selection" ) );
    updateHoldingOption();

    ui->pulseSizeEdit->setText( QString::number( s.loadDouble( "Pulse Size" ) ) );
    ui->pulseWidthEdit->setText( QString::number( s.loadDouble( "Pulse Width" ) ) );
    updatePulse();
    
    ui->zapSizeEdit->setText( QString::number( s.loadDouble( "Zap Size" ) ) );
    ui->zapWidthEdit->setText( QString::number( s.loadDouble( "Zap Width" ) ) );
    updateZap();

    ui->memRateSpinBox->setValue( s.loadInteger( "Update Rate" ) );
    ui->memModeComboBox->setCurrentItem( s.loadInteger( "Mem Test Mode" ) );
    ui->numStepAvgSpinBox->setValue( s.loadInteger( "Steps to Average" ) );
    updateMemTest();
}

void MembraneTest::Panel::updateAll( void ) {
    updateHoldingOption();
    updatePulse();
    updateZap();
    updateMemTest();
}

void MembraneTest::Panel::doSave( Settings::Object::State &s ) const {
    if (isMaximized())
        s.saveInteger("Maximized", 1);
    else if (isMinimized())
        s.saveInteger("Minimized", 1);

    // Window Position
    QPoint pos = parentWidget()->pos();
    s.saveInteger("X", pos.x());
    s.saveInteger("Y", pos.y());
    s.saveInteger("W", width());
    s.saveInteger("H", height());

    // Parameters
    s.saveInteger( "Holding Selection", holdingSelection );
    s.saveDouble( "Holding Option 1", holdingOption1 );
    s.saveDouble( "Holding Option 2", holdingOption2 );
    s.saveDouble( "Holding Option 3", holdingOption3 );
    s.saveDouble( "Pulse Size", pulseSize );
    s.saveDouble( "Pulse Width", pulseWidth );
    s.saveDouble( "Zap Size", zapSize );
    s.saveDouble( "Zap Width", zapWidth);
    s.saveInteger( "Update Rate", updateRate );
    s.saveInteger( "Steps to Average", numStepsAvg );
    s.saveInteger( "Mem Test Mode", static_cast<int>( memTestMode ) );
}

void MembraneTest::Panel::execute( void ) { 
    if( zapOn ) { // If zap is on
        if( ++zidx < zcnt )
            output(0) = ( zapSize + holdingOptionValue ) * 1e-3; // Zap: single voltage step
        else
            zapOn = false;
        
        return ;
    }
    
    if( idx < (cnt / 2) ) { // First section (voltage step on)
        if( idx > (cnt / 4) ) // 2nd half of first section
            I1 += input(0); // Current during 2nd half of voltage step
        
        if( collectMemTestData ) { // Collect data for membrane property test
            currentData[midx] += input(0); // After first step, start summation
            midx++;
        }
        
        output(0) = ( holdingOptionValue + pulseSize ) * 1e-3; // Holding potential and voltage step, Voltage step output (mV -> V)
    }
    else { // Second section ( voltage step off )
        if( idx > ( (3*cnt) / 4 ) ) // 2nd half second section
            I2 += input(0); // Current during 2nd half when voltage step is off

        if( collectMemTestData ) { // Collect data for membrane property test
            currentData[midx] += input(0); // After first step, start summation
            midx++;
        }
        
        output(0) = holdingOptionValue * 1e-3; // Holding potential only, Voltage step output (mV -> V)
    }
      
    if( !( ++idx %= cnt ) ) { // Increment index and check if step is over
        dI = ( I1 - I2 ) / ( cnt / 4 ); // Current when step is off is subtracted from current during step
        I1 = I2 = 0.0; // Reset current

        if( memTestOn ) {
            midx = 0; // Reset index            
            if( !collectMemTestData ) { // Make sure data is collected at the beginning of a step
                stepsSaved = 1;
                collectMemTestData = true;
                currentData.clear();
                currentData.resize( cnt, 0 );
            }
            else if( ++stepsSaved > numStepsAvg ) { // If number of steps saved has been reached
                stepsSaved = 1;
                data = currentData; // data will be used in membrane property calculation
                midx = 0;
                currentData.clear();
                currentData.resize( cnt, 0 );
                memTestDone = true; // Used to make sure display is not updated until data was collected correctly
            }
        }
    }
}

void MembraneTest::Panel::receiveEvent(const ::Event::Object *event) {
    if(event->getName() == Event::RT_POSTPERIOD_EVENT) { // When thread rate is changed, update rate dependent parameters
        cnt = ( ( 2.0 * pulseWidth ) * 1e-3 ) / ( RT::System::getInstance()->getPeriod() * 1e-9 ); // Number of loops for complete step (2x step width)
        zcnt = ( (zapWidth * 1e-3 ) / (RT::System::getInstance()->getPeriod()*1e-9) ); // Number of loops for complete zap pulse
        collectMemTestData = false; // If membrane test is running, data collection is paused until next loop
    }
}

void MembraneTest::Panel::pause1( void ) { // Called when pauseButton1 is clicked
    if( ui->pauseButton1->isOn() ) { // If button is down, paused
        ui->pauseButton2->setOn( true ); // Set second pause button to pause
        setActive( false );
        output(0) = 0;
        if( memTestTimer->isActive() )
            memTestTimer->stop();
        ui->pulseButton->setDisabled( true );
    }
    else { // Button is up, unpause
        ui->pauseButton2->setOn( false );
        if( memTestOn )
            memTestTimer->start( ( 1.0 / updateRate ) * 1e3 ); // Start timer, Hertz to ms conversion
        ui->pulseButton->setEnabled( true );
        if( ui->pulseButton->isOn() ) // If pulse is on
            setActive( true );        
    }        
}

void MembraneTest::Panel::pause2( void ) { // Called when either pauseButton2 is clicked
    if( ui->pauseButton2->isOn() ) { // If button is down, paused
        ui->pauseButton1->setOn( true ); // Set second pause button to pause
        setActive( false );
        output(0) = 0;
        if( memTestTimer->isActive() )
            memTestTimer->stop();
        ui->pulseButton->setDisabled( true );
    }
    else { // Button is up, unpause
        ui->pauseButton1->setOn( false );
        if( memTestOn )
            memTestTimer->start( ( 1.0 / updateRate ) * 1e3 ); // Start timer, Hertz to ms conversion
        ui->pulseButton->setEnabled( true );
        if( ui->pulseButton->isOn() ) // If pulse is on
            setActive( true );        
    }        
}

void MembraneTest::Panel::updateRDisplay( void ) {
    if( !getActive() )
        return ; // Return if pulse is not on

    double R = fabs( ( pulseSize * 1e-3 ) / dI ); // Resistance calculation
    size_t exp = 0; // Exponent of resistance

    if( R != INFINITY )
        while( R >= 1e3 ) { 
            R *= 1e-3; // Reduce R by an order of magnitude
            exp++; // Increase exponent counter
        }

    QString RString;
    RString.sprintf( "%7.3f", R );

    // Choose appropriate suffix based on exponent
    if( exp ) {
        if( exp == 1 )
            RString.append( " K" ).append( omega );
        else if( exp == 2 )
            RString.append( " M" ).append( omega );
        else if( exp == 3 )
            RString.append( " G" ).append( omega );
        else {
            QString suffix;
            suffix.sprintf(" * 1e%lu",3*exp);
        }
    }

    ui->pulseOutputLabel->setText(RString);    
}

// Membrane Test Slot Functions
void MembraneTest::Panel::updateMemTestDisplay( void ) { // Runs membrane test calculation and updates GUI output
    if( memTestDone ) { // Make sure data was collected first
        memTestCalculate();
        ui->membraneCapOutput->setText( QString::number( Cm ).append( " pF" ) );
        ui->accessResistOutput->setText( QString::number( Ra ).append( " M").append( QChar(0x3A9) ) );
        ui->membraneResistOutput->setText( QString::number( Rm ).append( " M").append( QChar(0x3A9) ) );

        if( memTestMode == SINGLE ) {
            ui->acquireMemPropButton->setOn( false );
            memTestTimer->stop();
        }
    }
}

void MembraneTest::Panel::memTestCalculate( void ) {
    double Vpp = pulseSize;
    data_size = cnt;
    if( data_size != data.size() ) // Check to make sure data size is correct
        return ;
    
    // Taken from electrophys_plugin, written by Jonathan Bettencourt
    for(size_t i = 0;i<data_size;++i)
                data[i] /= numStepsAvg;

            double I1 = 0.0;
            for(size_t i=static_cast<size_t>(round(data_size/2-ceil(data_size/8)));i<data_size/2;++i)
                I1 += data[i];
            I1 /= ceil(data_size/8);

            double I2 = 0.0;
            for(size_t i=static_cast<size_t>(round(data_size-ceil(data_size/8)));i<data_size;++i)
                I2 += data[i];
            I2 /= ceil(data_size/8);

            double dt = RT::System::getInstance()->getPeriod() * 1e-6;

            double Q11;
            double tau1;
            {
                Q11 = 0.0;
                for(size_t i=0;i<data_size/2-1;++i)
                    Q11 += dt*1e-3*(data[i]+data[i+1]-2*I1)/2;
                Q11 = fabs(Q11);

                size_t xi = 0;
                for(;data[xi] <= data[xi+1];++xi);

                double sy = 0.0;
                double Y = data[xi];
                double SY = sy;
                double tSY = 0.0;
                double YSY = data[xi]*sy;
                double SYSY = sy*sy;
                double t = 0.0;
                double tt = 0.0;
                double Yt = 0.0;
                for(size_t i=xi+1;i<data_size/2;++i) {
                    sy += dt*1e-3*(data[i-1]+data[i])/2;

                    Y += data[i];
                    SY += sy;
                    tSY += (i-xi)*dt*1e-3*sy;
                    YSY += data[i]*sy;
                    SYSY += sy*sy;
                    t += (i-xi)*dt*1e-3;
                    tt += ((i-xi)*dt*1e-3)*((i-xi)*dt*1e-3);
                    Yt += (i-xi)*dt*1e-3*data[i];
                }

                double A[3*3] = {
                    data_size/2-xi,  SY,       t,
                    SY,           SYSY,     tSY,
                    t,            tSY,      tt,
                };
                double B[3] = {
                    Y,
                    YSY,
                    Yt,
                };
                double V[3*3];
                double S[3];
                double x[3];

                gsl_matrix_view a = gsl_matrix_view_array(A,3,3);
                gsl_matrix_view b = gsl_matrix_view_array(V,3,3);
                gsl_vector_view c = gsl_vector_view_array(S,3);
                gsl_vector_view d = gsl_vector_view_array(x,3);
                gsl_vector_view e = gsl_vector_view_array(B,3);

                gsl_linalg_SV_decomp(
                    &a.matrix,
                    &b.matrix,
                    &c.vector,
                    &d.vector
                    );
                gsl_linalg_SV_solve(
                    &a.matrix,
                    &b.matrix,
                    &c.vector,
                    &e.vector,
                    &d.vector
                    );
                tau1 = fabs(1.0/x[1]);
            }

            double Q12;
            double tau2;
            {
                Q12 = 0.0;
                for(size_t i=data_size/2;i<data_size;++i)
                    Q12 += dt*1e-3*(data[i]+data[i+1]-2*I2)/2;
                Q12 = fabs(Q12);

                size_t xi = data_size/2;
                for(;data[xi] >= data[xi+1];++xi);

                double sy = 0.0;
                double Y = data[xi];
                double SY = sy;
                double tSY = 0.0;
                double YSY = data[xi]*sy;
                double SYSY = sy*sy;
                double t = 0.0;
                double tt = 0.0;
                double Yt = 0.0;
                for(size_t i=xi+1;i<data_size;++i) {
                    sy += dt*1e-3*(data[i-1]+data[i])/2;

                    Y += data[i];
                    SY += sy;
                    tSY += (i-xi)*dt*1e-3*sy;
                    YSY += data[i]*sy;
                    SYSY += sy*sy;
                    t += (i-xi)*dt*1e-3;
                    tt += ((i-xi)*dt*1e-3)*((i-xi)*dt*1e-3);
                    Yt += (i-xi)*dt*1e-3*data[i];
                }

                double A[3*3] = {
                    data_size-xi,  SY,       t,
                    SY,           SYSY,     tSY,
                    t,            tSY,      tt,
                };
                double B[3] = {
                    Y,
                    YSY,
                    Yt,
                };
                double V[3*3];
                double S[3];
                double x[3];

                gsl_matrix_view a = gsl_matrix_view_array(A,3,3);
                gsl_matrix_view b = gsl_matrix_view_array(V,3,3);
                gsl_vector_view c = gsl_vector_view_array(S,3);
                gsl_vector_view d = gsl_vector_view_array(x,3);
                gsl_vector_view e = gsl_vector_view_array(B,3);

                gsl_linalg_SV_decomp(
                    &a.matrix,
                    &b.matrix,
                    &c.vector,
                    &d.vector
                    );
                gsl_linalg_SV_solve(
                    &a.matrix,
                    &b.matrix,
                    &c.vector,
                    &e.vector,
                    &d.vector
                    );
                tau2 = fabs(1.0/x[1]);
            }

            double tau = (tau1+tau2)/2.0;

            double Q1 = (Q11+Q12)/2.0;
            double Q2 = fabs(I1-I2)*tau;
            double Qt = Q1+Q2;

            double Rt = Vpp*1e-3/fabs(I1-I2);

            Ra = tau*Vpp*1e-3/Qt;
            Rm = Rt-Ra;
            Cm = Qt*Rt/(Vpp*1e-3*Rm);

            Ra = round(Ra*1e-6*10)/10;
            Rm = round(Rm*1e-6*10)/10;
            Cm = round(Cm*1e12*10)/10;
}

// Toggle Slot Functions
void MembraneTest::Panel::togglePulse( bool on ) {
    setActive( on );
    if( !on ) {
        if( memTestTimer->isActive() ) // Pause timer if it is update to prevent display updates
            memTestTimer->stop();
        output(0) = 0;
    }
    else if( memTestOn )
            memTestTimer->start( ( 1.0 / updateRate ) * 1e3 ); // Start timer, Hertz to ms conversion
}

void MembraneTest::Panel::toggleZap( void ) {
    zidx = 0;
    zapOn = true;
}

void MembraneTest::Panel::toggleMemTest( bool on ) {    
    if( on ) {
        memTestOn = true;
        memTestDone = false;
        memTestTimer->start( ( 1.0 / updateRate ) * 1e3 ); // Start timer, Hertz to ms conversion
    }
    else {
        if( memTestTimer->isActive() ){ 
            memTestTimer->stop();}
        memTestOn = false;
        collectMemTestData = false;
        memTestDone = false;
    }
}

// Update Parameter Slot Functions
void MembraneTest::Panel::updateHoldingGroup( int ho ) {
    holdingSelection = ho;
    
    switch( holdingSelection ) {
    case 1:
        holdingOptionValue = holdingOption1;
        break;
    case 2:
        holdingOptionValue = holdingOption2;
        break;
    case 3:
        holdingOptionValue = holdingOption3;        
        break;
    default:
        holdingOptionValue = holdingOption1;
        std::cout << "Error: MembraneTest::Panel::updateHoldingGroup(int) default case called" << std::endl;
        break;
    }
        return ; // Return if parameters have not changed

    UpdateHoldingGroupEvent event ( this, ho );
    RT::System::getInstance()->postEvent( &event ); // Post parameter change event
}

void MembraneTest::Panel::updateHoldingOption( void ) {
    double h1 = ui->holdingOption1Edit->text().toDouble();
    double h2 = ui->holdingOption2Edit->text().toDouble();
    double h3 = ui->holdingOption3Edit->text().toDouble();

    if( h1 == holdingOption1 && h2 == holdingOption2 && h3 == holdingOption3 )
        return ; // Return if parameters have not changed

    UpdateHoldingOptionEvent event ( this, h1, h2, h3 );
    RT::System::getInstance()->postEvent( &event ); // Post parameter change event

    updateHoldingGroup( holdingSelection ); // Update holding potential based on selection
}

void MembraneTest::Panel::updatePulse( void ) {
    double ps = ui->pulseSizeEdit->text().toDouble();
    double pw = ui->pulseWidthEdit->text().toDouble();

    if( ps == pulseSize && pw == pulseWidth )
        return ; // Return if parameters have not changed

    UpdatePulseEvent event ( this, ps, pw );
    RT::System::getInstance()->postEvent( &event ); // Post parameter change event
}

void MembraneTest::Panel::updateZap( void ) {
    double zs =  ui->zapSizeEdit->text().toDouble();
    double zw = ui->zapWidthEdit->text().toDouble();

    if( zs == zapSize && zw == zapWidth )
        return ; // Return if parameters have not changed

    UpdateZapEvent event ( this, zs, zw );
    RT::System::getInstance()->postEvent( &event ); // Post parameter change event
}

void MembraneTest::Panel::updateMemTest( void ) {
    int ur = ui->memRateSpinBox->value();
    int nsa = ui->numStepAvgSpinBox->value();
    int mtm  = ui->memModeComboBox->currentItem();

    if( ur == updateRate && nsa == numStepsAvg && mtm == static_cast<int>( memTestMode ) )
        return ; // Return if parameters have not changed

    UpdateMemTestEvent event ( this, ur, nsa, mtm );
    RT::System::getInstance()->postEvent( &event ); // Post parameter change event
}

// Event Class UpdateHoldingGroupEvent
MembraneTest::Panel::UpdateHoldingGroupEvent::UpdateHoldingGroupEvent( Panel *p, int ho )
    : panel( p ), holdingOptionValue( ho ) {
}

int MembraneTest::Panel::UpdateHoldingGroupEvent::callback( void ) {
    panel->holdingOptionValue = holdingOptionValue;
    return 0;
}

// Event Class UpdateHoldingOptionEvent
MembraneTest::Panel::UpdateHoldingOptionEvent::UpdateHoldingOptionEvent( Panel *p, double h1, double h2, double h3 )
    : panel( p ), holdingOption1( h1 ), holdingOption2( h2 ), holdingOption3( h3 ) {
}

int MembraneTest::Panel::UpdateHoldingOptionEvent::callback( void ) {
    panel->holdingOption1 = holdingOption1;
    panel->holdingOption2 = holdingOption2;
    panel->holdingOption3 = holdingOption3;
    return 0;
}

// Event Class UpdatePulseEvent
MembraneTest::Panel::UpdatePulseEvent::UpdatePulseEvent( Panel *p, double ps, double pw )
    : panel( p ), pulseSize( ps ), pulseWidth( pw ) {
}

int MembraneTest::Panel::UpdatePulseEvent::callback( void ) {
    panel->pulseSize = pulseSize;
    panel->pulseWidth = pulseWidth;

    panel->idx = 0; // Reset idx
    panel->cnt = ( ( 2.0 * pulseWidth ) * 1e-3 ) / ( RT::System::getInstance()->getPeriod() * 1e-9 ); // Number of loops for complete step (2x step width)
    if( !panel->cnt ) // Prevent divide by 0 errors
        panel->cnt = 1;
    
    return 0;
}

// Event Class UpdateZapEvent
MembraneTest::Panel::UpdateZapEvent::UpdateZapEvent( Panel *p, double zs, double zw )
    : panel( p ), zapSize( zs ), zapWidth( zw ) {
}

int MembraneTest::Panel::UpdateZapEvent::callback( void ) {
    panel->zapSize = zapSize;
    panel->zapWidth = zapWidth;

    panel->zidx = 0; // Reset zidx
    panel->zcnt = ( (zapWidth * 1e-3 ) / (RT::System::getInstance()->getPeriod()*1e-9) ); // Number of loops for complete zap pulse
    
    return 0;
}

// Event Class UpdateMemTestEvent
MembraneTest::Panel::UpdateMemTestEvent::UpdateMemTestEvent( Panel *p, int ur, int nsa, int mtm )
    : panel( p ), updateRate( ur ), numStepsAvg( nsa ), memTestMode( static_cast<MemTestMode_t>( mtm ) ) {
}

int MembraneTest::Panel::UpdateMemTestEvent::callback( void ) {
    panel->updateRate = updateRate;
    panel->numStepsAvg = numStepsAvg;
    panel->memTestMode = memTestMode;
    
    panel->memTestTimer->changeInterval( ( 1.0 / updateRate ) * 1e3 );
    if( !panel->memTestOn ) // Updating intervals starts timer, so it must be stopped if membrane test is not running
        panel->memTestTimer->stop();
    
    return 0;
}

// Class Plugin
extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return MembraneTest::Plugin::getInstance();
}

MembraneTest::Plugin::Plugin( void ) {
    menuID = MainWindow::getInstance()->createPatchClampMenuItem( "Membrane Test", this, SLOT(createMembraneTestPanel(void)) );
}

MembraneTest::Plugin::~Plugin( void ) {
    MainWindow::getInstance()->removeUtilMenuItem(menuID);
    while(panelList.size())
        delete panelList.front();
    instance = 0;
}

void MembraneTest::Plugin::createMembraneTestPanel( void ) {
    Panel *panel = new Panel( MainWindow::getInstance()->centralWidget() );
    panelList.push_back( panel );
}

void MembraneTest::Plugin::removeMembraneTestPanel( MembraneTest::Panel *panel ) {
    panelList.remove( panel );
}

void MembraneTest::Plugin::doDeferred( const Settings::Object::State & ) {}

void MembraneTest::Plugin::doLoad( const Settings::Object::State &s ) {
    for( size_t i = 0 ; i < static_cast<size_t>(s.loadInteger("Num Panels")) ; ++i ) {
        Panel *panel = new Panel( MainWindow::getInstance()->centralWidget() );
        panelList.push_back ( panel );
        panel->load( s.loadState( QString::number(i) ) );
    }
}

void MembraneTest::Plugin::doSave( Settings::Object::State &s ) const {
    s.saveInteger("Num Panels",panelList.size());
    size_t n = 0;
    for( std::list<Panel *>::const_iterator i = panelList.begin(),end = panelList.end() ; i != end ; ++i )
        s.saveState( QString::number(n++), (*i)->save() );
}

static Mutex mutex;
MembraneTest::Plugin *MembraneTest::Plugin::instance = 0;

MembraneTest::Plugin *MembraneTest::Plugin::getInstance(void) {
    if(instance)
        return instance;

    Mutex::Locker lock(&::mutex);
    if(!instance)
        instance = new Plugin();

    return instance;
}
