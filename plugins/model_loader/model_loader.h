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
