{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}

    ///AUTOGEN-ADD-OVERLOAD: VolumeDataPageAccessor createVolumeDataPageAccessor(DimensionsND dimensionsND, int LOD, int channel, int maxPages, VolumeDataPageAccessor.AccessMode accessMode, int chunkMetadataPageSize) -> chunkMetadataPageSize = 1024

    ///AUTOGEN-ADD-OVERLOAD: long getVolumeSubsetBufferSize(int[] minVoxelCoordinates, int[] maxVoxelCoordinates, VCVoxelFormat format, int LOD, int channel) -> channel = 0
    ///AUTOGEN-ADD-OVERLOAD: long getVolumeSubsetBufferSize(int[] minVoxelCoordinates, int[] maxVoxelCoordinates, VCVoxelFormat format, int LOD, int channel) -> LOD = 0, channel = 0

    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest requestVolumeSubset(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, VCVoxelFormat format, Float replacementNoValue) -> replacementNoValue=null

    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest requestVolumeSubset(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, VCVoxelFormat format, Float replacementNoValue) -> replacementNoValue=null
    
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.java, <DataType> -> Byte
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.java, <DataType> -> UShort
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.java, <DataType> -> UInt
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.java, <DataType> -> ULong
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.java, <DataType> -> Float
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestVolumeSubsetTyped.java, <DataType> -> Double

    ///AUTOGEN-ADD-OVERLOAD: long getProjectedVolumeSubsetBufferSize(int[] minVoxelCoordinates, int[] maxVoxelCoordinates, DimensionsND projectedDimensions, VCVoxelFormat format, int LOD, int channel) -> channel = 0
    ///AUTOGEN-ADD-OVERLOAD: long getProjectedVolumeSubsetBufferSize(int[] minVoxelCoordinates, int[] maxVoxelCoordinates, DimensionsND projectedDimensions, VCVoxelFormat format, int LOD, int channel) -> LOD = 0, channel = 0

    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest requestProjectedVolumeSubset(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, VCVoxelFormat format, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest requestProjectedVolumeSubset(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, VCVoxelFormat format, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.java, <DataType> -> Byte
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.java, <DataType> -> UShort
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.java, <DataType> -> UInt
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.java, <DataType> -> ULong
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.java, <DataType> -> Float
    ///AUTOGEN-INCLUDE: VolumeDataAccessManager.RequestProjectedVolumeSubsetTyped.java, <DataType> -> Double

    ///AUTOGEN-ADD-OVERLOAD: long getVolumeSamplesBufferSize(int sampleCount, int channel) -> channel = 0

    ///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeSamples std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (float *, int64_t, OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, OpenVDS::optional<float>) FUNCTIONPROTO
    native private long RequestVolumeSamplesImpl(long native_object, ByteBuffer buffer, long dimensionsND, int LOD, int channel, ByteBuffer samplePositions, long interpolationMethod, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request sampling of the input VDS at the specified coordinates.
     * 
     * @param buffer Pointer to a preallocated buffer holding at least sampleCount elements.
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param samplePositions Pointer to array of VolumeDataLayout::Dimensionality_Max-elements indicating the positions to sample. May be deleted once RequestVolumeSamples return, as HueSpace makes a deep copy of the data.
     * @param interpolationMethod Interpolation method to use when sampling the buffer.
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequestFloat requestVolumeSamples(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray samplePositions, InterpolationMethod interpolationMethod, Float replacementNoValue) {{
        return VolumeDataRequestFloat.fromNativeObject(RequestVolumeSamplesImpl(getNativeObject(), buffer, dimensionsND.value(), LOD, channel, samplePositions.getBackingByteBuffer(), interpolationMethod.value(), replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}

    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequestFloat requestVolumeSamples(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray samplePositions, InterpolationMethod interpolationMethod, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequestFloat requestVolumeSamples(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray samplePositions, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null

    ///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeSamples std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, OpenVDS::optional<float>) FUNCTIONPROTO
    native private long RequestVolumeSamples2Impl(long native_object, long dimensionsND, int LOD, int channel, ByteBuffer samplePositions, long interpolationMethod, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request sampling of the input VDS at the specified coordinates, using an automatically allocated buffer.
     * 
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param samplePositions Pointer to array of VolumeDataLayout::Dimensionality_Max-elements indicating the positions to sample. May be deleted once RequestVolumeSamples return, as HueSpace makes a deep copy of the data.
     * @param interpolationMethod Interpolation method to use when sampling the buffer.
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequestFloat requestVolumeSamples(DimensionsND dimensionsND, int LOD, int channel, NDPosArray samplePositions, InterpolationMethod interpolationMethod, Float replacementNoValue) {{
        return VolumeDataRequestFloat.fromNativeObject(RequestVolumeSamples2Impl(getNativeObject(), dimensionsND.value(), LOD, channel, samplePositions.getBackingByteBuffer(), interpolationMethod.value(), replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}
    
    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequestFloat requestVolumeSamples(DimensionsND dimensionsND, int LOD, int channel, NDPosArray samplePositions, InterpolationMethod interpolationMethod, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequestFloat requestVolumeSamples(DimensionsND dimensionsND, int LOD, int channel, NDPosArray samplePositions, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    
    ///AUTOGEN-ADD-OVERLOAD: long getVolumeTracesBufferSize(int traceCount, int traceDimension, int LOD, int channel) -> channel = 0
    ///AUTOGEN-ADD-OVERLOAD: long getVolumeTracesBufferSize(int traceCount, int traceDimension, int LOD, int channel) -> LOD = 0, channel = 0
    
    ///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraces std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, OpenVDS::optional<float>) FUNCTIONPROTO
    native private long RequestVolumeTracesImpl(long native_object, long dimensionsND, int LOD, int channel, ByteBuffer tracePositions, long interpolationMethod, int traceDimension, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request traces from the input VDS, using an automatically allocated buffer.
     * 
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param tracePositions Pointer to array of traceCount VolumeDataLayout::Dimensionality_Max-elements indicating the trace positions.
     * @param interpolationMethod Interpolation method to use when sampling the buffer.
     * @param traceDimension The dimension to trace
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequestFloat requestVolumeTraces(DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, InterpolationMethod interpolationMethod, int traceDimension, Float replacementNoValue) {{
        return VolumeDataRequestFloat.fromNativeObject(RequestVolumeTracesImpl(getNativeObject(), dimensionsND.value(), LOD, channel, tracePositions.getBackingByteBuffer(), interpolationMethod.value(), traceDimension, replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}

    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequestFloat requestVolumeTraces(DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, InterpolationMethod interpolationMethod, int traceDimension, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequestFloat requestVolumeTraces(DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, InterpolationMethod interpolationMethod, int traceDimension, Float replacementNoValue) -> replacementNoValue=null

    ///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraces std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (float *, int64_t, OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, OpenVDS::optional<float>) FUNCTIONPROTO
    native private long RequestVolumeTraces2Impl(long native_object, ByteBuffer buffer, long dimensionsND, int LOD, int channel, ByteBuffer tracePositions, long interpolationMethod, int traceDimension, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request traces from the input VDS.
     * 
     * @param buffer Pointer to a preallocated buffer holding at least traceCount * number of samples in the traceDimension.
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param tracePositions Pointer to array of traceCount VolumeDataLayout::Dimensionality_Max-elements indicating the trace positions.
     * @param interpolationMethod Interpolation method to use when sampling the buffer.
     * @param traceDimension The dimension to trace
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequestFloat requestVolumeTraces(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, InterpolationMethod interpolationMethod, int traceDimension, Float replacementNoValue) {{
        return VolumeDataRequestFloat.fromNativeObject(RequestVolumeTraces2Impl(getNativeObject(), buffer, dimensionsND.value(), LOD, channel, tracePositions.getBackingByteBuffer(), interpolationMethod.value(), traceDimension, replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}
    
    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequestFloat requestVolumeTraces(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, InterpolationMethod interpolationMethod, int traceDimension, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequestFloat requestVolumeTraces(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, InterpolationMethod interpolationMethod, int traceDimension, Float replacementNoValue) -> replacementNoValue=null
    
    ///AUTOGEN-ADD-OVERLOAD: getVolumeTraceRangesBufferSize(int traceCount, int traceDimension, int traceMin, int traceMax, int LOD, int channel) -> channel = 0
    ///AUTOGEN-ADD-OVERLOAD: getVolumeTraceRangesBufferSize(int traceCount, int traceDimension, int traceMin, int traceMax, int LOD, int channel) -> LOD = 0, channel = 0


// FIXME? RequestVolumeTraceRanges does not exist in OpenVDS 
//    ///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraceRanges std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, int, int, OpenVDS::optional<float>) FUNCTIONPROTO
//    native private long RequestVolumeTraceRangesImpl(long native_object, long dimensionsND, int LOD, int channel, ByteBuffer tracePositions, int traceCount, long interpolationMethod, int traceDimension, int traceMin, int traceMax, float replacementNoValue, boolean use_replacementNoValue);
//
//    /**
//     * Request traces from the input VDS in a given range of samples, using an automatically allocated buffer.
//     * 
//     * @param dimensionsND The dimensiongroup the requested data is read from.
//     * @param LOD The LOD level the requested data is read from.
//     * @param channel The channel index the requested data is read from.
//     * @param tracePositions Pointer to array of traceCount VolumeDataLayout::Dimensionality_Max-elements indicating the trace positions.
//     * @param traceCount Number of traces to request.
//     * @param interpolationMethod Interpolation method to use when sampling the buffer.
//     * @param traceDimension The dimension to trace
//     * @param traceMin The first sample to include in the trace.
//     * @param traceMax The sample after the last to include in the trace.
//     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
//     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
//     */
//    public VolumeDataRequestFloat requestVolumeTraceRanges(DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, int traceCount, InterpolationMethod interpolationMethod, int traceDimension, int traceMin, int traceMax, Float replacementNoValue) {{
//        return VolumeDataRequestFloat.fromNativeObject(RequestVolumeTraceRangesImpl(getNativeObject(), dimensionsND.value(), LOD, channel, tracePositions.getBackingByteBuffer(), traceCount, interpolationMethod.value(), traceDimension, traceMin, traceMax, replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
//    }}
//
//    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequestFloat requestVolumeTraceRanges(DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, int traceCount, InterpolationMethod interpolationMethod, int traceDimension, int traceMin, int traceMax, Float replacementNoValue)
//    ///FIXME AUTOGEN-ADD-OVERLOAD: VolumeDataRequestFloat requestVolumeTraceRanges(DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, int traceCount, InterpolationMethod interpolationMethod, int traceDimension, int traceMin, int traceMax, Float replacementNoValue) -> replacementNoValue=null
//
//    ///AUTOGEN-IGNORE: CXX_METHOD RequestVolumeTraceRanges std::shared_ptr<OpenVDS::VolumeDataRequest_t<float>> (float *, int64_t, OpenVDS::DimensionsND, int, int, float const (*)[6], int, OpenVDS::InterpolationMethod, int, int, int, OpenVDS::optional<float>) FUNCTIONPROTO
//    native private long RequestVolumeTraceRanges2Impl(long native_object, ByteBuffer buffer, long dimensionsND, int LOD, int channel, ByteBuffer tracePositions, int traceCount, long interpolationMethod, int traceDimension, int traceMin, int traceMax, float replacementNoValue, boolean use_replacementNoValue);
//
//    /**
//     * Request traces from the input VDS in a given range of samples.
//     * 
//     * @param buffer Pointer to a preallocated buffer holding at least traceCount * number of samples in the traceDimension.
//     * @param dimensionsND The dimensiongroup the requested data is read from.
//     * @param LOD The LOD level the requested data is read from.
//     * @param channel The channel index the requested data is read from.
//     * @param tracePositions Pointer to array of traceCount VolumeDataLayout::Dimensionality_Max-elements indicating the trace positions.
//     * @param traceCount Number of traces to request.
//     * @param interpolationMethod Interpolation method to use when sampling the buffer.
//     * @param traceDimension The dimension to trace
//     * @param traceMin The first sample to include in the trace.
//     * @param traceMax The sample after the last to include in the trace.
//     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
//     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
//     */
//    public VolumeDataRequestFloat requestVolumeTraceRanges(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, int traceCount, InterpolationMethod interpolationMethod, int traceDimension, int traceMin, int traceMax, Float replacementNoValue) {{
//        return VolumeDataRequestFloat.fromNativeObject(RequestVolumeTraceRanges2Impl(getNativeObject(), buffer, dimensionsND.value(), LOD, channel, tracePositions.getBackingByteBuffer(), traceCount, interpolationMethod.value(), traceDimension, traceMin, traceMax, replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
//    }}
        
    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequestFloat requestVolumeTraceRanges(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, int traceCount, InterpolationMethod interpolationMethod, int traceDimension, int traceMin, int traceMax, Float replacementNoValue)
    ///FIXME AUTOGEN-ADD-OVERLOAD: VolumeDataRequestFloat requestVolumeTraceRanges(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, NDPosArray tracePositions, int traceCount, InterpolationMethod interpolationMethod, int traceDimension, int traceMin, int traceMax, Float replacementNoValue) -> replacementNoValue=null
    
    ///AUTOGEN-ADD-OVERLOAD: HistogramRequest requestVolumeSubsetHistogram(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, int bucketCount, boolean truncateValuesMin, boolean truncateValuesMax, boolean logarithmicScaleHistogram, LogScaleInvalidValueHandling logarithmicScaleInvalidValueHandling, Float overrideValueRangeMin, Float overrideValueRangeMax) -> overrideValueRangeMin=null, overrideValueRangeMax=null
    
    ///AUTOGEN-ADD-OVERLOAD: HistogramRequest requestVolumeSubsetHistogram(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, int bucketCount, boolean truncateValuesMin, boolean truncateValuesMax, boolean logarithmicScaleHistogram, LogScaleInvalidValueHandling logarithmicScaleInvalidValueHandling, Float overrideValueRangeMin, Float overrideValueRangeMax) -> overrideValueRangeMin=null, overrideValueRangeMax=null

    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest prefetchSerializedVolumeSubset(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, boolean autoComplete) -> autoComplete=false

    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest prefetchSerializedVolumeDataChunk(DimensionsND dimensionsND, int LOD, int channel, long chunkIndex, boolean autoComplete) -> autoComplete=false
    
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DInterpolatingAccessorR64 createVolumeData2DInterpolatingAccessorR64(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DInterpolatingAccessorR64 createVolumeData2DInterpolatingAccessorR64(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DInterpolatingAccessorR32 createVolumeData2DInterpolatingAccessorR32(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DInterpolatingAccessorR32 createVolumeData2DInterpolatingAccessorR32(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessor1Bit createVolumeData2DReadAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessor1Bit createVolumeData2DReadAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU8 createVolumeData2DReadAccessorU8(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU8 createVolumeData2DReadAccessorU8(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU16 createVolumeData2DReadAccessorU16(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU16 createVolumeData2DReadAccessorU16(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU32 createVolumeData2DReadAccessorU32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU32 createVolumeData2DReadAccessorU32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU64 createVolumeData2DReadAccessorU64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorU64 createVolumeData2DReadAccessorU64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorR32 createVolumeData2DReadAccessorR32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorR32 createVolumeData2DReadAccessorR32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorR64 createVolumeData2DReadAccessorR64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadAccessorR64 createVolumeData2DReadAccessorR64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessor1Bit createVolumeData2DReadWriteAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessor1Bit createVolumeData2DReadWriteAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU8 createVolumeData2DReadWriteAccessorU8(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU8 createVolumeData2DReadWriteAccessorU8(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU16 createVolumeData2DReadWriteAccessorU16(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU16 createVolumeData2DReadWriteAccessorU16(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU32 createVolumeData2DReadWriteAccessorU32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU32 createVolumeData2DReadWriteAccessorU32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU64 createVolumeData2DReadWriteAccessorU64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorU64 createVolumeData2DReadWriteAccessorU64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorR32 createVolumeData2DReadWriteAccessorR32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorR32 createVolumeData2DReadWriteAccessorR32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorR64 createVolumeData2DReadWriteAccessorR64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData2DReadWriteAccessorR64 createVolumeData2DReadWriteAccessorR64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DInterpolatingAccessorR64 createVolumeData3DInterpolatingAccessorR64(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DInterpolatingAccessorR64 createVolumeData3DInterpolatingAccessorR64(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DInterpolatingAccessorR32 createVolumeData3DInterpolatingAccessorR32(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DInterpolatingAccessorR32 createVolumeData3DInterpolatingAccessorR32(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessor1Bit createVolumeData3DReadAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessor1Bit createVolumeData3DReadAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU8 createVolumeData3DReadAccessorU8(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU8 createVolumeData3DReadAccessorU8(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU16 createVolumeData3DReadAccessorU16(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU16 createVolumeData3DReadAccessorU16(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU32 createVolumeData3DReadAccessorU32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU32 createVolumeData3DReadAccessorU32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU64 createVolumeData3DReadAccessorU64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorU64 createVolumeData3DReadAccessorU64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorR32 createVolumeData3DReadAccessorR32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorR32 createVolumeData3DReadAccessorR32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorR64 createVolumeData3DReadAccessorR64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadAccessorR64 createVolumeData3DReadAccessorR64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessor1Bit createVolumeData3DReadWriteAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessor1Bit createVolumeData3DReadWriteAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU8 createVolumeData3DReadWriteAccessorU8(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU8 createVolumeData3DReadWriteAccessorU8(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU16 createVolumeData3DReadWriteAccessorU16(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU16 createVolumeData3DReadWriteAccessorU16(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU32 createVolumeData3DReadWriteAccessorU32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU32 createVolumeData3DReadWriteAccessorU32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU64 createVolumeData3DReadWriteAccessorU64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorU64 createVolumeData3DReadWriteAccessorU64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorR32 createVolumeData3DReadWriteAccessorR32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorR32 createVolumeData3DReadWriteAccessorR32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorR64 createVolumeData3DReadWriteAccessorR64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData3DReadWriteAccessorR64 createVolumeData3DReadWriteAccessorR64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DInterpolatingAccessorR64 createVolumeData4DInterpolatingAccessorR64(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DInterpolatingAccessorR64 createVolumeData4DInterpolatingAccessorR64(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DInterpolatingAccessorR32 createVolumeData4DInterpolatingAccessorR32(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DInterpolatingAccessorR32 createVolumeData4DInterpolatingAccessorR32(DimensionsND dimensionsND, int LOD, int channel, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessor1Bit createVolumeData4DReadAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessor1Bit createVolumeData4DReadAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU8 createVolumeData4DReadAccessorU8(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU8 createVolumeData4DReadAccessorU8(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU16 createVolumeData4DReadAccessorU16(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU16 createVolumeData4DReadAccessorU16(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU32 createVolumeData4DReadAccessorU32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU32 createVolumeData4DReadAccessorU32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU64 createVolumeData4DReadAccessorU64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorU64 createVolumeData4DReadAccessorU64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorR32 createVolumeData4DReadAccessorR32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorR32 createVolumeData4DReadAccessorR32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorR64 createVolumeData4DReadAccessorR64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadAccessorR64 createVolumeData4DReadAccessorR64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessor1Bit createVolumeData4DReadWriteAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessor1Bit createVolumeData4DReadWriteAccessor1Bit(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU8 createVolumeData4DReadWriteAccessorU8(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU8 createVolumeData4DReadWriteAccessorU8(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU16 createVolumeData4DReadWriteAccessorU16(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU16 createVolumeData4DReadWriteAccessorU16(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU32 createVolumeData4DReadWriteAccessorU32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU32 createVolumeData4DReadWriteAccessorU32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU64 createVolumeData4DReadWriteAccessorU64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorU64 createVolumeData4DReadWriteAccessorU64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorR32 createVolumeData4DReadWriteAccessorR32(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorR32 createVolumeData4DReadWriteAccessorR32(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorR64 createVolumeData4DReadWriteAccessorR64(DimensionsND dimensionsND, int LOD, int channel, int maxPages, Float replacementNoValue) -> replacementNoValue=null
    ///AUTOGEN-ADD-OVERLOAD: VolumeData4DReadWriteAccessorR64 createVolumeData4DReadWriteAccessorR64(DimensionsND dimensionsND, int LOD, int channel, Float replacementNoValue) -> replacementNoValue=null

    /**
     * Get the default maxPages for VolumeDataPageAccessors
     */
    public static final int MaxPagesDefault = 8;
}}
