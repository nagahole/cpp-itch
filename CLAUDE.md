# cpp_pubsub → ITCH feed handler — project brief

## What this is

A C++20 market data feed handler: it parses a full trading day of Nasdaq TotalView-ITCH 5.0 data, rebuilds the limit order book for every symbol, and is benchmarked and optimised. It started as an in-process pub/sub library. The broker is kept as a finished warm-up, and the project now moves to the feed handler.

It's a learning project: the goal is to get fluent in modern, high-performance C++ by building something with a real workload, and to be able to explain every design choice — why each data structure, what was measured, and what changed the numbers.

Personal context (goals, schedule) lives in a local file that isn't committed:

@.claude/context.md

## How to help

- **This is a learning project. Don't write the implementation for me.** Review my code, point out bugs and better idioms, explain why, and give hints. Only write full code when I explicitly ask, or for boilerplate (CMake, test scaffolding, benchmark harness).
- I know C well (I taught systems programming in C) but I'm new to C++. Explaining C++ concepts by comparison to C works well (e.g. move semantics as "copy the pointer and NULL the source").
- Keep explanations plain and direct.
- Flag things that will matter later (threading, lifetimes, performance) even if they don't need fixing yet.
- **Keep me on scope.** Push back if I start adding features that aren't in the plan below. Depth on the planned stages beats breadth.
- When I finish a stage, ask me to explain the design and its trade-offs, then point out gaps in my explanation.

## Scope

Each stage should end with something that compiles, passes tests (including under sanitizers), and is worth showing. If time runs short, stopping after any stage is fine. Stage 3 is the minimum that counts as a finished project.

### Stage 1: basic broker (done, frozen)
- Topics by name, callbacks, synchronous delivery, templated message type, `Subscription` RAII handle, tests
- Frozen: no further features. Only tag it (see "Current status").

### Stage 2: ITCH 5.0 parser
- Get one day's sample file from Nasdaq (emi.nasdaq.com/ITCH, a few GB gzipped) and the ITCH 5.0 spec PDF. Keep data in `data/`, which is gitignored.
- File format: each message is preceded by a 2-byte big-endian length
- Memory-map the file (`mmap`) and parse in place. No per-message allocation or copying into intermediate objects.
- Message types needed for the book: `S` system event, `R` stock directory (locate → symbol), `A`/`F` add order, `E` executed, `C` executed with price, `X` partial cancel, `D` delete, `U` replace. Count and skip everything else.
- Unit tests use hand-built byte buffers for each message type, then a smoke test counts messages by type over the real file
- Learning: `std::span`, `std::bit_cast`, byte order, strict aliasing (`memcpy` vs pointer casts), RAII wrapper around `mmap`/`munmap`

### Stage 3: order book + correctness
- Order ID → order state (side, price, remaining shares, locate). Per-symbol price levels for bids and asks, with total shares per level.
- Handle the tricky cases: `E`/`X` reduce shares and the order is removed at zero. `U` replace is a delete plus an add with a new order ID; the side isn't in the message, so carry it over from the original order.
- First version: straightforward and obviously correct (`std::unordered_map` for orders, `std::map` per side for levels). **This becomes the reference implementation.** Keep it in the repo and don't optimise it.
- Correctness check: a test mode that runs the reference and the optimised book side by side and compares top of book (or the top N levels) every N messages over the full day
- Single-threaded: parse → apply → next message, in one loop
- First end-to-end number: messages/sec for the whole day in a Release build
- Learning: templates (the book is generic over the level container, so reference and optimised share tests), STL containers and their costs, testing against a reference

### Stage 4: make it fast (profile first)
- Profile before changing anything. On macOS use Instruments (Time Profiler); for cache-miss counters, run `perf` on Linux if I can get access.
- Candidate changes, **one at a time and each measured**:
  - Stock locate codes are small integers assigned for the day, so index books with an array instead of a hash map
  - Flat array of price levels indexed by price tick near the touch, instead of `std::map`
  - Open-addressing hash map for order IDs, preallocated for the day's volume
  - Object pools, so there's no allocation on the hot path
- Keep a running log in `docs/perf-log.md`: what changed, before/after numbers, and why I think it helped (or didn't). The README comes from this.
- Learning: cache behaviour, allocation cost, how to measure properly (Release builds, warm-up, repeated runs, variance)

### Stage 5: pipeline with a lock-free SPSC queue
- Split into two threads: parser → SPSC ring buffer → book builder
- Start with a mutex + condition variable queue, then write the lock-free SPSC ring with `std::atomic` and acquire/release ordering
- Cache-line padding to avoid false sharing between head and tail; fixed capacity, no allocation
- Compare with Google Benchmark for the queue alone (throughput), and end to end over the full day: throughput, plus p50/p99 latency from "parsed" to "applied to book"
- Run tests under TSan (`build-tsan/`)
- Be honest in the results: if the single-threaded version is faster for this workload, that's a valid finding. Explain why.
- Learning: threads, `std::jthread` / stop tokens, memory ordering, cross-thread lifetimes

### Out of scope
Networking (live feeds, UDP/MoldUDP64), other exchanges' protocols, trading strategies or matching, persistence, GUIs. Stretch goals only if ahead of schedule: a small replay tool that prints the book for one symbol at a given time, or `perf stat` cache-miss numbers on Linux.

### Finish
A short README covering what the system does, how it's built, how I checked correctness, and a table of benchmark results across versions, with one or two sentences on what each change did. Plain wording: state the numbers and the reasons, nothing inflated.

## Current status

**Done (stage 1):**
- Build: header-only `pubsub` INTERFACE library, `examples/basic.cpp`, tests via Catch2 v3 (FetchContent)
- `PUBSUB_SANITIZER` CMake cache variable (applied globally before targets)
- `CMakePresets.json` (Ninja): `debug` → `build/`, `release` → `build-release/` (`-O2 -DNDEBUG`), `asan` → `build-asan/` (`address,undefined`), `tsan` → `build-tsan/`. Each has configure, build, test and workflow presets.
- `include/pubsub/broker.hpp`: `Broker<T>` with `subscribe` / `publish`. Callbacks stored per topic as `std::unordered_map<int, std::function<void(const T&)>>`, keyed by a per-broker subscriber ID. `subscribe` returns a `Subscription`.
- `include/pubsub/subscription.hpp`: move-only RAII handle holding a `std::function<void()>` cleanup that unsubscribes in the destructor. Move ops leave the source empty; move assignment runs the current cleanup first.
- Broker lifetime guard: `Broker` owns a `shared_ptr<bool>` sentinel and each cleanup lambda holds a `weak_ptr` to it, so destroying a `Subscription` after its broker is a no-op
- `tests/test_broker.cpp` covers: single delivery, multiple subscribers (handles stored in a `std::vector`), unknown topic, publish with no subscribers, topic isolation, message order for one subscriber, same callback subscribed twice is called twice, non-string message type (`int`), unsubscribe on destruction, subscription outliving its broker
- Tests pass under the `asan` preset

**Next:**
1. Tag stage 1 (`git tag stage1-broker`)
2. Add `data/` to `.gitignore`; download one day's ITCH sample file and the ITCH 5.0 spec into `data/`
3. Start stage 2: `mmap` RAII wrapper, then the length-prefixed message loop counting messages by type

**Placeholder files (empty):** `include/pubsub/spsc_queue.hpp` and `tests/test_spsc_queue.cpp` (stage 5), `benchmarks/bench_queues.cpp` and `benchmarks/CMakeLists.txt` (stages 3–5), `src/broker.cpp` (unused, since the broker is header-only; can be deleted). Suggested new layout: `include/itch/` (parser, message structs), `include/book/` (reference and optimised books), `tools/` (the full-day replay executable).

## Design decisions so far

Broker (stage 1, kept for reference):
- Header-only, since templates need definitions in headers anyway
- Delivery order across different subscribers is not guaranteed; message order for a single subscriber is
- Subscribing the same callback twice means it is called twice (no deduplication)
- Subscriber IDs are per broker instance, not global
- `Subscription` is move-only: exactly one handle owns each cleanup
- A `Subscription` must not outlive its broker in a way that touches freed memory; the `weak_ptr` sentinel guards this (single-threaded only, since nothing stops the broker dying between `lock()` and the erase on another thread)
- Known issue, not being fixed since the broker is frozen: subscribing to, or destroying a `Subscription` for, the same topic from inside a callback during `publish` modifies the map being iterated (iterator invalidation, undefined behaviour)

Feed handler:
- Prices stay as integers (ITCH prices are fixed-point `uint32` with 4 decimal places). Never convert to floating point inside the book.
- ITCH is big-endian. `std::byteswap` is C++23, so on C++20 use shifts or a compiler builtin, wrapped in one small helper.
- Read fields with `memcpy` (or `std::bit_cast` from a byte array), not by casting pointers into the buffer, which breaks alignment and strict aliasing
- Timestamps are 6-byte nanoseconds since midnight; store them as `uint64_t`
- The reference book is the source of truth. Any optimisation that disagrees with it is a bug until proven otherwise.

## Conventions

- C++20, `-Wall -Wextra -Wpedantic`
- Private members use a trailing underscore (`subscribers_`)
- Include what you use; no `<iostream>` in headers
- Prefer explicit lambda captures (`[&received]`) over `[&]`, especially for callbacks that are stored or run on other threads
- Take sink parameters by value and `std::move` them into storage
- Tests record received values in a `std::vector` and compare against the full expected vector (catches duplicates and ordering issues)
- Fixed-width integer types (`uint32_t`, `uint64_t`) for anything that comes off the wire
- Out-of-source builds: `build/` (debug), `build-release/` (benchmarks), `build-asan/`, `build-tsan/` (ASan and TSan can't be combined). `build-*/` and `data/` are gitignored.
- Every performance claim comes with a number from a Release build and a line in `docs/perf-log.md`
- Commit messages: short imperative subject, small commits per step
- Likely on macOS: AddressSanitizer works but leak detection isn't supported, and `perf` isn't available (use Instruments)

## Build and test

```
cmake --workflow --preset debug     # configure + build + test
cmake --workflow --preset asan      # same, under ASan + UBSan

cmake --build --preset asan && ctest --preset asan   # day to day, after first configure
```

Benchmarks only from `--preset release`. If a `build-*/` directory was configured with a different generator or source path, delete it and reconfigure.
