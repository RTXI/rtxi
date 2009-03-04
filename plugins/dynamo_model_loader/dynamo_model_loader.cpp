#include <debug.h>

#define _GNU_SOURCE
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <qfiledialog.h>

#include <cmdline.h>
#include <main_window.h>
#include <dynamo_model_loader.h>


#define MODEL_MAKEFILE_PATH    "/usr/share/rtxi/Makefile.modcompile"
#define MAKE_CMD_PREFIX        "make -f "
#define MODEL_SOURCE_SUFFIX    ".dynamo"
#define MODEL_SUFFIX           ".so"


extern "C" Plugin::Object *createRTXIPlugin(void *) 
{
     return DynamoModelLoader::getInstance();
}

DynamoModelLoader::DynamoModelLoader(void) {
    DEBUG_MSG("DynamoModelLoader::DynamoModelLoader : starting\n");
    menuID = MainWindow::getInstance()->createControlMenuItem("Dynamo Model Loader",this,SLOT(load_dialog(void)));
    model_makefile_path = QString (MODEL_MAKEFILE_PATH);
    DEBUG_MSG("model_makefile_path = %s\n", model_makefile_path.ascii());
    DEBUG_MSG("DynamoModelLoader::DynamoModelLoader : finished\n");
}

DynamoModelLoader::~DynamoModelLoader(void) {
    MainWindow::getInstance()->removeControlMenuItem(menuID);
}

/* Set and retrieve the name of the model make file. */
QString DynamoModelLoader::get_model_makefile_path () const
{
     return model_makefile_path;
}

int DynamoModelLoader::set_model_makefile_path (char *s)
{
     if (s != NULL)
     {
	  model_makefile_path = QString (s);
     }
     else
	  return -EINVAL;

     return 0;
}

void DynamoModelLoader::load(char *srcpath) 
{
     int status;
     char *path;

     path = canonicalize_file_name (srcpath);
     QString model_name = QString (path);
     model_name.remove(MODEL_SOURCE_SUFFIX);

     if(model_name.isNull() || model_name.isEmpty())
	  return;

     if (model_makefile_path.isNull() || 
	 model_makefile_path.isEmpty())
     {
	  ERROR_MSG("invalid model makefile name\n");
	  return;
     }

     QString cmd = QString( "cd %1; %2 %3 %4%5" )
	  .arg( getcwd(NULL,0) )         
	  .arg( MAKE_CMD_PREFIX )         
	  .arg( model_makefile_path )
	  .arg( model_name )
	  .arg( MODEL_SUFFIX );

     QString module_name = QString( "%1%2" )
	  .arg( model_name )
	  .arg( MODEL_SUFFIX );

     DEBUG_MSG("about to compile model with command %s\n", cmd.ascii());
     status = CmdLine::getInstance()->execute(cmd.ascii());
     
     if (status != 0)
     {
	  ERROR_MSG("build process error\n");
	  return;
     }

     Plugin::Manager::getInstance()->load(module_name.latin1());

     if (path != NULL) free (path);
}

void DynamoModelLoader::load_dialog(void) 
{
     QString file_name = QFileDialog::getOpenFileName(QString::null,"Dynamo Models (*" MODEL_SOURCE_SUFFIX \
						      ");;All (*.*)",MainWindow::getInstance());

     load((char *)file_name.latin1());
}


static Mutex mutex;
DynamoModelLoader *DynamoModelLoader::instance = NULL;

DynamoModelLoader *DynamoModelLoader::getInstance(void) 
{
    if(instance != NULL)
        return instance;

    Mutex::Locker lock(&::mutex);
    if(instance == NULL)
        instance = new DynamoModelLoader();

    return instance;
}
