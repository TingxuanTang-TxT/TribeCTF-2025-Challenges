# Stage 1: Build a dynamically-linked QEMU and collect its dependencies
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install QEMU's build dependencies
RUN apt-get update && \
    apt-get install -y \
        git build-essential ninja-build pkg-config g++ \
        python3 python3-pip python3-venv \
        libglib2.0-dev libpixman-1-dev libmount-dev libblkid-dev && \
    pip install --no-cache-dir tomli && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /qemu
COPY ./qemu /qemu

# Configure and build a standard, dynamically-linked executable
RUN ./configure \
        --target-list=arm-softmmu \
        --enable-trace-backends=simple \
    && make -j$(nproc)

# After the build, collect the binary and all its shared library dependencies
RUN mkdir /dist && \
    # Copy the main binary
    cp /qemu/build/qemu-system-arm /dist/ && \
    # Use ldd to find all required .so files and copy them into /dist
    ldd /qemu/build/qemu-system-arm | grep "=> /" | awk '{print $3}' | xargs -I '{}' cp -v '{}' /dist/


# Stage 2: Prepare the final, minimal runtime environment
FROM ubuntu:22.04

# Disable Python's output buffering to see logs immediately
ENV PYTHONUNBUFFERED=1
# Set the library path so the linker can find our copied .so files
ENV LD_LIBRARY_PATH=/app/dist

# Install only the necessary runtime dependencies
RUN apt-get update && \
    apt-get install -y python3 && \
    rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the QEMU binary and all its required libraries from the builder stage
COPY --from=builder /dist /app/dist
COPY ./connection_manager.py .

# Update the QEMU_BINARY path in the Python script to point to our new location
# This assumes the original python script is in the build context
RUN sed -i 's|QEMU_BINARY = .*|QEMU_BINARY = "/app/dist/qemu-system-arm"|' connection_manager.py

CMD ["python3", "./connection_manager.py"]

