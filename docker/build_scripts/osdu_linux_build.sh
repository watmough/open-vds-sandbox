#!/bin/sh
REPO='open-vds'
if [ $# -gt 0 ]
  then
    REPO=$1
    shift
fi
cd $REPO
mkdir build_release
cd build_release
cmake -DCMAKE_BUILD_TYPE=Release "$@" -GNinja -DCMAKE_INSTALL_PREFIX=../../$REPO-install ..
ninja install
