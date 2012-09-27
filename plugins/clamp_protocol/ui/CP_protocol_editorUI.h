/****************************************************************************
** Form interface generated from reading ui file 'CP_protocol_editorUI.ui'
**
** Created: Wed Sep 26 15:49:24 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef PROTOCOLEDITORUI_H
#define PROTOCOLEDITORUI_H

#include <qvariant.h>
#include <qdialog.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QPushButton;
class QGroupBox;
class QLabel;
class QTable;
class QSpinBox;
class QListView;
class QListViewItem;

class ProtocolEditorUI : public QDialog
{
    Q_OBJECT

public:
    ProtocolEditorUI( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ProtocolEditorUI();

    QPushButton* saveProtocolButton;
    QPushButton* loadProtocolButton;
    QPushButton* exportProtocolButton;
    QPushButton* previewProtocolButton;
    QPushButton* clearProtocolButton;
    QGroupBox* protocolDescriptionBox;
    QLabel* segmentStepLabel;
    QTable* protocolTable;
    QPushButton* addStepButton;
    QPushButton* insertStepButton;
    QPushButton* deleteStepButton;
    QGroupBox* segmentSummaryGroup;
    QGroupBox* segmentSweepGroup;
    QLabel* segmentSweepLabel;
    QSpinBox* segmentSweepSpinBox;
    QListView* segmentListView;
    QPushButton* addSegmentButton;
    QPushButton* deleteSegmentButton;

public slots:
    virtual void protocolTable_currentChanged( int, int );
    virtual void protocolTable_verticalSliderReleased();

protected:
    QVBoxLayout* ProtocolEditorUILayout;
    QHBoxLayout* layout1;
    QSpacerItem* spacer1;
    QHBoxLayout* layout2;
    QVBoxLayout* layout3;
    QVBoxLayout* protocolDescriptionBoxLayout;
    QHBoxLayout* layout4;
    QSpacerItem* spacer2;
    QVBoxLayout* layout5;
    QVBoxLayout* segmentSummaryGroupLayout;
    QHBoxLayout* segmentSweepGroupLayout;
    QHBoxLayout* layout6;

protected slots:
    virtual void languageChange();

};

#endif // PROTOCOLEDITORUI_H
