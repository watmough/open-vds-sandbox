{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}

    /**
     * Get the buffer the request is writing to as an IntBuffer.
     * The backing data is actually stored as unsigned 32-bit integer values.
     * However, Java does not directly support unsigned integer types, so care should 
     * be taken when working with the values read from the buffer.
     * 
     * @return The buffer the request is writing to as an IntBuffer
     */
    public java.nio.IntBuffer getIntBuffer() {{
        return getBuffer().asIntBuffer();
    }}

	/**
	 * Copy buffer data to an array.
	 *
	 * @return An array containing the request buffer data.
	 */
	public int[] toArray() {{
		java.nio.IntBuffer buffer = getIntBuffer();
		int[] data = new int[buffer.capacity()];
		buffer.get(data);
		return data;
	}}
}}
