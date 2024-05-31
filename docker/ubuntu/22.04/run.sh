#!/bin/sh
docker build -t "hostie-ubuntu-2204" "docker/ubuntu/22.04"                       
docker run --net=host -it --rm -v `pwd`:/opt/shared hostie-ubuntu-2204 bash shared/packages/build-packages.sh 22.04
