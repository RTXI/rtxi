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

#ifndef RTXI_WIZARD_H
#define RTXI_WIZARD_H

#include <QtNetwork>
#include <main_window.h>
#include <plugin.h>
#include "module_utils.h"

namespace RTXIWizard
{
	class Panel : public QWidget
	{
		Q_OBJECT

		public:
			Panel(QWidget *);
			virtual ~Panel(void);
			void installFromString(std::string);

			private slots:
				void cloneModule(void);
			void getRepos(void);
			void getReadme(void);
			void parseRepos(void);
			void parseReadme(void);

			void updateButton(void);

		private:
			void initParameters(void);
			int printGitError(int);
			enum button_mode_t {DOWNLOAD, UPDATE } button_mode;

			QMdiSubWindow *subWindow;

			QNetworkAccessManager qnam;
			QNetworkReply *reply;
			QProgressDialog *progressDialog;

			QTextEdit *readmeWindow;
			QListWidget *moduleList;
			QListWidget *installedList;

			QPushButton *cloneButton;
			QPushButton *syncButton;
			QList<RTXIModule*> *allModules;
			QList<RTXIModule*> *installedModules;

			std::vector<QString> exclude_list;
	};

	class Plugin : public QObject, public ::Plugin::Object
	{
		Q_OBJECT
			friend class Panel;

		public:
		static Plugin *getInstance(void);

		public slots:
			void showRTXIWizardPanel(void);

		private:
		void removeRTXIWizardPanel(Panel *);
		Plugin(void);
		~Plugin(void);
		Plugin(const Plugin &) {};
		Plugin &operator=(const Plugin &)
		{
			return *getInstance();
		};
		static Plugin *instance;
		Panel *panel;
	}; // class Plugin

};

#endif
