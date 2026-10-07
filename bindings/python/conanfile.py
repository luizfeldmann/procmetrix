from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps

class Recipe(ConanFile):
    # Metadata
    license = "mit"
    url = "https://github.com/luizfeldmann/procmetrix"

    # Settings and options
    settings = "os", "arch", "compiler", "build_type"

    def requirements(self):
        # The exact version will be given by the lockfile
        self.requires("procmetrix/[>=0.0.0]")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
