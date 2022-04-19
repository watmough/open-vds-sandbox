{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}

    /**
     * Get the buffer the request is writing to as a FloatBuffer.
     * 
     * @return The buffer the request is writing to as a FloatBuffer
     */
    public java.nio.FloatBuffer getFloatBuffer() {{
        return getBuffer().asFloatBuffer();
    }}

	/**
	 * Copy buffer data to an array.
	 *
	 * @return An array containing the request buffer data.
	 */
	public float[] toArray() {{
		java.nio.FloatBuffer buffer = getFloatBuffer();
		float[] data = new float[buffer.capacity()];
		buffer.get(data);
		return data;
	}}
}}
