FROM quay.io/pypa/manylinux_2_24_x86_64

RUN curl -sSL  -o ninja.zip "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip"
RUN unzip ninja.zip && mv ninja /usr/local/bin/ && rm -vf ninja* && ln -s /usr/local/bin/ninja /usr/local/bin/ninja-build

ENV DEBIAN_FRONTEND="noninteractive" TZ="Europe/London"
RUN apt-get update
RUN apt-get install -y vim libboost-dev git doxygen openjdk-8-jdk libxml2-dev zlib1g-dev libcurl4-openssl-dev libuv1-dev uuid-dev gdb bash libssl-dev libboost-all-dev

RUN /opt/python/cp36-cp36m/bin/python -m pip install scikit-build
RUN /opt/python/cp37-cp37m/bin/python -m pip install scikit-build
RUN /opt/python/cp38-cp38/bin/python -m pip install scikit-build
RUN /opt/python/cp39-cp39/bin/python -m pip install scikit-build
RUN /opt/python/cp310-cp310/bin/python -m pip install scikit-build

COPY docker/build_scripts/centos7_build.sh /build_and_install_openvds.sh
# cmake -DCMAKE_BUILD_TYPE=Debug -GNinja -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 ..
