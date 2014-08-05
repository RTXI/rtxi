/*
	 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

	 This program is free software: you can redistribute it and/or modify
	 it under the terms of the GNU General Public License as published by
	 the Free Software Foundation, either version 3 of the License, or
	 (at your option) any later version.

	 This program is distributed in the hope that it will be useful,
	 but WITHOUT ANY WARRANTY; without even the implied warranty of
	 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	 GNU General Public License for more details.

	 You should have received a copy of the GNU General Public License
	 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

/*
	 This class instantiates a panel with GUI elements
	 for editing settings for a Scope object (see scope class).
	 The GUI elements can access properties for each scope
	 object. Only one Panel object is necessary for 
	 working with multiple Scope objects.
 */

#ifndef PANEL_H
#define PANEL_H

#include <QtGui>

#include <event.h>
#include <fifo.h>
#include <io.h>
#include <mutex.h>
#include <plugin.h>
#include <rt.h>
#include <settings.h>

class SpinBox;
class CheckBox;

class SpinBox : public QSpinBox {
public:
    SpinBox( int min, int max, int step, QWidget *parent ):
        QSpinBox( parent ) {
        setRange( min, max );
        setSingleStep( step );
    }
}; // Spinbox

class CheckBox : public QCheckBox {
public:
    CheckBox( const QString &title, QWidget *parent ):
			QCheckBox( title, parent ) {};

		void setChecked( bool checked ) {
			setCheckState( checked ? Qt::Checked : Qt::Unchecked );
    }

    bool isChecked() const {
        return checkState() == Qt::Checked;
    }
}; // Checkbox

class Panel : public RT::Thread, public virtual Settings::Object, public Event::Handler, public QTabWidget {

		Q_OBJECT

		public:
		Panel(QWidget * = NULL);
		virtual ~Panel(void);
		void execute(void);
		bool setInactiveSync(void);
		void flushFifo(void);
		void adjustDataSize(void);
		void doDeferred(const Settings::Object::State &);
		void doLoad(const Settings::Object::State &);
		void doSave(Settings::Object::State &) const;

		public slots:
		void showProperties(void);
		void timeoutEvent(void);

		protected:
		void mouseDoubleClickEvent(QMouseEvent *);

		private slots:
		void showChannelTab(void);
		void showDisplayTab(void);
		void buildChannelList(void);
		void screenshot(void);

		private:
		QMdiSubWindow *subWindow;

		// Group and layout information
		QGridLayout *layout;
		QGroupBox *scopeGroup;
		QGroupBox *bttnGroup;
		QGroupBox *scopeBttnGroup;
		QGroupBox *triggerBttnGroup;
		QGroupBox *setBttnGroup;

		// Scope element
		Scope *scopeWindow;

		// Properties
		QSpinBox *divsXSpin;
		QSpinBox *divsYSpin;
		QSpinBox *ratesSpin;
		QLineEdit *sizesEdit;
		QButtonGroup *trigsGroup;
		QComboBox *timesList;
		QComboBox *trigsChanList;
		QComboBox *trigsThreshList;
		QSpinBox *refreshsSpin;
		QLineEdit *trigsThreshEdit;
		QCheckBox *trigsHoldingCheck;
		QLineEdit *trigsHoldoffEdit;
		QComboBox *trigsHoldoffList;

		// Lists
		QComboBox *blocksList;
		QComboBox *typesList;
		QComboBox *channelsList;
		QComboBox *colorsList;
		QComboBox *offsetsList;
		QComboBox *scalesList;
		QComboBox *stylesList;
		QComboBox *widthsList;
		QLineEdit *offsetsEdit;

		// Buttons
		QPushButton *pauseButton;
		QPushButton *settingsButton;
		QPushButton *applyButton;
		QPushButton *activateButton;

		QSpinBox *refreshSpin;
		QSpinBox *divXSpin;
		QSpinBox *divYSpin;
		QSpinBox *rateSpin;
		QLineEdit *sizeEdit;

		void updateDownsampleRate(int);
		Fifo fifo;
		std::vector<IO::Block *> blocks;
		size_t counter;
		size_t downsample_rate;
	}; // Panel

#endif // PANEL_H
