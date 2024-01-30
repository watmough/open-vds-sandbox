FROM quay.io/pypa/manylinux_2_24_x86_64

RUN curl -sSL  -o ninja.zip "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip"
RUN unzip ninja.zip && mv ninja /usr/local/bin/ && rm -vf ninja* && ln -s /usr/local/bin/ninja /usr/local/bin/ninja-build

ENV DEBIAN_FRONTEND="noninteractive" TZ="Europe/London"

RUN sed -i 's@deb.debian.org@archive.kernel.org/debian-archive@g' /etc/apt/sources.list
RUN sed -i 's@security.debian.org@archive.kernel.org/debian-archive@g' /etc/apt/sources.list
RUN sed -i '/stretch-updates/d' /etc/apt/sources.list

RUN apt-get update
RUN apt-get install -y vim git doxygen openjdk-8-jdk gdb bash ccache perl libipc-run-perl

COPY python/requirements-dev-with-docs.txt /tmp/requirements-dev.txt

RUN /opt/python/cp36-cp36m/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp37-cp37m/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp38-cp38/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp39-cp39/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp310-cp310/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp311-cp311/bin/python -m pip install -r /tmp/requirements-dev.txt

ENV CMAKE_BUILD_PARALLEL_LEVEL=2