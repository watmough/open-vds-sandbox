.. _vds_cloud_format:

====================
Cloud Storage Format
====================

When VDS data are stored in a cloud object store, such as AWS S3 or Azure BLOB Store, each chunk is represented by a
separate object. A VDS will therefore consist of numerous separate objects, rather than a single file.

Chunk-metadata Pages
--------------------

In order to allow efficient detection of constant value chunks without the overhead of making a read/request, the chunk-metadata
can also be stored as a collection of pages with a fixed number of entries per page (defined in the LayerStatus JSON object) and
each of these pages is written as a separate object. For Wavelet compressed layers, the size of the chunk object is prepended to the chunk-metadata when storing the entry in the chunk-metadata page. This is used
when interpreting the adaptive levels metadata in order to figure out how much of the chunk data is necessary to download in order to decompress a particular level.

In order to support sparse VDSs, there is an optional page directory in the layer status
which gives the stored index of each page (which can be different from the page's index) or -1 if no page exists (which means no chunks on that page have been written). If the layer status indicates that
the layer has chunk-metadata pages but no page directory exists, all metadata pages are assumed to exist as object with their index as the name of the object.

Layers that don't have chunk-metadata pages are required to store chunk-metadata in each chunk object using the metadata mechanism of the cloud object store. It is an error
to have mismatching chunk-metadata in the chunk object and the chunk-metadata page, usually caused by an incomplete write/copy of the VDS. In order to fix this error, the chunk-metadata page
will have to be updated with the chunk-metadata from the chunk object.

URL Encoding
------------

All VDS objects will be accessed from a base URL, usually formed from a bucket URL and a GUID for the dataset. From this
URL you can get the VolumeDataLayout and LayerStatus JSON objects by appending ``/VolumeDataLayout`` and
``/LayerStatus``. The binary chunk data are available through appending the layer name (found in the LayerStatus object,
e.g. Dimensions_012LOD0) and chunk index to the base URL. The binary chunk metadata is available through the layer name
with ``/ChunkMetadata`` and the stored metadata page index appended.
