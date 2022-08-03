set(CMAKE_DEBUG_POSTFIX "")
include(UseJava)

set(OPENVDS_JAVA_VERSION 0.0.6)

set(CMAKE_JAVA_COMPILE_FLAGS "-source" "1.8" "-target" "1.8")

set(JAVA_SOURCE_FILES
	java/src/org/opengroup/openvds/AWSOpenOptions.java
	java/src/org/opengroup/openvds/AzureOpenOptions.java
	java/src/org/opengroup/openvds/AzurePresignedOpenOptions.java
	java/src/org/opengroup/openvds/AzureVDSGenerator.java
	java/src/org/opengroup/openvds/ByteBufferBackedObject.java
	java/src/org/opengroup/openvds/CompressionMethod.java
	java/src/org/opengroup/openvds/DimensionsND.java
	java/src/org/opengroup/openvds/DMSOpenOptions.java
	java/src/org/opengroup/openvds/DoubleMatrix3x3.java
	java/src/org/opengroup/openvds/DoubleMatrix4x4.java
	java/src/org/opengroup/openvds/DoubleRange.java
	java/src/org/opengroup/openvds/DoubleVector2.java
	java/src/org/opengroup/openvds/DoubleVector3.java
	java/src/org/opengroup/openvds/DoubleVector4.java
	java/src/org/opengroup/openvds/DownloadError.java
	java/src/org/opengroup/openvds/FloatRange.java
	java/src/org/opengroup/openvds/FloatVector2.java
	java/src/org/opengroup/openvds/FloatVector3.java
	java/src/org/opengroup/openvds/FloatVector4.java
	java/src/org/opengroup/openvds/GlobalState.java
	java/src/org/opengroup/openvds/GoogleCredentialsJson.java
	java/src/org/opengroup/openvds/GoogleCredentialsPath.java
	java/src/org/opengroup/openvds/GoogleCredentialsSignedUrl.java
	java/src/org/opengroup/openvds/GoogleCredentialsSignedUrlJson.java
	java/src/org/opengroup/openvds/GoogleCredentialsSignedUrlPath.java
	java/src/org/opengroup/openvds/GoogleCredentialsToken.java
	java/src/org/opengroup/openvds/GoogleOpenOptions.java
	java/src/org/opengroup/openvds/HttpOpenOptions.java
	java/src/org/opengroup/openvds/IJKCoordinateTransformer.java
	java/src/org/opengroup/openvds/IJKGridDefinition.java
	java/src/org/opengroup/openvds/IndexRegionFloatVector2.java
	java/src/org/opengroup/openvds/IndexRegionFloatVector3.java
	java/src/org/opengroup/openvds/IndexRegionFloatVector4.java
	java/src/org/opengroup/openvds/IndexRegionIntVector2.java
	java/src/org/opengroup/openvds/IndexRegionIntVector3.java
	java/src/org/opengroup/openvds/IndexRegionIntVector4.java
	java/src/org/opengroup/openvds/InMemoryOpenOptions.java
	java/src/org/opengroup/openvds/InMemoryVDSGenerator.java
	java/src/org/opengroup/openvds/InterpolationMethod.java
	java/src/org/opengroup/openvds/IntRange.java
	java/src/org/opengroup/openvds/IntVector2.java
	java/src/org/opengroup/openvds/IntVector3.java
	java/src/org/opengroup/openvds/IntVector4.java
	java/src/org/opengroup/openvds/IOManager.java
	java/src/org/opengroup/openvds/KnownAxisNames.java
	java/src/org/opengroup/openvds/KnownMetadata.java
	java/src/org/opengroup/openvds/KnownUnitNames.java
	java/src/org/opengroup/openvds/ManagedBase.java
	java/src/org/opengroup/openvds/ManagedBuffer.java
	java/src/org/opengroup/openvds/MetadataContainer.java
	java/src/org/opengroup/openvds/MetadataKey.java
	java/src/org/opengroup/openvds/MetadataReadAccess.java
	java/src/org/opengroup/openvds/MetadataType.java
	java/src/org/opengroup/openvds/MetadataWriteAccess.java
        java/src/org/opengroup/openvds/LogHandler.java
        java/src/org/opengroup/openvds/LogLevel.java
	java/src/org/opengroup/openvds/NDPos.java
	java/src/org/opengroup/openvds/NDPosArray.java
	java/src/org/opengroup/openvds/OpenOptions.java
	java/src/org/opengroup/openvds/OpenVDS.java
	java/src/org/opengroup/openvds/PODArray.java
	java/src/org/opengroup/openvds/RequestFormat.java
	java/src/org/opengroup/openvds/UploadError.java
	java/src/org/opengroup/openvds/VDS.java
	java/src/org/opengroup/openvds/VDSError.java
	java/src/org/opengroup/openvds/VDSFileOpenOptions.java
	java/src/org/opengroup/openvds/VDSIJKGridDefinition.java
	java/src/org/opengroup/openvds/VDSProduceStatus.java
	java/src/org/opengroup/openvds/VolumeData2DInterpolatingAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData2DInterpolatingAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData2DReadAccessor1Bit.java
	java/src/org/opengroup/openvds/VolumeData2DReadAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData2DReadAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData2DReadAccessorU16.java
	java/src/org/opengroup/openvds/VolumeData2DReadAccessorU32.java
	java/src/org/opengroup/openvds/VolumeData2DReadAccessorU64.java
	java/src/org/opengroup/openvds/VolumeData2DReadAccessorU8.java
	java/src/org/opengroup/openvds/VolumeData2DReadWriteAccessor1Bit.java
	java/src/org/opengroup/openvds/VolumeData2DReadWriteAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData2DReadWriteAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU16.java
	java/src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU32.java
	java/src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU64.java
	java/src/org/opengroup/openvds/VolumeData2DReadWriteAccessorU8.java
	java/src/org/opengroup/openvds/VolumeData3DInterpolatingAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData3DInterpolatingAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData3DReadAccessor1Bit.java
	java/src/org/opengroup/openvds/VolumeData3DReadAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData3DReadAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData3DReadAccessorU16.java
	java/src/org/opengroup/openvds/VolumeData3DReadAccessorU32.java
	java/src/org/opengroup/openvds/VolumeData3DReadAccessorU64.java
	java/src/org/opengroup/openvds/VolumeData3DReadAccessorU8.java
	java/src/org/opengroup/openvds/VolumeData3DReadWriteAccessor1Bit.java
	java/src/org/opengroup/openvds/VolumeData3DReadWriteAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData3DReadWriteAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU16.java
	java/src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU32.java
	java/src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU64.java
	java/src/org/opengroup/openvds/VolumeData3DReadWriteAccessorU8.java
	java/src/org/opengroup/openvds/VolumeData4DInterpolatingAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData4DInterpolatingAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData4DReadAccessor1Bit.java
	java/src/org/opengroup/openvds/VolumeData4DReadAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData4DReadAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData4DReadAccessorU16.java
	java/src/org/opengroup/openvds/VolumeData4DReadAccessorU32.java
	java/src/org/opengroup/openvds/VolumeData4DReadAccessorU64.java
	java/src/org/opengroup/openvds/VolumeData4DReadAccessorU8.java
	java/src/org/opengroup/openvds/VolumeData4DReadWriteAccessor1Bit.java
	java/src/org/opengroup/openvds/VolumeData4DReadWriteAccessorR32.java
	java/src/org/opengroup/openvds/VolumeData4DReadWriteAccessorR64.java
	java/src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU16.java
	java/src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU32.java
	java/src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU64.java
	java/src/org/opengroup/openvds/VolumeData4DReadWriteAccessorU8.java
	java/src/org/opengroup/openvds/VolumeDataAccessManager.java
	java/src/org/opengroup/openvds/VolumeDataAxisDescriptor.java
	java/src/org/opengroup/openvds/VolumeDataChannelDescriptor.java
	java/src/org/opengroup/openvds/VolumeDataComponents.java
	java/src/org/opengroup/openvds/VolumeDataFormat.java
	java/src/org/opengroup/openvds/VolumeDataLayout.java
	java/src/org/opengroup/openvds/VolumeDataLayoutDescriptor.java
	java/src/org/opengroup/openvds/VolumeDataMapping.java
	java/src/org/opengroup/openvds/VolumeDataPage.java
	java/src/org/opengroup/openvds/VolumeDataPageAccessor.java
	java/src/org/opengroup/openvds/VolumeDataRequest.java
	java/src/org/opengroup/openvds/VolumeDataRequest1Bit.java
	java/src/org/opengroup/openvds/VolumeDataRequestByte.java
	java/src/org/opengroup/openvds/VolumeDataRequestDouble.java
	java/src/org/opengroup/openvds/VolumeDataRequestFloat.java
	java/src/org/opengroup/openvds/VolumeDataRequestUInt.java
	java/src/org/opengroup/openvds/VolumeDataRequestULong.java
	java/src/org/opengroup/openvds/VolumeDataRequestUShort.java
	java/src/org/opengroup/openvds/WaveletAdaptiveMode.java

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
	java/test/src/test/org/opengroup/openvds/CreateVDSTest.java
	java/test/src/test/org/opengroup/openvds/InMemoryVDSGeneratorTest.java
	java/test/src/test/org/opengroup/openvds/MetaDataContainerTest.java
	java/test/src/test/org/opengroup/openvds/PageAccessorByteTest.java
	java/test/src/test/org/opengroup/openvds/PageAccessorFloatTest.java
	java/test/src/test/org/opengroup/openvds/PageAccessorShortTest.java
	java/test/src/test/org/opengroup/openvds/TestTemplate.java
	java/test/src/test/org/opengroup/openvds/WriteDataTest.java
  )

add_jar(openvds-java-test
        SOURCES ${JAVA_TEST_SOURCES_FILES}
        VERSION ${CMAKE_PROJECT_VERSION}
        INCLUDE_JARS
        ${testng_SOURCE_FILE}
        ${BUILT_JAR_FILE}
)

add_library(openvds-javacpp
  cpp/src/AzureVdsGenerator.cpp
  cpp/src/CoordinateTransformer.cpp
  cpp/src/CPPJNI.cpp
  cpp/src/CPPJNI.h
  cpp/src/CPPJNICommon.cpp
  cpp/src/CPPJNICommon.h
  cpp/src/Exceptions.cpp
  cpp/src/GlobalMetadataCommon.cpp
  cpp/src/GlobalState.cpp
  cpp/src/IJKCoordinateTransformer.cpp
  cpp/src/InMemoryVDSGenerator.cpp
  cpp/src/KnownMetadata.cpp
  cpp/src/MetadataAccess.cpp
  cpp/src/MetadataContainer.cpp
  cpp/src/MetadataKey.cpp
  cpp/src/NDVector.h
  cpp/src/OpenVDS.cpp
  cpp/src/Optional.cpp
  cpp/src/ValueConversion.cpp
  cpp/src/VDSWriteData.cpp
  cpp/src/VolumeData.cpp
  cpp/src/VolumeDataAccess.cpp
  cpp/src/VolumeDataAccessManager.cpp
  cpp/src/VolumeDataAxisDescriptor.cpp
  cpp/src/VolumeDataChannelDescriptor.cpp
  cpp/src/VolumeDataLayout.cpp
  cpp/src/VolumeDataLayoutDescriptor.cpp
  cpp/src/VolumeSampler.cpp
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
