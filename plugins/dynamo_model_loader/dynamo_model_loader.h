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

private:

    int menuID;

    QString model_makefile_path;
    static DynamoModelLoader *instance;

};

#endif /* DYNAMO_MODEL_LOADER_H */
