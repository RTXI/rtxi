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

#include "dlplugin.hpp"

#include "debug.hpp"

DLL::Loader::~Loader()
{
  this->unloadAll();
}

int DLL::Loader::load(const char* library)
{
  DLL::dll_info dll {std::string(library), nullptr};
  const std::unique_lock<std::mutex> lk(this->m_dll_mutex);
  auto loc_iter =
      std::find(this->loaded_plugins.begin(), this->loaded_plugins.end(), dll);
  if (loc_iter != this->loaded_plugins.end()) {
    return 0;
  }

  void* handle = ::dlopen(library, RTLD_LOCAL | RTLD_NOW);
  if (handle == nullptr) {
    ERROR_MSG("DLL::Loader::load : Unable to load library {}", library);
    // NOLINTNEXTLINE
    ERROR_MSG("dlopen : {}", dlerror());
    return -1;
  }
  dll.handle = handle;
  this->loaded_plugins.push_back(dll);
  return 0;
}

void DLL::Loader::unload(const char* library)
{
  const DLL::dll_info dll {std::string(library), nullptr};
  const std::unique_lock<std::mutex> lk(this->m_dll_mutex);
  auto loc_iter =
      std::find(this->loaded_plugins.begin(), this->loaded_plugins.end(), dll);
  if (loc_iter == this->loaded_plugins.end()) {
    return;
  }
  ::dlclose(loc_iter->handle);
  this->loaded_plugins.erase(loc_iter);
}

void DLL::Loader::unloadAll()
{
  for (auto& dll : this->loaded_plugins) {
    this->unload(dll.library_name.c_str());
  }
}
