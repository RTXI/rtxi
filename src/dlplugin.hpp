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

#ifndef DLLPLUGIN_H
#define DLLPLUGIN_H

#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

#include <dlfcn.h>

//! Classes associated with the loading/unloading of binaries at run-time.
/*!
 * Collection of classes to control the loading and unloading of rtxi plugins
 * at run-time.
 */
namespace DLL
{

typedef struct dll_info
{
  std::string library_name;
  void* handle;
  bool operator==(const dll_info& rhs) const
  {
    return this->library_name == rhs.library_name;
  }
  bool operator!=(const dll_info& rhs) const { return !operator==(rhs); }
} dll_info;

class Loader
{
public:
  Loader() = default;
  Loader(const Loader&) = delete;
  Loader(Loader&&) = delete;
  Loader& operator=(const Loader&) = delete;
  Loader& operator=(Loader&&) = delete;
  ~Loader();

  /*!
   * Function for loading a Plugin::Object from a shared library file.
   *
   * \param library The file name of a shared library.
   * \return A pointer to the newly created Plugin::Object.
   *
   * \sa Plugin::Object
   */
  int load(const char* library);

  /*!
   * Function for unloading a single Plugin::Object in the system.
   *
   * \param object The plugin object to be unloaded.
   */
  void unload(const char* library);

  /*!
   * Returns a pointer to the requested symbol inside the given library
   *
   * \param library location of the loaded library.
   * \param symbol name of the symbol to load.
   *
   * \return A pointer to the loaded symbol.
   */
  template<typename T>
  T dlsym(const char* library, const char* symbol)
  {
    dll_info temp = {std::string(library), nullptr};
    auto handle_loc = std::find(
        this->loaded_plugins.begin(), this->loaded_plugins.end(), temp);
    if (handle_loc == this->loaded_plugins.end()) {
      return nullptr;
    }
    return reinterpret_cast<T>(::dlsym(handle_loc->handle, symbol));  // NOLINT
  }

  /*!
   * Function for unloading all Plugin::Object's in the system.
   */
  void unloadAll();

private:
  // static const u_int32_t MAGIC_NUMBER = 0xCA24CB3F;
  // u_int32_t magic_number;
  // std::string library;
  std::vector<dll_info> loaded_plugins;
  std::mutex m_dll_mutex;
};  // class Loader

}  // namespace DLL

#endif  // PLUGIN_H
