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

#ifndef ELECTRODE_RESISTANCE_MEASUREMENT
#define ELECTRODE_RESISTANCE_MEASUREMENT

#include <io.h>
#include <plugin.h>
#include <qobject.h>
#include <qwidget.h>
#include <rt.h>
#include <settings.h>

class QLabel;
class QLineEdit;
class QPushButton;

namespace ElectrodeResistanceMeasurement {

    class Panel;

    class Plugin : public QObject, public ::Plugin::Object {

        Q_OBJECT

        friend class Panel;

    public:

        static Plugin *getInstance(void);

    public slots:

        void createElectrodeResistanceMeasurementPanel(void);

    protected:

        void doDeferred(const Settings::Object::State &);
        void doLoad(const Settings::Object::State &);
        void doSave(Settings::Object::State &) const;

    private:

        Plugin(void);
        ~Plugin(void);
        Plugin(const Plugin &) {};
        Plugin &operator=(const Plugin &) { return *getInstance(); };

        void removeElectrodeResistanceMeasurementPanel(Panel *);

        static Plugin *instance;

        int menuID;
        std::list<Panel *> panelList;

    }; // class Plugin

    class Panel : public QWidget, public RT::Thread, public IO::Block {

        Q_OBJECT

    public:

        Panel(QWidget *);
        ~Panel(void);

        void execute(void);

    public slots:

        void updateDisplay(void);
        void updateParameters(void);
        void togglePause(void);

    protected:

        void doDeferred(const Settings::Object::State &);
        void doLoad(const Settings::Object::State &);
        void doSave(Settings::Object::State &) const;

    private:

        friend class UpdateParametersEvent;

        class UpdateParametersEvent : public RT::Event {

        public:

            UpdateParametersEvent(Panel *,double,double,double);
            ~UpdateParametersEvent(void);

            int callback(void);

        private:

            Panel *panel;
            double V0;
            double dV;
            double T;

        }; // class UpdateParametersEvent

        double V0;
        double dV;
        double T;
        double I1, I2, dI;
        size_t cnt, idx;

        QLabel *RLabel;
        QLineEdit *V0Edit;
        QLineEdit *dVEdit;
        QLineEdit *TEdit;
        QPushButton *pauseButton;

    }; // class Panel

}; // namespace ElectrodeResistanceMeasurement

#endif // ELECTRODE_RESISTANCE_MEASUREMENT
