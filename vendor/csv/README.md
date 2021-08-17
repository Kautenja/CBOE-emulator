# CSV

[![build-status][]][build-server]

[build-status]: https://travis-ci.com/Kautenja/csv.svg
[build-server]: https://travis-ci.com/Kautenja/csv

# Roadmap

- [x] load
- [x] print
- [ ] load with types
- [ ] check column names
- [ ] error handling

# Usage

Simply add [include/csv.hpp](include/csv.hpp) to your C++ project either by
copying directly or using git submodules.

## Testing

To compile and run the [test suite](test):

```shell
scons test
```

## Benchmarking

To run benchmarks:

```shell
scons benchmark/benchmark_csv.cpp
```
