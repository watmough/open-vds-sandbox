.. _vds_metadata:

Metadata
--------

The VDS object annotates each dimension with a name, a unit, starting and ending coordinate. For example, a seismic
dataset with a certain number of samples in the time domain will annotate the trace dimension (typically dimension 0)
with "Sample", "ms", start time, and end time.

Each value channel is annotated with a name, a unit and an estimated value range (e.g. to use a transfer function to
show the value as a color).

The VDS system does not deal directly with spatial coordinate systems, it only defines an N-dimensional array of voxels.
Spatial coordinates are added through key/value-pair metadata that define the transformation from annotation coordinates
to a coordinate reference system.

Although VDS is a general volumetric format, for applications to properly understand the contents of a VDS dataset, the
OpenVDS subcommittee has defined a set of predefined metadata categories, and properties within each category. All OSDU
ingestion/delivery tools will be required to adhere to the list of predefined OpenVDS metadata names and types, to
ensure compatibility across OSDU implementations/instances/applications.

In the URL encoding scheme described above, the metadata described in this section are part of the VolumeDataLayout JSON
object.

Metadata Types
^^^^^^^^^^^^^^

Axis descriptors
~~~~~~~~~~~~~~~~

Each axis of a volume has a name, unit and annotation start/stop coordinates. The set of recognized axis descriptors
given in the name[unit] format. Where multiple options are accepted, the different options are separated by the ‘|’
character. The recognized axis descriptors are:

===============  ==================
Descriptor       Unit
===============  ==================
Inline           [unitless]
Crossline        [unitless]
CDP              [unitless]
Gather           [unitless]
Trace            [unitless]
Trace (offset)   [m|ft|ussft]
Trace (angle)    [deg|rad]
Trace (azimuth)  [deg|rad]
Sample           [ms|s|m|ft|ussft]
Shot             [unitless]
Receiver         [unitless]
Frequency        [Hz]
Time             [ms|s]
Depth            [m|ft|ussft]
Velocity         [m/s|ft/s|ussft/s]
Easting          [m|ft|ussft]
Northing         [m|ft|ussft]
===============  ==================

Key/Value pairs
~~~~~~~~~~~~~~~

“Key/Value pairs” are how you typically store single instance values, or simple arrays of data which is true for the
whole dataset and all of the channels. Examples include Survey Name, Survey Coordinate Systems, etc. In OpenVDS
key/value pairs are also associated with a category which gives a structure to key/value pairs that relate to each
other.

These key/value pairs can be of the following types:

=============  =========================================================
Type           Description
=============  =========================================================
Int            An integer type
IntVector2     A 2-component integer vector type
IntVector3     A 3-component integer vector type
IntVector4     A 4-component integer vector type
Float          A floating point type
FloatVector2   A 2-component floating point vector type
FloatVector3   A 3-component floating point vector type
FloatVector4   A 4-component floating point vector type
Double         A double precision floating point type
DoubleVector2  A 2-component double precision vector type
DoubleVector3  A 3-component double precision vector type
DoubleVector4  A 4-component double precision vector type
String         A string type (UTF-8)
BLOB           A base-64 encoded binary large object type
=============  =========================================================

Channel Descriptor
~~~~~~~~~~~~~~~~~~

Named channels are useful for storing additional information for individual voxel locations and/or individual trace
locations. Examples include Angles and/or Offsets, Trace Header, Trace number, Trace Coordinate, Mute, etc. Each named
channel is defined by the name, unit and one of two mapping types:

1. Volume Data

  This means that said named channel has the same dimensionality as the primary channel, so each value in the primary channel has a corresponding value in the named channel.
  This is useful for things like dip/azimuth for the corresponding seismic voxel.

2. Per Trace
   
   This means that said named channel has 1 dimension less than the primary channel, thus a set of values (each entry
   can be an array) is valid for a whole trace in the primary channel. This is useful for trace headers in SEGY, trace
   mute, etc.

Recognized Volume Types
^^^^^^^^^^^^^^^^^^^^^^^

The set of axis descriptors defines the volume type. In the following table the axis descriptors are listed as
Name[unit] dimension 0/…/Name[unit] dimension N, where dimension 0 is the fastest running indices (i.e. consecutive
values in memory). Where multiple options are accepted, the different options are separated by the ‘|’ character. The
recognized volume types are:

============  =====================================================================================================
Volume Type   Axis Descriptors
============  =====================================================================================================
3D Poststack  Sample|Time|Depth/Crossline/Inline
3D Prestack   Sample|Time|Depth/Trace(offset)|Velocity|Frequency/Crossline/Inline
2D Poststack  Sample|Time|Depth/Gather|CDP|Shot|Receiver
2D Prestack   Sample|Time|Depth/Trace(offset)|Velocity|Frequency/Gather|CDP|Shot|Receiver
3D Poststack  Horizon: Crossline/Inline, primary data channel is Sample|Time|Depth
3D Prestack   Horizon: Trace(offset)|Velocity|Frequency/Crossline/Inline, primary data channel is Sample|Time|Depth
============  =====================================================================================================

Recognized Key/Value Pairs
^^^^^^^^^^^^^^^^^^^^^^^^^^

The following is the recognized key/value categories and properties:

Category - ``SurveyCoordinateSystem``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This category of OpenVDS metadata contains two families that provide information to position a dataset in a coordinate
reference system.

In the absence of any of these families, a default setting is considered. In the following, these metadata families are
explained.

1. The Inline/Crossline system

  In this system a DoubleVector2 defines the origin, and two more DoubleVector2 define the inline and crossline spacing.
  These are applied to transform the OpenVDS dimensions with name Inline/Crossline to Easting/Northing coordinates. The
  Z coordinate is defined by the axis descriptor for the first dimension of the volume.

2. The 3D IJK System

  The Inline/Crossline system has flexibility for only two dimensions. In order to have more freedom, the 3DIJK metadata
  is defined. A DoubleVector3 is used to represent the origin and three step vectors that corresponded to the dimensions
  named "I", "J" and "K" respectively.

3. The Default System
   
   Other dimension names that are recognized and transformed to XYZ coordinates are X, Y and Z, which will be mapped
   directly to the corresponding XYZ coordinate.

.. table:: Keys in the SurveyCoordinateSystem category

  ================  =============  ==================================================================================================
  Key               Type           Description
  ================  =============  ==================================================================================================
  CrosslineSpacing  DoubleVector2  The XY spacing between units in the Crossline annotation dimension.
  CRSWkt            String         The appropriate OpenGIS Well Known Text description of the coordinate system used.
  InlineSegments    BLOB           An array of IntVector3 defining the inline, and crossline start and end for each inline segment.
  InlineSpacing     DoubleVector2  The XY spacing between units in the Inline annotation dimension.
  IStepVector       DoubleVector3  The step vector corresponded to dimension named 'I'.
  JStepVector       DoubleVector3  The step vector corresponded to dimension named 'J'.
  KStepVector       DoubleVector3  The step vector corresponded to dimension named 'K'.
  LatticeScale      Int            Scaling factor from SEG-Y import used on X/Y coordinates.
  Origin            DoubleVector2  The XY position of the origin of the annotation (Inline/Crossline/Time) coordinate system.
  Origin3D          DoubleVector3  The XYZ position of the origin of the annotation (I/J/K) coordinate system.
  Unit              String         String of the Lattice unit used, typically meter, decimeter, centimeter, kilometer, foot, or mile.
  ================  =============  ==================================================================================================

Category - ``SEGY``
~~~~~~~~~~~~~~~~~~~

The metadata in this category is only meant for round-tripping original SEG-Y data, and not for application parsing. The
SEG-Y specific metadata allow for exporting a SEG-Y identical to the original SEG-Y. Note that bitwise identity cannot
be achieved with Wavelet compressed files unless the WaveletLossless method is applied — a certain signal-loss
corresponding to the compression tolerance is to be expected.

.. table:: Keys in the SEGY category

  ================  =============  ==========================================================================================================================================================================================================================================
  Key               Type           Description
  ================  =============  ==========================================================================================================================================================================================================================================
  BinaryHeader      BLOB           The original SEG-Y binary header (400 bytes).
  TextHeader        BLOB           The original SEG-Y textual header (3200 bytes x binary header record count)
  PrimaryKey        String         The primary sort key of the original SEG-Y. For crossline-sorted files, the PrimaryKey will be ‘Crossline’ to indicate that this was the original order of the file even if the VDS has been normalized to a Time/Crossline/Inline volume.
  ================  =============  ==========================================================================================================================================================================================================================================

Category – ``TraceCoordinates``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A seismic line may populate the PositionProperty, VerticalOffsetProperty, EnergySourcePointNumberProperty and
EnsembleNumberProperty from metadata BLOBs found in the "TraceCoordinates" category.

The PositionProperty and VerticalOffsetProperty define the position of a seismic line.

.. table:: Keys in the TraceCoordinates category

  ========================  =============  ==========================================================================================================================================
  Key                       Type           Description
  ========================  =============  ==========================================================================================================================================
  TracePositions            BLOB           An array of DoubleVector2 defining the position for each trace, where (0, 0) is treated as an undefined position.
  TraceVerticalOffsets      BLOB           An array of doubles defining the offset for each trace from the vertical start position in the Time/Depth/Sample dimension of the OpenVDS.
  EnergySourcePointNumbers  BLOB           An array of scalar int32 values defining the energy source point number for each trace.
  EnsembleNumbers           BLOB           An array of scalar int32 values defining the ensemble number for each trace.
  ========================  =============  ==========================================================================================================================================

Category - ``ImportInformation``
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This category of VDS metadata contains information about the initial import to VDS. That is, information about the
original file (file name, last modification time etc.) and when/how it was imported. The intended use is e.g., to give a
default file name for an export operation or to inform the user about whether the VDS was imported from some particular
file.

.. table:: Keys in the ImportInformation category

  ========================  =============  =====================================================================================================================================================================================
  Key                       Type           Description
  ========================  =============  =====================================================================================================================================================================================
  DisplayName               String         An informative name (e.g. the survey name) that can be displayed to a user but is not necessarily a valid file name.
  InputFileName             String         The original input file name. In cases where the input is not a simple file this should still be a valid file name that can be used as the default for a subsequent export operation.
  InputFileSize             Double         The total size (in bytes) of the input file(s), which is an integer stored as a double because there is no 64-bit integer metadata type.
  InputTimeStamp            String         The last modified time of the input in ISO8601 format.
  ImportTimeStamp           String         The time in ISO8601 format when the data was imported to VDS.
  ========================  =============  =====================================================================================================================================================================================


Named Channels
^^^^^^^^^^^^^^

The following are known volume data channels:

==================  =====================================================
Name                Description
==================  =====================================================
Amplitude           Amplitude values are stored as float values.
Semblance           Semblance values are stored as float values.
Frequency           Frequency values are stored as float values.
Vrms/Vint/Vavg      Velocity values are stored as float values.
Intercept/Gradient  Intercept/Gradient values are stored as float values.
==================  =====================================================


The following are known Per Trace channels:

==================  ================================================================================================
Name                Description
==================  ================================================================================================
Mute                Mute values are stored as 2-component 16-bits values, representing mute start time and end time.
Offset              Offset values are stored as float values
Trace               Trace is a bool-mask if trace was present or not during conversion
Azimuth             Azimuth values are stored as float values
Angle               Angle values are stored as float values
SEGYTraceHeader     SEG-Y Trace headers are stored as 240x byte values
==================  ================================================================================================