{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}

    /**
     * Get the buffer the request is writing to as a DoubleBuffer.
     * 
     * @return The buffer the request is writing to as a DoubleBuffer
     */
    public java.nio.DoubleBuffer getDoubleBuffer() {{
        return getBuffer().asDoubleBuffer();
    }}
	
	/**
	 * Copy buffer data to an array.
	 *
	 * @return An array containing the request buffer data.
	 */
	public double[] toArray() {{
		java.nio.DoubleBuffer buffer = getDoubleBuffer();
		double[] data = new double[buffer.capacity()];
		buffer.get(data);
		return data;
	}}
}}
