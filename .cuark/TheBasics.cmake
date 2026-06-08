# C++23, cuz we're living in the future
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Extensions are a way to break your app on different OS'es. Bad bad bad
set(CMAKE_CXX_EXTENSIONS OFF)

# Auto-complete-n-stuff
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if(MSVC)
  # For windows/VC++, run normal exception handling & windows 10 APIs
  # And for the love of god, turn of the CRT security warnings.
  # Having deployed those things across all of Microsoft in 2005, let's
  # just say "LKG666" wasn't very fun.
  add_compile_definitions(
    _WIN32_WINNT=0x0A00
    WINVER=0x0A00
    _CRT_SECURE_NO_WARNINGS
  )
  add_compile_options(/EHsc /W4 /std:c++latest)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(/reference "std=${STATIC_DEBUG_STD_BMI_LOC}")
    # Hey, look, a feature I worked on!
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT EditAndContinue)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreadedDebug)
    # Hey look, I did this work, too!
    set(CMAKE_MSVC_RUNTIME_CHECKS "StackFrameErrorCheck;UninitializedVariable")
    # Explicitly debug build, and turn on mspdbsrv.exe,
    # cuz multiproc builds are fast
    add_compile_options(/Od /ZI /FS)
    add_link_options(/DEBUG:FULL /PROFILE)
  else()
    add_compile_options(/reference "std=${STATIC_STD_BMI_LOC}")
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT ProgramDatabase)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded)

    # Enable WPO explicitly, instead of relying on CMake's custom support
    add_compile_options(/GL /O2 /FS)
    add_link_options(/LTCG /OPT:REF /OPT:ICF)
  endif()
  # Define TARGET_OS for the OS-specific code in tools_lib
  set(TARGET_OS windows)
else()
  # For non-windows compilers, enable sanitizers and stuff
  add_compile_options(-Wpedantic -Wall -Wextra -pedantic)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-O0 -g) # -fsanitize=address,undefined
    add_link_options(-O0 -g) # -fsanitize=address,undefined
  else()
    add_compile_options(-flto -O2)
    add_link_options(-flto)
  endif()
  # Define TARGET_OS for the OS-specific code in tools_lib
  # Trigger CMake's module scanning:
  if(APPLE)
    # For clang, we specify where to find the std/std.compat BMI's
    set(CMAKE_CXX_SCAN_FOR_MODULES ON)
    add_compile_options("-fmodule-file=std=${STD_BMI_LOC}")
    add_compile_options("-fmodule-file=std.compat=${STD_COMPAT_BMI_LOC}")
    set(TARGET_OS macos)
  elseif(UNIX)
    set(TARGET_OS linux)
    # GCC Only: This triggers old-style Clang-only modules, not C++20 modules
    add_compile_options(-fmodules)
  endif()
endif()
