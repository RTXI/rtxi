/****************************************************************************
** Form interface generated from reading ui file 'membrane_testUI.ui'
**
** Created: Wed Sep 26 15:30:08 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MEMBRANETESTUI_H
#define MEMBRANETESTUI_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QTabWidget;
class QLabel;
class QPushButton;
class QGroupBox;
class QButtonGroup;
class QLineEdit;
class QRadioButton;
class QSpinBox;
class QComboBox;
class QFrame;

class MembraneTestUI : public QWidget
{
    Q_OBJECT

public:
    MembraneTestUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~MembraneTestUI();

    QTabWidget* mainTabWidget;
    QWidget* tab;
    QLabel* resistLabel;
    QPushButton* pauseButton1;
    QPushButton* modifyButton1;
    QGroupBox* pulseGroup;
    QPushButton* pulseButton;
    QButtonGroup* holdingGroup;
    QLineEdit* holdingOption1Edit;
    QLineEdit* holdingOption2Edit;
    QRadioButton* holdingOption3Radio;
    QLineEdit* holdingOption3Edit;
    QRadioButton* holdingOption2Radio;
    QRadioButton* holdingOption1Radio;
    QLabel* pulseStepSizeLabel;
    QLineEdit* pulseSizeEdit;
    QLabel* pulseStepWidthLabel;
    QLineEdit* pulseWidthEdit;
    QGroupBox* zapGroup;
    QPushButton* zapButton;
    QLabel* zapSizeLabel;
    QLineEdit* zapSizeEdit;
    QLabel* zapWidthLabel;
    QLineEdit* zapWidthEdit;
    QWidget* tab_2;
    QLabel* memPropLabel;
    QPushButton* pauseButton2;
    QPushButton* modifyButton2;
    QLabel* numStepAvgLabel;
    QSpinBox* numStepAvgSpinBox;
    QLabel* memRateLabel;
    QSpinBox* memRateSpinBox;
    QComboBox* memModeComboBox;
    QPushButton* acquireMemPropButton;
    QFrame* memPropOutputFrame;
    QLabel* accessResistOutput;
    QLabel* membraneResistLabel;
    QLabel* membraneCapLabel;
    QLabel* membraneCapOutput;
    QLabel* membraneResistOutput;
    QLabel* accessResistLabel;
    QLabel* pulseOutputLabel;

protected:
    QVBoxLayout* MembraneTestUILayout;
    QVBoxLayout* tabLayout;
    QSpacerItem* spacer1;
    QHBoxLayout* resistTitleLayout;
    QSpacerItem* spacerT1;
    QHBoxLayout* buttonLayout1;
    QVBoxLayout* pulseGroupLayout;
    QHBoxLayout* pulseButtonLayout;
    QSpacerItem* spacerP1;
    QSpacerItem* spaceP2;
    QHBoxLayout* paramLayout1;
    QSpacerItem* spacerP7;
    QSpacerItem* spacerP3;
    QSpacerItem* spacerP12;
    QGridLayout* holdingGroupLayout;
    QVBoxLayout* paramLayout2;
    QHBoxLayout* zapGroupLayout;
    QVBoxLayout* zapSizeLayout;
    QVBoxLayout* zapWidthLayout;
    QVBoxLayout* tabLayout_2;
    QSpacerItem* spacer2;
    QHBoxLayout* memPropLayout;
    QSpacerItem* spacerT2;
    QHBoxLayout* buttonLayout2;
    QGridLayout* memTestParamLayout;
    QSpacerItem* spacerM10;
    QSpacerItem* spacerM3;
    QSpacerItem* spacerM1;
    QSpacerItem* spacerM4;
    QSpacerItem* spacerM2;
    QSpacerItem* spacerM9;
    QSpacerItem* spacerM5;
    QSpacerItem* spacerM6;
    QHBoxLayout* numStepAvgLayout;
    QHBoxLayout* memRateLayout;
    QHBoxLayout* memPropOutputFrameLayout;
    QSpacerItem* spacerM7;
    QSpacerItem* spacerM8;
    QGridLayout* outputLayout;
    QHBoxLayout* pulseOutputLayout;
    QSpacerItem* spacerPO1;
    QSpacerItem* spacerPO2;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // MEMBRANETESTUI_H
