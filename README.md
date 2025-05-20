# umpire-cxx-allocator
This package provides a C++ standard-compliant adaptor for [LLNL/Umpire](https://github.com/LLNL/Umpire)'s allocators as a header-only library. It also supports building Umpire from source as a part of this package, including via CMake's [FetchContent module](https://cmake.org/cmake/help/latest/module/FetchContent.html).

# Building

This package provides a CMake harness. Configure and install the package as follows:
```shell
cmake -S . -B build [optional arguments]
cmake --build build --target install
```
By default the package will search for an existing installation of Umpire. If Umpire is installed in a non-standard location use the [`CMAKE_PREFIX_PATH`](https://cmake.org/cmake/help/latest/variable/CMAKE_PREFIX_PATH.html) `CACHE` variable to specify the installation prefix of Umpire. If Umpire was not found, it will be built from source.
In addition to the standard CMake `CACHE` variables used to control configuration (`CMAKE_CXX_COMPILER`, `CMAKE_CXX_STANDARD`, `BUILD_SHARED_LIBS`, etc.)
the following Umpire-specific CMake `CACHE` variables can be used to customize the configuration of Umpire when built from source:

* `UMPIRE_ENABLE_CUDA` (default=`OFF`) -- whether to enable CUDA support in Umpire
* `UMPIRE_ENABLE_HIP` (default=`OFF`) -- whether to enable HIP support in Umpire
* `UMPIRE_ENABLE_ASSERTS` (default=`OFF`) -- whether Umpire asserts are enabled
* `UMPIRE_ENABLE_LOGGING` (default=`OFF`) -- whether Umpire logging is enabled
