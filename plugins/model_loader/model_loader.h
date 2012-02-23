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

#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <plugin.h>
#include <qobject.h>

class ModelLoader : public QObject, public Plugin::Object
{

    Q_OBJECT

public:

    ModelLoader(void);
    virtual ~ModelLoader(void);

public slots:

    void load(void);
    void load_recent(int);

    //void load_setting(int);

private:

    int menuID;
    void updateRecentModules(QString, int);

};


#endif /* MODEL_LOADER_H */
