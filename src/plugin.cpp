/*
         The Real-Time eXperiment Interface (RTXI)
         Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
   Will Cornell Medical College

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

#include "debug.hpp"
#include "plugin.h"

DLL::Loader::~Loader()
{
  int result=0;
  for(auto& dll : this->loaded_plugins){
    result = ::dlclose(dll.handle);
    if(result != 0){
      ERROR_MSG("DLL::Loader::~Loader : Unable to close handle for library {}", dll.library_name);
    }
  }
}

int DLL::Loader::load(const char* library)
{
  std::unique_lock<std::mutex> lk(this->m_dll_mutex);
  void* handle = ::dlopen(library, RTLD_GLOBAL | RTLD_NOW);
  if(handle == nullptr){
    ERROR_MSG("DLL::Loader::load : Unable to load library {}", library);
    ERROR_MSG("dlopen : {}", dlerror());
    return -1;
  }
  this->loaded_plugins.emplace_back(std::string(library), handle);   
  return 0;
}

