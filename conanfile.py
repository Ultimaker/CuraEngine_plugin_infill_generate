from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.microsoft import check_min_vs, is_msvc_static_runtime, is_msvc
from conan.tools.files import apply_conandata_patches, export_conandata_patches, get, copy, rm, rmdir, replace_in_file
from conan.tools.build import check_min_cppstd
from conan.tools.scm import Version
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.env import VirtualBuildEnv
import os
from pathlib import Path


required_conan_version = ">=1.53.0"


class CuraEngineInfillGeneratePluginConan(ConanFile):
    name = "curaengine_plugin_infill_generate"
    description = "CuraEngine plugin for testing the infill generate slot"
    license = "agpl-3.0"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/Ultimaker/CuraEngine_plugin_infill_generate"
    topics = ("protobuf", "asio", "plugin", "curaengine", "gcode-generation", "3D-printing")
    package_type = "library"
    settings = "os", "arch", "compiler", "build_type"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    @property
    def _min_cppstd(self):
        return 20

    @property
    def _compilers_minimum_version(self):
        return {
            "gcc": "11",
            "clang": "14",
            "apple-clang": "13",
            "msvc": "192",
            "visual_studio": "17",
        }

    def export_sources(self):
        copy(self, "CMakeLists.txt", self.recipe_folder, self.export_sources_folder)
        copy(self, "*", os.path.join(self.recipe_folder, "src"), os.path.join(self.export_sources_folder, "src"))
        copy(self, "*", os.path.join(self.recipe_folder, "include"), os.path.join(self.export_sources_folder, "include"))
        copy(self, "*", os.path.join(self.recipe_folder, "tests"), os.path.join(self.export_sources_folder, "tests"))

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        self.options["boost"].header_only = True

        if self.options.shared:
            self.options.rm_safe("fPIC")
        self.options["grpc"].csharp_plugin = False
        self.options["grpc"].node_plugin = False
        self.options["grpc"].objective_c_plugin = False
        self.options["grpc"].php_plugin = False
        self.options["grpc"].python_plugin = False
        self.options["grpc"].ruby_plugin = False
        self.options["asio-grpc"].local_allocator = "recycling_allocator"

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("protobuf/3.21.9")
        self.requires("boost/1.81.0")
        self.requires("asio-grpc/2.4.0")
        self.requires("openssl/1.1.1l")
        self.requires("spdlog/1.11.0")
        self.requires("docopt.cpp/0.6.3")
        self.requires("range-v3/0.12.0")
        self.requires("clipper/6.4.2")
        self.requires("ctre/3.7.2")
        self.requires("curaengine_grpc_definitions/latest@ultimaker/cura_10619")

    def validate(self):
        # validate the minimum cpp standard supported. For C++ projects only
        if self.settings.compiler.cppstd:
            check_min_cppstd(self, self._min_cppstd)
        check_min_vs(self, 191)
        if not is_msvc(self):
            minimum_version = self._compilers_minimum_version.get(str(self.settings.compiler), False)
            if minimum_version and Version(self.settings.compiler.version) < minimum_version:
                raise ConanInvalidConfiguration(
                    f"{self.ref} requires C++{self._min_cppstd}, which your compiler does not support."
                )
        if is_msvc(self) and self.options.shared:
            raise ConanInvalidConfiguration(f"{self.ref} can not be built as shared on Visual Studio and msvc.")

    def build_requirements(self):
        self.tool_requires("protobuf/3.21.9")

    def generate(self):
        # BUILD_SHARED_LIBS and POSITION_INDEPENDENT_CODE are automatically parsed when self.options.shared or self.options.fPIC exist
        tc = CMakeToolchain(self)
        # Boolean values are preferred instead of "ON"/"OFF"
        if is_msvc(self):
            tc.variables["USE_MSVC_RUNTIME_LIBRARY_DLL"] = not is_msvc_static_runtime(self)
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
        cpp_info = self.dependencies["curaengine_grpc_definitions"].cpp_info
        tc.variables["GRPC_IMPORT_DIRS"] = cpp_info.resdirs[0].replace("\\", "/")
        tc.variables["GRPC_PROTOS"] = ";".join([str(p).replace("\\", "/") for p in Path(cpp_info.resdirs[0]).rglob("*.proto")])
        tc.generate()

        tc = CMakeDeps(self)
        tc.generate()

        tc = VirtualBuildEnv(self)
        tc.generate(scope="build")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, pattern="LICENSE", dst=os.path.join(self.package_folder, "licenses"), src=self.source_folder)
        ext = ".exe" if self.settings.os == "Windows" else ""
        copy(self, pattern=f"curaengine_plugin_infiil_generate{ext}", dst="bin", src=os.path.join(self.build_folder))

