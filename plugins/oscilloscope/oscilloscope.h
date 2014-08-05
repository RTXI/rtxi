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
	 Oscilloscope namespace. The namespace works with 
	 both Scope and Panel classes to instantiate an oscilloscope
	 plugin.
 */

#ifndef OSCILLOSCOPE_H
#define OSCILLOSCOPE_H

#include <QtGui>

#include <event.h>
#include <fifo.h>
#include <io.h>
#include <mutex.h>
#include <plugin.h>
#include <rt.h>
#include <settings.h>

namespace Oscilloscope {

	class Panel;
	class Scope;

	struct oscope{
		Panel *panelElement;
		Scope *scopeElement;
	}; // oscope

	class Plugin : public QObject, public ::Plugin::Object, public RT::Thread {
		Q_OBJECT

			friend class Panel;

		public:
		static Plugin *getInstance(void);

		public slots:
			void createOscilloscopePanel(void);

		protected:
		virtual void doDeferred(const Settings::Object::State &);
		virtual void doLoad(const Settings::Object::State &);
		virtual void doSave(Settings::Object::State &) const;

		private:

		Plugin(void);
		~Plugin(void);
		Plugin(const Plugin &) {};
		Plugin &operator=(const Plugin &) { return *getInstance(); };
		static Plugin *instance;
		void removeOscilloscopePanel(Panel *);
		int menuID;

		// List to maintain multiple scopes
		std::list<oscope *> scopeList;
	}; // Plugin
}; // Oscilloscope

#endif // OSCILLOSCOPE_H
