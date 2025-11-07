# clox

Implementation of Bob Nystrom's Lox language in C following his book
[Crafting Interpreters](https://craftinginterpreters.com/).

Lox is a simple dynamically-typed language similar in syntax to Python. It supports
imperitive style structures like `if`, `while` and `for` statements as well as functions.
It also has basic OOP capabilities such as classes and inheritence.

clox is a simple bytecode compiler and VM unified into a single simple interpreter
architecture meaning your Lox scripts are compiled to bytecode and executed on the fly.

This interpreter was used as a learning tool and is not recommended for professional
use.

## Building and Running

If you wish to build clox yourself you will need CMake (>=v3.21) and any relatively
recent C compiler. This project has a relatively simple CMake config utilising CMake
presets to control flags and build options.

```sh
cmake -S . -B build --preset=<platform>
cmake --build build
./build/clox
```

Available platforms:

- linux
- linux-dev
- macos
- macos-dev
- win64
- win64-dev
- sanitize : Turns on GCC or Clang compiler instrumentation for sanitizers on Linux
             platforms
- coverage : Turns on GCC or Clang compiler instrumentation for coverage reporting on
             Linux platforms
- linux-dev-strict : Additional checks made on linux using `clang-tidy` and `cpp-check`

> Note: There are addition targets that can be built using the `-t` flag during the build
> step called `spell-check`, `spell-fix`, `format-check` and `format-fix`. These require
> `clang-format` and `codespell` to work correctly.

## Testing

### Testing - Unit Tests (Mocking)

```sh
cmake -S . -B build --preset=<platform>
cmake --build build
ctest --test-dir build
```

> Note: Only the *-dev presets compile the Catch2 tests found in tests/.

> Note: ctest only indicates which tests failed, not what failed. You can run the tests
> directly to get the pretty output from Catch2 to see which REQUIRE and CHECK macros
> failed.

### Testing - Code Coverage

```sh
cmake -S . -B build/coverage –preset=coverage
cmake --build build/coverage
ctest --test-dir build/coverage
mkdir lcov-report
lcov --capture --branch-coverage --directory build/coverage/CMakeFiles/clox_lib.dir --output-file lcov-report/coverage.info
genhtml --branch-coverage  lcov-report/coverage.info --output-directory lcov-report
```

> Note: The unit tests must be run; either by CTest or directly so that the compiler
> instrumentation that has been injected into the final binaries can produce the relevant
> output files that lcov traces for coverage information.

> Note: HTML report is available at the path lcov-report/index.html to view in a browser.

### Testing - Sanitizers

```sh
cmake -S . -B build/sanitize --preset=sanitize -DQNAN_BOXING=ON
cmake --build build/sanitize
build/sanitize/clox <input-script>
```

### Testing - Profiling

#### Prerequisite

```sh
mkdir perf-reports
```

#### Tagged Union Build

```sh
cmake -S . -B build/profile/t-union --preset=profile
cmake --build build/profile/t-union
```

#### QNaN Boxing Build

```sh
cmake -S . -B build/profile/qnan --preset=profile -DQNAN_BOXING=ON
cmake --build build/profile/qnan
```

#### Fibonacci Profiling

##### Tagged Union

```sh
./build/profile/t-union/clox benchmarks/fib-recursive-30.lox
gprof ./build/profile/t-union/clox gmon.out > perf-reports/fib-t-union.txt
gprof ./build/profile/t-union/clox gmon.out | gprof2dot | dot -Tsvg -o perf-reports/fib-t-union-graph.svg
gprof ./build/profile/t-union/clox gmon.out | gprof2dot -n0 -e0 | dot -Tsvg -o perf-reports/fib-t-union-graph-full.svg
```

##### QNaN Boxing

```sh
./build/profile/qnan/clox benchmarks/fib-recursive-30.lox
gprof ./build/profile/qnan/clox gmon.out > perf-reports/fib-qnan.txt
gprof ./build/profile/qnan/clox gmon.out | gprof2dot | dot -Tsvg -o perf-reports/fib-qnan-graph.svg
gprof ./build/profile/qnan/clox gmon.out | gprof2dot -n0 -e0 | dot -Tsvg -o perf-reports/fib-qnan-graph-full.svg
```

##### Comparison

```sh
gprof2dot --compare perf-reports/fib-qnan.txt perf-reports/fib-t-union.txt | dot -Tsvg -o perf-reports/fib-compare.svg
gprof2dot -n0 -e0 --compare perf-reports/fib-qnan.txt perf-reports/fib-t-union.txt | dot -Tsvg -o perf-reports/fib-compare-full.svg
```

#### Leibniz Approx. of Pi

##### Tagged Union

```sh
./build/profile/t-union/clox benchmarks/leibniz-pi-100000.lox
gprof ./build/profile/t-union/clox gmon.out > perf-reports/leibniz-t-union.txt
gprof ./build/profile/t-union/clox gmon.out | gprof2dot | dot -Tsvg -o perf-reports/leibniz-t-union-graph.svg
gprof ./build/profile/t-union/clox gmon.out | gprof2dot -n0 -e0 | dot -Tsvg -o perf-reports/leibniz-t-union-graph-full.svg
```

##### QNaN Boxing

```sh
./build/profile/qnan/clox benchmarks/leibniz-pi-100000.lox
gprof ./build/profile/qnan/clox gmon.out > perf-reports/leibniz-qnan.txt
gprof ./build/profile/qnan/clox gmon.out | gprof2dot | dot -Tsvg -o perf-reports/leibniz-qnan-graph.svg
gprof ./build/profile/qnan/clox gmon.out | gprof2dot -n0 -e0 | dot -Tsvg -o perf-reports/leibniz-qnan-graph-full.svg
```

##### Comparison

```sh
gprof2dot --compare perf-reports/leibniz-qnan.txt perf-reports/leibniz-t-union.txt | dot -Tsvg -o perf-reports/leibniz-compare.svg
gprof2dot -n0 -e0 --compare perf-reports/leibniz-qnan.txt perf-reports/leibniz-t-union.txt | dot -Tsvg -o perf-reports/leibniz-compare-full.svg
```

## Changes

This project is almost entirely par-for-par with Bob Nystrom's version from his book.
It differs in setup with this project using a somewhat proper CMake config. I also do
not like the usage of global variables in Nystrom's version and thus mine weaves the
global variables throughout the compiler and VM as needed.

I also made [jlox](https://github.com/oraqlle/jlox) based on the first part of Nystrom's
book.

