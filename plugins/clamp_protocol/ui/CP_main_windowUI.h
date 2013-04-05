/****************************************************************************
** Form interface generated from reading ui file 'CP_main_windowUI.ui'
**
** Created: Wed Sep 26 15:47:51 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef CP_MAIN_WINDOWUI_H
#define CP_MAIN_WINDOWUI_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QButtonGroup;
class QPushButton;
class QFrame;
class QLineEdit;
class QGroupBox;
class QSpinBox;
class QLabel;
class QCheckBox;

class CP_main_windowUI : public QWidget
{
    Q_OBJECT

public:
    CP_main_windowUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~CP_main_windowUI();

    QButtonGroup* protocolControlButtonGroup;
    QPushButton* pauseButton;
    QPushButton* runProtocolButton;
    QPushButton* modifyButton;
    QPushButton* loadProtocolButton;
    QPushButton* protocolEditorButton;
    QPushButton* plotWindowButton;
    QFrame* protocolControlLine;
    QLineEdit* protocolNameEdit;
    QGroupBox* protocolOptionGroup;
    QSpinBox* trialsEdit;
    QLabel* intervalTimeLabel;
    QSpinBox* intervalTimeEdit;
    QLabel* trialsLabel;
    QLabel* junctionPotentialLabel;
    QCheckBox* dataRecordCheckBox;
    QLineEdit* junctionPotentialEdit;
    QGroupBox* protocolStatesGroup;
    QLabel* trialNumberLabel;
    QLineEdit* trialNumberEdit;
    QLabel* segmentLabel;
    QLineEdit* segmentEdit;
    QLabel* sweepLabel;
    QLineEdit* sweepEdit;
    QLabel* timeLabel;
    QLineEdit* timeEdit;

protected:
    QVBoxLayout* CP_main_windowUILayout;
    QVBoxLayout* protocolControlButtonGroupLayout;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QHBoxLayout* layout1;
    QGridLayout* protocolOptionGroupLayout;
    QSpacerItem* spacer13;
    QSpacerItem* spacer6;
    QSpacerItem* spacer12;
    QSpacerItem* spacer7;
    QSpacerItem* spacer11;
    QSpacerItem* spacer11_2;
    QSpacerItem* spacer7_2;
    QVBoxLayout* protocolStatesGroupLayout;
    QSpacerItem* spacer8;
    QSpacerItem* spacer9;
    QSpacerItem* spacer10;
    QHBoxLayout* layout2;
    QHBoxLayout* layout3;
    QHBoxLayout* layout4;
    QHBoxLayout* layout5;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;
    QPixmap image2;
    QPixmap image3;
    QPixmap image4;
    QPixmap image5;

};

#endif // CP_MAIN_WINDOWUI_H
