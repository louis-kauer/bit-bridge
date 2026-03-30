FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive
ENV CXX=g++-14
ENV CC=gcc-14

# System packages
RUN apt-get update && apt-get install -y --no-install-recommends \
    g++-14 \
    gcc-14 \
    cmake \
    make \
    libwxgtk3.2-dev \
    libyaml-cpp-dev \
    libboost-dev \
    wget \
    ca-certificates \
    git \
    && rm -rf /var/lib/apt/lists/*

# clang-tidy 19 from LLVM repo
RUN mkdir -p /etc/apt/keyrings \
    && wget -qO- https://apt.llvm.org/llvm-snapshot.gpg.key | tee /etc/apt/keyrings/llvm.asc > /dev/null \
    && echo "deb [signed-by=/etc/apt/keyrings/llvm.asc] https://apt.llvm.org/noble/ llvm-toolchain-noble-19 main" | tee /etc/apt/sources.list.d/llvm-19.list \
    && apt-get update && apt-get install -y --no-install-recommends clang-tidy-19 \
    && ln -sf /usr/bin/clang-tidy-19 /usr/bin/clang-tidy \
    && rm -rf /var/lib/apt/lists/*

# tomlplusplus v3.4.0
RUN git clone --depth 1 --branch v3.4.0 https://github.com/marzer/tomlplusplus.git /tmp/tomlplusplus \
    && cd /tmp/tomlplusplus && cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local && cmake --build build && cmake --install build \
    && rm -rf /tmp/tomlplusplus

# googletest v1.17.0
RUN git clone --depth 1 --branch v1.17.0 https://github.com/google/googletest.git /tmp/googletest \
    && cd /tmp/googletest && cmake -S . -B build -DCMAKE_INSTALL_PREFIX=/usr/local && cmake --build build && cmake --install build \
    && rm -rf /tmp/googletest