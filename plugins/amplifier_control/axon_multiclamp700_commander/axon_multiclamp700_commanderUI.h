/****************************************************************************
** Form interface generated from reading ui file 'axon_multiclamp700_commanderUI.ui'
**
** Created: Tue Oct 23 16:14:25 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef MULTICLAMP700_UI_H
#define MULTICLAMP700_UI_H

#include <qvariant.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QLabel;
class QSpinBox;
class QFrame;
class QGroupBox;
class QComboBox;
class QButtonGroup;
class QRadioButton;

class Multiclamp700_UI : public QWidget
{
    Q_OBJECT

public:
    Multiclamp700_UI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~Multiclamp700_UI();

    QLabel* inputLabel;
    QSpinBox* inputChannelSpinBox;
    QLabel* outputLabel;
    QSpinBox* outputChannelSpinBox;
    QFrame* line;
    QGroupBox* vcGroup;
    QLabel* vcCommandLabel;
    QLabel* vcGainLabel;
    QComboBox* vcecsComboBox;
    QComboBox* vcigComboBox;
    QGroupBox* icGroup;
    QLabel* icGainLabel;
    QComboBox* icigComboBox;
    QLabel* icCommandLabel;
    QComboBox* icecsComboBox;
    QFrame* ampModeFrame;
    QLabel* ampModeLabel;
    QButtonGroup* buttonGroup;
    QRadioButton* vclampButton;
    QRadioButton* iclampButton;
    QRadioButton* izeroButton;

protected:
    QVBoxLayout* Multiclamp700_UILayout;
    QHBoxLayout* layout;
    QGridLayout* vcGroupLayout;
    QGridLayout* icGroupLayout;
    QVBoxLayout* ampModeFrameLayout;

protected slots:
    virtual void languageChange();

};

#endif // MULTICLAMP700_UI_H
