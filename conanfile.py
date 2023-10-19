from conan import ConanFile


class Recipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps", "VirtualRunEnv"
    #default_options = {
    #    "fmt/*:shared": True,
    #    "gtest/*:shared": True, 
    #    "qt/*:shared": True, 
    #    "hdf5/*:shared": True, 
    #    #"libgit2/*:shared": True, 
    #    "qwt/*:shared": True 
    #}

    def layout(self):
        self.folders.generators = "conan"

    def requirements(self):
        #self.requires("libiconv/1.17")
        self.requires("qt/[~5.15]")
        self.requires("fmt/8.1.1")
        self.requires("hdf5/[~1.10]")
        #self.requires("libgit2/1.5.0")
        self.requires("qwt/[~6]")
        #self.requires("zlib/1.2.13")
        #self.requires("openssl/1.1.1q")
        self.requires("gsl/2.7")

        # Testing only dependencies below
        self.requires("gtest/[~1.14]")

