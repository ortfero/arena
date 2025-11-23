# syntax=docker/dockerfile:1

FROM ghcr.io/void-linux/void-musl-busybox:latest AS build

RUN xbps-install -Sy xbps && xbps-install -Syu
RUN xbps-install -Sy gcc ninja cmake python3 python3-pip git pkg-config
RUN pip3 install --no-cache-dir --break-system-packages conan
WORKDIR /app

COPY conanfile.txt CMakeLists.txt ./
COPY src src
COPY test test
COPY include include

RUN conan profile detect
RUN conan install . \
    -s build_type=RelWithDebInfo \
    -s compiler.cppstd=20 \
    -c tools.cmake.cmaketoolchain:generator=Ninja \
    --build=missing \
    --output-folder=.
RUN cmake --preset conan-relwithdebinfo
RUN cmake --build --preset conan-relwithdebinfo --target arena

FROM ghcr.io/void-linux/void-musl-busybox:latest AS runtime
RUN xbps-install -Sy xbps && xbps-install -Syu
RUN xbps-install -Sy libstdc++ libgcc
COPY --from=build /app/build/RelWithDebInfo/arena /usr/local/bin/arena
ENTRYPOINT ["arena"]