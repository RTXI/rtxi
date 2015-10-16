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
