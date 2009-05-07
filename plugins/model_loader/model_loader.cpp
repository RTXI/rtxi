#include <debug.h>
#include <main_window.h>
#include <model_loader.h>
#include <qfiledialog.h>

extern "C" Plugin::Object *createRTXIPlugin(void *) {
    return new ModelLoader();
}

ModelLoader::ModelLoader(void) {
    menuID = MainWindow::getInstance()->createControlMenuItem("Plugin Loader",this,SLOT(load(void)));
}

ModelLoader::~ModelLoader(void) {
    MainWindow::getInstance()->removeControlMenuItem(menuID);
}

void ModelLoader::load(void) {
    QString plugin_dir = QString(EXEC_PREFIX)+QString("/lib/rtxi/");
    QString filename = QFileDialog::getOpenFileName(plugin_dir,"Plugins (*.so);;All (*.*)",MainWindow::getInstance());

    if(filename.isNull() || filename.isEmpty())
        return;

    if(filename.startsWith(plugin_dir))
        filename.remove(0,plugin_dir.length());

    Plugin::Manager::getInstance()->load(filename.latin1());
}
