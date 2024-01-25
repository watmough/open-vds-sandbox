FROM alpine:latest

# Install the various required packages
RUN apk add python3 python3-dev py3-pip gcc g++ cmake make ninja git ccache doxygen openjdk8 perl linux-headers

ENV JAVA_HOME=/usr/lib/jvm/java-1.8-openjdk
ENV PATH="$JAVA_HOME/bin:${PATH}"

# Copy the python requirement list into the container and download the packages
ENV PIP_BREAK_SYSTEM_PACKAGES=1
COPY python/requirements-dev-with-docs.txt /tmp/requirements-dev.txt
RUN pip3 install -r tmp/requirements-dev.txt

COPY docker/build_scripts/generic_linux_build.sh /build_and_install_openvds.sh

# These are the extra development packages
RUN apk add gdb vim bash

ENTRYPOINT ["/bin/bash"]