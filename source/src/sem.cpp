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

#include <rt.h>
#include <sem.h>
#include <unistd.h>

Semaphore::Semaphore(size_t n)
    : count(n) {}

Semaphore::~Semaphore(void) {}

void Semaphore::down(void) {
#ifdef DEBUG
    if(RT::OS::isRealtime()) {
        ERROR_MSG("Detected unsafe down attempt in RT thread\n");
        PRINT_BACKTRACE();
        return;
    }
#endif // DEBUG

    --count;
    while(count < 0) usleep(1000);
}

void Semaphore::up(void) {
    ++count;
}

int Semaphore::value(void) {
    return count;
}
