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

#ifndef DYNAMO_MODEL_LOADER_H
#define DYNAMO_MODEL_LOADER_H

#include <plugin.h>
#include <qobject.h>
#include <qstring.h>

class DynamoModelLoader : public QObject, public Plugin::Object
{

    Q_OBJECT

public:

    DynamoModelLoader(void);
    virtual ~DynamoModelLoader(void);

    static DynamoModelLoader *getInstance(void);

    void load (char *path);

public slots:

     QString get_model_makefile_path (void) const;
     int set_model_makefile_path (char *s);
     void load_dialog(void);
     void load_recent(int);
     void load_setting(int);

private:

    int menuID;

    QString model_makefile_path;
    static DynamoModelLoader *instance;

};

#endif /* DYNAMO_MODEL_LOADER_H */
