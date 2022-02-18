{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}

    ///AUTOGEN-IGNORE: CXX_METHOD GetMetadataBLOB void (const char *, const char *, const void **, uint64_t *) const FUNCTIONPROTO
	
    public void setMetadataBLOB(String category, String name, byte[] data) {{
        SetMetadataBLOBImpl(getNativeObject(), category, name, ByteBuffer.wrap(data));
    }}	
}}


