from conans import ConanFile, CMake, tools


class CsvParserConan(ConanFile):
    name = "csv-parser"
    version = "2.1.3"
    license = "MIT"
    
    settings = "os", "compiler", "build_type", "arch"

    
    def build(self):
        url = 'https://github.com/vincentlaucsb/csv-parser/archive/refs/tags/2.1.3.tar.gz'
        tools.get(url, strip_root=True)

    def package(self):
        self.copy('single_include/*')

    def package_info(self):
        self.cpp_info.includedirs = ["single_include"]

