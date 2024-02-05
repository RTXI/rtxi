# Hacking

Here is some wisdom to help you build and test this project as a developer and
potential contributor.

If you plan to contribute, please read the [CONTRIBUTING](CONTRIBUTING.md)
guide.

## Prerequisite

In order to develop and maintain RTXI under Ubuntu you will need the following:

1. **cmake** >= 3.19
2. **clang-tidy** >= 12
3. **clang-format** >= 14
4. **conan** (optional) >= 2.0
5. **codespell** >= 2.0

Executables like codespell and clang-format can be obtained through python's
package manager pip, while other like cmake need apt or snap package managers
to work in older systems (Ubuntu 20.04 and older). Your mileage may vary in 
other platforms.

Presets were added for cmake 3.19 so it makes sense to make this a strict
requirement when developing RTXI, however this is not necessary for consumers
as they won't need to use presets and therefore can get away with older versions.

Development with the conan package manager can be fickle, especially since their
update to version 2. Make sure to avoid installing system wide dependencies that will
be used by RTXI as this may conflict with conan installed packages. As of 12/4/23
this is still a problem.

## Developer mode

Build system targets that are only useful for developers of this project are
hidden if the `rtxi_DEVELOPER_MODE` option is disabled. Enabling this
option makes tests and other developer targets and options available. Not
enabling this option means that you are a consumer of this project and thus you
have no need for these targets and options. This option is autoamtically added to
dev preset.

Developer mode is always set to on in CI workflows.

### Presets

This project makes use of [presets][1] to simplify the process of configuring
the project. As a developer, you are recommended to always have the [latest
CMake version][2] installed to make use of the latest Quality-of-Life
additions.

You have a few options to pass `rtxi_DEVELOPER_MODE` to the configure
command, but this project prefers to use presets.

As a developer, you should create a `CMakeUserPresets.json` file at the root of
the project:

```json
{
  "version": 2,
  "cmakeMinimumRequired": {
    "major": 3,
    "minor": 19,
    "patch": 0
  },
  "configurePresets": [
    {
      "name": "dev",
      "binaryDir": "${sourceDir}/build/dev",
      "inherits": ["dev-mode", "conan", "ci-<os>"],
      "cacheVariables": {
        "CMAKE_BUILD_TYPE": "Debug"
      }
    }
  ],
  "buildPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "configuration": "Debug"
    }
  ],
  "testPresets": [
    {
      "name": "dev",
      "configurePreset": "dev",
      "configuration": "Debug",
      "output": {
        "outputOnFailure": true
      }
    }
  ]
}
```

You should replace `<os>` in your newly created presets file with the name of
the operating system you have, which may be `win64`, `linux` or `darwin`. You
can see what these correspond to in the
[`CMakePresets.json`](CMakePresets.json) file.

> **Note**
> While RTXI was built with corss-platform compilation in mind, it does not yet
> fully support other platforms. In the future, RTXI will be installable in 
> Windows and Mac, but only their non-realtime versions. This could change however
> if there are real-time options available for those platforms. Until then, the
> only fully supported platform is **Linux**.

`CMakeUserPresets.json` is also the perfect place in which you can put all
sorts of things that you would otherwise want to pass to the configure command
in the terminal.

> **Note**
> Some editors are pretty greedy with how they open projects with presets.
> Some just randomly pick a preset and start configuring without your consent,
> which can be confusing. Make sure that your editor configures when you
> actually want it to, for example in CLion you have to make sure only the
> `dev-dev preset` has `Enable profile` ticked in
> `File > Settings... > Build, Execution, Deployment > CMake` and in Visual
> Studio you have to set the option `Never run configure step automatically`
> in `Tools > Options > CMake` **prior to opening the project**, after which
> you can manually configure using `Project > Configure Cache`.

### Dependency manager

The above preset will make use of the [conan][conan] dependency manager. After
installing it, make sure you have a [Conan profile][profile] setup, then
download the dependencies and generate the necessary CMake files by running
this command in the project root:

```sh
conan install . -s build_type=Debug -b missing
```

Note that if your conan profile does not specify the same compiler, standard
level, build type and runtime library as CMake, then that could potentially
cause issues. See the link above for profiles documentation.

This has been temporarily disabled due to conan migration issues from 1.x to 2.x
but can be easily added back by editing presets and inheriting the conan preset.

> **Note**
> Make sure to also enable conan toolchains and settings under the Github workflows
> for support sanitizer and testing support. Of course do this once conan supporort
> improves.

[conan]: https://conan.io/
[profile]: https://docs.conan.io/2/reference/config_files/profiles.html

### Configure, build and test

If you followed the above instructions, then you can configure, build and test
the project respectively with the following commands from the project root on
any operating system with any build system:

```sh
cmake --preset=dev
cmake --build --preset=dev
ctest --preset=dev
```

If you are using a compatible editor (e.g. VSCode) or IDE (e.g. CLion, VS), you
will also be able to select the above created user presets for automatic
integration.

Please note that both the build and test commands accept a `-j` flag to specify
the number of jobs to use, which should ideally be specified to the number of
threads your CPU has. You may also want to add that to your preset using the
`jobs` property, see the [presets documentation][1] for more details.

### Developer mode targets

These are targets you may invoke using the build command from above, with an
additional `-t <target>` flag:

#### `coverage`

Available if `ENABLE_COVERAGE` is enabled. This target processes the output of
the previously run tests when built with coverage configuration. The commands
this target runs can be found in the `COVERAGE_TRACE_COMMAND` and
`COVERAGE_HTML_COMMAND` cache variables. The trace command produces an info
file by default, which can be submitted to services with CI integration. The
HTML command uses the trace command's output to generate an HTML document to
`<binary-dir>/coverage_html` by default.

#### `docs`

Available if `BUILD_MCSS_DOCS` is enabled. Builds to documentation using
Doxygen and m.css. The output will go to `<binary-dir>/docs` by default
(customizable using `DOXYGEN_OUTPUT_DIRECTORY`).

#### `format-check` and `format-fix`

These targets run the clang-format tool on the codebase to check errors and to
fix them respectively. Customization available using the `FORMAT_PATTERNS` and
`FORMAT_COMMAND` cache variables.

#### `run-exe`

Runs the executable target `rtxi_exe`. Understand that this may not
provide full access to all features of rtxi, and that is mainly because 
hardware access requires sudo privileges, which may break exe linking with 
this target. This target is a convenience function for running the binary
to quickly test whether it runs properly after compilation. To circumvent 
this problem, run the build and install targets for dev preset, then directly
run the binary with sudo. This will ensure that your build process does not 
create root permissions in build byproducts.

#### `spell-check` and `spell-fix`

These targets run the codespell tool on the codebase to check errors and to fix
them respectively. Customization available using the `SPELL_COMMAND` cache
variable.

[1]: https://cmake.org/cmake/help/latest/manual/cmake-presets.7.html
[2]: https://cmake.org/download/
