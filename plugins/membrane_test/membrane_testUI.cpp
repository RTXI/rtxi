/****************************************************************************
** Form implementation generated from reading ui file 'membrane_testUI.ui'
**
** Created: Wed Sep 26 15:30:08 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "membrane_testUI.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qtabwidget.h>
#include <qlabel.h>
#include <qgroupbox.h>
#include <qbuttongroup.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static const unsigned char image0_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x00,
    0xef, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xc5, 0xd3, 0x31, 0x6e, 0x84,
    0x30, 0x10, 0x05, 0xd0, 0x6f, 0x13, 0x17, 0x9c, 0x87, 0x43, 0xc0, 0x4d,
    0xb6, 0xe5, 0x28, 0xb4, 0xa9, 0xb7, 0xe3, 0x10, 0xdc, 0x80, 0x7b, 0x20,
    0x81, 0x05, 0x02, 0x34, 0x32, 0x63, 0x0b, 0xa7, 0xd9, 0x25, 0xeb, 0xac,
    0x51, 0x22, 0x51, 0xc4, 0xdd, 0x3c, 0xd9, 0x7f, 0x7e, 0x63, 0xe1, 0xbd,
    0xc7, 0x95, 0x23, 0x2f, 0xbd, 0x06, 0xf0, 0xf1, 0x3a, 0x54, 0x55, 0xf5,
    0x09, 0x20, 0x7b, 0x8c, 0xad, 0xf7, 0xfe, 0x26, 0x84, 0x08, 0xac, 0x2c,
    0xcb, 0xdb, 0x69, 0x80, 0xb5, 0x36, 0x2b, 0x8a, 0x22, 0x63, 0x66, 0x34,
    0x4d, 0xf3, 0xe4, 0x2c, 0xcf, 0xf3, 0xcc, 0x5a, 0xfb, 0x6a, 0xf1, 0x00,
    0x66, 0x06, 0x11, 0x61, 0x9a, 0x26, 0x30, 0xf3, 0xe1, 0xeb, 0xba, 0x62,
    0x59, 0x96, 0xc0, 0xa2, 0x01, 0xc6, 0x18, 0x10, 0x11, 0xe6, 0x79, 0x86,
    0x31, 0xe6, 0xf0, 0x61, 0x18, 0xd0, 0x75, 0x5d, 0x60, 0xa7, 0x01, 0xcc,
    0x8c, 0x71, 0x1c, 0x83, 0xcb, 0x5a, 0x6b, 0xf4, 0x7d, 0xff, 0x7b, 0x00,
    0x11, 0x41, 0x6b, 0x0d, 0xad, 0x35, 0x88, 0xe8, 0x70, 0xe7, 0x1c, 0xa4,
    0x94, 0x81, 0x9d, 0x36, 0x70, 0xce, 0x21, 0x49, 0x92, 0x60, 0x9b, 0xf7,
    0xfe, 0xcd, 0x4e, 0x1b, 0xec, 0xfb, 0x0e, 0xa5, 0xd4, 0xdb, 0xb6, 0x98,
    0x45, 0x1b, 0xc4, 0xb6, 0xfd, 0xb9, 0xc1, 0xb6, 0x6d, 0x90, 0x52, 0x22,
    0x4d, 0x53, 0x6c, 0xdb, 0x76, 0xb8, 0x10, 0x02, 0x4a, 0xa9, 0xc0, 0xa2,
    0x01, 0xd6, 0xda, 0xf6, 0x7e, 0xbf, 0x3f, 0x1f, 0xb5, 0x0f, 0x6e, 0xeb,
    0xba, 0xfe, 0x69, 0xdf, 0xe1, 0xff, 0xfe, 0x99, 0x2e, 0x07, 0x7c, 0x01,
    0x38, 0xff, 0xa9, 0x75, 0x2d, 0x27, 0xb3, 0x2a, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};

static const unsigned char image1_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x02,
    0xab, 0x49, 0x44, 0x41, 0x54, 0x38, 0x8d, 0xa5, 0x93, 0xbf, 0x4b, 0x1b,
    0x71, 0x18, 0xc6, 0x9f, 0xfb, 0x95, 0x86, 0xa4, 0x9e, 0x21, 0xa9, 0xb1,
    0xed, 0xc5, 0x80, 0xda, 0x06, 0x0c, 0xb4, 0x0a, 0x69, 0x17, 0x43, 0xc8,
    0x18, 0x67, 0x85, 0x0e, 0xae, 0x1d, 0xba, 0x77, 0x2b, 0x88, 0x43, 0xb1,
    0x6b, 0xe9, 0x7f, 0xe0, 0x22, 0x0e, 0x15, 0x14, 0x0c, 0x14, 0x62, 0xc7,
    0xd0, 0x50, 0xd0, 0xeb, 0xd2, 0x36, 0x43, 0x8a, 0x8a, 0xd2, 0xbb, 0x9e,
    0x5c, 0x08, 0x77, 0x67, 0x72, 0x3f, 0xbe, 0xdf, 0xe4, 0x7b, 0x1d, 0x34,
    0x56, 0xbb, 0xfa, 0xac, 0x2f, 0x7c, 0xde, 0xe7, 0x7d, 0x1e, 0x5e, 0x2e,
    0x0c, 0x43, 0xdc, 0x46, 0x22, 0x00, 0xac, 0xad, 0xad, 0x41, 0x92, 0x24,
    0x88, 0xa2, 0x08, 0x9e, 0xe7, 0x13, 0x1c, 0xc7, 0x95, 0x00, 0x94, 0x01,
    0x3c, 0x07, 0x10, 0x07, 0xd0, 0x02, 0xd0, 0x08, 0xc3, 0x70, 0x9b, 0x31,
    0xa6, 0xf7, 0xfb, 0x7d, 0x50, 0x4a, 0xb1, 0xb2, 0xb2, 0x72, 0x01, 0xb8,
    0xa6, 0x29, 0x41, 0x10, 0x5e, 0x4c, 0x4c, 0x4c, 0x3c, 0x4d, 0xa7, 0xd3,
    0x33, 0xf1, 0x78, 0x7c, 0x0a, 0x80, 0xdc, 0xeb, 0xf5, 0x66, 0x4c, 0xd3,
    0x2c, 0x9d, 0x9e, 0x9e, 0xbe, 0x64, 0x8c, 0xad, 0x02, 0xa8, 0xde, 0x70,
    0x00, 0x00, 0x61, 0x18, 0x3e, 0x90, 0x24, 0xa9, 0x54, 0x2e, 0x97, 0xef,
    0x3b, 0x8e, 0x23, 0xf7, 0xfb, 0xfd, 0x3b, 0xb6, 0x6d, 0x0f, 0xc7, 0xb1,
    0x54, 0x2a, 0x15, 0x9b, 0x9c, 0x9c, 0xcc, 0xd4, 0xeb, 0xf5, 0x77, 0x94,
    0x52, 0x0c, 0x21, 0x22, 0x00, 0x30, 0xc6, 0x22, 0x00, 0xf2, 0xa5, 0x52,
    0x49, 0x36, 0x0c, 0x03, 0xae, 0xeb, 0x32, 0x5d, 0xd7, 0x7f, 0x77, 0x3a,
    0x1d, 0x13, 0x40, 0x3c, 0x99, 0x4c, 0x26, 0x32, 0x99, 0x8c, 0xe2, 0xba,
    0x6e, 0xb4, 0x58, 0x2c, 0x3e, 0xd9, 0xdb, 0xdb, 0x7b, 0xcb, 0x18, 0x53,
    0x01, 0xe8, 0x3c, 0x00, 0x10, 0x42, 0x52, 0xe3, 0xe3, 0xe3, 0x71, 0x4d,
    0xd3, 0xd0, 0x6e, 0xb7, 0x39, 0x55, 0x55, 0x9b, 0x86, 0x61, 0x7c, 0x60,
    0x8c, 0x95, 0x19, 0x63, 0xcf, 0x0c, 0xc3, 0x58, 0xdd, 0xdf, 0xdf, 0xff,
    0x69, 0x59, 0x96, 0xaf, 0x69, 0x1a, 0x14, 0x45, 0xb9, 0x47, 0x08, 0x59,
    0x04, 0x80, 0x21, 0xe0, 0xae, 0x2c, 0xcb, 0x11, 0xcb, 0xb2, 0xc2, 0xa3,
    0xa3, 0xa3, 0x3f, 0x94, 0xd2, 0x8f, 0x94, 0xd2, 0x4f, 0xdd, 0x6e, 0x77,
    0x78, 0xde, 0x26, 0xa5, 0xf4, 0xfd, 0xf1, 0xf1, 0xb1, 0xe6, 0x38, 0x0e,
    0x46, 0x47, 0x47, 0x93, 0x84, 0x90, 0xe2, 0x15, 0x20, 0x08, 0x02, 0x1a,
    0x8d, 0x46, 0x4d, 0xcf, 0xf3, 0x7e, 0x98, 0xa6, 0xf9, 0xb9, 0xd7, 0xeb,
    0x7d, 0xb3, 0x6d, 0x1b, 0xe7, 0xe7, 0xe7, 0xd0, 0x34, 0x0d, 0x41, 0x10,
    0x80, 0x10, 0xb2, 0xa9, 0xeb, 0xba, 0xc5, 0x18, 0x03, 0xcf, 0xf3, 0x31,
    0xdf, 0xf7, 0x1f, 0x5f, 0x65, 0xe0, 0xfb, 0xfe, 0x9b, 0x6a, 0xb5, 0x5a,
    0xf8, 0xbf, 0xe3, 0x30, 0x0c, 0x55, 0x00, 0xaf, 0x2c, 0xcb, 0x02, 0x00,
    0x08, 0x82, 0x00, 0xc6, 0x18, 0x7c, 0xdf, 0x07, 0x21, 0xe4, 0x5f, 0x0b,
    0x41, 0x10, 0x14, 0x16, 0x16, 0x16, 0x0a, 0x41, 0x10, 0x00, 0x00, 0x46,
    0x46, 0x46, 0xb0, 0xbb, 0xbb, 0xab, 0x72, 0x1c, 0x57, 0x67, 0x8c, 0x81,
    0x10, 0x82, 0x48, 0x24, 0xb2, 0x9c, 0xcd, 0x66, 0x13, 0x97, 0x6e, 0x5c,
    0xdf, 0xf7, 0x7f, 0x5d, 0x07, 0xc0, 0xf7, 0x7d, 0xd8, 0xb6, 0x8d, 0xb1,
    0xb1, 0x31, 0x6c, 0x6d, 0x6d, 0xa9, 0x00, 0xbe, 0x32, 0xc6, 0x36, 0x06,
    0x83, 0x01, 0x44, 0x51, 0x5c, 0x26, 0x84, 0xbc, 0x9e, 0x9e, 0x9e, 0x56,
    0x5c, 0xd7, 0xc5, 0xd9, 0xd9, 0x59, 0x27, 0x08, 0x82, 0x2f, 0x57, 0x80,
    0xa1, 0x9d, 0x74, 0x3a, 0x0d, 0xd3, 0x34, 0x51, 0xa9, 0x54, 0x0a, 0xcd,
    0x66, 0x33, 0x71, 0x78, 0x78, 0x78, 0x00, 0x00, 0xd9, 0x6c, 0x36, 0x91,
    0xcf, 0xe7, 0x15, 0xd7, 0x75, 0xa3, 0xd1, 0x68, 0x14, 0xad, 0x56, 0xab,
    0x4d, 0x29, 0xdd, 0xbe, 0x01, 0x90, 0x65, 0x19, 0xeb, 0xeb, 0xeb, 0xea,
    0xd2, 0xd2, 0x52, 0x81, 0x52, 0x8a, 0x5c, 0x2e, 0x37, 0x3d, 0x3b, 0x3b,
    0x0b, 0x00, 0xf0, 0x3c, 0x0f, 0xdd, 0x6e, 0x17, 0x92, 0x24, 0xa1, 0x56,
    0xab, 0x7d, 0xf7, 0x3c, 0x6f, 0x55, 0x10, 0x04, 0x1d, 0x00, 0xb8, 0x30,
    0x0c, 0x51, 0xa9, 0x54, 0x0e, 0x2e, 0x73, 0xab, 0x45, 0x22, 0x91, 0xf9,
    0xb9, 0xb9, 0xb9, 0x47, 0x8a, 0xa2, 0x24, 0x05, 0x41, 0x88, 0x11, 0x42,
    0xe0, 0x38, 0x8e, 0xab, 0x69, 0x5a, 0xe7, 0xe4, 0xe4, 0xa4, 0x4d, 0x29,
    0x5d, 0x95, 0x24, 0xa9, 0x2a, 0x8a, 0x22, 0x76, 0x76, 0x76, 0x2e, 0x1c,
    0x0c, 0x06, 0x03, 0x15, 0x40, 0x9d, 0xe7, 0xf9, 0x0d, 0xcf, 0xf3, 0x1e,
    0x36, 0x1a, 0x8d, 0x45, 0x00, 0xf3, 0x00, 0x72, 0x97, 0xe0, 0x16, 0x80,
    0x06, 0xc7, 0x71, 0xdb, 0xc3, 0xcd, 0x43, 0x71, 0xb7, 0x7d, 0xe7, 0xbf,
    0x9b, 0xa0, 0x7b, 0x14, 0xf8, 0x77, 0x91, 0x3e, 0x00, 0x00, 0x00, 0x00,
    0x49, 0x45, 0x4e, 0x44, 0xae, 0x42, 0x60, 0x82
};


/*
 *  Constructs a MembraneTestUI as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 */
MembraneTestUI::MembraneTestUI( QWidget* parent, const char* name, WFlags fl )
    : QWidget( parent, name, fl )
{
    QImage img;
    img.loadFromData( image0_data, sizeof( image0_data ), "PNG" );
    image0 = img;
    img.loadFromData( image1_data, sizeof( image1_data ), "PNG" );
    image1 = img;
    if ( !name )
	setName( "MembraneTestUI" );
    QFont f( font() );
    f.setBold( TRUE );
    setFont( f ); 
    MembraneTestUILayout = new QVBoxLayout( this, 11, 6, "MembraneTestUILayout"); 

    mainTabWidget = new QTabWidget( this, "mainTabWidget" );
    mainTabWidget->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, mainTabWidget->sizePolicy().hasHeightForWidth() ) );
    QFont mainTabWidget_font(  mainTabWidget->font() );
    mainTabWidget->setFont( mainTabWidget_font ); 
    mainTabWidget->setTabPosition( QTabWidget::Top );
    mainTabWidget->setTabShape( QTabWidget::Rounded );

    tab = new QWidget( mainTabWidget, "tab" );
    tabLayout = new QVBoxLayout( tab, 11, 6, "tabLayout"); 

    resistTitleLayout = new QHBoxLayout( 0, 0, 6, "resistTitleLayout"); 

    resistLabel = new QLabel( tab, "resistLabel" );
    resistLabel->setMinimumSize( QSize( 0, 0 ) );
    resistLabel->setMaximumSize( QSize( 32767, 14 ) );
    QFont resistLabel_font(  resistLabel->font() );
    resistLabel_font.setUnderline( TRUE );
    resistLabel->setFont( resistLabel_font ); 
    resistLabel->setAlignment( int( QLabel::AlignCenter ) );
    resistTitleLayout->addWidget( resistLabel );
    spacerT1 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    resistTitleLayout->addItem( spacerT1 );

    buttonLayout1 = new QHBoxLayout( 0, 0, 0, "buttonLayout1"); 

    pauseButton1 = new QPushButton( tab, "pauseButton1" );
    pauseButton1->setMaximumSize( QSize( 25, 25 ) );
    pauseButton1->setPixmap( image0 );
    pauseButton1->setToggleButton( TRUE );
    buttonLayout1->addWidget( pauseButton1 );

    modifyButton1 = new QPushButton( tab, "modifyButton1" );
    modifyButton1->setMaximumSize( QSize( 25, 25 ) );
    modifyButton1->setPixmap( image1 );
    buttonLayout1->addWidget( modifyButton1 );
    resistTitleLayout->addLayout( buttonLayout1 );
    tabLayout->addLayout( resistTitleLayout );

    pulseGroup = new QGroupBox( tab, "pulseGroup" );
    pulseGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, pulseGroup->sizePolicy().hasHeightForWidth() ) );
    pulseGroup->setMinimumSize( QSize( 0, 155 ) );
    pulseGroup->setMaximumSize( QSize( 32767, 155 ) );
    pulseGroup->setFrameShadow( QGroupBox::Raised );
    pulseGroup->setLineWidth( 2 );
    pulseGroup->setMidLineWidth( 0 );
    pulseGroup->setAlignment( int( QGroupBox::AlignCenter ) );
    pulseGroup->setColumnLayout(0, Qt::Vertical );
    pulseGroup->layout()->setSpacing( 6 );
    pulseGroup->layout()->setMargin( 11 );
    pulseGroupLayout = new QVBoxLayout( pulseGroup->layout() );
    pulseGroupLayout->setAlignment( Qt::AlignTop );

    pulseButtonLayout = new QHBoxLayout( 0, 0, 6, "pulseButtonLayout"); 
    spacerP1 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    pulseButtonLayout->addItem( spacerP1 );

    pulseButton = new QPushButton( pulseGroup, "pulseButton" );
    pulseButton->setMinimumSize( QSize( 125, 0 ) );
    pulseButton->setMaximumSize( QSize( 125, 32767 ) );
    QFont pulseButton_font(  pulseButton->font() );
    pulseButton->setFont( pulseButton_font ); 
    pulseButton->setToggleButton( TRUE );
    pulseButtonLayout->addWidget( pulseButton );
    spaceP2 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    pulseButtonLayout->addItem( spaceP2 );
    pulseGroupLayout->addLayout( pulseButtonLayout );

    paramLayout1 = new QHBoxLayout( 0, 0, 6, "paramLayout1"); 
    spacerP7 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    paramLayout1->addItem( spacerP7 );

    holdingGroup = new QButtonGroup( pulseGroup, "holdingGroup" );
    holdingGroup->setAlignment( int( QButtonGroup::AlignCenter ) );
    holdingGroup->setColumnLayout(0, Qt::Vertical );
    holdingGroup->layout()->setSpacing( 0 );
    holdingGroup->layout()->setMargin( 11 );
    holdingGroupLayout = new QGridLayout( holdingGroup->layout() );
    holdingGroupLayout->setAlignment( Qt::AlignTop );

    holdingOption1Edit = new QLineEdit( holdingGroup, "holdingOption1Edit" );
    holdingOption1Edit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, holdingOption1Edit->sizePolicy().hasHeightForWidth() ) );
    holdingOption1Edit->setMinimumSize( QSize( 77, 24 ) );
    holdingOption1Edit->setMaximumSize( QSize( 77, 32767 ) );
    holdingOption1Edit->setAlignment( int( QLineEdit::AlignHCenter ) );

    holdingGroupLayout->addWidget( holdingOption1Edit, 0, 1 );

    holdingOption2Edit = new QLineEdit( holdingGroup, "holdingOption2Edit" );
    holdingOption2Edit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, holdingOption2Edit->sizePolicy().hasHeightForWidth() ) );
    holdingOption2Edit->setMinimumSize( QSize( 77, 24 ) );
    holdingOption2Edit->setMaximumSize( QSize( 77, 32767 ) );
    holdingOption2Edit->setAlignment( int( QLineEdit::AlignHCenter ) );

    holdingGroupLayout->addWidget( holdingOption2Edit, 1, 1 );

    holdingOption3Radio = new QRadioButton( holdingGroup, "holdingOption3Radio" );
    holdingGroup->insert( holdingOption3Radio, 3 );

    holdingGroupLayout->addWidget( holdingOption3Radio, 2, 0 );

    holdingOption3Edit = new QLineEdit( holdingGroup, "holdingOption3Edit" );
    holdingOption3Edit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, holdingOption3Edit->sizePolicy().hasHeightForWidth() ) );
    holdingOption3Edit->setMinimumSize( QSize( 77, 24 ) );
    holdingOption3Edit->setMaximumSize( QSize( 77, 32767 ) );
    holdingOption3Edit->setAlignment( int( QLineEdit::AlignHCenter ) );

    holdingGroupLayout->addWidget( holdingOption3Edit, 2, 1 );

    holdingOption2Radio = new QRadioButton( holdingGroup, "holdingOption2Radio" );
    holdingGroup->insert( holdingOption2Radio, 2 );

    holdingGroupLayout->addWidget( holdingOption2Radio, 1, 0 );

    holdingOption1Radio = new QRadioButton( holdingGroup, "holdingOption1Radio" );
    holdingOption1Radio->setMinimumSize( QSize( 0, 0 ) );
    holdingOption1Radio->setChecked( TRUE );
    holdingGroup->insert( holdingOption1Radio, 1 );

    holdingGroupLayout->addWidget( holdingOption1Radio, 0, 0 );
    paramLayout1->addWidget( holdingGroup );
    spacerP3 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    paramLayout1->addItem( spacerP3 );

    paramLayout2 = new QVBoxLayout( 0, 0, 6, "paramLayout2"); 

    pulseStepSizeLabel = new QLabel( pulseGroup, "pulseStepSizeLabel" );
    pulseStepSizeLabel->setMinimumSize( QSize( 77, 0 ) );
    pulseStepSizeLabel->setMaximumSize( QSize( 32767, 14 ) );
    QFont pulseStepSizeLabel_font(  pulseStepSizeLabel->font() );
    pulseStepSizeLabel->setFont( pulseStepSizeLabel_font ); 
    pulseStepSizeLabel->setFrameShape( QLabel::NoFrame );
    pulseStepSizeLabel->setAlignment( int( QLabel::AlignCenter ) );
    paramLayout2->addWidget( pulseStepSizeLabel );

    pulseSizeEdit = new QLineEdit( pulseGroup, "pulseSizeEdit" );
    pulseSizeEdit->setMinimumSize( QSize( 77, 24 ) );
    pulseSizeEdit->setMaximumSize( QSize( 50, 32767 ) );
    pulseSizeEdit->setAlignment( int( QLineEdit::AlignHCenter ) );
    paramLayout2->addWidget( pulseSizeEdit );

    pulseStepWidthLabel = new QLabel( pulseGroup, "pulseStepWidthLabel" );
    pulseStepWidthLabel->setMinimumSize( QSize( 77, 0 ) );
    QFont pulseStepWidthLabel_font(  pulseStepWidthLabel->font() );
    pulseStepWidthLabel->setFont( pulseStepWidthLabel_font ); 
    pulseStepWidthLabel->setFrameShape( QLabel::NoFrame );
    pulseStepWidthLabel->setAlignment( int( QLabel::AlignCenter ) );
    paramLayout2->addWidget( pulseStepWidthLabel );

    pulseWidthEdit = new QLineEdit( pulseGroup, "pulseWidthEdit" );
    pulseWidthEdit->setMinimumSize( QSize( 77, 24 ) );
    pulseWidthEdit->setMaximumSize( QSize( 50, 32767 ) );
    pulseWidthEdit->setAlignment( int( QLineEdit::AlignHCenter ) );
    paramLayout2->addWidget( pulseWidthEdit );
    paramLayout1->addLayout( paramLayout2 );
    spacerP12 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    paramLayout1->addItem( spacerP12 );
    pulseGroupLayout->addLayout( paramLayout1 );
    tabLayout->addWidget( pulseGroup );
    spacer1 = new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Preferred );
    tabLayout->addItem( spacer1 );

    zapGroup = new QGroupBox( tab, "zapGroup" );
    zapGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)1, 0, 0, zapGroup->sizePolicy().hasHeightForWidth() ) );
    zapGroup->setFrameShadow( QGroupBox::Raised );
    zapGroup->setLineWidth( 2 );
    zapGroup->setAlignment( int( QGroupBox::AlignCenter ) );
    zapGroup->setColumnLayout(0, Qt::Vertical );
    zapGroup->layout()->setSpacing( 6 );
    zapGroup->layout()->setMargin( 11 );
    zapGroupLayout = new QHBoxLayout( zapGroup->layout() );
    zapGroupLayout->setAlignment( Qt::AlignTop );

    zapButton = new QPushButton( zapGroup, "zapButton" );
    zapButton->setEnabled( FALSE );
    zapButton->setMinimumSize( QSize( 0, 45 ) );
    QFont zapButton_font(  zapButton->font() );
    zapButton->setFont( zapButton_font ); 
    zapButton->setFlat( FALSE );
    zapGroupLayout->addWidget( zapButton );

    zapSizeLayout = new QVBoxLayout( 0, 0, 6, "zapSizeLayout"); 

    zapSizeLabel = new QLabel( zapGroup, "zapSizeLabel" );
    zapSizeLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, zapSizeLabel->sizePolicy().hasHeightForWidth() ) );
    zapSizeLabel->setMinimumSize( QSize( 77, 0 ) );
    QFont zapSizeLabel_font(  zapSizeLabel->font() );
    zapSizeLabel->setFont( zapSizeLabel_font ); 
    zapSizeLabel->setFrameShape( QLabel::NoFrame );
    zapSizeLayout->addWidget( zapSizeLabel );

    zapSizeEdit = new QLineEdit( zapGroup, "zapSizeEdit" );
    zapSizeEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, zapSizeEdit->sizePolicy().hasHeightForWidth() ) );
    zapSizeEdit->setMinimumSize( QSize( 77, 0 ) );
    QFont zapSizeEdit_font(  zapSizeEdit->font() );
    zapSizeEdit->setFont( zapSizeEdit_font ); 
    zapSizeEdit->setFrameShape( QLineEdit::LineEditPanel );
    zapSizeEdit->setAlignment( int( QLineEdit::AlignHCenter ) );
    zapSizeLayout->addWidget( zapSizeEdit );
    zapGroupLayout->addLayout( zapSizeLayout );

    zapWidthLayout = new QVBoxLayout( 0, 0, 6, "zapWidthLayout"); 

    zapWidthLabel = new QLabel( zapGroup, "zapWidthLabel" );
    zapWidthLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, zapWidthLabel->sizePolicy().hasHeightForWidth() ) );
    zapWidthLabel->setMinimumSize( QSize( 77, 0 ) );
    QFont zapWidthLabel_font(  zapWidthLabel->font() );
    zapWidthLabel->setFont( zapWidthLabel_font ); 
    zapWidthLabel->setFrameShape( QLabel::NoFrame );
    zapWidthLayout->addWidget( zapWidthLabel );

    zapWidthEdit = new QLineEdit( zapGroup, "zapWidthEdit" );
    zapWidthEdit->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)1, (QSizePolicy::SizeType)0, 0, 0, zapWidthEdit->sizePolicy().hasHeightForWidth() ) );
    zapWidthEdit->setMinimumSize( QSize( 77, 0 ) );
    QFont zapWidthEdit_font(  zapWidthEdit->font() );
    zapWidthEdit->setFont( zapWidthEdit_font ); 
    zapWidthEdit->setFrameShape( QLineEdit::LineEditPanel );
    zapWidthEdit->setAlignment( int( QLineEdit::AlignHCenter ) );
    zapWidthLayout->addWidget( zapWidthEdit );
    zapGroupLayout->addLayout( zapWidthLayout );
    tabLayout->addWidget( zapGroup );
    mainTabWidget->insertTab( tab, QString::fromLatin1("") );

    tab_2 = new QWidget( mainTabWidget, "tab_2" );
    tabLayout_2 = new QVBoxLayout( tab_2, 11, 6, "tabLayout_2"); 

    memPropLayout = new QHBoxLayout( 0, 0, 6, "memPropLayout"); 

    memPropLabel = new QLabel( tab_2, "memPropLabel" );
    memPropLabel->setMaximumSize( QSize( 32767, 14 ) );
    QFont memPropLabel_font(  memPropLabel->font() );
    memPropLabel_font.setUnderline( TRUE );
    memPropLabel->setFont( memPropLabel_font ); 
    memPropLabel->setAlignment( int( QLabel::AlignCenter ) );
    memPropLayout->addWidget( memPropLabel );
    spacerT2 = new QSpacerItem( 0, 16, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    memPropLayout->addItem( spacerT2 );

    buttonLayout2 = new QHBoxLayout( 0, 0, 0, "buttonLayout2"); 

    pauseButton2 = new QPushButton( tab_2, "pauseButton2" );
    pauseButton2->setMaximumSize( QSize( 25, 25 ) );
    pauseButton2->setPixmap( image0 );
    pauseButton2->setToggleButton( TRUE );
    buttonLayout2->addWidget( pauseButton2 );

    modifyButton2 = new QPushButton( tab_2, "modifyButton2" );
    modifyButton2->setMaximumSize( QSize( 25, 25 ) );
    modifyButton2->setPixmap( image1 );
    buttonLayout2->addWidget( modifyButton2 );
    memPropLayout->addLayout( buttonLayout2 );
    tabLayout_2->addLayout( memPropLayout );

    memTestParamLayout = new QGridLayout( 0, 1, 1, 0, 6, "memTestParamLayout"); 
    spacerM10 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM10, 3, 0 );
    spacerM3 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM3, 2, 0 );

    numStepAvgLayout = new QHBoxLayout( 0, 0, 6, "numStepAvgLayout"); 

    numStepAvgLabel = new QLabel( tab_2, "numStepAvgLabel" );
    numStepAvgLabel->setMinimumSize( QSize( 130, 0 ) );
    numStepAvgLabel->setFrameShape( QLabel::Box );
    numStepAvgLabel->setFrameShadow( QLabel::Plain );
    numStepAvgLayout->addWidget( numStepAvgLabel );

    numStepAvgSpinBox = new QSpinBox( tab_2, "numStepAvgSpinBox" );
    numStepAvgSpinBox->setMaxValue( 100 );
    numStepAvgSpinBox->setMinValue( 1 );
    numStepAvgSpinBox->setLineStep( 1 );
    numStepAvgSpinBox->setValue( 5 );
    numStepAvgLayout->addWidget( numStepAvgSpinBox );

    memTestParamLayout->addLayout( numStepAvgLayout, 2, 1 );

    memRateLayout = new QHBoxLayout( 0, 0, 6, "memRateLayout"); 

    memRateLabel = new QLabel( tab_2, "memRateLabel" );
    memRateLabel->setMinimumSize( QSize( 130, 0 ) );
    memRateLabel->setFrameShape( QLabel::Box );
    memRateLayout->addWidget( memRateLabel );

    memRateSpinBox = new QSpinBox( tab_2, "memRateSpinBox" );
    memRateSpinBox->setMaxValue( 100 );
    memRateSpinBox->setMinValue( 1 );
    memRateSpinBox->setLineStep( 1 );
    memRateSpinBox->setValue( 1 );
    memRateLayout->addWidget( memRateSpinBox );

    memTestParamLayout->addLayout( memRateLayout, 1, 1 );
    spacerM1 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM1, 0, 0 );

    memModeComboBox = new QComboBox( FALSE, tab_2, "memModeComboBox" );
    memModeComboBox->setMinimumSize( QSize( 105, 0 ) );

    memTestParamLayout->addWidget( memModeComboBox, 3, 1 );
    spacerM4 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM4, 1, 2 );
    spacerM2 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM2, 1, 0 );
    spacerM9 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM9, 3, 2 );

    acquireMemPropButton = new QPushButton( tab_2, "acquireMemPropButton" );
    acquireMemPropButton->setEnabled( FALSE );
    QFont acquireMemPropButton_font(  acquireMemPropButton->font() );
    acquireMemPropButton->setFont( acquireMemPropButton_font ); 
    acquireMemPropButton->setToggleButton( TRUE );

    memTestParamLayout->addWidget( acquireMemPropButton, 0, 1 );
    spacerM5 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM5, 0, 2 );
    spacerM6 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memTestParamLayout->addItem( spacerM6, 2, 2 );
    tabLayout_2->addLayout( memTestParamLayout );
    spacer2 = new QSpacerItem( 20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding );
    tabLayout_2->addItem( spacer2 );

    memPropOutputFrame = new QFrame( tab_2, "memPropOutputFrame" );
    memPropOutputFrame->setFrameShape( QFrame::StyledPanel );
    memPropOutputFrame->setFrameShadow( QFrame::Sunken );
    memPropOutputFrameLayout = new QHBoxLayout( memPropOutputFrame, 2, 0, "memPropOutputFrameLayout"); 
    spacerM7 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memPropOutputFrameLayout->addItem( spacerM7 );

    outputLayout = new QGridLayout( 0, 1, 1, 0, 0, "outputLayout"); 

    accessResistOutput = new QLabel( memPropOutputFrame, "accessResistOutput" );
    QFont accessResistOutput_font(  accessResistOutput->font() );
    accessResistOutput_font.setPointSize( 10 );
    accessResistOutput->setFont( accessResistOutput_font ); 
    accessResistOutput->setAlignment( int( QLabel::AlignCenter ) );

    outputLayout->addWidget( accessResistOutput, 2, 1 );

    membraneResistLabel = new QLabel( memPropOutputFrame, "membraneResistLabel" );
    QFont membraneResistLabel_font(  membraneResistLabel->font() );
    membraneResistLabel_font.setPointSize( 10 );
    membraneResistLabel->setFont( membraneResistLabel_font ); 
    membraneResistLabel->setAlignment( int( QLabel::WordBreak | QLabel::AlignVCenter | QLabel::AlignRight ) );
    membraneResistLabel->setIndent( -1 );

    outputLayout->addWidget( membraneResistLabel, 3, 0 );

    membraneCapLabel = new QLabel( memPropOutputFrame, "membraneCapLabel" );
    QFont membraneCapLabel_font(  membraneCapLabel->font() );
    membraneCapLabel_font.setPointSize( 10 );
    membraneCapLabel->setFont( membraneCapLabel_font ); 
    membraneCapLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    outputLayout->addWidget( membraneCapLabel, 0, 0 );

    membraneCapOutput = new QLabel( memPropOutputFrame, "membraneCapOutput" );
    QFont membraneCapOutput_font(  membraneCapOutput->font() );
    membraneCapOutput_font.setPointSize( 10 );
    membraneCapOutput->setFont( membraneCapOutput_font ); 
    membraneCapOutput->setAlignment( int( QLabel::AlignCenter ) );

    outputLayout->addMultiCellWidget( membraneCapOutput, 0, 1, 1, 1 );

    membraneResistOutput = new QLabel( memPropOutputFrame, "membraneResistOutput" );
    QFont membraneResistOutput_font(  membraneResistOutput->font() );
    membraneResistOutput_font.setPointSize( 10 );
    membraneResistOutput->setFont( membraneResistOutput_font ); 
    membraneResistOutput->setAlignment( int( QLabel::AlignCenter ) );

    outputLayout->addWidget( membraneResistOutput, 3, 1 );

    accessResistLabel = new QLabel( memPropOutputFrame, "accessResistLabel" );
    QFont accessResistLabel_font(  accessResistLabel->font() );
    accessResistLabel_font.setPointSize( 10 );
    accessResistLabel->setFont( accessResistLabel_font ); 
    accessResistLabel->setAlignment( int( QLabel::AlignVCenter | QLabel::AlignRight ) );

    outputLayout->addMultiCellWidget( accessResistLabel, 1, 2, 0, 0 );
    memPropOutputFrameLayout->addLayout( outputLayout );
    spacerM8 = new QSpacerItem( 0, 16, QSizePolicy::Preferred, QSizePolicy::Minimum );
    memPropOutputFrameLayout->addItem( spacerM8 );
    tabLayout_2->addWidget( memPropOutputFrame );
    mainTabWidget->insertTab( tab_2, QString::fromLatin1("") );
    MembraneTestUILayout->addWidget( mainTabWidget );

    pulseOutputLayout = new QHBoxLayout( 0, 0, 0, "pulseOutputLayout"); 
    spacerPO1 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    pulseOutputLayout->addItem( spacerPO1 );

    pulseOutputLabel = new QLabel( this, "pulseOutputLabel" );
    pulseOutputLabel->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)0, 0, 0, pulseOutputLabel->sizePolicy().hasHeightForWidth() ) );
    pulseOutputLabel->setMinimumSize( QSize( 200, 0 ) );
    pulseOutputLabel->setPaletteForegroundColor( QColor( 0, 0, 255 ) );
    QFont pulseOutputLabel_font(  pulseOutputLabel->font() );
    pulseOutputLabel_font.setPointSize( 24 );
    pulseOutputLabel->setFont( pulseOutputLabel_font ); 
    pulseOutputLabel->setFrameShape( QLabel::NoFrame );
    pulseOutputLabel->setScaledContents( TRUE );
    pulseOutputLabel->setAlignment( int( QLabel::AlignCenter ) );
    pulseOutputLayout->addWidget( pulseOutputLabel );
    spacerPO2 = new QSpacerItem( 0, 16, QSizePolicy::Expanding, QSizePolicy::Minimum );
    pulseOutputLayout->addItem( spacerPO2 );
    MembraneTestUILayout->addLayout( pulseOutputLayout );
    languageChange();
    resize( QSize(322, 389).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
MembraneTestUI::~MembraneTestUI()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void MembraneTestUI::languageChange()
{
    setCaption( tr( "Membrane Test" ) );
    QToolTip::add( mainTabWidget, QString::null );
    resistLabel->setText( tr( "Resistance Measurement" ) );
    pauseButton1->setText( QString::null );
    QToolTip::add( pauseButton1, tr( "Pause module" ) );
    modifyButton1->setText( QString::null );
    QToolTip::add( modifyButton1, tr( "Refresh all parameters" ) );
    pulseGroup->setTitle( QString::null );
    QToolTip::add( pulseGroup, QString::null );
    QWhatsThis::add( pulseGroup, QString::null );
    pulseButton->setText( tr( "Pulse" ) );
    QToolTip::add( pulseButton, tr( "Toggle to start voltage pulse" ) );
    holdingGroup->setTitle( tr( "Holding (mV)" ) );
    holdingOption1Edit->setText( QString::null );
    QToolTip::add( holdingOption1Edit, tr( "Holding potential (mV) *Hit enter to update*" ) );
    QToolTip::add( holdingOption2Edit, tr( "Holding potential (mV) *Hit enter to update*" ) );
    holdingOption3Radio->setText( QString::null );
    QToolTip::add( holdingOption3Radio, tr( "Toggle to choose desired holding potenial" ) );
    QToolTip::add( holdingOption3Edit, tr( "Holding potential (mV) *Hit enter to update*" ) );
    holdingOption2Radio->setText( QString::null );
    QToolTip::add( holdingOption2Radio, tr( "Toggle to choose desired holding potenial" ) );
    holdingOption1Radio->setText( QString::null );
    QToolTip::add( holdingOption1Radio, tr( "Toggle to choose desired holding potenial" ) );
    pulseStepSizeLabel->setText( tr( "Size (mV)" ) );
    QToolTip::add( pulseSizeEdit, tr( "Step size of voltage pulse (mV) *Hit enter to update*" ) );
    pulseStepWidthLabel->setText( tr( "Width (ms)" ) );
    QToolTip::add( pulseWidthEdit, tr( "Width of voltage pulse (ms) *Hit enter to update*" ) );
    zapGroup->setTitle( QString::null );
    QToolTip::add( zapGroup, QString::null );
    zapButton->setText( tr( "Zap" ) );
    QToolTip::add( zapButton, tr( "Click to deliver a single voltage pulse. Pulse must be on." ) );
    zapSizeLabel->setText( tr( "Size (mV)" ) );
    zapSizeEdit->setText( QString::null );
    QToolTip::add( zapSizeEdit, tr( "Size of voltage zap (mV) *Hit enter to update*" ) );
    zapWidthLabel->setText( tr( "Width (ms)" ) );
    zapWidthEdit->setText( QString::null );
    QToolTip::add( zapWidthEdit, tr( "Width of zap pulse (ms) *Hit enter to update*" ) );
    mainTabWidget->changeTab( tab, trUtf8( "\x52\x65\x73\x69\x73\x74\x61\x6e\x63\x65\x20\x28\xce\xa9\x29" ) );
    memPropLabel->setText( tr( "Membrane Properties" ) );
    pauseButton2->setText( QString::null );
    QToolTip::add( pauseButton2, tr( "Pause module" ) );
    modifyButton2->setText( QString::null );
    QToolTip::add( modifyButton2, tr( "Update all parameters" ) );
    numStepAvgLabel->setText( tr( "Steps to Average" ) );
    QToolTip::add( numStepAvgLabel, tr( "Number of voltage steps that are averaged for calculations." ) );
    QToolTip::add( numStepAvgSpinBox, tr( "Number of voltage steps that are averaged for calculations." ) );
    memRateLabel->setText( tr( "Update Rate (Hz)" ) );
    QToolTip::add( memRateLabel, tr( "Update rate of membrane test when in continuous mode" ) );
    QToolTip::add( memRateSpinBox, tr( "Update rate of membrane test when in continuous mode" ) );
    memModeComboBox->clear();
    memModeComboBox->insertItem( tr( "Single" ) );
    memModeComboBox->insertItem( tr( "Continuous" ) );
    QToolTip::add( memModeComboBox, tr( "Single: Test is run once. Continuous: Test is run according to update rate." ) );
    acquireMemPropButton->setText( tr( "Acquire" ) );
    QToolTip::add( acquireMemPropButton, tr( "Toggle to acquire membrane properties. Pulse must be on." ) );
    accessResistOutput->setText( trUtf8( "\x31\x2e\x30\x30\x20\x4d\xce\xa9" ) );
    QToolTip::add( accessResistOutput, tr( "Access Resistance" ) );
    membraneResistLabel->setText( tr( "<b><font size=\"+1\">R</font><font size=\"-2\">m</font></b> :" ) );
    QToolTip::add( membraneResistLabel, tr( "Membrane resistance" ) );
    membraneCapLabel->setText( tr( "<b><font size=\"+1\">C</font><font size=\"-2\">m</font></b> :" ) );
    QToolTip::add( membraneCapLabel, tr( "Membrane capacitance" ) );
    membraneCapOutput->setText( tr( "32.4 pF" ) );
    QToolTip::add( membraneCapOutput, tr( "Membrane capacitance" ) );
    membraneResistOutput->setText( trUtf8( "\x31\x2e\x30\x30\x20\x4d\xce\xa9" ) );
    QToolTip::add( membraneResistOutput, tr( "Membrane resistance" ) );
    accessResistLabel->setText( tr( "<b><font size=\"+1\">R</font><font size=\"-3\">a</font></b> :" ) );
    QToolTip::add( accessResistLabel, tr( "Access resistance" ) );
    mainTabWidget->changeTab( tab_2, tr( "Membrane Properties" ) );
    pulseOutputLabel->setText( trUtf8( "\x31\x2e\x37\x35\x33\x31\x20\x47\xce\xa9" ) );
    QToolTip::add( pulseOutputLabel, trUtf8( "\x45\x6c\x65\x63\x74\x72\x6f\x64\x65\x20\x72\x65\x73\x69\x73\x74\x61\x6e\x63\x65\x20"
    "\x6d\x65\x61\x73\x75\x72\x65\x6d\x65\x6e\x74\x20\x28\xce\xa9\x29" ) );
}

