/****************************************************************************
** Form interface generated from reading ui file 'axon_1d_amp_commanderUI.ui'
**
** Created: Thu Aug 30 15:16:27 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef AXON_1D_AMP_COMMANDERUI_H
#define AXON_1D_AMP_COMMANDERUI_H

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

class Axon_1D_Amp_CommanderUI : public QWidget
{
    Q_OBJECT

public:
    Axon_1D_Amp_CommanderUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Axon_1D_Amp_CommanderUI();

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
    QLabel* commandLabel;
    QComboBox* commandComboBox;
    QFrame* frame3;
    QLabel* ampModeLabel;
    QPushButton* autoModeButton;
    QButtonGroup* ampModeButtonGroup;
    QRadioButton* vclampButton;
    QRadioButton* izeroButton;
    QRadioButton* iclampButton;

protected:
    QVBoxLayout* Axon_1D_Amp_CommanderUILayout;
    QHBoxLayout* layout5;
    QHBoxLayout* layout4;
    QVBoxLayout* layout3;
    QVBoxLayout* layout6;
    QHBoxLayout* frame3Layout;
    QVBoxLayout* layout1;
    QVBoxLayout* ampModeButtonGroupLayout;

protected slots:
    virtual void languageChange();

};

#endif // AXON_1D_AMP_COMMANDERUI_H
