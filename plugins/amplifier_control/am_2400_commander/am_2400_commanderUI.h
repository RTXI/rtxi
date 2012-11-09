/****************************************************************************
** Form interface generated from reading ui file 'am_amp_commanderUI.ui'
**
** Created: Mon Jul 23 16:20:37 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef AM_AMP_COMMANDERUI_H
#define AM_AMP_COMMANDERUI_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QSpinBox;
class QLabel;
class QFrame;
class QButtonGroup;
class QRadioButton;

class AM_Amp_CommanderUI : public QWidget
{
    Q_OBJECT

public:
    AM_Amp_CommanderUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~AM_Amp_CommanderUI();

    QSpinBox* inputChannelSpinBox;
    QLabel* outputLabel;
    QLabel* inputLabel;
    QSpinBox* outputChannelSpinBox;
    QFrame* line1;
    QFrame* frame3;
    QLabel* ampModeLabel;
    QButtonGroup* ampModeButtonGroup;
    QRadioButton* vclampButton;
    QRadioButton* izeroButton;
    QRadioButton* iclampButton;
    QRadioButton* vcompButton;
    QRadioButton* vtestButton;
    QRadioButton* iresistButton;
    QRadioButton* ifollowButton;

protected:
    QVBoxLayout* AM_Amp_CommanderUILayout;
    QGridLayout* layout1;
    QHBoxLayout* frame3Layout;
    QSpacerItem* spacer1;
    QSpacerItem* spacer2;
    QVBoxLayout* layout2;
    QVBoxLayout* ampModeButtonGroupLayout;

protected slots:
    virtual void languageChange();

};

#endif // AM_AMP_COMMANDERUI_H
