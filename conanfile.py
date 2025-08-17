from conan import ConanFile
from conan.tools.cmake import cmake_layout
from conan.tools.cmake import CMakeToolchain


class KVS(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps"
    options = {"build_tests" : [True, False], "enable_logs": [True, False]}

    default_options = {"build_tests" : False, "enable_logs" : False}

    def requirements(self):
        self.requires("gtest/1.16.0")

    def configure(self):
        self.options["gtest/*"].shared = False

    def layout(self):
        cmake_layout(self)
    
    def generate(self):
        toolchain = CMakeToolchain(self)
        toolchain.variables["BUILD_TESTS"] = "ON" if self.options.build_tests else "OFF"
        toolchain.variables["KVS_ENABLE_LOGS"] = "ON" if self.options.enable_logs else "OFF"
        toolchain.generate()