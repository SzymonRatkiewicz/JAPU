FROM alpine:3.14

WORKDIR /app

RUN apk add --no-cache gcc make libc-dev zlib-dev zlib-static

COPY . .
RUN mkdir -p /output
RUN make comp-stat
