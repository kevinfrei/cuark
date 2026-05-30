# The Homebrew output from "brew install llvm"

CLANG_CONFIG_FILE_SYSTEM_DIR: /opt/homebrew/etc/clang
CLANG_CONFIG_FILE_USER_DIR: ~/.config/clang

LLD is now provided in a separate formula: `brew install lld`

Using `clang`, `clang++`, etc., requires a CLT installation at
`/Library/Developer/CommandLineTools`. If you don't want to install the CLT, you
can write appropriate configuration files pointing to your SDK at
`~/.config/clang`.

To use the bundled libunwind please use the following LDFLAGS:

```sh
LDFLAGS="-L/opt/homebrew/opt/llvm/lib/unwind -lunwind"
```

To use the bundled libc++ please use the following LDFLAGS:

```sh
LDFLAGS="-L/opt/homebrew/opt/llvm/lib/c++ -L/opt/homebrew/opt/llvm/lib/unwind -lunwind"
```

Features newer than system libc++ will require the following define to enable
(support for this may be removed in a future major LLVM release):

```sh
CPPFLAGS="-D_LIBCPP_DISABLE_AVAILABILITY"
```

NOTE: You probably want to use the libunwind and libc++ provided by macOS unless
you know what you're doing.

llvm is keg-only, which means it was not symlinked into /opt/homebrew, because
macOS already provides this software and installing another version in parallel
can cause all kinds of trouble.

If you need to have llvm first in your PATH, run:

```sh
  echo 'export PATH="/opt/homebrew/opt/llvm/bin:$PATH"' >> ~/.zshrc
```

For compilers to find llvm you may need to set:

```sh
  export LDFLAGS="-L/opt/homebrew/opt/llvm/lib"
  export CPPFLAGS="-I/opt/homebrew/opt/llvm/include"
```

For cmake to find llvm you may need to set:

```sh
  export CMAKE_PREFIX_PATH="/opt/homebrew/opt/llvm"
```
