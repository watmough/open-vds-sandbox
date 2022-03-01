FROM quay.io/pypa/manylinux2014_x86_64

RUN curl -sSL  -o ninja.zip "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip"
RUN unzip ninja.zip && mv ninja /usr/local/bin/ && rm -vf ninja* && ln -s /usr/local/bin/ninja /usr/local/bin/ninja-build

RUN yum install -y vim less gdb java-1.8.0-openjdk-devel libxml2-devel zlib-devel boost169-devel openssl-devel libcurl-devel libuv-devel libuuid-devel ccache

COPY python/requirements-dev-with-docs.txt /tmp/requirements-dev.txt

RUN /opt/python/cp36-cp36m/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp37-cp37m/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp38-cp38/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp39-cp39/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp310-cp310/bin/python -m pip install -r /tmp/requirements-dev.txt

COPY docker/build_scripts/centos7_build.sh /build_and_install_openvds.sh
# cmake -DCMAKE_BUILD_TYPE=Debug -GNinja -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 ..
