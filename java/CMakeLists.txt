set(CMAKE_DEBUG_POSTFIX "")
include(UseJava)

set(OPENVDS_JAVA_VERSION 0.0.6)

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.8" "-target" "1.8")

set(JAVA_SOURCE_FILES
	src/org/opengroup/openvds/AWSOpenOptions.java
	src/org/opengroup/openvds/AzureOpenOptions.java
	src/org/opengroup/openvds/AzurePresignedOpenOptions.java
	src/org/opengroup/openvds/AzureVDSGenerator.java
	src/org/opengroup/openvds/ByteBufferBackedObject.java
	src/org/opengroup/openvds/CompressionMethod.java
	src/org/opengroup/openvds/DimensionsND.java
	src/org/opengroup/openvds/DMSOpenOptions.java
	src/org/opengroup/openvds/DoubleMatrix3x3.java
	src/org/opengroup/openvds/DoubleMatrix4x4.java
	src/org/opengroup/openvds/DoubleRange.java
	src/org/opengroup/openvds/DoubleVector2.java
	src/org/opengroup/openvds/DoubleVector3.java
	src/org/opengroup/openvds/DoubleVector4.java
	src/org/opengroup/openvds/DownloadError.java
	src/org/opengroup/openvds/Error.java
	src/org/opengroup/openvds/FloatRange.java
	src/org/opengroup/openvds/FloatVector2.java
	src/org/opengroup/openvds/FloatVector3.java
	src/org/opengroup/openvds/FloatVector4.java
	src/org/opengroup/openvds/GlobalState.java
	src/org/opengroup/openvds/GoogleCredentialsJson.java
	src/org/opengroup/openvds/GoogleCredentialsPath.java
	src/org/opengroup/openvds/GoogleCredentialsSignedUrl.java
	src/org/opengroup/openvds/GoogleCredentialsSignedUrlJson.java
	src/org/opengroup/openvds/GoogleCredentialsSignedUrlPath.java
	src/org/opengroup/openvds/GoogleCredentialsToken.java
	src/org/opengroup/openvds/GoogleOpenOptions.java
	src/org/opengroup/openvds/HttpOpenOptions.java
	src/org/opengroup/openvds/IJKCoordinateTransformer.java
	src/org/opengroup/openvds/IJKGridDefinition.java
	src/org/opengroup/openvds/IndexRegionFloatVector2.java
	src/org/opengroup/openvds/IndexRegionFloatVector3.java
	src/org/opengroup/openvds/IndexRegionFloatVector4.java
	src/org/opengroup/openvds/IndexRegionIntVector2.java
	src/org/opengroup/openvds/IndexRegionIntVector3.java
	src/org/opengroup/openvds/IndexRegionIntVector4.java
	src/org/opengroup/openvds/InMemoryOpenOptions.java
	src/org/opengroup/openvds/InMemoryVDSGenerator.java
	src/org/opengroup/openvds/InterpolationMethod.java
	src/org/opengroup/openvds/IntRange.java
	src/org/opengroup/openvds/IntVector2.java
	src/org/opengroup/openvds/IntVector3.java
	src/org/opengroup/openvds/IntVector4.java
	src/org/opengroup/openvds/IOManager.java
	src/org/opengroup/openvds/KnownAxisNames.java
	src/org/opengroup/openvds/KnownMetadata.java
	src/org/opengroup/openvds/KnownUnitNames.java
	src/org/opengroup/openvds/ManagedBase.java
	src/org/opengroup/openvds/ManagedBuffer.java
	src/org/opengroup/openvds/MetadataContainer.java
	src/org/opengroup/openvds/MetadataKey.java
	src/org/opengroup/openvds/MetadataReadAccess.java
	src/org/opengroup/openvds/MetadataType.java
	src/org/opengroup/openvds/MetadataWriteAccess.java
	src/org/opengroup/openvds/NDPos.java
	src/org/opengroup/openvds/NDPosArray.java
	src/org/opengroup/openvds/OpenOptions.java
	src/org/opengroup/openvds/OpenVDS.java
	src/org/opengroup/openvds/PODArray.java
	src/org/opengroup/openvds/RequestFormat.java
	src/org/opengroup/openvds/UploadError.java
	src/org/opengroup/openvds/VDS.java
	src/org/opengroup/openvds/VDSError.java
	src/org/opengroup/openvds/VDSFileOpenOptions.java
	src/org/opengroup/openvds/VDSIJKGridDefinition.java
	src/org/opengroup/openvds/VDSProduceStatus.java
	src/org/opengroup/openvds/VolumeData2DInterpolatingAccessorR32.java
	src/org/opengroup/openvds/VolumeData2DInterpolatingAccessorR64.java
	src/org/opengroup/openvds/VolumeData2DReadAccessor1Bit.java
	src/org/opengroup/openvds/VolumeData2DReadAccessorR32.java
	src/org/opengroup/openvds/VolumeData2DReadAccessorR64.java
	src/org/opengroup/openvds/VolumeData2DReadAccessorU16.java
	src/org/opengroup/openvds/VolumeData2DReadAccessorU32.java
	src/org/opengroup/openvds/VolumeData2DReadAccessorU64.java
	src/org/opengroup/openvds/VolumeData2DReadAccessorU8.java
	src/org/opengroup/openvds/VolumeData2DReadWriteAccessor1Bit.java
	src/org/opengroup/openvds/VolumeData2DReadWriteAccessorR32.java
	src/org/opengroup/openvds/VolumeData2DReadWriteAccessorR64.java
	src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU16.java
	src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU32.java
	src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU64.java
	src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU8.java
	src/org/opengroup/openvds/VolumeData3DInterpolatingAccessorR32.java
	src/org/opengroup/openvds/VolumeData3DInterpolatingAccessorR64.java
	src/org/opengroup/openvds/VolumeData3DReadAccessor1Bit.java
	src/org/opengroup/openvds/VolumeData3DReadAccessorR32.java
	src/org/opengroup/openvds/VolumeData3DReadAccessorR64.java
	src/org/opengroup/openvds/VolumeData3DReadAccessorU16.java
	src/org/opengroup/openvds/VolumeData3DReadAccessorU32.java
	src/org/opengroup/openvds/VolumeData3DReadAccessorU64.java
	src/org/opengroup/openvds/VolumeData3DReadAccessorU8.java
	src/org/opengroup/openvds/VolumeData3DReadWriteAccessor1Bit.java
	src/org/opengroup/openvds/VolumeData3DReadWriteAccessorR32.java
	src/org/opengroup/openvds/VolumeData3DReadWriteAccessorR64.java
	src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU16.java
	src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU32.java
	src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU64.java
	src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU8.java
	src/org/opengroup/openvds/VolumeData4DInterpolatingAccessorR32.java
	src/org/opengroup/openvds/VolumeData4DInterpolatingAccessorR64.java
	src/org/opengroup/openvds/VolumeData4DReadAccessor1Bit.java
	src/org/opengroup/openvds/VolumeData4DReadAccessorR32.java
	src/org/opengroup/openvds/VolumeData4DReadAccessorR64.java
	src/org/opengroup/openvds/VolumeData4DReadAccessorU16.java
	src/org/opengroup/openvds/VolumeData4DReadAccessorU32.java
	src/org/opengroup/openvds/VolumeData4DReadAccessorU64.java
	src/org/opengroup/openvds/VolumeData4DReadAccessorU8.java
	src/org/opengroup/openvds/VolumeData4DReadWriteAccessor1Bit.java
	src/org/opengroup/openvds/VolumeData4DReadWriteAccessorR32.java
	src/org/opengroup/openvds/VolumeData4DReadWriteAccessorR64.java
	src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU16.java
	src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU32.java
	src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU64.java
	src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU8.java
	src/org/opengroup/openvds/VolumeDataAccessManager.java
	src/org/opengroup/openvds/VolumeDataAxisDescriptor.java
	src/org/opengroup/openvds/VolumeDataChannelDescriptor.java
	src/org/opengroup/openvds/VolumeDataComponents.java
	src/org/opengroup/openvds/VolumeDataFormat.java
	src/org/opengroup/openvds/VolumeDataLayout.java
	src/org/opengroup/openvds/VolumeDataLayoutDescriptor.java
	src/org/opengroup/openvds/VolumeDataMapping.java
	src/org/opengroup/openvds/VolumeDataPage.java
	src/org/opengroup/openvds/VolumeDataPageAccessor.java
	src/org/opengroup/openvds/VolumeDataRequest.java
	src/org/opengroup/openvds/VolumeDataRequest1Bit.java
	src/org/opengroup/openvds/VolumeDataRequestByte.java
	src/org/opengroup/openvds/VolumeDataRequestDouble.java
	src/org/opengroup/openvds/VolumeDataRequestFloat.java
	src/org/opengroup/openvds/VolumeDataRequestUInt.java
	src/org/opengroup/openvds/VolumeDataRequestULong.java
	src/org/opengroup/openvds/VolumeDataRequestUShort.java
	src/org/opengroup/openvds/WaveletAdaptiveMode.java

  #java/demo/AWSSliceDumpDemo.java
  #java/demo/CreateVDS.java
  #java/demo/OpenVdsDemo.java
  #java/demo/SliceDump.java

  #java/experimental/OpenVdsDemoWithArrayIntoBuffers.java
  #java/experimental/OpenVdsDemoWithBuffers.java
  #java/experimental/OpenVdsDemoWithSubset.java
  #java/experimental/OpenVdsDemoWithSubsetAndBytes.java
  #java/experimental/TestVariousAllocationAndTransmissionMethods.java
)


# Java compilation
# Enable JNI header generation for java >=8
add_jar(openvds-java
        SOURCES ${JAVA_SOURCE_FILES}
        VERSION ${CMAKE_PROJECT_VERSION}
#        GENERATE_NATIVE_HEADERS openvds-java-generated-headers DESTINATION ${GENERATED_JNI_HEADERS_DIR}
#        ENTRY_POINT OpenVdsDemo
		)

add_dependencies(openvds-java openvds)

get_property(BUILT_JAR_FILE TARGET openvds-java PROPERTY JAR_FILE)

if (BUILD_DOCS)
  set(JAVADOC_OPTIONS -d ${CMAKE_BINARY_DIR}/docs/html/javadoc -windowtitle '/"OpenVDS-Java/"' -doctitle '/"<h1>OpenVDS-Java</h1>/"' -author -use -version)
  add_custom_target(openvds-javadoc ALL
    COMMAND ${Java_JAVADOC_EXECUTABLE} ${JAVADOC_OPTIONS}
    ${JAVA_SOURCE_FILES}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
endif()


set(JAVA_TEST_SOURCES_FILES
	test/src/test/org/opengroup/openvds/CreateVDSTest.java
	test/src/test/org/opengroup/openvds/InMemoryVDSGeneratorTest.java
	test/src/test/org/opengroup/openvds/MetaDataContainerTest.java
	test/src/test/org/opengroup/openvds/PageAccessorByteTest.java
	test/src/test/org/opengroup/openvds/PageAccessorFloatTest.java
	test/src/test/org/opengroup/openvds/PageAccessorShortTest.java
	test/src/test/org/opengroup/openvds/TestTemplate.java
	test/src/test/org/opengroup/openvds/WriteDataTest.java
  )

add_jar(openvds-java-test
        SOURCES ${JAVA_TEST_SOURCES_FILES}
        VERSION ${CMAKE_PROJECT_VERSION}
        INCLUDE_JARS
        ${testng_SOURCE_FILE}
        ${BUILT_JAR_FILE}
)

add_library(openvds-javacpp
  cpp/AzureVdsGenerator.cpp
  cpp/CoordinateTransformer.cpp
  cpp/Exceptions.cpp
  cpp/GlobalMetadataCommon.cpp
  cpp/GlobalState.cpp
  cpp/IJKCoordinateTransformer.cpp
  cpp/InMemoryVDSGenerator.cpp
  cpp/KnownMetadata.cpp
  cpp/Marshaling.cpp
  cpp/Marshaling.h
  cpp/MetadataAccess.cpp
  cpp/MetadataContainer.cpp
  cpp/MetadataKey.cpp
  cpp/NDVector.h
  cpp/OpenVDS.cpp
  cpp/OpenVDSJava
  cpp/Optional.cpp
  cpp/ValueConversion.cpp
  cpp/VDSWriteData.cpp
  cpp/VolumeData.cpp
  cpp/VolumeDataAccess.cpp
  cpp/VolumeDataAccessManager.cpp
  cpp/VolumeDataAxisDescriptor.cpp
  cpp/VolumeDataChannelDescriptor.cpp
  cpp/VolumeDataLayout.cpp
  cpp/VolumeDataLayoutDescriptor.cpp
  cpp/VolumeSampler.cpp
)

set_target_properties(openvds-javacpp PROPERTIES
  FOLDER java
  VERSION ${CMAKE_PROJECT_VERSION}
)
copyDllForTarget(openvds-javacpp)
#target_include_directories(openvds-javacpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/cpp/include)
target_include_directories(openvds-javacpp PUBLIC ${JAVA_INCLUDE_PATH} ${JAVA_INCLUDE_PATH2})
target_include_directories(openvds-javacpp PRIVATE ${PROJECT_SOURCE_DIR}/src/OpenVDS ${PROJECT_SOURCE_DIR}/src/OpenVDS/OpenVDS ${COMMON_DIR})
target_link_libraries(openvds-javacpp PRIVATE openvds) # openvds-java-generated-headers)

# Install jar and natives
install(TARGETS openvds-javacpp
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}${LIB_TOOLSET_DIR}
  LIBRARY 
    DESTINATION ${CMAKE_INSTALL_LIBDIR}${LIB_TOOLSET_DIR}
  ARCHIVE 
    DESTINATION ${CMAKE_INSTALL_LIBDIR}${LIB_TOOLSET_DIR}
)

if (WIN32)
   install(FILES ${BUILT_JAR_FILE} DESTINATION java)
else()
  install_jar(openvds-java DESTINATION java)
endif()
