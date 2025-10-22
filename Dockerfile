FROM ubuntu:22.04

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    g++ \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY *.cpp *.h CMakeLists.txt ./
COPY openings/ ./openings/

RUN mkdir build && cd build && \
    cmake -G "Unix Makefiles" .. && \
    cmake --build .

EXPOSE 10000

CMD ["./build/chess_server"]
