from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"
    default_options = {
        "fmt:shared": True,
        "gtest:shared": True, 
        "qt:shared": True, 
        "hdf5:shared": True, 
        "libgit2:shared": True, 
        "qwt:shared": True 
    }

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        self.requires("fmt/8.1.1")
        self.requires("gtest/1.11.0")
        self.requires("qt/6.3.1")
        self.requires("hdf5/1.12.1")
        self.requires("libgit2/1.3.0")
        self.requires("qwt/6.2.0")

        # Testing only dependencies below
        self.requires("gtest/1.11.0")

