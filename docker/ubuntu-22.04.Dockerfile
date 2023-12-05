FROM ubuntu:22.04

#ENV DEBIAN_FRONTEND="noninteractive" TZ="Europe/London"
RUN apt-get update
RUN apt-get install -y build-essential
RUN apt-get install -y git cmake ninja-build
RUN apt-get install -y python3-dev python3-pip
RUN apt-get install -y openjdk-8-jdk
RUN apt-get install -y doxygen

# Install requirements for python development
RUN python3 -m pip install --upgrade pip
COPY python/requirements-dev-with-docs.txt /tmp/requirements-dev.txt
RUN python3 -m pip install -r tmp/requirements-dev.txt

COPY docker/build_scripts/generic_linux_build.sh /build_and_install_openvds.sh
