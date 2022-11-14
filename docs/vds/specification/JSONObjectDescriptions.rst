.. _vds_specification_JSON_object_descriptions:

============================
VDS JSON Object Descriptions
============================

The following chapter contains a specification of the JSON objects that describe a VDS dataset.

VolumeDataLayout
----------------

==================  ======  ========================================================================================================================================================================================================
Property            Type    Description
==================  ======  ========================================================================================================================================================================================================
axisDescriptors     Array   An array of AxisDescriptor objects that describe each axis of the dataset, the dimensionality of the dataset is determined by the number of axis descriptors in this list.
channelDescriptors  Array   An array of ChannelDescriptor objects that describe each data channel of the dataset. The first ChannelDescriptor is considered the primary channel of the dataset and is not allowed to have a mapping.
layoutDescriptor    Object  An object describing the overall partitioning of the dataset into chunks.
metadata            Array   An array of Metadata JSON objects that can be used to encode any additional information about the volume. See section about recognized metadata.
==================  ======  ========================================================================================================================================================================================================

AxisDescriptor
--------------

==================  ======  ===================================================================================
Property            Type    Description
==================  ======  ===================================================================================
numSamples          Int     The number of samples along this axis.
name                String  The name of this axis, see section about recognized names and units for dimensions.
unit                String  The unit of this axis, see section about recognized names and units for dimensions.
coordinateMin       Float   The annotation coordinate of the first sample.
coordinateMax       Float   The annotation coordinate of the last sample.
==================  ======  ===================================================================================

ChannelDescriptor
-----------------

+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Property              | Type        | Description                                                                                                                                                                                       |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| format                | Enumeration | The format for the samples in the data channel. Recognized values are:                                                                                                                            |
|                       |             | ``Format_R32``, ``Format_R64``, ``Format_U8``, ``Format_U16``, ``Format_U32``, ``Format_U64`` or ``Format_1Bit``.                                                                                 |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| components            | Enumeration | The number of components per sample in the data channel. Recognized values are: ``Components_1``, ``Component_2`` or ``Components_4``                                                             |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| name                  | string      | The name for the additional data channel, see section about recognized names and units for data channels.                                                                                         |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| unit                  | string      | The unit for the additional data channel, see section about recognized names and units for data channels.                                                                                         |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| valueRange            | Float[2]    | The estimated value range (min and max) for the additional data channel. This should not include outliers and is suitable for e.g., can be used to mapping values to colors for display purposes. |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| channelMapping        | Enumeration | Describes how this channel is mapped to the primary volume. Recognized values are: ``Direct``, ``PerTrace``.                                                                                      |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| mappedValues          | Int         | If the ChannelMapping property is set to PerTraceindicates a per-trace mapping, this a the number of data values in this data channel for each trace in the primary volumedata channel.           |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| discrete              | Boolean     | If this is ‘true’, the values should not be interpolated. For example, this is true for a classification volume as interpolating Zone IDs make no sense.                                          |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| allowLossyCompression | Boolean     | If this is ‘true’, the data values can be approximated by a value close to the true value.                                                                                                        |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| renderable            | Boolean     | If this is ‘true’, this data channel has LODs as specified in the LayoutDescriptor.                                                                                                               |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| useNoValue            | Boolean     | If this is ‘true’, there is a reserved value that indicates that a sample is missing.                                                                                                             |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| noValue               | Float       | This value (converted to the current voxel format using the NoValue conversion rules) is used to indicate a missing sample if UseNoValue is True.                                                 |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| integerScale          | Float       | This value is used to scale U8 and U16 integer formats when converting to floating point values.                                                                                                  |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| integerOffset         | Float       | This value is used to offset the scaled U8 and U16 integer format values when converting to floating point values.                                                                                |
+-----------------------+-------------+---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

LayoutDescriptor
----------------

+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| Property                     | Type        | Description                                                                                                                                                                                                                                                                                                                    |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| brickSize                    | Enumeration | The size of the bricks used for partitioning the layers. Recommended brick size for uncompressed volumes is 64 and for compressed volumes 128. Recognized values are: ``BrickSize_32``, ``BrickSize_64``, ``BrickSize_128``, ``BrickSize_256``, ``BrickSize_512``, ``BrickSize_1024``, ``BrickSize_2048``, ``BrickSize_4096``. |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| brickSize2DMultiplier        | Int         | The number of 3D bricks per 2D brick in the two dimensions of the 2D brick. Recommended value is 4, which means there are 16 3D bricks overlapping a 2D brick.                                                                                                                                                                 |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| create2DLODs                 | Boolean     | If this is ‘true’, layers with 2D bricks also have LODs. The default is that LODs are only created for layers with 3D bricks.                                                                                                                                                                                                  |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| forceFullResolutionDimension | Boolean     | If this is ‘true’, the fullResolutionDimension indicates that one of the dimensions of the volume is never LOD decimated.                                                                                                                                                                                                      |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| fullResolutionDimension      | Int         | If forceFullResolutionDimension is ‘true’, this property indicates which dimension of the volume that is never LOD decimated.                                                                                                                                                                                                  |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| lodLevels                    | Enumeration | The number of LOD levels of the volume. Recognized values are ``LODLevels_None``, ``LODLevels_1``, ``LODLevels_2`` and up to ``LODLevels_12``. Each LOD level has bricks that contain half as many values in each dimension (except the full resolution dimension, if specified) as the previous LOD level.                    |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| negativeMargin               | Int         | This is the amount of overlap with the previous brick preceding the valid area of the brick.                                                                                                                                                                                                                                   |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+
| positiveMargin               | Int         | This is the amount of overlap with the next brick following the valid area of the brick.                                                                                                                                                                                                                                       |
+------------------------------+-------------+--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------+

Metadata
--------

==================  ===========  =============================================================================================================================================================================================================================================
Property            Type         Description
==================  ===========  =============================================================================================================================================================================================================================================
type                Enumeration  Recognized values are: ``int``, ``IntVector2``, ``IntVector3``, ``IntVector3``, ``Float``, ``FloatVector2``, ``FloatVector3``, ``FloatVector4``, ``Double``, ``DoubleVector2``, ``DoubleVector3``, ``DoubleVector4``, ``String`` or ``BLOB``.
name                String       The name of the additional metadata. See section about recognized names and categories for additional metadata.
category            String       The category of the additional metadata. See section about recognized names and categories for additional metadata.
value               Variant      Depending of the type this is an integer, integer array, float, float array, double, double array, string or base-64 encoded binary object (BLOB).
==================  ===========  =============================================================================================================================================================================================================================================

LayerStatus
-----------

==========================  ===========  ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
Property                    Type         Description
==========================  ===========  ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
layerName                   String       The name of the layer used to form the URL of chunks and chunk metadata pages.
channelName                 String       The name of the data channel for this layer.
dimensionGroup              Enumeration  The dimensions used to form the chunks of this layer, e.g., ``Dimensions_012``.
lod                         Int          The LOD level of this layer.
produceStatus               Enumeration  The produce status of the layer. Recognized values are: ``Normal`` or ``Remapped``.
chunkCount                  Int          The total number of chunks in this layer.
validChunkCount (optional)  Int          The number of valid chunks in this layer.
hasChunkMetadataPages       Boolean      If ‘true’ it is possible to request chunk metadata pages with an URL using the layerName appended with /ChunkMetadata and the page number.
chunkMetadataPageSize       Int          The number of chunk metadata entries in each chunk metadata page.
pageDirectory (optional)    Int[]        The stored index of each page (which can be different from the page's index) or -1 if no page exists
chunkMetadataByteSize       Int          The size of each chunk metadata entry.
compressionMethod           Enumeration  This indicates which method has been used to serialize chunks in this layer. Recognized values are ``NONE``, ``Wavelet``, ``RLE``, ``Zip``, and ``WaveletLossless``.
compressionTolerance        Float        For Wavelet and WaveletLossless this indicates how much precision is kept after transforming the data to wavelet domain. A value of 1.0 is equivalent to 8 bits of precision, and each doubling of the tolerance represents a loss of ~1 bit of precision while each halving of the tolerance represents adding ~1 bit of precision. Note that since this is done in wavelet domain it does not directly translate to how much precision is lost from the original. The WaveletLossless method adds a entropy-encoded delta between the Wavelet compressed data and the original, so the tolerance will still affect the compression ratio achieved.
serializedSize (optional)   Int64        The combined serialized size of all non-empty chunks.
uncompressedSize            Int64        The combined original size of all non-empty chunks.
adaptiveLevelSizes          Int64[16]    The combined compressed size of each adaptive level (if applicable).
==========================  ===========  ====================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================================
