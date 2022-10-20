FROM docker.io/centos:7

RUN yum install -y centos-release-scl epel-release

RUN yum install -y rh-python36-python-devel rh-python36-numpy rh-python36-python-pip devtoolset-7

SHELL [ "/usr/bin/scl", "enable", "devtoolset-7", "rh-python36" ]

RUN curl -OL https://github.com/Kitware/CMake/releases/download/v3.22.0/cmake-3.22.0-linux-x86_64.tar.gz
RUN tar xzvf cmake-3.22.0-linux-x86_64.tar.gz -C /opt
RUN ln -s /opt/cmake-3.22.0-linux-x86_64/bin/* /usr/bin/

RUN pip install ninja scikit-build
RUN yum install -y git doxygen java-1.8.0-openjdk-devel libxml2-devel zlib-devel boost169-devel openssl-devel libcurl-devel libuv-devel libuuid-devel ccache

COPY python/requirements-dev.txt /tmp/requirements-dev.txt
RUN pip install -r tmp/requirements-dev.txt

COPY docker/build_scripts/centos7_build.sh /build_and_install_openvds.sh

RUN echo "unset BASH_ENV PROMPT_COMMAND ENV" > /root/scl_enable
RUN echo "source scl_source enable devtoolset-7 rh-python36" >> /root/scl_enable
ENV BASH_ENV=/root/scl_enable \
    ENV=/root/scl_enable \
    PROMPT_COMMAND=". /root/scl_enable"

# cmake -DCMAKE_BUILD_TYPE=Debug -GNinja -DBOOST_INCLUDEDIR=/usr/include/boost169 -DBOOST_LIBRARYDIR=/usr/lib64/boost169 ..
