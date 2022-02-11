{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}

    /**
     * Get the buffer the request is writing to as a ShortBuffer.
     * The backing data is actually stored as unsigned 16-bit integer values.
     * However, Java does not directly support unsigned integer types, so care should 
     * be taken when working with the values read from the buffer.
     * 
     * @return The buffer the request is writing to as a ShortBuffer
     */
    public java.nio.ShortBuffer getShortBuffer() {{
        return getBuffer().asShortBuffer();
    }}

}}
