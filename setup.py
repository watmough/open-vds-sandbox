#!/usr/bin/env python3
""" Open-source implementation of the Volume Data Store (VDS) standard for fast random access to multi-dimensional volumetric data.

OpenVDS is a specification and an open source reference implementation of a
storage format for fast random access to multi-dimensional (up to 6D)
volumetric data stored in an object storage cloud service (e.g.
Amazon S3, Azure Blob storage or Google Cloud Storage). The specification is
based on, but not similar to, the existing Volume Data Store (VDS) file format.
The VDS format is a Bluware Inc. proprietary format, which has seen extensive
industrial deployments over the last two decades. The design of the VDS format
is contributed by Bluware Inc. to the Open Group Open Subsurface Data Universe
Forum (OSDU) (The Open Group, u.d.).

OpenVDS has been designed to handle extremely large volumes, up to petabytes in
size, with variable sized compressed bricks. The OpenVDS format is very
flexible and can store any kind data representable as arrays with
key/value-pair metadata. In particular, data commonly used in seismic
processing can be stored along with all necessary metadata. This makes it
possible to go from legacy formats to OpenVDS and back, while retaining all
metadata.

OpenVDS may be used to store E&P data types such as regularized single-Z
horizons/height-maps (2D), seismic lines (2D), pre-stack volumes (3D-5D),
post-stack volumes (3D), geobody volumes (3D-5D), and attribute volumes of any
dimensionality up to 6D.
The format has been designed primarily to support random access and on-demand
fetching of data, this enables applications that are responsive and interactive
as well as efficient I/O for high-performance computing or machine learning
workloads.

"""
DOCLINES = (__doc__ or '').split("\n")

import sys
import os
import re
from pathlib import Path

if sys.version_info[0] < 3:
    raise Exception("OpenVDS only supports Python 3")

openvds_version=""
dir_path = os.path.dirname(os.path.realpath(__file__))
with open(dir_path + "/VERSION", "r") as file:
    for line in file:
             openvds_version = line.strip()
             break

if not openvds_version:
    print("Fatal error: Failed to parse version from VERSION")
    exit(1)

try:
    from skbuild import setup
except ImportError:
    print('scikit-build is required to build from source.', file=sys.stderr)
    print('Please run:', file=sys.stderr)
    print('', file=sys.stderr)
    print('  python -m pip install scikit-build')
    sys.exit(1)

python_root_path = "-DPython3_ROOT_DIR={}".format(Path(sys.executable).parent.absolute().as_posix())
setup(
    name="openvds",
    version=openvds_version,
    maintainer="OpenVDS Developers",
    maintainer_email="openvds@bluware.com",
    url="https://bluware.com/data-solutions/vds",
    download_url="https://bluware.jfrog.io/ui/native/Releases-OpenVDSPlus/2.1",
    project_urls={
        "Bug Tracker": "https://community.opengroup.org/osdu/platform/domain-data-mgmt-services/seismic/open-vds/-/issues",
        "Documentation": "https://osdu.pages.opengroup.org/platform/domain-data-mgmt-services/seismic/open-vds/",
        "Source Code": "https://community.opengroup.org/osdu/platform/domain-data-mgmt-services/seismic/open-vds"
    },
    description=DOCLINES[0],
    long_description="\n".join(DOCLINES[2:]),
    platforms=["Windows", "Linux"],
    author='The Open Group / Bluware, Inc.',
    license="Apache License, Version 2.0",
    packages=['openvds'],
    package_dir={'': 'python'},
    cmake_args=[python_root_path, "-DENABLE_MSVC_TOOLSET_DIR=OFF"],
    install_requires=['numpy']

)
