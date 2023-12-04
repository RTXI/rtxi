# Building with CMake

## Build

This project doesn't require any special command-line flags to build to keep
things simple.

Here are the steps for building in release mode with a single-configuration
generator, like the Unix Makefiles one:

```sh
cmake -S . -B build -D CMAKE_BUILD_TYPE=Release -D RTXI_RT_CORE=evl
cmake --build build
```

Here are the steps for building in release mode with a multi-configuration
generator, like the Visual Studio ones:

```sh
cmake -S . -B build
cmake --build build --config Release
```

### Building with MSVC

Note that MSVC by default is not standards compliant and you need to pass some
flags to make it behave properly. See the `flags-windows` preset in the
[CMakePresets.json](CMakePresets.json) file for the flags and with what
variable to provide them to CMake during configuration.

### Building on Apple Silicon

CMake supports building on Apple Silicon properly since 3.20.1. Make sure you
have the [latest version][1] installed.

## Install

This project doesn't require any special command-line flags to install to keep
things simple. As a prerequisite, the project has to be built with the above
commands already.

It is recommended that the project by installed through cpack package generator.
Under Linux distribution with debian packaging the command would be:

```sh
cd build/
cpack . -G DEB
dpkg -i rtxi*.deb
```

The generated package will be in the format rtxi_x.y.z_arch.deb where x.y.z is
the rtxi version being installed and arch is the current machine's architecture.
Note that the last command requires root privileges.

If in the future you wish to uninstall rtxi, then follow the package management
(or installer) instructions. Following the example above for linux the commands
would be (with root privileges):

```sh
dpkg -r rtxi
dpkg -p rtxi
```

For a more direct installation, the below commands require at least CMake 3.15 
to run, because that is the version in which [Install a Project][2] was added.

Here is the command for installing the release mode artifacts with a
single-configuration generator, like the Unix Makefiles one:

```sh
cmake --install build
```

Here is the command for installing the release mode artifacts with a
multi-configuration generator, like the Visual Studio ones:

```sh
cmake --install build --config Release
```

### CMake package

This project exports a CMake package to be used with the [`find_package`][3]
command of CMake:

* Package name: `rtxi`
* Cache variable: `RTXI_EXECUTABLE`

Example usage:

```cmake
find_package(rtxi REQUIRED)
)
```

RTXI has the following components provided:
1.**rtxi** : General RTXI library containing Widget class definitions
2.**xfifo** : Implementation of realtime and non-realtime message passing structure
3.**rtpal** : A Platform Abstraction Layer (PAL) where RT::OS namespace lives
4.**dlplugin** : Plugin loading and unloading facilities
5.**rtdsp** : Realtime digital signals processing library internal to RTXI
6.**rtgen** : Signal and Filter generation library
7.**rtmath** : A realtime library math with utility functions
8.**rtplot** : A realtime plotting library used primarily by the Data Analysis plugin

All of these targets are available under the rtxi package. Therefore if building
a custom plugin you would consume rtxi package like so:

```cmake
find_package(rtxi REQUIRED)
add_library(myplugin MODULE widget.hpp widget.cpp)
target_link_libraries(myplugin PRIVATE rtxi::rtxi)
```

[1]: https://cmake.org/download/
[2]: https://cmake.org/cmake/help/latest/manual/cmake.1.html#install-a-project
[3]: https://cmake.org/cmake/help/latest/command/find_package.html
