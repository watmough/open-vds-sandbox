{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} implements AutoCloseable {{
{class_body}
///AUTOGEN-IGNORE: CXX_METHOD GetError OpenVDS::ReadErrorException () const FUNCTIONPROTO
///AUTOGEN-IGNORE: CXX_METHOD GetBuffer const void *(int (&)[6]) FUNCTIONPROTO
///AUTOGEN-IGNORE: CXX_METHOD GetBuffer const void *(int (&)[6], int (&)[6]) FUNCTIONPROTO
    native ByteBuffer GetBufferImpl(long native_object, int[] size, int[] pitch);
    
    /**
     * @param size An array of length 6 which describes the layout of the returned buffer : Each 
	               non-zero element is equal to the number of voxels allocated along each dimension in the buffer. 
     * @param pitch An array of length 6 which describes the layout of the returned buffer : Each 
     *              non-zero element is equal to the distance between neighboring elements (bytes, floats, etc.)
     *              along each dimension in the buffer. 
     * @return A read-only ByteBuffer
     */
    public ByteBuffer getBuffer(int[] size, int[] pitch) {{
        if (size.length != 6) throw new IllegalArgumentException("Array \"size\" must have length 6");
        if (pitch.length != 6) throw new IllegalArgumentException("Array \"pitch\" must have length 6");
        ByteBuffer buffer = GetBufferImpl(getNativeObject(), size, pitch);
        return buffer.asReadOnlyBuffer().order(java.nio.ByteOrder.nativeOrder());
    }}

    public ByteBuffer getBuffer(int[] pitch) {{
		return getBuffer(new int[VolumeDataLayout.Dimensionality_Max], pitch);
	}}
	
///AUTOGEN-IGNORE: CXX_METHOD GetWritableBuffer void *(int (&)[6]) FUNCTIONPROTO
///AUTOGEN-IGNORE: CXX_METHOD GetWritableBuffer void *(int (&)[6], int (&)[6]) FUNCTIONPROTO

    native ByteBuffer GetWritableBufferImpl(long native_object, int[] size, int[] pitch);
    
    /**
     * @param size An array of length 6 which describes the layout of the returned buffer : Each 
	               non-zero element is equal to the number of voxels allocated along each dimension in the buffer. 
     * @param pitch An array of length 6 which describes the layout of the returned buffer : Each 
     *              non-zero element is equal to the distance between neighboring elements (bytes, floats, etc.)
     *              along each dimension in the buffer. 
     * @return A writable ByteBuffer
     */
    public ByteBuffer getWritableBuffer(int[] size, int[] pitch) {{
        if (size.length != 6) throw new IllegalArgumentException("Array \"size\" must have length 6");
        if (pitch.length != 6) throw new IllegalArgumentException("Array \"pitch\" must have length 6");
        ByteBuffer buffer = GetWritableBufferImpl(getNativeObject(), size, pitch);
        return buffer.order(java.nio.ByteOrder.nativeOrder());
    }}

    public ByteBuffer getWritableBuffer(int[] pitch) {{
		return getWritableBuffer(new int[VolumeDataLayout.Dimensionality_Max], pitch);
	}}
	
///AUTOGEN-IGNORE: CXX_METHOD Release void () FUNCTIONPROTO
	
	/**
	 * Translate n-dimensional local/chunk index to 1-dimensional data index.
	 * @param localIndex An array of length <= 6 which represents the n-dimensional index into a chunk.
     * @param pitch An array of length 6 which describes the layout of the returned buffer : Each 
     *              non-zero element is equal to the distance between neighboring elements (bytes, floats, etc.)
     *              along each dimension in the buffer. 
	 * @return Data index.
	 */
    public static int localIndexToDataIndex(int[] localIndex, int[] pitch) {{
        assert(localIndex.length <= pitch.length);
        int dataIndex = 0;
        for (int i = 0; i < localIndex.length; i++) {{
            dataIndex += localIndex[i] * pitch[i];
        }}
        return dataIndex;
    }}

	/**
	 * Translate n-dimensional local/chunk index to global n-dimensional index.
	 * @param localIndex An array of length <= 6 which represents the n-dimensional index into a chunk.
	 * @param chunkMin An array of length <= representing the global index of the chunk's 0th element.
	 * @param log The LOD level of the chunk.
	 */
    public static void localIndexToGlobalIndex(int[] globalIndex, int[] localIndex, int[] chunkMin, int lod) {{
        assert(globalIndex.length == localIndex.length);
        assert(localIndex.length <= chunkMin.length);
        for (int i = 0; i < localIndex.length; i++) {{
            globalIndex[i] = chunkMin[i] + (localIndex[i] << lod);
        }}
    }}
	
	@Override
	protected void onDisposing(long native_object, boolean isDisposing) {{
		dtorImpl(native_object, isDisposing);
	}}
	
    public void release() {{
		this.dispose();
    }}
	
	public void close() {{
		this.dispose();
	}}
	
}}
