/****************************************************************************
** Form interface generated from reading ui file 'CP_plot_windowUI.ui'
**
** Created: Wed Sep 26 15:47:57 2012
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/

#ifndef PLOTWINDOWUI_H
#define PLOTWINDOWUI_H

#include <qvariant.h>
#include <qpixmap.h>
#include <qwidget.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QSpacerItem;
class QFrame;
class QLabel;
class QComboBox;
class QSpinBox;
class QPushButton;
class QCheckBox;

class PlotWindowUI : public QWidget
{
    Q_OBJECT

public:
    PlotWindowUI( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );
    ~PlotWindowUI();

    QFrame* frame;
    QLabel* currentScaleLabel;
    QComboBox* currentScaleEdit;
    QSpinBox* currentY2Edit;
    QComboBox* timeScaleEdit;
    QSpinBox* timeX2Edit;
    QSpinBox* currentY1Edit;
    QLabel* timeScaleLabel;
    QSpinBox* timeX1Edit;
    QPushButton* setAxesButton;
    QCheckBox* overlaySweepsCheckBox;
    QCheckBox* plotAfterCheckBox;
    QLabel* textLabel1;
    QComboBox* colorByComboBox;
    QPushButton* clearButton;

protected:
    QHBoxLayout* frameLayout;
    QSpacerItem* spacer;
    QGridLayout* layout1;
    QVBoxLayout* layout2;
    QVBoxLayout* layout3;

protected slots:
    virtual void languageChange();

private:
    QPixmap image0;
    QPixmap image1;

};

#endif // PLOTWINDOWUI_H
