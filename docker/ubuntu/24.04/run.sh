#!/bin/sh
docker build -t "hostie-ubuntu-2404" "docker/ubuntu/24.04"                       
docker run --net=host -it --rm -v `pwd`:/opt/shared hostie-ubuntu-2404 bash shared/packages/build-packages.sh 24.04
