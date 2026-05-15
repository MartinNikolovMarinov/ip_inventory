FROM ubuntu:24.04 AS build

# Install build dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
        git

# Removes the package index files downloaded by apt-get update -- this helps reduce the image size
RUN rm -rf /var/lib/apt/lists/*

# copies the entire repo int /src inside the container:
WORKDIR /src
COPY . .

# Configure
RUN cmake -S . -B /build \
        -DCMAKE_BUILD_TYPE=Release \
        -DIP_INVENTORY_BUILD_TESTS=OFF \
        -DIP_INVENTORY_SQLITE3_DB_ROOT=/db \
        -DIP_INVENTORY_SOURCE_ROOT= \
        -DIP_INVENTORY_LISTEN_ADDRESS=0.0.0.0

# Build
RUN cmake --build /build --parallel

# Starting from the initial clean version of the image:
FROM ubuntu:24.04

# Install run dependencies
RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates

# Removes the package index files downloaded by apt-get update -- this helps reduce the image size
RUN rm -rf /var/lib/apt/lists/*

# Copy the binary file
COPY --from=build /build/ip_inventory /usr/local/bin/ip_inventory

# Copy the required folders with static files
COPY schema /schema
COPY api /api
COPY gui /gui

# Create a volume for the database
VOLUME ["/db"]

# Expose 8080
EXPOSE 8080

# Run the binary
ENTRYPOINT ["/usr/local/bin/ip_inventory"]
