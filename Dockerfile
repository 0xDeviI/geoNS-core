FROM alpine:latest

# Install the required libraries and tools
RUN apk update && apk add make clang pkgconfig sqlite-dev file-dev curl-dev musl-dev libmagic

# Enable core dumps
RUN echo "kernel.core_pattern=/app/core.%e.%p.%h.%t" >> /etc/sysctl.conf && \
    echo "kernel.core_uses_pid=1" >> /etc/sysctl.conf && \
    ulimit -c unlimited

# Set ulimit for core dumps (unlimited size)
RUN echo "ulimit -n 65535" >> /etc/profile && \
    echo "ulimit -u unlimited" >> /etc/profile

# Create app directory
WORKDIR /src

# Copy the entire proejct directory
COPY . ./

# Compiling the source code
RUN make

# Make sure the binary is executable
RUN chmod +x ./bin/geoNS-core

# Create necessary directories that might be created by the app
# RUN mkdir -p /app/bin/logs

# Expose the port the app runs on
EXPOSE 8000
EXPOSE 9060

# Copy the build
RUN cp -r ./bin /app

# Create public web directory
RUN mkdir -p /app/www

# Set the working directory to bin for relative paths
WORKDIR /app

# Default command (can be overridden in docker-compose)
CMD ["./geoNS-core", "-sd"]