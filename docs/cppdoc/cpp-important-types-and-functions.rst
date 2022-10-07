.. _cpp-important-types-and-functions:

C++ important types and functions
*********************************

.. doxygenfunction:: OpenVDS::Open(const OpenOptions &options, Error &error)

.. doxygenfunction:: OpenVDS::Open(std::string url, std::string connectionString, Error &error)

.. doxygenfunction:: OpenVDS::OpenWithAdaptiveCompressionTolerance(std::string url, std::string connectionString, float waveletAdaptiveTolerance, Error &error)
  
.. doxygenfunction:: OpenVDS::Create(const OpenOptions &options, VolumeDataLayoutDescriptor const &layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, MetadataReadAccess const &metadata, CompressionMethod compressionMethod, float compressionTolerance, Error &error)

.. doxygenfunction:: OpenVDS::Create(std::string url, std::string connectionString, VolumeDataLayoutDescriptor const &layoutDescriptor, std::vector<VolumeDataAxisDescriptor> axisDescriptors, std::vector<VolumeDataChannelDescriptor> channelDescriptors, MetadataReadAccess const &metadata, CompressionMethod compressionMethod, float compressionTolerance, Error &error)

.. doxygenfunction:: OpenVDS::Close(VDSHandle handle, bool flush)

.. doxygenstruct:: OpenVDS::AWSOpenOptions
  :members:

.. doxygenstruct:: OpenVDS::AzureOpenOptions
  :members:

.. doxygenstruct:: OpenVDS::GoogleOpenOptions
  :members:

.. doxygenfunction:: OpenVDS::GetAccessManager(VDSHandle handle)

.. doxygenfunction:: OpenVDS::GetLayout(VDSHandle handle)

.. doxygenclass:: OpenVDS::VolumeDataAccessManager
  :members:

.. doxygenclass:: OpenVDS::VolumeDataLayout
  :members:

.. doxygenclass:: OpenVDS::VolumeDataAxisDescriptor
  :members:

.. doxygenclass:: OpenVDS::VolumeDataChannelDescriptor
  :members:

.. doxygenclass:: OpenVDS::VolumeDataLayoutDescriptor
  :members:

.. doxygenclass:: OpenVDS::MetadataContainer
  :members:

.. doxygenclass:: OpenVDS::MetadataReadAccess
  :members:

.. doxygenclass:: OpenVDS::MetadataWriteAccess
  :members:
