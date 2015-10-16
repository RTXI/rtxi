#include <QtWidgets>

#include "module_utils.h"

/*
 * Class RTXIModule
 */
RTXIModule::RTXIModule(QListWidget *parent) : QListWidgetItem(parent) {
	readme_url = QUrl("");
	clone_url = QUrl("");
	location = QUrl("");
	name = QString("");
	readme = QString("");

	installed = false;
}

void RTXIModule::setLocation(QString str) {
	location = QUrl(str);
	return;
}

void RTXIModule::setReadmeUrl(QString str) {
	readme_url = QUrl(str);
	return;
}

void RTXIModule::setCloneUrl(QString str) {
	clone_url = QUrl(str);
	return;
}

void RTXIModule::setName(QString str) {
	name = str;
	return;
}

void RTXIModule::setReadme(QString str) {
	readme = str;
	return;
}

/*
void RTXIModule::setInstalled(bool state) {
	installed = state;
	return;
}
*/

QUrl RTXIModule::getLocation(void) {
	return location;
}

QUrl RTXIModule::getReadmeUrl(void) {
	return readme_url;
}

QUrl RTXIModule::getCloneUrl(void) {
	return clone_url;
}

QString RTXIModule::getName(void) {
	return name;
}

QString RTXIModule::getReadme(void) {
	return readme;
}
