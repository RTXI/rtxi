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

#include <cstdlib>
#include <debug.h>
#include <errno.h>
#include <event.h>
#include <io.h>
#include <plugin.h>
#include <qdom.h>
#include <qfile.h>
#include <rt.h>
#include <settings.h>
#include <qsettings.h>
#include <sstream>
#include <main_window.h>

namespace {

    struct defer_t {
        Settings::Object *object;
        Settings::Object::State s;
    }; // struct defer_t

}; // namespace

Settings::Object::Object(void) : id(0) {
    Manager::getInstance()->insertObject(this);
}

Settings::Object::~Object(void) {
    Manager::getInstance()->removeObject(this);
}

Settings::Object::State::State(void) : id(0) {};

Settings::Object::State::State(Settings::Object::ID n) : id(n) {};

Settings::Object::State::~State(void) {};

double Settings::Object::State::loadDouble(const std::string &name) const {
    double value = 0.0;
    std::istringstream s;
    std::map<std::string,std::string>::const_iterator n;

    n = paramMap.find(name);
    if(n != paramMap.end()) {
        s.str(n->second);
        s >> value;
    }

    return value;
}

int Settings::Object::State::loadInteger(const std::string &name) const {
    int value = 0;
    std::istringstream s;
    std::map<std::string,std::string>::const_iterator n;

    n = paramMap.find(name);
    if(n != paramMap.end()) {
        s.str(n->second);
        s >> value;
    }

    return value;
}

std::string Settings::Object::State::loadString(const std::string &name) const {
    std::string value = "";
    std::map<std::string,std::string>::const_iterator n;

    n = paramMap.find(name);
    if(n != paramMap.end())
        value = n->second;

    return value;
}

void Settings::Object::State::saveDouble(const std::string &name,double value) {
    std::ostringstream s;
    s << value;
    paramMap[name] = s.str();
}

void Settings::Object::State::saveInteger(const std::string &name,int value) {
    std::ostringstream s;
    s << value;
    paramMap[name] = s.str();
}

void Settings::Object::State::saveString(const std::string &name,const std::string &value) {
    paramMap[name] = value;
}

Settings::Object::State Settings::Object::State::loadState(const std::string &name) const {
    State value;
    std::map<std::string,State>::const_iterator n;

    n = stateMap.find(const_cast<std::string &>(name));
    if(n != stateMap.end())
        value = n->second;

    return value;
}

void Settings::Object::State::saveState(const std::string &name,const Settings::Object::State &value) {
    stateMap[name] = value;
}

QDomElement Settings::Object::State::xml(QDomDocument &doc) const {
    QDomElement e = doc.createElement("OBJECT");
    e.setAttribute("id",QString::number(id));

    for(std::map<std::string,std::string>::const_iterator i = paramMap.begin();i != paramMap.end();++i) {
        QDomElement e1 = doc.createElement("PARAM");
        e1.setAttribute("name",i->first);
        e.appendChild(e1);

        QDomText t = doc.createTextNode(i->second);
        e1.appendChild(t);
    }

    for(std::map<std::string,State>::const_iterator i = stateMap.begin();i!= stateMap.end();++i) {
        QDomElement e1 = i->second.xml(doc);
        e1.setAttribute("name",i->first);
        e.appendChild(e1);
    }

    return e;
}

void Settings::Object::State::xml(const QDomElement &e1) {
    paramMap.clear();
    stateMap.clear();

    if(e1.tagName().upper() != "OBJECT") {
        ERROR_MSG("Settings::Object::State::xml : invalid element\n");
        return;
    }
    id = e1.attribute("id","0").toULong();

    for(QDomElement e2 = e1.firstChild().toElement();!e2.isNull();e2 = e2.nextSibling().toElement()) {
        if(e2.tagName().upper() == "PARAM" && e2.attribute("name") != QString::null)
            paramMap[e2.attribute("name")] = e2.text().latin1();
        else if(e2.tagName().upper() == "OBJECT" && e2.attribute("name") != QString::null)
            stateMap[e2.attribute("name")].xml(e2);
    }
}

Settings::Object::State Settings::Object::save(void) const {
    State s(id);
    doSave(s);
    return s;
}

void Settings::Object::load(const Settings::Object::State &s) {
    if(id != s.id) {
        Mutex::Locker lock(&Manager::getInstance()->mutex);

        Manager::getInstance()->releaseID(this);
        Manager::getInstance()->acquireID(this,s.id);
    }

    doLoad(s);
}

void Settings::Object::deferred(const Settings::Object::State &s) {
    doDeferred(s);
}

Settings::Object *Settings::Manager::getObject(Settings::Object::ID id) const {
    Mutex::Locker lock(&mutex);

    std::map<Object::ID,Object *>::const_iterator i = objectMap.find(id);
    if(i != objectMap.end())
        return i->second;
    else
        return 0;
}

void Settings::Manager::foreachObject(void (*callback)(Object *,void *),void *param) {
    Mutex::Locker lock(&mutex);
    for(std::list<Object *>::iterator i = objectList.begin(),end = objectList.end();i != end;++i)
        callback(*i,param);
}

int Settings::Manager::load(const std::string &filename) {
    QFile file(filename);
    if(!file.open(IO_ReadOnly)) {
        ERROR_MSG("Settings::Manager::load : failed to open %s for reading\n",filename.c_str());
        return -EPERM;
    }

    QDomDocument doc;
    QString errorMsg;
    int errorLine, errorColumn;

    if(!doc.setContent(&file,false,&errorMsg,&errorLine,&errorColumn)) {
        ERROR_MSG("Settings::Manager::load : %s:%d:%d: %s\n",filename.c_str(),errorLine,errorColumn,errorMsg.latin1());
        return -EINVAL;
    }

    QDomElement e1 = doc.documentElement();

    if(e1.tagName() != "RTXI" || e1.attribute("class") != "settings") {
        ERROR_MSG("Settings::Manager::load : invalid document element\n");
        return -EINVAL;
    }

    /*
     * Return RTXI to a startup like state.
     */

    Plugin::Manager::getInstance()->unloadAll();
	MainWindow::getInstance()->clearFileMenu();
	MainWindow::getInstance()->clearModuleMenu();

	long long period = RT::System::getInstance()->getPeriod();
    RT::System::getInstance()->setPeriod(1000000);

    Object::State s;
    Plugin::Object *plugin;
    std::list<defer_t> deferList;
    for(QDomElement e2 = e1.firstChild().toElement();!e2.isNull();e2 = e2.nextSibling().toElement()) {
        if(e2.tagName() != "OBJECT" || e2.attribute("component") == QString::null) continue;

        s.xml(e2);
        if(e2.attribute("component") == "plugin") {
            if((plugin = Plugin::Manager::getInstance()->load(e2.attribute("library")))) {
                defer_t defer = { plugin, s };
                deferList.push_back(defer);
                plugin->load(s);
            }
        } else if(e2.attribute("component") == "rt") {
            period = strtoll(s.loadString("Period").c_str(),0,10);
            // Legacy case, period is stored as a double in scientific notation
            if(period < 1000)
                period = s.loadDouble("Period");
        } else if(e2.attribute("component") == "io") {
            defer_t defer = { IO::Connector::getInstance(), s };
            deferList.push_back(defer);
        }
    }
    for(std::list<defer_t>::iterator i = deferList.begin(),end = deferList.end();i != end;++i)
        i->object->deferred(i->s);

    if(period)
        RT::System::getInstance()->setPeriod(period);

    // create QSettings
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);

	int oldestsetting = userprefs.readNumEntry("/recentSettingsList/start", 0);
	int num_settings = userprefs.readNumEntry("/recentSettingsList/num", 0);

	QStringList entries = userprefs.entryList("/recentSettingsList");
	int numRecentFiles = entries.size();
	QString listsetting;
	bool doesnotexist = true;

	for (int i = 0; i < numRecentFiles; ++i) {
		listsetting = userprefs.readEntry("/recentSettingsList/" + entries[i]);
		if (filename == listsetting)
			doesnotexist = false;
	}

	if (filename == "/etc/rtxi.conf")
		doesnotexist = false;

	if (doesnotexist) {
		if (num_settings == 10) {
			userprefs.writeEntry("/recentSettingsList/" + QString::number(
					oldestsetting), filename);
			oldestsetting++;
			if (oldestsetting == 10)
				oldestsetting = 0;
			userprefs.writeEntry("/recentSettingsList/start", oldestsetting);
		} else {
			userprefs.writeEntry("/recentSettingsList/" + QString::number(
					num_settings++), filename);
			userprefs.writeEntry("/recentSettingsList/num", num_settings);
		}
	}

    return 0;
}

static void saveState(Plugin::Object *plugin,void *param) {
    QDomDocument *doc = reinterpret_cast<QDomDocument *>(param);
    QDomElement e = plugin->save().xml(*doc);

    e.setAttribute("component","plugin");
    e.setAttribute("library",plugin->getLibrary());
    doc->documentElement().appendChild(e);
}

int Settings::Manager::save(const std::string &filename) {
    QDomDocument doc;
    QDomElement e;

    doc.appendChild(doc.createElement("RTXI"));
    doc.documentElement().setAttribute("class","settings");
    doc.documentElement().setAttribute("version","1.3");

    /*
     * Save RT System period
     */
    Object::State s;
    char buffer[256];
    snprintf(buffer,256,"%lld",RT::System::getInstance()->getPeriod());
    s.saveString("Period",buffer);
    e = s.xml(doc);
    e.setAttribute("component","rt");
    doc.documentElement().appendChild(e);

    /*
     * Save IO Connection information
     */
    e = IO::Connector::getInstance()->save().xml(doc);
    e.setAttribute("component","io");
    doc.documentElement().appendChild(e);

    /*
     * Save Plugin Settings information
     */
    Plugin::Manager::getInstance()->foreachPlugin(saveState,&doc);

    QFile file(filename);
    if(!file.open(IO_WriteOnly)) {
        ERROR_MSG("Settings::Manager::save : failed to open %s for writing\n",filename.c_str());
        return -EPERM;
    }

    QTextStream stream(&file);
    stream << doc;

    // create QSettings
	QSettings userprefs;
	userprefs.setPath("RTXI.org", "RTXI", QSettings::User);

	int oldestsetting = userprefs.readNumEntry("/recentSettingsList/start", 0);
	int num_settings = userprefs.readNumEntry("/recentSettingsList/num", 0);

	QStringList entries = userprefs.entryList("/recentSettingsList");
	int numRecentFiles = entries.size();
	QString listsetting;
	bool doesnotexist = true;

	for (int i = 0; i < numRecentFiles; ++i) {
		listsetting = userprefs.readEntry("/recentSettingsList/" + entries[i]);
		if (filename == listsetting)
			doesnotexist = false;
	}

	if (filename == "/etc/rtxi.conf")
		doesnotexist = false;

	if (doesnotexist) {
		if (num_settings == 10) {
			userprefs.writeEntry("/recentSettingsList/" + QString::number(
					oldestsetting), filename);
			oldestsetting++;
			if (oldestsetting == 11)
				oldestsetting = 1;
			userprefs.writeEntry("/recentSettingsList/start", oldestsetting);
		} else {
			userprefs.writeEntry("/recentSettingsList/" + QString::number(
					num_settings++), filename);
			userprefs.writeEntry("/recentSettingsList/num", num_settings);
		}
	}

    return 0;
}

void Settings::Manager::acquireID(Settings::Object *object,Settings::Object::ID id) {
    Mutex::Locker lock(&mutex);

    if(id != Object::INVALID) {
        if(objectMap.find(id) == objectMap.end()) {
            objectMap[id] = object;
            object->id = id;
            return;
        } else
            DEBUG_MSG("Settings::Manager::acquireID : requested ID in use\n");
    }

    /***********************************************************
     * Traverse the ID space to find an available ID.          *
     *   It ain't pretty, O(n^2) if my math is correct...      *
     ***********************************************************/

    while(objectMap.find(currentID) != objectMap.end() && currentID != Object::INVALID) ++currentID;

    /*****************************************************************************
     * It is possible that there are no more available IDs, and in that case     *
     *   the insertion fails and the object is assigned an invalid ID.           *
     *   In practice this shouldn't ever happen, because on the x86 we have 2^32 *
     *     IDs, so we would probably run out of memory before hitting this limit *
     *****************************************************************************/

    if(currentID == Object::INVALID) {
        ERROR_MSG("Settings::Manager::acquireID : maximum number of settings objects loaded.\n");
        object->id = Object::INVALID;
    } else {
        object->id = currentID;
        objectMap[currentID++] = object;
    }
}

void Settings::Manager::releaseID(Settings::Object *object) {
    Mutex::Locker lock(&mutex);

    if(object->id != Object::INVALID) {
        objectMap.erase(object->id);
        if(object->id < currentID) currentID = object->id;
        object->id = Object::INVALID;
    }
}

void Settings::Manager::insertObject(Settings::Object *object) {
    if(!object) {
        ERROR_MSG("Settings::Manager::insertObject : invalid object\n");
        return;
    }

    Mutex::Locker lock(&mutex);

    acquireID(object);
    std::list<Object *>::iterator i;
    for(i = objectList.begin();i != objectList.end() && (*i)->id < object->id;++i);

    Event::Object event(Event::SETTINGS_OBJECT_INSERT_EVENT);
    event.setParam("object",object);
    Event::Manager::getInstance()->postEvent(&event);

    objectList.insert(i,object);
}

void Settings::Manager::removeObject(Settings::Object *object) {
    Mutex::Locker lock(&mutex);

    Event::Object event(Event::SETTINGS_OBJECT_REMOVE_EVENT);
    event.setParam("object",object);
    Event::Manager::getInstance()->postEvent(&event);

    objectList.remove(object);
    releaseID(object);
}

static Mutex mutex;
Settings::Manager *Settings::Manager::instance = 0;

Settings::Manager *Settings::Manager::getInstance(void) {
    if(instance)
        return instance;

    /*************************************************************************
     * Seems like alot of hoops to jump through, but static allocation isn't *
     *   thread-safe. So effort must be taken to ensure mutual exclusion.    *
     *************************************************************************/

    Mutex::Locker lock(&::mutex);
    if(!instance) {
        static Manager manager;
        instance = &manager;
    }
    return instance;
}
