# cpp-itch

[![CI](https://github.com/nagahole/cpp-itch/actions/workflows/ci.yml/badge.svg)](https://github.com/nagahole/cpp-itch/actions/workflows/ci.yml)

A C++20 feed handler for Nasdaq TotalView-ITCH 5.0 market data, built as a project to learn modern C++. The goal is to parse a full trading day of ITCH data, rebuild the limit order book for every symbol, check it against a simple reference implementation, then profile and optimise it.

## Status

- **Done:** big-endian field readers for ITCH's integer types, and a move-only RAII wrapper over `mmap` for reading the data file in place. Tests use Catch2 and run under AddressSanitizer and UBSan.
- **In progress:** message framing and decoders for the order-book message types.
- **Planned:** a reference order book, optimised books checked against it over a full day, profiling, and a two-thread pipeline with a lock-free SPSC queue.

An in-process pub/sub broker in `include/pubsub/` was the warm-up for this project.

## Build

Requires CMake 3.25+, Ninja and a C++20 compiler.

```
cmake --workflow --preset debug
```

This configures, builds and runs the tests. Other presets: `release`, `asan`, `tsan`.

## AI Use

I use Claude as a tutor and code reviewer, not to write the implementation. It reviews my code, explains C++ idioms and flags issues. It only writes boilerplate (CMake, test scaffolding, benchmark harness) or code I explicitly ask for. The full instructions it works under are in [`CLAUDE.md`](CLAUDE.md).

