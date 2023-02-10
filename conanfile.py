from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv", "cmake_find_package"
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
        #self.requires("libiconv/1.17")
        self.requires("qt/5.15.7")
        self.requires("fmt/8.1.1")
        self.requires("hdf5/1.14.0")
        self.requires("libgit2/1.5.0")
        self.requires("qwt/6.2.0")
        self.requires("zlib/1.2.13")
        self.requires("openssl/1.1.1q")

        # Testing only dependencies below
        self.requires("gtest/1.11.0")

