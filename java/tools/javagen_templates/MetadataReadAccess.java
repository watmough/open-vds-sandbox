{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
{class_body}
    native int[] GetMetadataKeyTypesImpl(long native_object);
    native String[] GetMetadataKeyCategoriesImpl(long native_object);
    native String[] GetMetadataKeyNamesImpl(long native_object);

    /**
     * Get metadata keys.
     * 
     * @return The metadata keys
     */
    public MetadataKey[] getMetadataKeys () {{
        int[] metadataKeyTypes = GetMetadataKeyTypesImpl(getNativeObject());
        String[] metadataKeyCategories = GetMetadataKeyCategoriesImpl(getNativeObject());
        String[] metadataKeyNames = GetMetadataKeyNamesImpl(getNativeObject());

        int nMetadataKey = metadataKeyTypes.length;
        MetadataKey[] metadataKeys = new MetadataKey[nMetadataKey];

        for (int iMetadataKey = 0; iMetadataKey < nMetadataKey; iMetadataKey++) {{
            metadataKeys[iMetadataKey] = new MetadataKey(MetadataType.fromInt(metadataKeyTypes[iMetadataKey]), metadataKeyCategories[iMetadataKey], metadataKeyNames[iMetadataKey]);
        }}

        return metadataKeys;
    }}
	
	private static native byte[] GetMetadataBLOBImpl(long native_handle, String category, String name);
	
	public byte[] getMetadataBLOB(String category, String name) {{
		return GetMetadataBLOBImpl(getNativeObject(), category, name);
	}}
	
}}
