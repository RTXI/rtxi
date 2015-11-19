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

#ifndef MODULEUTILS_H
#define MODULEUTILS_H

#include <QtWidgets>

class RTXIModule : QListWidgetItem
{
//	Q_OBJECT

	public:
		RTXIModule(QListWidget *parent =0);

		void setReadmeUrl(QString);
		void setCloneUrl(QString);
		void setName(QString);
		void setReadme(QString);
		void setLocation(QString);
		
/*
		void cloneModule();
		void syncModule();
		void updateModule();
		void installModule();
		void clearRepo();
*/

		QUrl getReadmeUrl(void);
		QUrl getCloneUrl(void);
		QUrl getLocation(void);
		QString getName(void);
		QString getReadme(void);

		bool installed;

	private: 
		QUrl readme_url;
		QUrl clone_url;
//		QUrl screenshot_url;		
		QString name;
		QString readme;
		QUrl location;

	private slots:

};

#endif
