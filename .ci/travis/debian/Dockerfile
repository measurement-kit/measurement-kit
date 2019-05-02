FROM debian:testing-slim AS build
MAINTAINER Simone Basso (bassosimone@gmail.com)
RUN apt-get update
RUN apt-get install -y autoconf automake clang cmake curl g++ git iproute2     \
      lcov libc++-dev libc++abi-dev libc-ares-dev libcurl4-openssl-dev         \
      libevent-dev libmaxminddb-dev libssl-dev libtool make ninja-build        \
      libgeoip-dev golang
RUN sed -i 's/DEFAULT@SECLEVEL=2/DEFAULT@SECLEVEL=1/g' /etc/ssl/openssl.cnf
RUN sed -i 's/MinProtocol = TLSv1.2/MinProtocol = TLSv1.0/g' \
    /etc/ssl/openssl.cnf
