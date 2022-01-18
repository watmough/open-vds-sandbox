.. _vds_cloud_format:

====================
Cloud Storage Format
====================

When VDS data are stored in a cloud object store, such as AWS S3 or Azure BLOB Store, each chunk is represented by a
separate object. A VDS will therefore consist of numerous separate objects, rather than a single file.

URL Encoding
------------

All VDS objects will be accessed from a base URL, usually formed from a bucket URL and a GUID for the dataset. From this
URL you can get the VolumeDataLayout and LayerStatus JSON objects by appending ``/VolumeDataLayout`` and
``/LayerStatus``. The binary chunk data are available through appending the layer name (found in the LayerStatus object,
e.g. Dimensions_012LOD0) and chunk index to the base URL. The binary chunk metadata is available through the layer name
with ``/ChunkMetadata`` and the metadata page index appended.