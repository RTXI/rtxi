#include <QtNetwork>
#include <git2.h>
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

	private slots:
		void cloneModule(void);
		void getRepos(void);
		void getReadme(void);
		void parseRepos(void);
		void parseReadme(void);

		void updateButton(void);

/*
		void cloneModules();
		void installModules();
		void syncModules();
		void updateModules();
		void repairRepos();
		void addRemote();

		void reloadUI();
*/

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
//		QList<RTXIModule> *remoteModules;

		std::vector<QString> exclude_list;

};
