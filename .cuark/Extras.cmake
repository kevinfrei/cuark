# This is just the extra stuff I use for the overal cuark project

enable_testing()
include(GoogleTest)

# webview isn't on conan, or vcpkg, but this is how it says to use it
# And I switched to my fork, so I could make it compile with C++23
include(FetchContent)
FetchContent_Declare(
  webview
  GIT_REPOSITORY
  https://github.com/kevinfrei/webview
  GIT_TAG
  0.12.3
)
FetchContent_MakeAvailable(webview)
# This let's me use the same syntax as all my dependencies:
set(WEBVIEW_LIB webview::core)
