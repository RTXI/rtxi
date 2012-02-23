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
