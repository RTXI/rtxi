from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"
    default_options = {
        "qt/*:qtsvg": True,
        "qt/*:shared": True
    }

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("qt/[~5.15]")
        self.requires("fmt/10.0.0")
        self.requires("hdf5/[~1.10]")
        self.requires("qwt/[~6]")
        self.requires("zlib/1.2.13")
        self.requires("openssl/1.1.1q")
        self.requires("gsl/2.7")
        self.requires("boost/1.83.0")

        # Testing only dependencies below
        self.requires("gtest/[~1.14]")

