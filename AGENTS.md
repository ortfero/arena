# Repository Guidelines

## Project Structure & Module Organization
- Core library headers live in `include/arena/` (`base_book_map.hpp`, `base_book_vector.hpp`, `vector.hpp`, `probe.hpp`). Keep new containers and utilities there so they can be shared across builds.
- `src/main.cpp` is a minimal entrypoint; extend it only for runnable demos. Tests live in `test/test.cpp`; benchmarks in `bench/bench.cpp`.
- CMake configuration is in `CMakeLists.txt`; dependency pins are in `conanfile.txt`; repeatable task recipes are in `justfile`. Build outputs land in `build/Debug` and `build/RelWithDebInfo` via the Conan-generated presets.

## Build, Test, and Development Commands
- `just build` (or `just b`): Debug build with Conan dependency restore, `cmake --preset conan-debug`, then Ninja. Outputs to `build/Debug/arena`.
- `just test` (or `just t`): Runs the doctest suite via `ctest --test-dir build/Debug`.
- `just benchmark` (or `just bm`): Release-with-debug-info build, then executes `build/RelWithDebInfo/arena_bench`.
- `just tidy`: Formats C++ sources with `clang-format` and runs `clang-tidy` using the Debug compile commands. Run this before submitting patches touching headers.
- Without `just`, equivalent commands are shown in the recipes; keep using the presets (`conan-debug`, `conan-relwithdebinfo`) for consistent flags.

## Coding Style & Naming Conventions
- C++20 with warnings on (`-Wall -Wextra -Wpedantic`); prefer STL over custom helpers unless performance dictates otherwise.
- Use 4-space indentation, brace-on-same-line, and keep includes ordered: standard, third-party, then `arena/…`.
- Types and classes use `snake_case` (e.g., `base_book_map`); functions and variables also prefer `snake_case`; macros and feature toggles stay `UPPER_SNAKE` (`ARENA_ENABLE_TEST`, `ARENA_ENABLE_BENCH`).
- Keep headers self-contained and constexpr-friendly where practical; favor small, focused functions with clear invariants.

## Testing Guidelines
- doctest powers unit tests; main test aggregation is in `test/test.cpp`. Header-local tests are guarded by `ARENA_ENABLE_TEST`; mirror that pattern when adding coverage inside new headers.
- Name test cases descriptively (e.g., `"modify_moves_order_between_levels"`), and cover both success and failure paths (e.g., duplicate IDs, invalid amounts).
- Run `just test` before pushing; add regression cases alongside new features and any bug fixes.

## Commit & Pull Request Guidelines
- Follow existing history: short, imperative commits (`add base book benchmark`). Group related changes; avoid sprawling diffs.
- PRs should include: clear description of behavior changes, linked issues (if any), test/benchmark results (`just test`, optionally `just bm` for perf-sensitive work), and notes on new dependencies or flags.
- Keep public headers stable—call out any API breaks or ABI risks in the PR description, and provide migration notes when altering data structures.
