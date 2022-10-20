from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conans import tools


class TensorflowConan(ConanFile):
    name = "tensorflow"
    version = "2.10"

    # Optional metadata
    license = "Apache-2.0"

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"

    def build(self):
        url = "https://storage.googleapis.com/tensorflow/libtensorflow/libtensorflow-cpu-linux-x86_64-2.10.0.tar.gz"
        tools.get(url)

    def package(self):
        self.copy('*')

    def package_info(self):
        self.cpp_info.libs = ["tensorflow"]
        self.cpp_info.includedirs = ['include']

