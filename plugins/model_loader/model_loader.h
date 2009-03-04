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

private:

    int menuID;

};

#endif /* MODEL_LOADER_H */
