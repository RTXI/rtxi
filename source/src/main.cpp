/*
 Copyright (C) 2011 Georgia Institute of Technology, University of Utah, Weill Cornell Medical College

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */

#include <iostream>

#include <cstdlib>
#include <dirent.h>
#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <string>
#include <unistd.h>

#include <qapplication.h>

#include <cmdline.h>
#include <ctype.h>
#include <daq.h>
#include <debug.h>
#include <main_window.h>
#include <plugin.h>

static pid_t parentThread;

struct cli_options_t {
    std::string config_file;
    std::string plugins_path;
};

static bool parse_cli_options(int,char *[],cli_options_t *);
static void signal_handler(int);

int main(int argc,char *argv[]) {
    int retval = 0;

    /* Try to Exit Cleanly on Signals */
    parentThread = getpid();
    signal(SIGINT,signal_handler);
    signal(SIGABRT,signal_handler);
    signal(SIGSEGV,signal_handler);

    /* Handle Command-Line Options */
    cli_options_t cli_options;
    if(!parse_cli_options(argc,argv,&cli_options))
        return -EINVAL;

    /* Find Configuration File */
    std::string config_file;
    if(cli_options.config_file.length())
        config_file = cli_options.config_file;
    else if(getenv("RTXI_CONF"))
        config_file = getenv("RTXI_CONF");
    else
        config_file = "/etc/rtxi.conf";

    /************************************************************
     * Create Main System Components                            *
     *                                                          *
     *  These need to be created early because they should have *
     *  Settings::IDs of 0 and 1.                               *
     ************************************************************/


    /* Create GUI Objects */
    QApplication *app = new QApplication(argc,argv);
    app->connect(app,SIGNAL(lastWindowClosed()),app,SLOT(quit()));
    MainWindow::getInstance()->showMaximized();

    CmdLine::getInstance();

    RT::System::getInstance();
    IO::Connector::getInstance();

    /* Bootstrap the System */
    Settings::Manager::getInstance()->load(config_file);

    retval = app->exec();

    Plugin::Manager::getInstance()->unloadAll();
    return retval;
}

static void error_msg(const std::string &self) {
    std::cout << "Try \'" << self << " --help\' for more information.\n";
}

static void help_msg(const std::string &self) {
    std::cout << "Usage: " << self << " [options]\n";
    std::cout << "  where options include:\n";
    std::cout << "    --help,         -h  - Displays this message\n";
    std::cout << "    --config-file,  -c  - Pick a custom configuration file\n";
    std::cout << "    --plugins-path, -p  - Specify a plugins directory\n";
}

static bool parse_cli_options(int argc,char *argv[],cli_options_t *cli_options) {
    int opt, index;

    struct option options[] = {
        { "help",        no_argument,       0, 'h' },
        { "config-file", required_argument, 0, 'c' },
        { "plugins-path", required_argument, 0, 'p' },
        { "models-path",  required_argument, 0, 'm' },
        { 0,0,0,0 }
    };

    for(;;) {
        opt = getopt_long(argc,argv,"hc:p:m:",options,&index);

        if(opt < 0) break;

        switch(opt) {
          case 'c':
              cli_options->config_file = optarg;
              break;
          case 'h':
              help_msg(argv[0]);
              return false;
          case 'p':
              cli_options->plugins_path = optarg;
              break;
          default:
              error_msg(argv[0]);
              return false;
        }
    }

    return true;
}

static void signal_handler(int signum) {
    static int count = 0;

    /* Only handler handle signals in the parent */
    if(getpid() != parentThread) return;

    DEBUG_MSG("signal_handler : signal received\n");
#ifdef DEBUG
    PRINT_BACKTRACE();
#endif // DEBUG

    if(count++) _exit(-EFAULT);

    DEBUG_MSG("signal_handler : finished\n");
    exit(0);
}
