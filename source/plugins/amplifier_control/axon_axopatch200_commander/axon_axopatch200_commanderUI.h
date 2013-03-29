/****************************************************************************
** Form interface generated from reading ui file 'axon_200_amp_commanderUI.ui'
**
** Created: Tue Jun 19 16:36:08 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef AXON_200_AMP_COMMANDERUI_H
#define AXON_200_AMP_COMMANDERUI_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QSpinBox;
class QFrame;
class QComboBox;
class QPushButton;
class QButtonGroup;
class QRadioButton;

class Axon_200_Amp_CommanderUI : public QWidget
{
    Q_OBJECT

public:
    Axon_200_Amp_CommanderUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Axon_200_Amp_CommanderUI();

    QLabel* inputLabel;
    QSpinBox* inputChannelSpinBox;
    QLabel* outputLabel;
    QSpinBox* outputChannelSpinBox;
    QFrame* line1;
    QLabel* gainLabel;
    QComboBox* gainComboBox;
    QFrame* line1_2;
    QLabel* configLabel;
    QComboBox* configComboBox;
    QFrame* frame3;
    QLabel* ampModeLabel;
    QPushButton* autoModeButton;
    QButtonGroup* ampModeButtonGroup;
    QRadioButton* vclampButton;
    QRadioButton* iclampButton;

protected:
    QVBoxLayout* Axon_200_Amp_CommanderUILayout;
    QHBoxLayout* layout5;
    QHBoxLayout* layout4;
    QVBoxLayout* layout3;
    QHBoxLayout* frame3Layout;
    QVBoxLayout* layout1;
    QVBoxLayout* ampModeButtonGroupLayout;

protected slots:
    virtual void languageChange();

};

#endif // AXON_200_AMP_COMMANDERUI_H
