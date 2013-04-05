/****************************************************************************
** Form implementation generated from reading ui file 'am_amp_commanderUI.ui'
**
** Created: Mon Jul 23 16:20:37 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "am_2400_commanderUI.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qlabel.h>
#include <qframe.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a AM_Amp_CommanderUI as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
AM_Amp_CommanderUI::AM_Amp_CommanderUI( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "AM_Amp_CommanderUI" );
    AM_Amp_CommanderUILayout = new QVBoxLayout( this, 11, 6, "AM_Amp_CommanderUILayout"); 

    layout1 = new QGridLayout( 0, 1, 1, 0, 6, "layout1"); 

    inputChannelSpinBox = new QSpinBox( this, "inputChannelSpinBox" );
    QFont inputChannelSpinBox_font(  inputChannelSpinBox->font() );
    inputChannelSpinBox_font.setBold( TRUE );
    inputChannelSpinBox->setFont( inputChannelSpinBox_font ); 

    layout1->addWidget( inputChannelSpinBox, 0, 1 );

    outputLabel = new QLabel( this, "outputLabel" );
    QFont outputLabel_font(  outputLabel->font() );
    outputLabel_font.setBold( TRUE );
    outputLabel->setFont( outputLabel_font ); 
    outputLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout1->addWidget( outputLabel, 1, 0 );

    inputLabel = new QLabel( this, "inputLabel" );
    QFont inputLabel_font(  inputLabel->font() );
    inputLabel_font.setBold( TRUE );
    inputLabel->setFont( inputLabel_font ); 
    inputLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    layout1->addWidget( inputLabel, 0, 0 );

    outputChannelSpinBox = new QSpinBox( this, "outputChannelSpinBox" );
    QFont outputChannelSpinBox_font(  outputChannelSpinBox->font() );
    outputChannelSpinBox_font.setBold( TRUE );
    outputChannelSpinBox->setFont( outputChannelSpinBox_font ); 

    layout1->addWidget( outputChannelSpinBox, 1, 1 );
    AM_Amp_CommanderUILayout->addLayout( layout1 );

    line1 = new QFrame( this, "line1" );
    line1->setFrameShape( QFrame::HLine );
    line1->setFrameShadow( QFrame::Sunken );
    line1->setFrameShape( QFrame::HLine );
    AM_Amp_CommanderUILayout->addWidget( line1 );

    frame3 = new QFrame( this, "frame3" );
    frame3->setEnabled( TRUE );
    QFont frame3_font(  frame3->font() );
    frame3_font.setBold( TRUE );
    frame3->setFont( frame3_font ); 
    frame3->setFrameShape( QFrame::NoFrame );
    frame3->setFrameShadow( QFrame::Plain );
    frame3Layout = new QHBoxLayout( frame3, 11, 6, "frame3Layout"); 
    spacer1 = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    frame3Layout->addItem( spacer1 );

    layout2 = new QVBoxLayout( 0, 0, 6, "layout2"); 

    ampModeLabel = new QLabel( frame3, "ampModeLabel" );
    QFont ampModeLabel_font(  ampModeLabel->font() );
    ampModeLabel->setFont( ampModeLabel_font ); 
    ampModeLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
    layout2->addWidget( ampModeLabel );

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

    vcompButton = new QRadioButton( ampModeButtonGroup, "vcompButton" );
    QFont vcompButton_font(  vcompButton->font() );
    vcompButton->setFont( vcompButton_font ); 
    ampModeButtonGroupLayout->addWidget( vcompButton );

    vtestButton = new QRadioButton( ampModeButtonGroup, "vtestButton" );
    QFont vtestButton_font(  vtestButton->font() );
    vtestButton->setFont( vtestButton_font ); 
    ampModeButtonGroupLayout->addWidget( vtestButton );

    iresistButton = new QRadioButton( ampModeButtonGroup, "iresistButton" );
    QFont iresistButton_font(  iresistButton->font() );
    iresistButton->setFont( iresistButton_font ); 
    ampModeButtonGroupLayout->addWidget( iresistButton );

    ifollowButton = new QRadioButton( ampModeButtonGroup, "ifollowButton" );
    QFont ifollowButton_font(  ifollowButton->font() );
    ifollowButton->setFont( ifollowButton_font ); 
    ampModeButtonGroupLayout->addWidget( ifollowButton );
    layout2->addWidget( ampModeButtonGroup );
    frame3Layout->addLayout( layout2 );
    spacer2 = new QSpacerItem( 0, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    frame3Layout->addItem( spacer2 );
    AM_Amp_CommanderUILayout->addWidget( frame3 );
    languageChange();
    resize( QSize(167, 332).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
AM_Amp_CommanderUI::~AM_Amp_CommanderUI()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void AM_Amp_CommanderUI::languageChange()
{
    setCaption( tr( "AM Amp Commander" ) );
    QToolTip::add( inputChannelSpinBox, tr( "Input Channel" ) );
    outputLabel->setText( tr( "Output" ) );
    QToolTip::add( outputLabel, tr( "Output Channel" ) );
    inputLabel->setText( tr( "Input" ) );
    QToolTip::add( inputLabel, tr( "Input Channel" ) );
    QToolTip::add( outputChannelSpinBox, tr( "Output Channel" ) );
    ampModeLabel->setText( tr( "Amplifier Mode" ) );
    QToolTip::add( ampModeLabel, tr( "Amplifier Mode" ) );
    ampModeButtonGroup->setTitle( QString::null );
    vclampButton->setText( tr( "Vclamp" ) );
    QToolTip::add( vclampButton, tr( "Voltage clamp" ) );
    izeroButton->setText( tr( "I = 0" ) );
    QToolTip::add( izeroButton, tr( "Current Clamp" ) );
    iclampButton->setText( tr( "Iclamp" ) );
    QToolTip::add( iclampButton, tr( "Current Clamp" ) );
    vcompButton->setText( tr( "Vcomp" ) );
    QToolTip::add( vcompButton, tr( "Current Clamp" ) );
    vtestButton->setText( tr( "Vtest" ) );
    QToolTip::add( vtestButton, tr( "Current Clamp" ) );
    iresistButton->setText( tr( "Iresist" ) );
    QToolTip::add( iresistButton, tr( "Current Clamp" ) );
    ifollowButton->setText( tr( "Ifollow" ) );
    QToolTip::add( ifollowButton, tr( "Current Clamp" ) );
}

