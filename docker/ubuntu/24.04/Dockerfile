FROM ubuntu:24.04

WORKDIR /opt
ENV LD_LIBRARY_PATH /usr/local/lib
ENV DEBIAN_FRONTEND "noninteractive"
ENV TZ "Europe/London"

RUN apt-get -y --allow-unauthenticated update && \
    apt-get -y --allow-unauthenticated upgrade && \
    apt-get -y install curl \
  cmake \
  pkg-config \
  build-essential \
  devscripts \
  debhelper \
  libbz2-dev \
  libssl-dev \
  git

COPY build-build2.sh build-build2.sh
RUN bash build-build2.sh

RUN apt-get -y install libboost-all-dev

ENV BUILD_DIR hostie-package
RUN mkdir /opt/package-src
RUN bpkg create -d $BUILD_DIR cc config.cxx=g++ config.install.root=/opt/package-src
WORKDIR /opt/hostie-package
RUN bpkg add "https://github.com/crails-framework/hostie-cli.git#master"

RUN echo "Rebuild 30/05/2024#3"
RUN bpkg fetch && bpkg build --yes hostie-cli '?sys:libboost-program-options/*' '?sys:libboost-date-time/*' '?sys:libboost-process/*' '?sys:libboost-asio/*' '?sys:libcrypto/*' && bpkg build --yes crails-backup

WORKDIR /opt
