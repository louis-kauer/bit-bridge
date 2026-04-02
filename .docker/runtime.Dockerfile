FROM ghcr.io/louis-kauer/bit-bridge/ci:latest AS build

WORKDIR /src
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14 \
    && cmake --build build --target bit_bridge_lb -j"$(nproc)"

FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    libstdc++6 \
    && rm -rf /var/lib/apt/lists/* \
    && useradd -r -s /usr/sbin/nologin bitbridge

COPY --from=build /src/build/core/bit_bridge_lb /usr/local/bin/bit_bridge_lb
COPY --from=build /usr/lib/*-linux-gnu/libyaml-cpp.so* /usr/lib/

USER bitbridge

ENTRYPOINT ["bit_bridge_lb"]
CMD ["/etc/bit-bridge/config.yaml"]
