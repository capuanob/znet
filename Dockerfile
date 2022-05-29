# Build Stage
FROM --platform=linux/amd64 ubuntu:20.04 as builder

## Install build dependencies.
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y clang make git libc6-dbg netcat

## Add source code to the build stage.
ADD https://api.github.com/repos/capuanob/znet/git/refs/heads/mayhem version.json
RUN git clone -b mayhem https://github.com/capuanob/znet.git
WORKDIR /znet

## Build fuzzer
RUN clang -O0 -g -fsanitize=fuzzer fuzz/fuzz.c -o znet-fuzz

## Package dependencies
RUN mkdir /deps
RUN cp `ldd znet-fuzz | grep so | sed -e '/^[^\t]/ d' | sed -e 's/\t//' | sed -e 's/.*=..//' | sed -e 's/ (0.*)//' | sort | uniq` /deps 2>/dev/null || :

# Package Stage
FROM --platform=linux/amd64 ubuntu:20.04
RUN apt-get update && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y libc6-dbg netcat
COPY --from=builder /znet/znet-fuzz /znet-fuzz
COPY --from=builder /deps /usr/lib

CMD /znet-fuzz -close_fd_mask=2

