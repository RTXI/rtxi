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

#include <default_gui_model.h>
#include <main_window.h>
#include <qgridview.h>
#include <qhbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qtimer.h>
#include <qtooltip.h>
#include <qvalidator.h>
#include <qvbox.h>
#include <qwhatsthis.h>
#include <iostream>
namespace {

class SyncEvent: public RT::Event {

public:

	int callback(void) {
		return 0;
	}
	;

}; // class SyncEvent

} // namespace

DefaultGUILineEdit::DefaultGUILineEdit(QWidget *parent) :
	QLineEdit(parent) {
QObject::connect(this,SIGNAL(textChanged(const QString &)),this,SLOT(redden(void)));
}

DefaultGUILineEdit::~DefaultGUILineEdit(void) {
}

void DefaultGUILineEdit::blacken(void) {
	setPaletteForegroundColor(Qt::black);
	setEdited(false);
}

void DefaultGUILineEdit::redden(void) {
	if (edited())
		setPaletteForegroundColor(Qt::red);
}

DefaultGUIModel::DefaultGUIModel(std::string name,
		DefaultGUIModel::variable_t *var, size_t size) :
	QWidget(MainWindow::getInstance()->centralWidget()), Workspace::Instance(
			name, var, size), myname(name) {
	setCaption(QString::number(getID()) + " " + name);

	QTimer *timer = new QTimer(this);
	timer->start(1000);
	QObject::connect(timer,SIGNAL(timeout(void)),this,SLOT(refresh(void)));

}

DefaultGUIModel::~DefaultGUIModel(void) {
    // Ensure that the realtime thread isn't in the middle of executing DefaultGUIModel::execute()    
    setActive( false );
	SyncEvent event;
	RT::System::getInstance()->postEvent(&event);
    
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		if (i->second.type & PARAMETER)
			delete i->second.str_value;
}

void DefaultGUIModel::createGUI(DefaultGUIModel::variable_t *var, int size) {
	QBoxLayout *layout = new QVBoxLayout(this);

	QScrollView *sv = new QScrollView(this);
	sv->setResizePolicy(QScrollView::AutoOneFit);
	layout->addWidget(sv);

	QWidget *viewport = new QWidget(sv->viewport());
	sv->addChild(viewport);
	QGridLayout *scrollLayout = new QGridLayout(viewport, 1, 2);

	size_t nstate = 0, nparam = 0, nevent = 0, ncomment = 0;
	for (size_t i = 0; i < size; i++) {
		if (var[i].flags & (PARAMETER | STATE | EVENT | COMMENT)) {
			param_t param;

			param.label = new QLabel(var[i].name, viewport);
			scrollLayout->addWidget(param.label, parameter.size(), 0);
			param.edit = new DefaultGUILineEdit(viewport);
			scrollLayout->addWidget(param.edit, parameter.size(), 1);

			QToolTip::add(param.label, var[i].description);
			QToolTip::add(param.edit, var[i].description);

			if (var[i].flags & PARAMETER) {
				if (var[i].flags & DOUBLE) {
					param.edit->setValidator(new QDoubleValidator(param.edit));
					param.type = PARAMETER | DOUBLE;
				} else if (var[i].flags & UINTEGER) {
					QIntValidator *validator = new QIntValidator(param.edit);
					param.edit->setValidator(validator);
					validator->setBottom(0);
					param.type = PARAMETER | UINTEGER;
				} else if (var[i].flags & INTEGER) {
					param.edit->setValidator(new QIntValidator(param.edit));
					param.type = PARAMETER | INTEGER;
				} else
					param.type = PARAMETER;
				param.index = nparam++;
				param.str_value = new QString;
			} else if (var[i].flags & STATE) {
				param.edit->setReadOnly(true);
				param.edit->setPaletteForegroundColor(Qt::darkGray);
				param.type = STATE;
				param.index = nstate++;
			} else if (var[i].flags & EVENT) {
				param.edit->setReadOnly(true);
				param.type = EVENT;
				param.index = nevent++;
			} else if (var[i].flags & COMMENT) {
				param.type = COMMENT;
				param.index = ncomment++;
			}

			parameter[var[i].name] = param;
		}
	}

	QHBox *hbox1 = new QHBox(this);
	pauseButton = new QPushButton("Pause", hbox1);
	pauseButton->setToggleButton(true);
	QObject::connect(pauseButton,SIGNAL(toggled(bool)),this,SLOT(pause(bool)));
	QPushButton *modifyButton = new QPushButton("Modify", hbox1);
	QObject::connect(modifyButton,SIGNAL(clicked(void)),this,SLOT(modify(void)));
	QPushButton *unloadButton = new QPushButton("Unload", hbox1);
	QObject::connect(unloadButton,SIGNAL(clicked(void)),this,SLOT(exit(void)));
	layout->addWidget(hbox1);

	show();
}

void DefaultGUIModel::update(DefaultGUIModel::update_flags_t) {
}

void DefaultGUIModel::exit(void) {    
    // Ensure that the realtime thread isn't in the middle of executing DefaultGUIModel::execute()    
    setActive( false );
	SyncEvent event;
	RT::System::getInstance()->postEvent(&event);
    
	update(EXIT);
	Plugin::Manager::getInstance()->unload(this);
}

void DefaultGUIModel::refresh(void) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i) {
		if (i->second.type & (STATE | EVENT)) {
			i->second.edit->setText(QString::number(getValue(i->second.type,
					i->second.index)));
			i->second.edit->setPaletteForegroundColor(Qt::darkGray);
		} else if ((i->second.type & PARAMETER) && !i->second.edit->edited()
				&& i->second.edit->text() != *i->second.str_value) {
			i->second.edit->setText(*i->second.str_value);
		} else if ((i->second.type & COMMENT) && !i->second.edit->edited()
				&& i->second.edit->text() != getValueString(COMMENT,
						i->second.index)) {
			i->second.edit->setText(getValueString(COMMENT, i->second.index));
		}
	}
	pauseButton->setOn(!getActive());
}

void DefaultGUIModel::modify(void) {
	bool active = getActive();

	setActive(false);

	// Ensure that the realtime thread isn't in the middle of executing DefaultGUIModel::execute()
	SyncEvent event;
	RT::System::getInstance()->postEvent(&event);

	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		if (i->second.type & COMMENT)
			Workspace::Instance::setComment(i->second.index,
					i->second.edit->text().latin1());

	update(MODIFY);
	setActive(active);

	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		i->second.edit->blacken();
}

QString DefaultGUIModel::getComment(const QString &name) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if (n != parameter.end() && (n->second.type & COMMENT))
		return QString(getValueString(COMMENT, n->second.index));
	return "";
}

void DefaultGUIModel::setComment(const QString &name, QString comment) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if (n != parameter.end() && (n->second.type & COMMENT)) {
		n->second.edit->setText(comment);
		Workspace::Instance::setComment(n->second.index, comment.latin1());
	}
}

QString DefaultGUIModel::getParameter(const QString &name) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
		*n->second.str_value = n->second.edit->text();
		setValue(n->second.index, n->second.edit->text().toDouble());
		return n->second.edit->text();
	}
	return "";
}

void DefaultGUIModel::setParameter(const QString &name, double value) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
		n->second.edit->setText(QString::number(value));
		*n->second.str_value = n->second.edit->text();
		setValue(n->second.index, n->second.edit->text().toDouble());
	}
}

void DefaultGUIModel::setParameter(const QString &name, const QString value) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & PARAMETER)) {
		n->second.edit->setText(value);
		*n->second.str_value = n->second.edit->text();
		setValue(n->second.index, n->second.edit->text().toDouble());
	}
}

void DefaultGUIModel::setState(const QString &name, double &ref) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & STATE)) {
		setData(Workspace::STATE, n->second.index, &ref);
		n->second.edit->setText(QString::number(ref));
	}
}

void DefaultGUIModel::setEvent(const QString &name, double &ref) {
	std::map<QString, param_t>::iterator n = parameter.find(name);
	if ((n != parameter.end()) && (n->second.type & EVENT)) {
		setData(Workspace::EVENT, n->second.index, &ref);
		n->second.edit->setText(QString::number(ref));
	}
}

void DefaultGUIModel::pause(bool p) {
	if (pauseButton->isOn() != p)
		pauseButton->setDown(p);

	setActive(!p);
	if (p)
		update(PAUSE);
	else
		update(UNPAUSE);
}

void DefaultGUIModel::doDeferred(const Settings::Object::State &) {
	setCaption(QString::number(getID()) + " " + myname);
}

void DefaultGUIModel::doLoad(const Settings::Object::State &s) {
	for (std::map<QString, param_t>::iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		i->second.edit->setText(s.loadString(i->first));
	if (s.loadInteger("Maximized"))
		showMaximized();
	else if (s.loadInteger("Minimized"))
		showMinimized();
	// this only exists in RTXI versions >1.3
	if (s.loadInteger("W") != NULL) {
		resize(s.loadInteger("W"), s.loadInteger("H"));
		parentWidget()->move(s.loadInteger("X"), s.loadInteger("Y"));
	}

	pauseButton->setOn(s.loadInteger("paused"));
	modify();
}

void DefaultGUIModel::doSave(Settings::Object::State &s) const {
	s.saveInteger("paused", pauseButton->isOn());
	if (isMaximized())
		s.saveInteger("Maximized", 1);
	else if (isMinimized())
		s.saveInteger("Minimized", 1);

	QPoint pos = parentWidget()->pos();
	s.saveInteger("X", pos.x());
	s.saveInteger("Y", pos.y());
	s.saveInteger("W", width());
	s.saveInteger("H", height());

	for (std::map<QString, param_t>::const_iterator i = parameter.begin(); i
			!= parameter.end(); ++i)
		s.saveString(i->first, i->second.edit->text());
}

void DefaultGUIModel::receiveEvent(const Event::Object *event) {
	if (event->getName() == Event::RT_PREPERIOD_EVENT) {
		periodEventPaused = getActive();
		setActive(false);
	} else if (event->getName() == Event::RT_POSTPERIOD_EVENT) {
#ifdef DEBUG
		if(getActive())
		ERROR_MSG("DefaultGUIModel::receiveEvent : model unpaused during a period update\n");
#endif
		update(PERIOD);
		setActive(periodEventPaused);
	}
}
