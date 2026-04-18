# Generated file: DO NOT EDIT
# Add your library to the conanfile.py libraries list

find_package(Crow REQUIRED)
set(CROW_LIB Crow::Crow)

find_package(GTest REQUIRED)
set(GTEST_LIB gtest::gtest)

find_package(platform_folders REQUIRED)
set(PLATFORMFOLDERS_LIB sago::platform_folders)

find_package(portable-file-dialogs REQUIRED)
set(PFD_LIB portable-file-dialogs::portable-file-dialogs)

find_package(reproc++ REQUIRED)
set(REPROC_LIB reproc::reproc)

find_package(taglib REQUIRED)
set(TAGLIB_LIB taglib::taglib)

find_package(zstd REQUIRED)
set(ZSTD_LIB zstd::libzstd_static)
