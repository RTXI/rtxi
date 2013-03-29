/****************************************************************************
** Form implementation generated from reading ui file 'CP_protocol_editorUI.ui'
**
** Created: Wed Sep 26 15:49:24 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#include "CP_protocol_editorUI.h"

#include <qvariant.h>
#include <qpushbutton.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qtable.h>
#include <qspinbox.h>
#include <qheader.h>
#include <qlistview.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>

/*
 *  Constructs a ProtocolEditorUI as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
ProtocolEditorUI::ProtocolEditorUI( QWidget* parent, const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "ProtocolEditorUI" );
    setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)5, (QSizePolicy::SizeType)5, 0, 0, sizePolicy().hasHeightForWidth() ) );
    ProtocolEditorUILayout = new QVBoxLayout( this, 11, 6, "ProtocolEditorUILayout"); 

    layout1 = new QHBoxLayout( 0, 0, 6, "layout1"); 

    saveProtocolButton = new QPushButton( this, "saveProtocolButton" );
    QFont saveProtocolButton_font(  saveProtocolButton->font() );
    saveProtocolButton_font.setBold( TRUE );
    saveProtocolButton->setFont( saveProtocolButton_font ); 
    saveProtocolButton->setFocusPolicy( QPushButton::TabFocus );
    layout1->addWidget( saveProtocolButton );

    loadProtocolButton = new QPushButton( this, "loadProtocolButton" );
    QFont loadProtocolButton_font(  loadProtocolButton->font() );
    loadProtocolButton_font.setBold( TRUE );
    loadProtocolButton->setFont( loadProtocolButton_font ); 
    layout1->addWidget( loadProtocolButton );
    spacer1 = new QSpacerItem( 330, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout1->addItem( spacer1 );

    exportProtocolButton = new QPushButton( this, "exportProtocolButton" );
    QFont exportProtocolButton_font(  exportProtocolButton->font() );
    exportProtocolButton_font.setBold( TRUE );
    exportProtocolButton->setFont( exportProtocolButton_font ); 
    layout1->addWidget( exportProtocolButton );

    previewProtocolButton = new QPushButton( this, "previewProtocolButton" );
    QFont previewProtocolButton_font(  previewProtocolButton->font() );
    previewProtocolButton_font.setBold( TRUE );
    previewProtocolButton->setFont( previewProtocolButton_font ); 
    layout1->addWidget( previewProtocolButton );

    clearProtocolButton = new QPushButton( this, "clearProtocolButton" );
    QFont clearProtocolButton_font(  clearProtocolButton->font() );
    clearProtocolButton_font.setBold( TRUE );
    clearProtocolButton->setFont( clearProtocolButton_font ); 
    layout1->addWidget( clearProtocolButton );
    ProtocolEditorUILayout->addLayout( layout1 );

    layout2 = new QHBoxLayout( 0, 0, 6, "layout2"); 

    layout3 = new QVBoxLayout( 0, 0, 6, "layout3"); 

    protocolDescriptionBox = new QGroupBox( this, "protocolDescriptionBox" );
    protocolDescriptionBox->setMinimumSize( QSize( 520, 295 ) );
    protocolDescriptionBox->setMaximumSize( QSize( 32767, 295 ) );
    QFont protocolDescriptionBox_font(  protocolDescriptionBox->font() );
    protocolDescriptionBox_font.setBold( TRUE );
    protocolDescriptionBox->setFont( protocolDescriptionBox_font ); 
    protocolDescriptionBox->setAlignment( int( QGroupBox::AlignTop | QGroupBox::AlignHCenter ) );
    protocolDescriptionBox->setColumnLayout(0, Qt::Vertical );
    protocolDescriptionBox->layout()->setSpacing( 6 );
    protocolDescriptionBox->layout()->setMargin( 11 );
    protocolDescriptionBoxLayout = new QVBoxLayout( protocolDescriptionBox->layout() );
    protocolDescriptionBoxLayout->setAlignment( Qt::AlignTop );

    segmentStepLabel = new QLabel( protocolDescriptionBox, "segmentStepLabel" );
    segmentStepLabel->setAlignment( int( QLabel::AlignCenter ) );
    protocolDescriptionBoxLayout->addWidget( segmentStepLabel );

    protocolTable = new QTable( protocolDescriptionBox, "protocolTable" );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, tr( "Amplifier Mode" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, tr( "Step Type" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, tr( "Step Duration (ms)" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, trUtf8( "\xce\x94\x20\x53\x74\x65\x70\x20\x44\x75\x72\x61\x74\x69\x6f\x6e\x20\x28\x6d\x73\x29"
    "" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, tr( "Holding Level 1 (mV/pA)" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, trUtf8( "\xce\x94\x20\x48\x6f\x6c\x64\x69\x6e\x67\x20\x4c\x65\x76\x65\x6c\x20\x31\x20\x28\x6d"
    "\x56\x2f\x70\x41\x29" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, tr( "Holding Level 2 (mV/pA)" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, trUtf8( "\xce\x94\x20\x48\x6f\x6c\x64\x69\x6e\x67\x20\x4c\x65\x76\x65\x6c\x20\x32\x20\x28\x6d"
    "\x56\x2f\x70\x41\x29" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, tr( "Pulse Width (ms)" ) );
    protocolTable->setNumRows( protocolTable->numRows() + 1 );
    protocolTable->verticalHeader()->setLabel( protocolTable->numRows() - 1, tr( "Pulse Train Rate" ) );
    protocolTable->setVScrollBarMode( QTable::AlwaysOff );
    protocolTable->setHScrollBarMode( QTable::AlwaysOn );
    protocolTable->setNumRows( 10 );
    protocolTable->setNumCols( 0 );
    protocolTable->setSelectionMode( QTable::Single );
    protocolDescriptionBoxLayout->addWidget( protocolTable );
    layout3->addWidget( protocolDescriptionBox );

    layout4 = new QHBoxLayout( 0, 0, 6, "layout4"); 

    addStepButton = new QPushButton( this, "addStepButton" );
    addStepButton->setMaximumSize( QSize( 32767, 27 ) );
    QFont addStepButton_font(  addStepButton->font() );
    addStepButton_font.setBold( TRUE );
    addStepButton->setFont( addStepButton_font ); 
    layout4->addWidget( addStepButton );

    insertStepButton = new QPushButton( this, "insertStepButton" );
    insertStepButton->setMaximumSize( QSize( 32767, 27 ) );
    QFont insertStepButton_font(  insertStepButton->font() );
    insertStepButton_font.setBold( TRUE );
    insertStepButton->setFont( insertStepButton_font ); 
    layout4->addWidget( insertStepButton );

    deleteStepButton = new QPushButton( this, "deleteStepButton" );
    deleteStepButton->setMaximumSize( QSize( 32767, 27 ) );
    QFont deleteStepButton_font(  deleteStepButton->font() );
    deleteStepButton_font.setBold( TRUE );
    deleteStepButton->setFont( deleteStepButton_font ); 
    layout4->addWidget( deleteStepButton );
    spacer2 = new QSpacerItem( 220, 27, QSizePolicy::Expanding, QSizePolicy::Minimum );
    layout4->addItem( spacer2 );
    layout3->addLayout( layout4 );
    layout2->addLayout( layout3 );

    layout5 = new QVBoxLayout( 0, 0, 6, "layout5"); 

    segmentSummaryGroup = new QGroupBox( this, "segmentSummaryGroup" );
    segmentSummaryGroup->setSizePolicy( QSizePolicy( (QSizePolicy::SizeType)0, (QSizePolicy::SizeType)5, 0, 0, segmentSummaryGroup->sizePolicy().hasHeightForWidth() ) );
    segmentSummaryGroup->setMinimumSize( QSize( 242, 295 ) );
    segmentSummaryGroup->setMaximumSize( QSize( 242, 295 ) );
    QFont segmentSummaryGroup_font(  segmentSummaryGroup->font() );
    segmentSummaryGroup_font.setBold( TRUE );
    segmentSummaryGroup->setFont( segmentSummaryGroup_font ); 
    segmentSummaryGroup->setAlignment( int( QGroupBox::AlignHCenter ) );
    segmentSummaryGroup->setColumnLayout(0, Qt::Vertical );
    segmentSummaryGroup->layout()->setSpacing( 6 );
    segmentSummaryGroup->layout()->setMargin( 11 );
    segmentSummaryGroupLayout = new QVBoxLayout( segmentSummaryGroup->layout() );
    segmentSummaryGroupLayout->setAlignment( Qt::AlignTop );

    segmentSweepGroup = new QGroupBox( segmentSummaryGroup, "segmentSweepGroup" );
    segmentSweepGroup->setColumnLayout(0, Qt::Vertical );
    segmentSweepGroup->layout()->setSpacing( 6 );
    segmentSweepGroup->layout()->setMargin( 11 );
    segmentSweepGroupLayout = new QHBoxLayout( segmentSweepGroup->layout() );
    segmentSweepGroupLayout->setAlignment( Qt::AlignTop );

    segmentSweepLabel = new QLabel( segmentSweepGroup, "segmentSweepLabel" );
    QFont segmentSweepLabel_font(  segmentSweepLabel->font() );
    segmentSweepLabel->setFont( segmentSweepLabel_font ); 
    segmentSweepGroupLayout->addWidget( segmentSweepLabel );

    segmentSweepSpinBox = new QSpinBox( segmentSweepGroup, "segmentSweepSpinBox" );
    segmentSweepGroupLayout->addWidget( segmentSweepSpinBox );
    segmentSummaryGroupLayout->addWidget( segmentSweepGroup );

    segmentListView = new QListView( segmentSummaryGroup, "segmentListView" );
    segmentListView->addColumn( tr( "Segment #" ) );
    segmentListView->header()->setResizeEnabled( FALSE, segmentListView->header()->count() - 1 );
    segmentListView->setVScrollBarMode( QListView::AlwaysOn );
    segmentListView->setHScrollBarMode( QListView::AlwaysOff );
    segmentListView->setResizeMode( QListView::LastColumn );
    segmentListView->setTreeStepSize( 20 );
    segmentSummaryGroupLayout->addWidget( segmentListView );
    layout5->addWidget( segmentSummaryGroup );

    layout6 = new QHBoxLayout( 0, 0, 6, "layout6"); 

    addSegmentButton = new QPushButton( this, "addSegmentButton" );
    addSegmentButton->setMaximumSize( QSize( 32767, 27 ) );
    QFont addSegmentButton_font(  addSegmentButton->font() );
    addSegmentButton_font.setBold( TRUE );
    addSegmentButton->setFont( addSegmentButton_font ); 
    layout6->addWidget( addSegmentButton );

    deleteSegmentButton = new QPushButton( this, "deleteSegmentButton" );
    deleteSegmentButton->setMaximumSize( QSize( 32767, 27 ) );
    QFont deleteSegmentButton_font(  deleteSegmentButton->font() );
    deleteSegmentButton_font.setBold( TRUE );
    deleteSegmentButton->setFont( deleteSegmentButton_font ); 
    layout6->addWidget( deleteSegmentButton );
    layout5->addLayout( layout6 );
    layout2->addLayout( layout5 );
    ProtocolEditorUILayout->addLayout( layout2 );
    languageChange();
    resize( QSize(797, 431).expandedTo(minimumSizeHint()) );
    clearWState( WState_Polished );
}

/*
 *  Destroys the object and frees any allocated resources
 */
ProtocolEditorUI::~ProtocolEditorUI()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void ProtocolEditorUI::languageChange()
{
    setCaption( tr( "Protocol Editor" ) );
    saveProtocolButton->setText( tr( "Save" ) );
    QToolTip::add( saveProtocolButton, tr( "Save protocol to .csp file for loading into clamp suite" ) );
    loadProtocolButton->setText( tr( "Load" ) );
    QToolTip::add( loadProtocolButton, tr( "Load a previously saved .csp file for editting" ) );
    exportProtocolButton->setText( tr( "Export" ) );
    previewProtocolButton->setText( tr( "Preview" ) );
    clearProtocolButton->setText( tr( "Clear" ) );
    QToolTip::add( clearProtocolButton, tr( "Clear protocol" ) );
    protocolDescriptionBox->setTitle( tr( "Protocol Description" ) );
    segmentStepLabel->setText( QString::null );
    protocolTable->verticalHeader()->setLabel( 0, tr( "Amplifier Mode" ) );
    protocolTable->verticalHeader()->setLabel( 1, tr( "Step Type" ) );
    protocolTable->verticalHeader()->setLabel( 2, tr( "Step Duration (ms)" ) );
    protocolTable->verticalHeader()->setLabel( 3, trUtf8( "\xce\x94\x20\x53\x74\x65\x70\x20\x44\x75\x72\x61\x74\x69\x6f\x6e\x20\x28\x6d\x73\x29"
    "" ) );
    protocolTable->verticalHeader()->setLabel( 4, tr( "Holding Level 1 (mV/pA)" ) );
    protocolTable->verticalHeader()->setLabel( 5, trUtf8( "\xce\x94\x20\x48\x6f\x6c\x64\x69\x6e\x67\x20\x4c\x65\x76\x65\x6c\x20\x31\x20\x28\x6d"
    "\x56\x2f\x70\x41\x29" ) );
    protocolTable->verticalHeader()->setLabel( 6, tr( "Holding Level 2 (mV/pA)" ) );
    protocolTable->verticalHeader()->setLabel( 7, trUtf8( "\xce\x94\x20\x48\x6f\x6c\x64\x69\x6e\x67\x20\x4c\x65\x76\x65\x6c\x20\x32\x20\x28\x6d"
    "\x56\x2f\x70\x41\x29" ) );
    protocolTable->verticalHeader()->setLabel( 8, tr( "Pulse Width (ms)" ) );
    protocolTable->verticalHeader()->setLabel( 9, tr( "Pulse Train Rate" ) );
    addStepButton->setText( tr( "Add Step" ) );
    QToolTip::add( addStepButton, tr( "Add a step to the end of the segment" ) );
    insertStepButton->setText( tr( "Insert Step" ) );
    QToolTip::add( insertStepButton, tr( "Insert a step after the currently selected step" ) );
    deleteStepButton->setText( tr( "Delete Step" ) );
    QToolTip::add( deleteStepButton, tr( "Delete a step" ) );
    segmentSummaryGroup->setTitle( tr( "Segment Summary" ) );
    segmentSweepGroup->setTitle( QString::null );
    segmentSweepLabel->setText( tr( "Segment Sweeps" ) );
    QToolTip::add( segmentSweepLabel, tr( "Number of times a segment will be swept through" ) );
    QToolTip::add( segmentSweepSpinBox, tr( "Number of times a segment will be swept through" ) );
    segmentListView->header()->setLabel( 0, tr( "Segment #" ) );
    QToolTip::add( segmentListView, tr( "Click a segment to open it in protocol description" ) );
    addSegmentButton->setText( tr( "Add Segment" ) );
    QToolTip::add( addSegmentButton, tr( "Add a segment after the current highlighted segment" ) );
    deleteSegmentButton->setText( tr( "Delete Segment" ) );
    QToolTip::add( deleteSegmentButton, tr( "Delete a segment" ) );
}

void ProtocolEditorUI::protocolTable_currentChanged(int,int)
{
    qWarning( "ProtocolEditorUI::protocolTable_currentChanged(int,int): Not implemented yet" );
}

void ProtocolEditorUI::protocolTable_verticalSliderReleased()
{
    qWarning( "ProtocolEditorUI::protocolTable_verticalSliderReleased(): Not implemented yet" );
}

