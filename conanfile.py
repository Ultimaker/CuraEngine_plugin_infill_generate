import os

from conan import ConanFile
from conan.errors import ConanInvalidConfiguration
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.env import VirtualBuildEnv
from conan.tools.files import copy
from conan.tools.microsoft import check_min_vs, is_msvc_static_runtime, is_msvc
from conan.tools.scm import Version

required_conan_version = ">=1.58.0 <2.0.0"


class CuraEngineInfillGeneratePluginConan(ConanFile):
    name = "curaengine_plugin_infill_generate"
    description = "CuraEngine plugin for testing the infill generate slot"
    license = ("agpl-3.0", "lgpl-3.0", "bsd-4")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/Ultimaker/CuraEngine_plugin_infill_generate"
    topics = ("protobuf", "asio", "plugin", "curaengine", "gcode-generation", "3D-printing")
    package_type = "library"
    settings = "os", "arch", "compiler", "build_type"
    exports = "LICENSE*"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }

    def set_version(self):
        if not self.version:
            self.version = "0.2.0-beta.1"
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
    @property
    def _cura_plugin_name(self):
        return "CuraEngineTiledInfill"

    @property
    def _api_version(self):
        return "8"

    @property
    def _sdk_versions(self):
        return ["8.5.0"]

    @property
    def _max_sdk_version(self):
        sorted_versions = sorted(self._sdk_versions, key=lambda v: Version(v))
        return sorted_versions[-1]

    def export_sources(self):
        copy(self, "CMakeLists.txt", self.recipe_folder, self.export_sources_folder)
        copy(self, "*", os.path.join(self.recipe_folder, "src"), os.path.join(self.export_sources_folder, "src"))
        copy(self, "*", os.path.join(self.recipe_folder, "include"), os.path.join(self.export_sources_folder, "include"))
        copy(self, "*", os.path.join(self.recipe_folder, "tests"), os.path.join(self.export_sources_folder, "tests"))
        copy(self, "*", os.path.join(self.recipe_folder, self._cura_plugin_name), os.path.join(self.export_sources_folder, self._cura_plugin_name))

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        self.options["boost"].header_only = True
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def build_requirements(self):
        self.test_requires("standardprojectsettings/[>=0.1.0]@ultimaker/stable")

    def requirements(self):
        self.requires("boost/1.82.0")
        self.requires("openssl/1.1.1l")
        self.requires("asio-grpc/2.6.0")
        self.requires("spdlog/1.10.0")
        self.requires("docopt.cpp/0.6.3")
        self.requires("range-v3/0.12.0")
        self.requires("clipper/6.4.2")
        self.requires("ctre/3.7.2")
        self.requires("neargye-semver/0.3.0")
        self.requires("curaengine_grpc_definitions/latest@ultimaker/testing")

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

    def generate(self):
        # BUILD_SHARED_LIBS and POSITION_INDEPENDENT_CODE are automatically parsed when self.options.shared or self.options.fPIC exist
        tc = CMakeToolchain(self)
        # Boolean values are preferred instead of "ON"/"OFF"
        if is_msvc(self):
            tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0091"] = "NEW"
            tc.variables["USE_MSVC_RUNTIME_LIBRARY_DLL"] = not is_msvc_static_runtime(self)
        tc.cache_variables["CMAKE_POLICY_DEFAULT_CMP0077"] = "NEW"
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
        copy(self, pattern=f"curaengine_plugin_infill_generate{ext}", dst="bin", src=os.path.join(self.build_folder))
        copy(self, pattern=f"bundled_{self._cura_plugin_name}.json", dst=os.path.join(self.package_folder, "res", "bundled_packages"), src=os.path.join(self.source_folder, self._cura_plugin_name))
        copy(self, pattern="*", dst=os.path.join(self.package_folder, "res", "plugins", self._cura_plugin_name), src=os.path.join(self.source_folder, self._cura_plugin_name))


def deploy(self):
        ext = ".exe" if self.settings.os == "Windows" else ""
        copy(self, pattern=f"curaengine_plugin_infill_generate{ext}", dst=self.install_folder, src=os.path.join(self.package_folder, "bin"))
        copy(self, pattern="*", dst=os.path.join(self.install_folder, self._cura_plugin_name), src=os.path.join(self.package_folder, "res"))
