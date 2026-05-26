# Multi-stage build for ohtoai-webhook
FROM alpine:3.19 AS builder

RUN apk add --no-cache \
    cmake \
    make \
    g++ \
    openssl-dev \
    linux-headers \
    git

WORKDIR /src
COPY . .

RUN cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=OFF
RUN cmake --build build --config Release --target ohtoai-webhook -j$(nproc)

FROM alpine:3.19

RUN apk add --no-cache \
    libstdc++ \
    openssl \
    ca-certificates

COPY --from=builder /src/build/src/ohtoai-webhook /usr/local/bin/ohtoai-webhook

RUN mkdir -p /root/.ohtoai/ohtoai-webhook
WORKDIR /root/.ohtoai/ohtoai-webhook

EXPOSE 8080

ENTRYPOINT ["/usr/local/bin/ohtoai-webhook"]
