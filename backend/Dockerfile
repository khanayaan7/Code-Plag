FROM debian:bookworm AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
        build-essential \
        ca-certificates \
        cmake \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
COPY CMakeLists.txt ./
COPY src ./src

RUN cmake -S . -B build \
        -DCMAKE_BUILD_TYPE=Release \
        -DCODEPLAG_BUILD_TESTS=OFF \
    && cmake --build build --target codeplag_server --parallel

FROM debian:bookworm-slim AS runtime

RUN apt-get update \
    && apt-get install -y --no-install-recommends ca-certificates curl \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/codeplag_server /usr/local/bin/codeplag_server

ENV PORT=8080
ENV FRONTEND_ORIGIN=http://localhost:3000
EXPOSE 8080

HEALTHCHECK --interval=10s --timeout=3s --start-period=3s --retries=3 \
    CMD curl --fail --silent "http://localhost:${PORT}/health" > /dev/null || exit 1

USER nobody
CMD ["codeplag_server"]
