/****************************************************************************
** Form implementation generated from reading ui file 'axon_1d_amp_commanderUI.ui'
**
** Created: Thu Aug 30 15:16:27 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "axon_axopatch1d_commanderUI.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a Axon_1D_Amp_CommanderUI as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
Axon_1D_Amp_CommanderUI::Axon_1D_Amp_CommanderUI( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "Axon_1D_Amp_CommanderUI" );
    Axon_1D_Amp_CommanderUILayout = new QVBoxLayout( this, 11, 6, "Axon_1D_Amp_CommanderUILayout"); 

    layout5 = new QHBoxLayout( 0, 0, 6, "layout5"); 

    inputLabel = new QLabel( this, "inputLabel" );
    QFont inputLabel_font(  inputLabel->font() );
    inputLabel_font.setBold( TRUE );
    inputLabel->setFont( inputLabel_font ); 
    inputLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout5->addWidget( inputLabel );

    inputChannelSpinBox = new QSpinBox( this, "inputChannelSpinBox" );
    QFont inputChannelSpinBox_font(  inputChannelSpinBox->font() );
    inputChannelSpinBox_font.setBold( TRUE );
    inputChannelSpinBox->setFont( inputChannelSpinBox_font ); 
    layout5->addWidget( inputChannelSpinBox );

    outputLabel = new QLabel( this, "outputLabel" );
    QFont outputLabel_font(  outputLabel->font() );
    outputLabel_font.setBold( TRUE );
    outputLabel->setFont( outputLabel_font ); 
    outputLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout5->addWidget( outputLabel );

    outputChannelSpinBox = new QSpinBox( this, "outputChannelSpinBox" );
    QFont outputChannelSpinBox_font(  outputChannelSpinBox->font() );
    outputChannelSpinBox_font.setBold( TRUE );
    outputChannelSpinBox->setFont( outputChannelSpinBox_font ); 
    layout5->addWidget( outputChannelSpinBox );
    Axon_1D_Amp_CommanderUILayout->addLayout( layout5 );

    line1 = new QFrame( this, "line1" );
    line1->setFrameShape( QFrame::HLine );
    line1->setFrameShadow( QFrame::Sunken );
    line1->setFrameShape( QFrame::HLine );
    Axon_1D_Amp_CommanderUILayout->addWidget( line1 );

    layout4 = new QHBoxLayout( 0, 0, 6, "layout4"); 

    gainLabel = new QLabel( this, "gainLabel" );
    QFont gainLabel_font(  gainLabel->font() );
    gainLabel_font.setBold( TRUE );
    gainLabel->setFont( gainLabel_font ); 
    gainLabel->setAlignment( int( QLabel::AlignCenter ) );
    layout4->addWidget( gainLabel );

    gainComboBox = new QComboBox( FALSE, this, "gainComboBox" );
    QFont gainComboBox_font(  gainComboBox->font() );
    gainComboBox_font.setBold( TRUE );
    gainComboBox->setFont( gainComboBox_font ); 
    layout4->addWidget( gainComboBox );
    Axon_1D_Amp_CommanderUILayout->addLayout( layout4 );

    line1_2 = new QFrame( this, "line1_2" );
    line1_2->setFrameShape( QFrame::HLine );
    line1_2->setFrameShadow( QFrame::Sunken );
    line1_2->setFrameShape( QFrame::HLine );
    Axon_1D_Amp_CommanderUILayout->addWidget( line1_2 );

    layout3 = new QVBoxLayout( 0, 0, 6, "layout3"); 

    configLabel = new QLabel( this, "configLabel" );
    QFont configLabel_font(  configLabel->font() );
    configLabel_font.setBold( TRUE );
    configLabel->setFont( configLabel_font ); 
    configLabel->setAlignment( int( QLabel::AlignCenter ) );
    layout3->addWidget( configLabel );

    configComboBox = new QComboBox( FALSE, this, "configComboBox" );
    configComboBox->setEnabled( TRUE );
    QFont configComboBox_font(  configComboBox->font() );
    configComboBox_font.setBold( TRUE );
    configComboBox->setFont( configComboBox_font ); 
    layout3->addWidget( configComboBox );
    Axon_1D_Amp_CommanderUILayout->addLayout( layout3 );

    layout6 = new QVBoxLayout( 0, 0, 6, "layout6"); 

    commandLabel = new QLabel( this, "commandLabel" );
    QFont commandLabel_font(  commandLabel->font() );
    commandLabel_font.setBold( TRUE );
    commandLabel->setFont( commandLabel_font ); 
    commandLabel->setAlignment( int( QLabel::AlignCenter ) );
    layout6->addWidget( commandLabel );

    commandComboBox = new QComboBox( FALSE, this, "commandComboBox" );
    commandComboBox->setEnabled( TRUE );
    QFont commandComboBox_font(  commandComboBox->font() );
    commandComboBox_font.setBold( TRUE );
    commandComboBox->setFont( commandComboBox_font ); 
    layout6->addWidget( commandComboBox );
    Axon_1D_Amp_CommanderUILayout->addLayout( layout6 );

    frame3 = new QFrame( this, "frame3" );
    frame3->setEnabled( TRUE );
    QFont frame3_font(  frame3->font() );
    frame3_font.setBold( TRUE );
    frame3->setFont( frame3_font ); 
    frame3->setFrameShape( QFrame::StyledPanel );
    frame3->setFrameShadow( QFrame::Plain );
    frame3Layout = new QHBoxLayout( frame3, 11, 6, "frame3Layout"); 

    layout1 = new QVBoxLayout( 0, 0, 6, "layout1"); 

    ampModeLabel = new QLabel( frame3, "ampModeLabel" );
    QFont ampModeLabel_font(  ampModeLabel->font() );
    ampModeLabel->setFont( ampModeLabel_font ); 
    ampModeLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
    layout1->addWidget( ampModeLabel );

    autoModeButton = new QPushButton( frame3, "autoModeButton" );
    QFont autoModeButton_font(  autoModeButton->font() );
    autoModeButton->setFont( autoModeButton_font ); 
    autoModeButton->setToggleButton( TRUE );
    layout1->addWidget( autoModeButton );
    frame3Layout->addLayout( layout1 );

    ampModeButtonGroup = new QButtonGroup( frame3, "ampModeButtonGroup" );
    ampModeButtonGroup->setEnabled( TRUE );
    ampModeButtonGroup->setFrameShape( QButtonGroup::NoFrame );
    ampModeButtonGroup->setFrameShadow( QButtonGroup::Sunken );
    ampModeButtonGroup->setColumnLayout(0, Qt::Vertical );
    ampModeButtonGroup->layout()->setSpacing( 6 );
    ampModeButtonGroup->layout()->setMargin( 11 );
    ampModeButtonGroupLayout = new QVBoxLayout( ampModeButtonGroup->layout() );
    ampModeButtonGroupLayout->setAlignment( Qt::AlignTop );

    vclampButton = new QRadioButton( ampModeButtonGroup, "vclampButton" );
    QFont vclampButton_font(  vclampButton->font() );
    vclampButton->setFont( vclampButton_font ); 
    ampModeButtonGroupLayout->addWidget( vclampButton );

    izeroButton = new QRadioButton( ampModeButtonGroup, "izeroButton" );
    QFont izeroButton_font(  izeroButton->font() );
    izeroButton->setFont( izeroButton_font ); 
    ampModeButtonGroupLayout->addWidget( izeroButton );

    iclampButton = new QRadioButton( ampModeButtonGroup, "iclampButton" );
    QFont iclampButton_font(  iclampButton->font() );
    iclampButton->setFont( iclampButton_font ); 
    ampModeButtonGroupLayout->addWidget( iclampButton );
    frame3Layout->addWidget( ampModeButtonGroup );
    Axon_1D_Amp_CommanderUILayout->addWidget( frame3 );
    languageChange();
    resize( QSize(236, 318).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
Axon_1D_Amp_CommanderUI::~Axon_1D_Amp_CommanderUI()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Axon_1D_Amp_CommanderUI::languageChange()
{
    setCaption( tr( "Axon 1D Amp Commander" ) );
    inputLabel->setText( tr( "Input" ) );
    QToolTip::add( inputLabel, tr( "Input Channel" ) );
    QToolTip::add( inputChannelSpinBox, tr( "Input Channel" ) );
    outputLabel->setText( tr( "Output" ) );
    QToolTip::add( outputLabel, tr( "Output Channel" ) );
    QToolTip::add( outputChannelSpinBox, tr( "Output Channel" ) );
    gainLabel->setText( tr( "Output Gain" ) );
    QToolTip::add( gainLabel, tr( "Scaled output gain setting" ) );
    gainComboBox->clear();
    gainComboBox->insertItem( tr( "0.5" ) );
    gainComboBox->insertItem( tr( "1" ) );
    gainComboBox->insertItem( tr( "2" ) );
    gainComboBox->insertItem( tr( "5" ) );
    gainComboBox->insertItem( tr( "10" ) );
    gainComboBox->insertItem( tr( "20" ) );
    gainComboBox->insertItem( tr( "50" ) );
    gainComboBox->insertItem( tr( "100" ) );
    gainComboBox->setCurrentItem( 1 );
    QToolTip::add( gainComboBox, tr( "Scaled output gain setting" ) );
    configLabel->setText( tr( "Headstage Config" ) );
    QToolTip::add( configLabel, tr( "Headstage configuration setting" ) );
    configComboBox->clear();
    configComboBox->insertItem( trUtf8( "\xce\xb2\x3d\x31\x30\x30" ) );
    configComboBox->insertItem( trUtf8( "\xce\xb2\x3d\x31" ) );
    configComboBox->insertItem( trUtf8( "\xce\xb2\x3d\x30\x2e\x31" ) );
    configComboBox->setCurrentItem( 1 );
    QToolTip::add( configComboBox, tr( "Headstage configuration setting" ) );
    commandLabel->setText( tr( "Ext. Command Sensitivity" ) );
    QToolTip::add( commandLabel, tr( "Headstage configuration setting" ) );
    commandComboBox->clear();
    commandComboBox->insertItem( tr( "20 mv / V" ) );
    commandComboBox->insertItem( tr( "1 mv / V" ) );
    commandComboBox->setCurrentItem( 0 );
    QToolTip::add( commandComboBox, tr( "Headstage configuration setting" ) );
    ampModeLabel->setText( tr( "Amp Mode" ) );
    QToolTip::add( ampModeLabel, tr( "Amplifier Mode" ) );
    autoModeButton->setText( tr( "Auto" ) );
    QToolTip::add( autoModeButton, tr( "Auto mode - Use amplifier's telegraph outputs to automatically retrieve settings." ) );
    ampModeButtonGroup->setTitle( QString::null );
    vclampButton->setText( tr( "V Clamp" ) );
    QToolTip::add( vclampButton, tr( "Voltage clamp" ) );
    izeroButton->setText( tr( "I = 0" ) );
    QToolTip::add( izeroButton, tr( "Current = Zero" ) );
    iclampButton->setText( tr( "I Clamp" ) );
    QToolTip::add( iclampButton, tr( "Current Clamp" ) );
}

