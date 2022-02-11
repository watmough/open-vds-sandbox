{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}
///AUTOGEN-IGNORE: CXX_METHOD Buffer void *() const FUNCTIONPROTO
    native ByteBuffer GetBufferImpl(long native_object);

    /**
     * Get the buffer the request is writing to.
     * Note that each call to this function will return a new ByteBuffer object, 
     * but each ByteBuffer will wrap the same underlying memory buffer.
     * 
     * @return The buffer the request is writing to
     */
    public ByteBuffer getBuffer() {{
        ByteBuffer buffer = GetBufferImpl(getNativeObject());
        ByteBuffer readOnlyBuffer = buffer.asReadOnlyBuffer();
        readOnlyBuffer.order(java.nio.ByteOrder.nativeOrder());
        return readOnlyBuffer;
    }}
	
    private String getErrorMessage() {{
        return "Error!"; // SteinFIXME!
    }}
	
	private int getErrorCode() {{
		return 0; // SteinFIXME!
	}}
        
    private void ensureRequestCompleted() {{
		if (!waitForCompletion()) {{
			if (isCanceled() && getErrorCode() == 0) {{
				throw new UnsupportedOperationException("Volume data request was canceled");
			}} else {{
				throw new RuntimeException(getErrorMessage() + ", Errorcode: " + Integer.toString(getErrorCode())); // SteinFIXME!
			}}
		}}
    }}
    
    /**
     * Wait for the VolumeDataRequest to complete successfully, and then get the buffer the request has written to.
     * If the request cannot be completed (e.g. it is canceled), the error that caused it to not be completed will 
     * be thrown as an exception.
     * Note that each call to this function will return a new ByteBuffer object, 
     * but each ByteBuffer will wrap the same underlying memory buffer.
     * 
     * @return The buffer the request has written to
     */
    public ByteBuffer getBufferSync() {{
        ensureRequestCompleted();
        return getBuffer();
    }}


    ///AUTOGEN-ADD-OVERLOAD: boolean waitForCompletion(int millisecondsBeforeTimeout) -> millisecondsBeforeTimeout = 0

}}
