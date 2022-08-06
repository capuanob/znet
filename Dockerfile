# Build Stage
FROM --platform=linux/amd64 ubuntu:20.04 as builder

## Install build dependencies.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y clang make

## Add source code to the build stage.
ADD . /znet
WORKDIR /znet

## Build fuzzer
RUN clang -O0 -g -fsanitize=fuzzer fuzz/fuzz_znet_server.c -o znet-fuzz

# Package Stage
FROM --platform=linux/amd64 ubuntu:20.04
COPY --from=builder /znet/znet-fuzz /znet-fuzz

CMD /znet-fuzz

