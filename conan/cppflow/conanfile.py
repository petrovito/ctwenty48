from conans import ConanFile, CMake, tools


class CppflowConan(ConanFile):
    name = "cppflow"
    version = "2.0.0"
    license = "MIT"
    settings = "os", "compiler", "build_type", "arch"

    requires = "tensorflow/2.10"
    generators = "cmake_find_package"

    def source(self):
        url = 'https://github.com/serizba/cppflow/archive/refs/tags/v2.0.0.tar.gz'
        tools.get(url, strip_root=True)

    def build(self):
        cmake = CMake(self)
        cmake.configure(defs={'BUILD_EXAMPLES': 'OFF'})
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.includedirs = ['include']

