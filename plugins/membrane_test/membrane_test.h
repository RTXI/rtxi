/*
 * Copyright (C) 2012 Weill Medical College of Cornell University
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*** INTRO
 * Membrane Test
 * 
 * membrane_test.h, v1.0
 *
 * Author: Francis A. Ortega (2012)
 *
 *** NOTES
 *
 * Electrode Resistance Measurement and Clamp Utilities
 *
 * v1.0 - Initial Version, replaces original electrode test
 *          electrode membrane resistance test
 *          membrane properties test
 * v1.1 - Renamed from clamp_utilities to membrane_test
 *
 ***/

#ifndef MEMBRANE_TEST_H
#define MEMBRANE_TEST_H

#include "membrane_testUI.h"
#include <vector>

#include <qtimer.h>

#include <io.h>
#include <event.h>
#include <plugin.h>
#include <qobject.h>
#include <qwidget.h>
#include <rt.h>
#include <settings.h>

namespace MembraneTest {

    class Panel;

    class Plugin : public QObject, public ::Plugin::Object {

        Q_OBJECT

        friend class Panel;

    public:
        static Plugin *getInstance( void );

    public slots:
        void createMembraneTestPanel( void ); // Create panel GUI through menu item

    protected:
        void doDeferred( const Settings::Object::State & );
        void doLoad( const Settings::Object::State & );
        void doSave( Settings::Object::State & ) const;

    private:
        Plugin( void );
        ~Plugin( void );
        Plugin( const Plugin & ) { };
        Plugin &operator=( const Plugin & ) { return *getInstance(); };

        void removeMembraneTestPanel( Panel * ); // Remove panel GUI

        static Plugin *instance;

        int menuID;
        std::list< Panel * > panelList;

    }; // class Plugin

    class Panel : public QWidget, public RT::Thread, public IO::Block, public Event::Handler {

        Q_OBJECT
        
    public:
        Panel( QWidget * );
        ~Panel( void );

        void execute( void );
        void receiveEvent( const ::Event::Object * );

    public slots:
        void updateRDisplay( void ); // Update resistance measurement reading
        void updateMemTestDisplay( void ); // Update membrane properties display
        void updateHoldingGroup( int ); // Update user chosen holding potential
        void updateHoldingOption( void ); // Update holding potential values
        void updatePulse( void ); // Update pulse parameters
        void updateZap( void ); // Update zap parameters
        void updateMemTest( void ); // Update membrane properties test parameters
        void updateAll( void ); // Update all parameters, modify/refresh button is clicked
        void togglePulse( bool ); // Toggle pulse on or off
        void toggleZap( void ); // Toggle a zap (single pulse)
        void toggleMemTest( bool ); // Toggle membrane property test on or off
        void memTestCalculate( void ); // Performs the membrane properties calculation
        void pause1( void ); // Set RT thread and display inactive or active, parameters may be changed
        void pause2( void ); // Set RT thread and display inactive or active, parameters may be changed

    protected:
        void doDeferred( const Settings::Object::State & );
        void doLoad( const Settings::Object::State & );
        void doSave( Settings::Object::State & ) const;

    private:
        // Events are used to post async GUI update events
        friend class UpdateHoldingGroupEvent;
        friend class UpdateHoldingOptionEvent;
        friend class UpdatePulseEvent;
        friend class UpdateZapEvent;
        friend class UpdateMemTestEvent;

        enum MemTestMode_t { SINGLE = 0, CONTINUOUS = 1 };

        class UpdateHoldingGroupEvent : public RT::Event {

        public:
            UpdateHoldingGroupEvent( Panel *, int );
            ~UpdateHoldingGroupEvent( void ) { };

            int callback( void );

        private:            
            Panel *panel;
            int holdingOptionValue;

        }; // class UpdateHoldingGroupEvent

        class UpdateHoldingOptionEvent : public RT::Event {

        public:
            UpdateHoldingOptionEvent( Panel *, double, double, double );
            ~UpdateHoldingOptionEvent( void ) { };

            int callback( void );

        private:            
            Panel *panel;
            double holdingOption1, holdingOption2, holdingOption3; // Pulse holding potential options

        }; // class UpdateHoldingOptionEvent

        class UpdatePulseEvent : public RT::Event {

        public:
            UpdatePulseEvent( Panel *, double, double );
            ~UpdatePulseEvent( void ) { };

            int callback( void );

        private:            
            Panel *panel;
            double pulseSize, pulseWidth; // Pulse parameters

        }; // class UpdatePulseEvent

        class UpdateZapEvent : public RT::Event {

        public:
            UpdateZapEvent( Panel *, double, double );
            ~UpdateZapEvent( void ) { };

            int callback( void );

        private:            
            Panel *panel;
            double zapSize, zapWidth; // Zap parameters

        }; // class UpdateZapEvent

        class UpdateMemTestEvent : public RT::Event {

        public:
            UpdateMemTestEvent( Panel *, int, int, int );
            ~UpdateMemTestEvent( void ) { };

            int callback( void );

        private:            
            Panel *panel;
            int updateRate, numStepsAvg;
            MemTestMode_t memTestMode;

        }; // class UpdateMemTestEvent

        MembraneTestUI *ui;
        QChar omega;
        
        // Electrode resistance measurement parameters
        bool zapOn;
        int holdingSelection;
        double holdingOptionValue;
        double holdingOption1, holdingOption2, holdingOption3; // Pulse holding potential options
        double pulseSize, pulseWidth; // Pulse parameters
        double zapSize, zapWidth; // Zap parameters

        // Membrane test parameters
        int updateRate; // Rate calculation is performed and display updated
        int numStepsAvg, stepsSaved;
        MemTestMode_t memTestMode;
        bool memTestOn;
        bool collectMemTestData;
        bool memTestDone;
        double Ra, Rm, Cm; // Membrane properties
        int data_size;
        QTimer *memTestTimer;

        // Calculation parameters
        size_t idx, cnt; // Pulse
        size_t zidx, zcnt; // Zap
        double I1, I2, dI; // Resistance calculation
        size_t midx; // Membrane properties calculation
        std::vector<double> currentData, data; // Vectors holding data for membrane properties calculation

    signals:
        
    }; // class Panel

}; // namespace MembraneTest

#endif // MEMBRANE_TEST_H
