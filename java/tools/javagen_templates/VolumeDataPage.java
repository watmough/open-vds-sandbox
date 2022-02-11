{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} implements AutoCloseable {{
{class_body}
///AUTOGEN-IGNORE: CXX_METHOD GetBuffer const void *(int (&)[6]) FUNCTIONPROTO
    native ByteBuffer GetBufferImpl(long native_object, int[] pitch);
    
    /**
     * @param pitch An array of length 6 which describes the layout of the returned buffer : Each 
     *              non-zero element is equal to the distance between neighboring elements (bytes, floats, etc.)
     *              along each dimension in the buffer. 
     * @return A read-only ByteBuffer
     */
    public ByteBuffer getBuffer(int[] pitch) {{
        if (pitch.length != 6) throw new IllegalArgumentException("Array \"pitch\" must have length 6");
        ByteBuffer buffer = GetBufferImpl(getNativeObject(), pitch);
        buffer.order(java.nio.ByteOrder.nativeOrder());
        return buffer.asReadOnlyBuffer();
    }}

///AUTOGEN-IGNORE: CXX_METHOD GetWritableBuffer void *(int (&)[6]) FUNCTIONPROTO

    native ByteBuffer GetWritableBufferImpl(long native_object, int[] pitch);
    
    /**
     * @param pitch An array of length 6 which describes the layout of the returned buffer : Each 
     *              non-zero element is equal to the distance between neighboring elements (bytes, floats, etc.)
     *              along each dimension in the buffer. 
     * @return A writable ByteBuffer
     */
    public ByteBuffer getWritableBuffer(int[] pitch) {{
        if (pitch.length != 6) throw new IllegalArgumentException("Array \"pitch\" must have length 6");
        ByteBuffer buffer = GetWritableBufferImpl(getNativeObject(), pitch);
        buffer.order(java.nio.ByteOrder.nativeOrder());
        return buffer;
    }}
	
///AUTOGEN-IGNORE: CXX_METHOD Release void () FUNCTIONPROTO
    native private void ReleaseImpl(long native_object);
	
	@Override
	protected void onDisposing(long native_object) {{
        ReleaseImpl(native_object);
		dtorImpl(native_object);
	}}
	
    public void release() {{
		this.dispose();
    }}
	
	public void close() {{
		this.dispose();
	}}
	
}}
