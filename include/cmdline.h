#include <string>

class CmdLine {

public:

    static int execute(const std::string &);
    static void initialize(void);

private:

    /*****************************************************************
     * The constructor, destructor, and assignment operator are made *
     *   private to control instantiation of the class.              *
     *****************************************************************/

    CmdLine(void);
    ~CmdLine(void);
    CmdLine(const CmdLine &) {};
    CmdLine &operator=(const CmdLine &) { return *instance; };

    static CmdLine *instance;

    pid_t child;

    int fdm[2];
    int fds[2];

}; // class CmdLine
