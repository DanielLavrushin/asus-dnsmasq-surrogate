# Use Debian as the base image
FROM debian:bullseye

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=Etc/UTC

# Update and install dependencies
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y \
    wget \
    git \
    build-essential \
    cmake \
    gdb \
    texinfo \
    gawk \
    bison \
    gettext \
    sed \
    python3-dev \
    python3-pip \
    qemu-user-static \
    binutils-arm-linux-gnueabihf \
    gcc-arm-linux-gnueabihf \
    g++-arm-linux-gnueabihf \
    libc6-dev-armhf-cross \
    && pip3 install pexpect \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

# Download and compile glibc 2.32 for 32-bit ARM
RUN wget http://ftp.gnu.org/gnu/libc/glibc-2.32.tar.gz && \
    tar -xvzf glibc-2.32.tar.gz && \
    mkdir /glibc-build && cd /glibc-build && \
    ../glibc-2.32/configure --prefix=/usr/arm-linux-gnueabihf --host=arm-linux-gnueabihf --build=x86_64-linux-gnu && \
    make -j$(nproc) && make install

# Set environment variables for cross-compilation
ENV PATH=/usr/arm-linux-gnueabihf/bin:$PATH
ENV LD_LIBRARY_PATH=/usr/arm-linux-gnueabihf/lib:$LD_LIBRARY_PATH

# Set working directory
WORKDIR /workspace

# Run your build script
CMD ["./build.sh"]
