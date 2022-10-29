from conans import ConanFile, CMake, tools


class NanaConan(ConanFile):
    name = "nana"
    version = "v1.7.4"
    license = "BSL-1.0"
    
    settings = "os", "compiler", "build_type", "arch"
    
    # requires = 'libxft/2.3.6'

    def source(self):
        self.run("git clone https://github.com/cnjinhao/nana.git --branch v1.7.4")
    
    def build(self):
        cmake = CMake(self)
        cmake.configure(source_folder='nana/')
        cmake.build(args=['-j6'])

    def package(self):
        self.copy('libnana.a')
        self.copy('nana/include/**')

    def package_info(self):
        self.cpp_info.libdirs = ['.']
        self.cpp_info.libs = ["libnana.a"]
        self.cpp_info.includedirs = ['nana/include']

