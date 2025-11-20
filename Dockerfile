FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    libpq-dev \
    postgresql-client \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy source files
COPY *.cpp *.h CMakeLists.txt ./

# Copy external libraries (ONNX Runtime)
COPY external/ ./external/

# Copy trained model
COPY models/ ./models/

RUN mkdir build && cd build && \
    cmake -G "Unix Makefiles" .. && \
    cmake --build .

EXPOSE 10000

CMD ["./build/chess_server"]
