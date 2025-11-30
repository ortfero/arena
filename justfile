target_name := "arena"
test_target_name := target_name + "_test"
bench_target_name := target_name + "_bench"
build_dir := "build"
debug_dir := build_dir + "/Debug"
release_dir := build_dir + "/RelWithDebInfo"

alias b := build
alias r := run
alias t := test
alias bm := benchmark
alias d := debug
alias dt := debug-test
alias br := build-release
alias rr := run-release
alias bd := build-docker
alias rd := run-docker

default: build

build:
    conan install . \
      -s build_type=Debug \
      -s compiler.cppstd=20 \
      -c tools.cmake.cmaketoolchain:generator=Ninja \
      --build=missing \
      --output-folder=.
    cmake --preset conan-debug
    cmake --build --preset conan-debug

build-release:
    conan install . \
      -s build_type=RelWithDebInfo \
      -s compiler.cppstd=20 \
      -c tools.cmake.cmaketoolchain:generator=Ninja \
      --build=missing \
      --output-folder=.
    cmake --preset conan-relwithdebinfo
    cmake --build --preset conan-relwithdebinfo

run: build
    {{debug_dir}}/{{target_name}}

run-release: build-release
    {{release_dir}}/{{target_name}}

test: build
    ctest --test-dir {{debug_dir}} --output-on-failure

benchmark: build-release
    {{release_dir}}/{{bench_target_name}}

tidy:
    echo "formatting..."
    find src include test -type f \( -name '*.cpp' -o -name '*.hpp' \) \
        -exec clang-format -i {} +

    conan install . -s build_type=Debug --build=missing --output-folder=.
    cmake -S . -B {{debug_dir}} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

    echo "analyzing..."
    clang-tidy \
        $(find src include test -type f -name '*.cpp') \
        -p {{debug_dir}}

debug: build
    lldb {{debug_dir}}/{{target_name}}

debug-test: build
    lldb {{debug_dir}}/{{test_target_name}}

build-docker:
    docker buildx build -t {{target_name}} .

run-docker:
    docker run --rm --name {{target_name}} {{target_name}}:latest
