#include <mutex.h>
#include <string>

class CmdLine {

public:

     int execute(const std::string &);

    static CmdLine *getInstance(void);

private:

    /*****************************************************************
     * The constructor, destructor, and assignment operator are made *
     *   private to control instantiation of the class.              *
     *****************************************************************/

    CmdLine(void);
    ~CmdLine(void);
    CmdLine(const CmdLine &) {};
    CmdLine &operator=(const CmdLine &) { return *getInstance(); };

    static CmdLine *instance;

    volatile bool done;
    pid_t child;

    int fdm[2];
    int fds[2];

    Mutex mutex;

}; // class CmdLine
