#include <string>
#include <vector>
#include <qobject.h>

#include <pthread.h>

#include <mutex.h>
#include <qonsole.h>

class ConsoleIO;

class Console : public QObject
{

     Q_OBJECT

public:


    static Console *getInstance(void);

    int *requestIoFd(void);
    void setTabLabel (unsigned int, char *);

public slots:
    void show (void);


private:

    Console(int nsessions, char *prompt = "rtxi> ");
    ~Console(void);
    Console(const Console &) {};
    Console &operator=(const Console &) { return *getInstance(); };

    static Console *instance;

    Qonsole *qon;

    int menuID;
    int nsessions;
    std::vector<ConsoleIO *> cios;

    Mutex mutex;

}; // class Console

class ConsoleIO
{
public:

     ConsoleIO (int, char*);
     ~ConsoleIO(void);

     bool isRequested (void);
     int  getInputFd (void);
     int  getOutputFd (void);
     
     void input_func (void);
     void output_func (void);
     void run (void);
     unsigned int getId (void);
     
private:
    unsigned int id;

    char *prompt;

    int ifd[2];
    int ofd[2];

    bool requested;

    pthread_t input_tid;
    pthread_t output_tid;

};

