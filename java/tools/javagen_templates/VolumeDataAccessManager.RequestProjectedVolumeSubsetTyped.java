
    native private long RequestProjectedVolumeSubset<DataType>Impl(long native_object, ByteBuffer buffer, long dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, ByteBuffer voxelPlane, long voxelPlane_byteoffset, long projectedDimensions, long interpolationMethod, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request a subset projected from an arbitrary 3D plane through the subset onto one of the sides of the subset.
     * 
     * @param buffer Pointer to a preallocated buffer holding at least as many elements of format as indicated by minVoxelCoordinates and maxVoxelCoordinates for the projected dimensions.
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param minVoxelCoordinates The minimum voxel coordinates to request in each dimension (inclusive).
     * @param maxVoxelCoordinates The maximum voxel coordinates to request in each dimension (exclusive).
     * @param voxelPlane The plane equation for the projection from the dimension source to the projected dimensions (which must be a 2D subset of the source dimensions).
     * @param projectedDimensions The 2D dimension group that the plane in the source dimensiongroup is projected into. It must be a 2D subset of the source dimensions.
     * @param interpolationMethod Interpolation method to use when sampling the buffer.
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequest<DataType> requestProjectedVolumeSubset<DataType>(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, InterpolationMethod interpolationMethod, Float replacementNoValue) {{
        if (minVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"minVoxelCoordinates\" must have length 6");
        if (maxVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"maxVoxelCoordinates\" must have length 6");
        return VolumeDataRequest<DataType>.fromNativeObject(RequestProjectedVolumeSubset<DataType>Impl(getNativeObject(), buffer, dimensionsND.value(), LOD, channel, minVoxelCoordinates, maxVoxelCoordinates, voxelPlane.getBackingByteBuffer(), voxelPlane.getByteBufferOffset(), projectedDimensions.value(), interpolationMethod.value(), replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}
    
    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequest<DataType> requestProjectedVolumeSubset<DataType>(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, InterpolationMethod interpolationMethod, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest<DataType> requestProjectedVolumeSubset<DataType>(ByteBuffer buffer, DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null

    native private long RequestProjectedVolumeSubset<DataType>2Impl(long native_object, long dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, ByteBuffer voxelPlane, long voxelPlane_byteoffset, long projectedDimensions, long interpolationMethod, float replacementNoValue, boolean use_replacementNoValue);

    /**
     * Request a subset projected from an arbitrary 3D plane through the subset onto one of the sides of the subset, using an automatically allocated buffer.
     * 
     * @param dimensionsND The dimensiongroup the requested data is read from.
     * @param LOD The LOD level the requested data is read from.
     * @param channel The channel index the requested data is read from.
     * @param minVoxelCoordinates The minimum voxel coordinates to request in each dimension (inclusive).
     * @param maxVoxelCoordinates The maximum voxel coordinates to request in each dimension (exclusive).
     * @param voxelPlane The plane equation for the projection from the dimension source to the projected dimensions (which must be a 2D subset of the source dimensions).
     * @param projectedDimensions The 2D dimension group that the plane in the source dimensiongroup is projected into. It must be a 2D subset of the source dimensions.
     * @param interpolationMethod Interpolation method to use when sampling the buffer.
     * @param replacementNoValue If specified, this value is used to replace regions of the input VDS that has no data.
     * @return A VolumeDataRequest instance encapsulating the request status and buffer.
     */
    public VolumeDataRequest<DataType> requestProjectedVolumeSubset<DataType>(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, InterpolationMethod interpolationMethod, Float replacementNoValue) {{
        if (minVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"minVoxelCoordinates\" must have length 6");
        if (maxVoxelCoordinates.length != 6) throw new IllegalArgumentException("Array \"maxVoxelCoordinates\" must have length 6");
        return VolumeDataRequest<DataType>.fromNativeObject(RequestProjectedVolumeSubset<DataType>2Impl(getNativeObject(), dimensionsND.value(), LOD, channel, minVoxelCoordinates, maxVoxelCoordinates, voxelPlane.getBackingByteBuffer(), voxelPlane.getByteBufferOffset(), projectedDimensions.value(), interpolationMethod.value(), replacementNoValue == null ? (float)0 : (float)replacementNoValue, replacementNoValue != null));
    }}
    
    ///AUTOGEN-REGISTER-OVERLOADABLE-FUNCTION: VolumeDataRequest<DataType> requestProjectedVolumeSubset<DataType>(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, InterpolationMethod interpolationMethod, Float replacementNoValue)
    ///AUTOGEN-ADD-OVERLOAD: VolumeDataRequest<DataType> requestProjectedVolumeSubset<DataType>(DimensionsND dimensionsND, int LOD, int channel, int[] minVoxelCoordinates, int[] maxVoxelCoordinates, FloatVector4 voxelPlane, DimensionsND projectedDimensions, InterpolationMethod interpolationMethod, Float replacementNoValue) -> replacementNoValue=null
