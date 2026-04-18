from conan import ConanFile
from conan.tools.microsoft import vs_layout

import os
from typing import NamedTuple, Optional

class CMakeInfo(NamedTuple):
    # Name of the package to find with find_package
    package: str
    # CMake target to use with target_link_libraries, if needed
    target: str
    # CMake variable to use in CMakeLists, defaults to ucase "${package}_LIB"
    var: Optional[str] = None

class Library(NamedTuple):
    # Name of library from conan-center
    name: str
    # Version of package from conan-center
    version: str
    # CMake package info
    info: Optional[CMakeInfo] = None


libraries = [
    # Library("boost", "1.90.0", CMakeInfo("Boost", "boost::boost")),
    Library("crowcpp-crow", "1.3.1", CMakeInfo("Crow", "Crow::Crow")),
    Library("efsw", "1.5.1"),
    Library("gtest", "1.17.0", CMakeInfo("GTest", "gtest::gtest")),
    # Library("libjpeg", "9f"),
    # Using TagLib instead of this one:
    # Library("libmediainfo", "22.03", CMakeInfo("MediaInfoLib", "mediainfo", "MEDIAINFO_LIB")),
    # Library("libpng", "1.6.48", CMakeInfo("PNG", "PNG::PNG")),
    # Library("libtiff", "4.7.0", CMakeInfo("TIFF", "TIFF::TIFF")),
    Library("platformfolders", "4.3.0", CMakeInfo("platform_folders", "sago::platform_folders", "PLATFORMFOLDERS_LIB")),
    Library("portable-file-dialogs", "0.1.0", CMakeInfo("portable-file-dialogs", "portable-file-dialogs::portable-file-dialogs", "PFD_LIB")),
    Library("reproc", "14.2.5", CMakeInfo("reproc++", "reproc::reproc", "REPROC_LIB")),
    Library("taglib", "2.0", CMakeInfo("taglib", "taglib::taglib")),
    # Library("zlib", "1.3.1", CMakeInfo("ZLIB", "ZLIB::ZLIB")),
    Library("zstd", "1.5.7", CMakeInfo("zstd", "zstd::libzstd_static")),
]

# I don't think I want anything other than just the pair for these
tools = [] # [Library("doxygen", "1.14.0")]

class CuarkRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps" #, "CMakeToolchain"

    def init(self):
        # emit the find_package calls in the ConanLibImports.cmake file,
        # along with the variables for later use in the CMakeLists
        imports = [
          '# Generated file: DO NOT EDIT\n',
          '# Add your library to the conanfile.py libraries list\n','\n']
        for library in libraries:
            if library.info:
              imports.append(f'find_package({library.info.package} REQUIRED)\n')
              var_name = library.info.var if library.info.var else f'{library.info.package.upper()}_LIB'
              imports.append(f'set({var_name} {library.info.target})\n')
              imports.append('\n')
        file_location = os.path.join(self.recipe_folder, ".cuark", "ConanLibImports.cmake")
        # Remove newlines from the end of the output:
        while imports and imports[-1] == '\n':
            imports.pop()
        if os.path.exists(file_location):
            # Check to see if the file is the same as what we're about to write, so we don't
            # unnecessarily trigger a camke reconfig
            with open(file_location, "r", newline='\n') as f:
                existing = f.readlines()
            if len(imports) == len(existing):
                for i in range(len(imports)):
                    if imports[i] != existing[i]:
                        break
                else:
                    # File is the same, so don't write it out
                    return
        with open(os.path.join(self.recipe_folder, ".cuark", "ConanLibImports.cmake"), "w", newline='\n') as f:
            f.writelines(imports)

    def requirements(self):
        for requirement in libraries:
            self.requires(requirement.name + "/" + requirement.version)

    def build_requirements(self):
        # Add the cmake toolchain as a build requirement
        for requirement in tools:
          self.tool_requires(requirement.name + "/" + requirement.version)
    def layout(self):
        vs_layout(self)
