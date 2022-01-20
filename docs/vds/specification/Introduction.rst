.. _vds_specification_introduction:

============
Introduction
============

`Volume Data Store` (VDS) is a storage format for fast random access to multi-dimensional (up to 6D) volumetric data. The
VDS format has been developed by Bluware Inc. and has seen extensive industrial deployments over the last two decades.
The format is contributed by Bluware Inc. to the  Open Group Open Subsurface Data Universe Forum (OSDU) (The Open Group,
u.d.).

An open-source reference software implementation which can read and write VDS has been contributed by Bluware Inc. to
the OSDU. This implementation is named `OpenVDS` and supports several programming languages. It can be included in
software products with no encumberments towards Bluware Inc.

VDS has been designed to handle extremely large volumes, up to petabytes in size, with variable sized compressed bricks.
The VDS format is very flexible and can store any kind data representable as arrays with key/value-pair metadata. In
particular, data commonly used in seismic processing can be stored along with all necessary metadata. This makes it
possible to go from legacy formats to VDS and back, while retaining all metadata.

VDS files may be used to represent E&P data types such as regularized single-Z horizons/height-maps (2D), seismic lines
(2D), pre-stack volumes (3D-5D), post-stack volumes (3D), geobody volumes (3D-5D), and attributes volumes of any
dimensionality up to 6D.

The format has been designed primarily to support random access and on-demand fetching of data, this enables
applications that are responsive and interactive as well as efficient I/O for high-performance computing or machine
learning workloads.

.. note::

  While the VDS specification supports user-defined named metadata, :ref:`the Metadata chapter <vds_metadata>` describes
  the set of predefined required and optional metadata, as defined and managed by the OSDU Data Definitions
  subcommittee, for VDS data to be used in seismic applications. Additional named metadata types should be brought up
  with the OSDU Data Definitions subcommittee for inclusion in the predefined list.

There are two different storage backends for VDSs:

1. Single-file Container
   
   The single-file version of VDS contains all data (main data, auxiliary channels, metadata, indexes) in a
   single-file and is suitable for storing and retrieval on a filesystem.

2. Cloud Object Store
   
   The object store version is an adaptation of the single-file container for storage and retrieval on cloud-based object
   stores. Utilizing a similar structure in the object store version allows applications a trivial transition from local
   legacy type access to cloud-based object store access.


For the remainder of this document, a VDS dataset will refer to a single dataset organized as described by this
specification. The actual realization of the dataset will not be a single object when the data is organized in a
cloud-based object store. 

Terminology
-----------

- Metadata
  
  The term `metadata` used in this document refers to what is commonly called `header data`` or `name-value pair data`
  embedded within the OpenVDS format or any legacy format, e.g. nDI vt, SEG-Y etc. This is distinct and separate from
  the Open Subsurface Data Universe (OSDU) work product, work product component and file `metadata`. Clearly, specific,
  individual OSDU metadata values may be obtained from corresponding OpenVDS metadata values.

