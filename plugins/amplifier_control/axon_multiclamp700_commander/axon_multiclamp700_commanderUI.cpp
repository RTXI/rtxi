/****************************************************************************
** Form implementation generated from reading ui file 'axon_multiclamp700_commanderUI.ui'
**
** Created: Tue Oct 23 16:14:25 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "axon_multiclamp700_commanderUI.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qspinbox.h>
#include <qframe.h>
#include <qgroupbox.h>
#include <qcombobox.h>
#include <qbuttongroup.h>
#include <qradiobutton.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a Multiclamp700_UI as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
Multiclamp700_UI::Multiclamp700_UI( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    if ( !name )
	setName( "Multiclamp700_UI" );
    setMinimumSize( QSize( 0, 390 ) );
    Multiclamp700_UILayout = new QVBoxLayout( this, 11, 6, "Multiclamp700_UILayout"); 

    layout = new QHBoxLayout( 0, 0, 6, "layout"); 

    inputLabel = new QLabel( this, "inputLabel" );
    QFont inputLabel_font(  inputLabel->font() );
    inputLabel_font.setBold( TRUE );
    inputLabel->setFont( inputLabel_font ); 
    inputLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout->addWidget( inputLabel );

    inputChannelSpinBox = new QSpinBox( this, "inputChannelSpinBox" );
    QFont inputChannelSpinBox_font(  inputChannelSpinBox->font() );
    inputChannelSpinBox_font.setBold( TRUE );
    inputChannelSpinBox->setFont( inputChannelSpinBox_font ); 
    layout->addWidget( inputChannelSpinBox );

    outputLabel = new QLabel( this, "outputLabel" );
    QFont outputLabel_font(  outputLabel->font() );
    outputLabel_font.setBold( TRUE );
    outputLabel->setFont( outputLabel_font ); 
    outputLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );
    layout->addWidget( outputLabel );

    outputChannelSpinBox = new QSpinBox( this, "outputChannelSpinBox" );
    QFont outputChannelSpinBox_font(  outputChannelSpinBox->font() );
    outputChannelSpinBox_font.setBold( TRUE );
    outputChannelSpinBox->setFont( outputChannelSpinBox_font ); 
    layout->addWidget( outputChannelSpinBox );
    Multiclamp700_UILayout->addLayout( layout );

    line = new QFrame( this, "line" );
    line->setFrameShape( QFrame::HLine );
    line->setFrameShadow( QFrame::Sunken );
    line->setFrameShape( QFrame::HLine );
    Multiclamp700_UILayout->addWidget( line );

    vcGroup = new QGroupBox( this, "vcGroup" );
    QFont vcGroup_font(  vcGroup->font() );
    vcGroup_font.setBold( TRUE );
    vcGroup->setFont( vcGroup_font ); 
    vcGroup->setAlignment( int( QGroupBox::AlignHCenter ) );
    vcGroup->setColumnLayout(0, Qt::Vertical );
    vcGroup->layout()->setSpacing( 6 );
    vcGroup->layout()->setMargin( 11 );
    vcGroupLayout = new QGridLayout( vcGroup->layout() );
    vcGroupLayout->setAlignment( Qt::AlignTop );

    vcCommandLabel = new QLabel( vcGroup, "vcCommandLabel" );
    QFont vcCommandLabel_font(  vcCommandLabel->font() );
    vcCommandLabel->setFont( vcCommandLabel_font ); 
    vcCommandLabel->setAlignment( int( QLabel::AlignCenter ) );

    vcGroupLayout->addWidget( vcCommandLabel, 0, 1 );

    vcGainLabel = new QLabel( vcGroup, "vcGainLabel" );
    QFont vcGainLabel_font(  vcGainLabel->font() );
    vcGainLabel->setFont( vcGainLabel_font ); 
    vcGainLabel->setAlignment( int( QLabel::AlignCenter ) );

    vcGroupLayout->addWidget( vcGainLabel, 0, 0 );

    vcecsComboBox = new QComboBox( FALSE, vcGroup, "vcecsComboBox" );
    vcecsComboBox->setEnabled( TRUE );
    QFont vcecsComboBox_font(  vcecsComboBox->font() );
    vcecsComboBox->setFont( vcecsComboBox_font ); 

    vcGroupLayout->addWidget( vcecsComboBox, 1, 1 );

    vcigComboBox = new QComboBox( FALSE, vcGroup, "vcigComboBox" );
    vcigComboBox->setEnabled( TRUE );
    QFont vcigComboBox_font(  vcigComboBox->font() );
    vcigComboBox->setFont( vcigComboBox_font ); 

    vcGroupLayout->addWidget( vcigComboBox, 1, 0 );
    Multiclamp700_UILayout->addWidget( vcGroup );

    icGroup = new QGroupBox( this, "icGroup" );
    QFont icGroup_font(  icGroup->font() );
    icGroup_font.setBold( TRUE );
    icGroup->setFont( icGroup_font ); 
    icGroup->setAlignment( int( QGroupBox::AlignHCenter ) );
    icGroup->setColumnLayout(0, Qt::Vertical );
    icGroup->layout()->setSpacing( 6 );
    icGroup->layout()->setMargin( 11 );
    icGroupLayout = new QGridLayout( icGroup->layout() );
    icGroupLayout->setAlignment( Qt::AlignTop );

    icGainLabel = new QLabel( icGroup, "icGainLabel" );
    QFont icGainLabel_font(  icGainLabel->font() );
    icGainLabel->setFont( icGainLabel_font ); 
    icGainLabel->setAlignment( int( QLabel::AlignCenter ) );

    icGroupLayout->addWidget( icGainLabel, 0, 0 );

    icigComboBox = new QComboBox( FALSE, icGroup, "icigComboBox" );
    icigComboBox->setEnabled( TRUE );
    QFont icigComboBox_font(  icigComboBox->font() );
    icigComboBox->setFont( icigComboBox_font ); 

    icGroupLayout->addWidget( icigComboBox, 1, 0 );

    icCommandLabel = new QLabel( icGroup, "icCommandLabel" );
    QFont icCommandLabel_font(  icCommandLabel->font() );
    icCommandLabel->setFont( icCommandLabel_font ); 
    icCommandLabel->setAlignment( int( QLabel::AlignCenter ) );

    icGroupLayout->addWidget( icCommandLabel, 0, 1 );

    icecsComboBox = new QComboBox( FALSE, icGroup, "icecsComboBox" );
    icecsComboBox->setEnabled( TRUE );
    QFont icecsComboBox_font(  icecsComboBox->font() );
    icecsComboBox->setFont( icecsComboBox_font ); 

    icGroupLayout->addWidget( icecsComboBox, 1, 1 );
    Multiclamp700_UILayout->addWidget( icGroup );

    ampModeFrame = new QFrame( this, "ampModeFrame" );
    ampModeFrame->setEnabled( TRUE );
    QFont ampModeFrame_font(  ampModeFrame->font() );
    ampModeFrame_font.setBold( TRUE );
    ampModeFrame->setFont( ampModeFrame_font ); 
    ampModeFrame->setFrameShape( QFrame::StyledPanel );
    ampModeFrame->setFrameShadow( QFrame::Plain );
    ampModeFrameLayout = new QVBoxLayout( ampModeFrame, 11, 6, "ampModeFrameLayout"); 

    ampModeLabel = new QLabel( ampModeFrame, "ampModeLabel" );
    QFont ampModeLabel_font(  ampModeLabel->font() );
    ampModeLabel->setFont( ampModeLabel_font ); 
    ampModeLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignCenter ) );
    ampModeFrameLayout->addWidget( ampModeLabel );

    buttonGroup = new QButtonGroup( ampModeFrame, "buttonGroup" );
    buttonGroup->setEnabled( TRUE );
    buttonGroup->setFrameShape( QButtonGroup::NoFrame );
    buttonGroup->setFrameShadow( QButtonGroup::Sunken );

    vclampButton = new QRadioButton( buttonGroup, "vclampButton" );
    vclampButton->setGeometry( QRect( 51, 11, 82, 20 ) );
    QFont vclampButton_font(  vclampButton->font() );
    vclampButton->setFont( vclampButton_font ); 
    buttonGroup->insert( vclampButton, 0 );

    iclampButton = new QRadioButton( buttonGroup, "iclampButton" );
    iclampButton->setGeometry( QRect( 51, 63, 82, 20 ) );
    QFont iclampButton_font(  iclampButton->font() );
    iclampButton->setFont( iclampButton_font ); 
    buttonGroup->insert( iclampButton, 2 );

    izeroButton = new QRadioButton( buttonGroup, "izeroButton" );
    izeroButton->setGeometry( QRect( 51, 37, 82, 20 ) );
    QFont izeroButton_font(  izeroButton->font() );
    izeroButton->setFont( izeroButton_font ); 
    buttonGroup->insert( izeroButton, 1 );
    ampModeFrameLayout->addWidget( buttonGroup );
    Multiclamp700_UILayout->addWidget( ampModeFrame );
    languageChange();
    resize( QSize(246, 390).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
Multiclamp700_UI::~Multiclamp700_UI()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void Multiclamp700_UI::languageChange()
{
    setCaption( tr( "Multiclamp 700 Series Commander" ) );
    inputLabel->setText( tr( "Input" ) );
    QToolTip::add( inputLabel, tr( "Input Channel" ) );
    QToolTip::add( inputChannelSpinBox, tr( "Input Channel" ) );
    outputLabel->setText( tr( "Output" ) );
    QToolTip::add( outputLabel, tr( "Output Channel" ) );
    QToolTip::add( outputChannelSpinBox, tr( "Output Channel" ) );
    vcGroup->setTitle( tr( "VClamp Settings" ) );
    vcCommandLabel->setText( tr( "Command\n"
"Sensitivity" ) );
    QToolTip::add( vcCommandLabel, tr( "Headstage configuration setting" ) );
    vcGainLabel->setText( tr( "Output\n"
"Gain" ) );
    QToolTip::add( vcGainLabel, tr( "Headstage configuration setting" ) );
    vcecsComboBox->clear();
    vcecsComboBox->insertItem( tr( "20 mv / V" ) );
    vcecsComboBox->insertItem( tr( "100 mv / V" ) );
    vcecsComboBox->setCurrentItem( 0 );
    QToolTip::add( vcecsComboBox, tr( "Headstage configuration setting" ) );
    vcigComboBox->clear();
    vcigComboBox->insertItem( tr( "1" ) );
    vcigComboBox->insertItem( tr( "2" ) );
    vcigComboBox->insertItem( tr( "5" ) );
    vcigComboBox->insertItem( tr( "10" ) );
    vcigComboBox->insertItem( tr( "20" ) );
    vcigComboBox->insertItem( tr( "50" ) );
    vcigComboBox->insertItem( tr( "100" ) );
    vcigComboBox->insertItem( tr( "200" ) );
    vcigComboBox->insertItem( tr( "500" ) );
    vcigComboBox->insertItem( tr( "1000" ) );
    vcigComboBox->insertItem( tr( "2000" ) );
    vcigComboBox->setCurrentItem( 0 );
    QToolTip::add( vcigComboBox, tr( "Headstage configuration setting" ) );
    icGroup->setTitle( tr( "IClamp Settings" ) );
    icGainLabel->setText( tr( "Output\n"
"Gain" ) );
    QToolTip::add( icGainLabel, tr( "Headstage configuration setting" ) );
    icigComboBox->clear();
    icigComboBox->insertItem( tr( "1" ) );
    icigComboBox->insertItem( tr( "2" ) );
    icigComboBox->insertItem( tr( "5" ) );
    icigComboBox->insertItem( tr( "10" ) );
    icigComboBox->insertItem( tr( "20" ) );
    icigComboBox->insertItem( tr( "50" ) );
    icigComboBox->insertItem( tr( "100" ) );
    icigComboBox->insertItem( tr( "200" ) );
    icigComboBox->insertItem( tr( "500" ) );
    icigComboBox->insertItem( tr( "1000" ) );
    icigComboBox->insertItem( tr( "2000" ) );
    icigComboBox->setCurrentItem( 0 );
    QToolTip::add( icigComboBox, tr( "Headstage configuration setting" ) );
    icCommandLabel->setText( tr( "Command\n"
"Sensitivity" ) );
    QToolTip::add( icCommandLabel, tr( "Headstage configuration setting" ) );
    icecsComboBox->clear();
    icecsComboBox->insertItem( tr( "20 mv / V" ) );
    icecsComboBox->insertItem( tr( "100 mv / V" ) );
    icecsComboBox->setCurrentItem( 0 );
    QToolTip::add( icecsComboBox, tr( "Headstage configuration setting" ) );
    ampModeLabel->setText( tr( "Amp Mode" ) );
    QToolTip::add( ampModeLabel, tr( "Amplifier Mode" ) );
    buttonGroup->setTitle( QString::null );
    vclampButton->setText( tr( "V Clamp" ) );
    QToolTip::add( vclampButton, tr( "Voltage clamp" ) );
    iclampButton->setText( tr( "I Clamp" ) );
    QToolTip::add( iclampButton, tr( "Current Clamp" ) );
    izeroButton->setText( tr( "I = 0" ) );
    QToolTip::add( izeroButton, tr( "Current = Zero" ) );
}

