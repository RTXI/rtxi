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

#include <QtWidgets>

#include "module_utils.h"

/*
 * Class RTXIModule. All the functions here, apart from the constructor, are
 * essentially 'get' and 'set' methods.
 */
RTXIModule::RTXIModule(QListWidget *parent) : QListWidgetItem(parent)
{
    readme_url = QUrl("");
    clone_url = QUrl("");
    location = QUrl("");
    name = QString("");
    readme = QString("");

    installed = false;
}

void RTXIModule::setLocation(QString str)
{
    location = QUrl(str);
    return;
}

void RTXIModule::setReadmeUrl(QString str)
{
    readme_url = QUrl(str);
    return;
}

void RTXIModule::setCloneUrl(QString str)
{
    clone_url = QUrl(str);
    return;
}

void RTXIModule::setName(QString str)
{
    name = str;
    return;
}

void RTXIModule::setReadme(QString str)
{
    readme = str;
    return;
}

QUrl RTXIModule::getLocation(void)
{
    return location;
}

QUrl RTXIModule::getReadmeUrl(void)
{
    return readme_url;
}

QUrl RTXIModule::getCloneUrl(void)
{
    return clone_url;
}

QString RTXIModule::getName(void)
{
    return name;
}

QString RTXIModule::getReadme(void)
{
    return readme;
}
