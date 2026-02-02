# Use Ubuntu as base image for C++ development
FROM ubuntu:22.04

# Prevent interactive prompts during package installation
ENV DEBIAN_FRONTEND=noninteractive

# Install build essentials and development tools
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    make \
    cmake \
    gdb \
    vim \
    && rm -rf /var/lib/apt/lists/*

# Set working directory
WORKDIR /app

# Copy all project files
COPY . /app/

# Default to bash shell for development
CMD ["/bin/bash"]