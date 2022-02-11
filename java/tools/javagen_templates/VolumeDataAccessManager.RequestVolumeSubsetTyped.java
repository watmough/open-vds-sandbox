
    native private long RequestVolumeSubset<DataType>Impl(long native_object, ByteBuffer buffer, long dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request a subset of the input VDS.
     * 
     * @param buffer Pointer to a preallocated buffer holding at least as many elements of format as indicated by minVoxelCoordinates and maxVoxelCoordinates.
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param minVoxelCoordinates The minimum voxel coordinates to request in each dimension (inclusive).
     * @param maxVoxelCoordinates The maximum voxel coordinates to request in each dimension (exclusive).
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequest<DataType> requestVolumeSubset<DataType>(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, Float replacementNoValue) {{
        if (minVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"minVoxelCoordinates\" must have length 6");
        if (maxVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"maxVoxelCoordinates\" must have length 6");
        return VolumeDataRequest<DataType>.fromNativeObject(RequestVolumeSubset<DataType>Impl(getNativeObject(), buffer, dimensionsND.value(), LOD, channel, minVoxelCoordinates, maxVoxelCoordinates, replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}
    
    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequest<DataType> requestVolumeSubset<DataType>(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest<DataType> requestVolumeSubset<DataType>(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, Float replacementNoValue) -> replacementNoValue=null

    native private long RequestVolumeSubset<DataType>2Impl(long native_object, long dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request a subset of the input VDS, using an automatically allocated buffer.
     * 
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param minVoxelCoordinates The minimum voxel coordinates to request in each dimension (inclusive).
     * @param maxVoxelCoordinates The maximum voxel coordinates to request in each dimension (exclusive).
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequest<DataType> requestVolumeSubset<DataType>(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, Float replacementNoValue) {{
        if (minVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"minVoxelCoordinates\" must have length 6");
        if (maxVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"maxVoxelCoordinates\" must have length 6");
        return VolumeDataRequest<DataType>.fromNativeObject(RequestVolumeSubset<DataType>2Impl(getNativeObject(), dimensionsND.value(), LOD, channel, minVoxelCoordinates, maxVoxelCoordinates, replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}

    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequest<DataType> requestVolumeSubset<DataType>(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest<DataType> requestVolumeSubset<DataType>(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, Float replacementNoValue) -> replacementNoValue=null
