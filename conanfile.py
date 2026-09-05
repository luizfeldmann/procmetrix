from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout, CMakeDeps
from conan.tools.files import copy
from conan.tools.build import can_run
import os

class Recipe(ConanFile):
    # Metadata
    name = "procmetrix"
    version = "1.0.0"
    license = "mit"
    package_type = "library"
    url = "https://github.com/luizfeldmann/procmetrix"

    # Settings and options
    settings = "os", "arch", "compiler", "build_type"

    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    # Project's source code
    exports_sources = [
        "CMakeLists.txt",
        "README.md",
        "LICENSE",
        "src/*",
        "include/*",
        "tests/*",
    ]

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

        # Lib is not using C++
        self.settings.rm_safe("compiler.libcxx")
        self.settings.rm_safe("compiler.cppstd")

    @property
    def _skip_tests(self):
        return self.conf.get("tools.build:skip_test", default=False)

    def build_requirements(self):
        if not self._skip_tests:
            self.test_requires("gtest/1.16.0")
    
    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()

        tc = CMakeToolchain(self)
        tc.cache_variables["PROCMETRIX_BUILD_TESTS"] = not self._skip_tests
        tc.cache_variables["PROCMETRIX_SHARED"] = self.options.shared

        major, minor, patch = self.version.split(".")
        tc.cache_variables["PROCMETRIX_VERSION_MAJOR"] = major
        tc.cache_variables["PROCMETRIX_VERSION_MINOR"] = minor
        tc.cache_variables["PROCMETRIX_VERSION_PATCH"] = patch

        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "licenses"))

        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = [ self.name ]
        if self.options.shared:
            self.cpp_info.defines.append("PROCMETRIX_SHARED")
