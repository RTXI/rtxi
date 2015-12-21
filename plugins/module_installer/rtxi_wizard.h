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

#include <QtNetwork>
#include <default_gui_model.h>

#include "module_utils.h"

class RTXIWizard : public DefaultGUIModel
{
	Q_OBJECT

	public:
		RTXIWizard(void);
		~RTXIWizard(void);
		void initParameters(void);
		void customizeGUI(void);

		void installFromString(std::string);

	private slots:
		void cloneModule(void);
		void getRepos(void);
		void getReadme(void);
		void parseRepos(void);
		void parseReadme(void);

		void updateButton(void);

	private:

		enum button_mode_t {DOWNLOAD, UPDATE } button_mode;

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
