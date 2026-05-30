# Using a custom Clang to build 'import std' support on MacOS:

First, `brew install llvm`. LLVM is a "keg-only" installation. You have to
trigger its usage manually. Currently,
[here's the output of the installation command, just to have it around](homebrew-output.md)

If you need details, go look at that file. I just dumped this into my .zshrc
file:

```sh
# Some stuff for MacOS custome clang:
if [[ -d /opt/homebrew/opt/llvm/bin ]] ; then
  export PATH="/opt/homebrew/opt/llvm/bin:$PATH"
  export LDFLAGS="-L/opt/homebrew/opt/llvm/lib"
  export CPPFLAGS="-I/opt/homebrew/opt/llvm/include"
  export CMAKE_PREFIX_PATH="/opt/homebrew/opt/llvm"
fi
```

But that's just the bare minimum to make a compiler available to use by default.
Here's what else you have to do:

## Conan:

First, set some Python configuration stuff in `~/.conan2/global.conf`:

```python
# Core configuration (type 'conan config list' to list possible values)
tools.cmake:configure_args = ['-DCMAKE_PREFIX_PATH=/opt/homebrew/opt/llvm']
tools.build:compiler_executables = {'c': '/opt/homebrew/opt/llvm/bin/clang', 'cpp': '/opt/homebrew/opt/llvm/bin/clang++'}
```

Next, you need to set the profile properly. I just set the default profile,
because why not:

First, delete `~/.conan2/profiles/default` if you have it laying round, then
type:

```sh
conan profile detect
```

and that should set up the profile. Check to make sure it looks something like
this (`cat ~/.conan2/profiles/default`)

```ini
[settings]
arch=armv8
build_type=Release
compiler=clang
compiler.cppstd=gnu17
compiler.libcxx=libc++
compiler.version=22
os=Macos
```

## Building the STL's std and std.compat modules from source

[This page is useful](https://andsav.wordpress.com/2026/04/28/c-std-module-on-macos-with-clang/)

The command to build the STL import modules is pretty straight forward:

```sh
cd /opt/homebrew/opt/llvm/share/libc++/v1
clang++ -std=c++23 --precompile std.cppm -o std.pcm -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -Wno-reserved-module-identifier
clang++ -std=c++23 --precompile std.compat.cppm -o std.compat.pcm -fmodule-file=std=std.pcm -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk -Wno-reserved-module-identifier
```

then you add these two items to your compilation flags:

```
-fmodule-file=std=/opt/homebrew/opt/llvm/share/libc++/v1/std.pcm
-fmodule-file=std.compat=/opt/homebrew/opt/llvm/share/libc++/v1/std.compat.pcm
```

That _does_ pollute the homebrew installation directory, but I didn't really
care, and it's not really a big deal for me right now.
