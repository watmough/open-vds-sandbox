FROM quay.io/pypa/manylinux2014_x86_64

RUN curl -sSL  -o ninja.zip "https://github.com/ninja-build/ninja/releases/download/v1.10.2/ninja-linux.zip"
RUN unzip ninja.zip && mv ninja /usr/local/bin/ && rm -vf ninja* && ln -s /usr/local/bin/ninja /usr/local/bin/ninja-build

RUN yum install -y vim less gdb java-1.8.0-openjdk-devel ccache perl perl-IPC-Cmd

RUN curl -LO https://www.openssl.org/source/openssl-3.0.8.tar.gz
RUN tar xzvf openssl-3.0.8.tar.gz
RUN cd openssl-3.0.8 && ./config --prefix=/opt/openssl3.0 no-module no-shared no-zlib && make -j && make install
RUN rm -rf openssl-3.0.8*

COPY python/requirements-dev-with-docs.txt /tmp/requirements-dev.txt

RUN /opt/python/cp36-cp36m/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp37-cp37m/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp38-cp38/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp39-cp39/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp310-cp310/bin/python -m pip install -r /tmp/requirements-dev.txt
RUN /opt/python/cp311-cp311/bin/python -m pip install -r /tmp/requirements-dev.txt

