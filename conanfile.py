from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout
import os

class CalculatorProjectConan(ConanFile):
    name = "calculator_project"
    version = "1.0"
    settings = "os", "compiler", "build_type", "arch"
    # requires = "fastdds/2.8.0"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        # self.requires("cyclonedds/0.10.4")
        self.requires("cyclonedds-cxx/0.10.4")
        self.requires("flatbuffers/24.12.23")

    def layout(self):
        cmake_layout(self)

    def build(self):
        # Run fastrtpsgen to generate C++ files from IDL
        self.run("cyclonedds-idlc -l c++ -o src idl/calculator.idl")
        
        cmake = CMake(self)
        cmake.configure()
        cmake.build()