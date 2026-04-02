FROM ghcr.io/louis-kauer/bit-bridge/ci:latest AS build

WORKDIR /src
COPY . .

RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_CXX_COMPILER=g++-14 -DCMAKE_C_COMPILER=gcc-14 \
    && cmake --build build --target bit_bridge_lb -j"$(nproc)"

FROM gcr.io/distroless/cc-debian12:nonroot

COPY --from=build /src/build/core/bit_bridge_lb /usr/local/bin/bit_bridge_lb
COPY --from=build /usr/lib/*-linux-gnu/libyaml-cpp.so* /usr/lib/

ENTRYPOINT ["bit_bridge_lb"]
CMD ["/etc/bit-bridge/config.yaml"]
