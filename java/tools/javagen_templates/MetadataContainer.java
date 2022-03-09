{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}

    ///AUTOGEN-IGNORE: CXX_METHOD GetMetadataBLOB void (const char *, const char *, const void **, uint64_t *) const FUNCTIONPROTO
	
    ///AUTOGEN-IGNORE: CXX_METHOD SetMetadataBLOB void (const char *, const char *, const void *, uint64_t) FUNCTIONPROTO
    native private void SetMetadataBLOBImpl(long native_object, String category, String name, byte[] data);
    public void setMetadataBLOB(String category, String name, byte[] data) {{
        SetMetadataBLOBImpl(getNativeObject(), ManagedBase.requireNonNull(category, "category may not be null"), ManagedBase.requireNonNull(name, "name may not be null"), ManagedBase.requireNonNull(data, "data may not be null"));
    }}
	
    public void setMetadataBLOB(String category, String name, ByteBuffer data) {{
        SetMetadataBLOBImpl(getNativeObject(), ManagedBase.requireNonNull(category, "category may not be null"), ManagedBase.requireNonNull(name, "name may not be null"), ManagedBase.requireNonNull(data, "data may not be null").array());
    }}
	
    public ByteBuffer getMetadataBLOBAsBuffer(String category, String name) {{
        return ByteBuffer.wrap(getMetadataBLOB(category, name));
    }}	
	
	public void setMetadataIntVector2(String category, String name, int[] value) {{ setMetadataIntVector2(category, name, new IntVector2(value)); }}
	public void setMetadataIntVector3(String category, String name, int[] value) {{ setMetadataIntVector3(category, name, new IntVector3(value)); }}
	public void setMetadataIntVector4(String category, String name, int[] value) {{ setMetadataIntVector4(category, name, new IntVector4(value)); }}
	public void setMetadataFloatVector2(String category, String name, float[] value) {{ setMetadataFloatVector2(category, name, new FloatVector2(value)); }}
	public void setMetadataFloatVector3(String category, String name, float[] value) {{ setMetadataFloatVector3(category, name, new FloatVector3(value)); }}
	public void setMetadataFloatVector4(String category, String name, float[] value) {{ setMetadataFloatVector4(category, name, new FloatVector4(value)); }}
	public void setMetadataDoubleVector2(String category, String name, double[] value) {{ setMetadataDoubleVector2(category, name, new DoubleVector2(value)); }}
	public void setMetadataDoubleVector3(String category, String name, double[] value) {{ setMetadataDoubleVector3(category, name, new DoubleVector3(value)); }}
	public void setMetadataDoubleVector4(String category, String name, double[] value) {{ setMetadataDoubleVector4(category, name, new DoubleVector4(value)); }}
	
}}


