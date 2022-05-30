{class_docstring}
{class_visibility}{class_type} {class_name}{class_extends_txt}{class_implements_txt} {{
	static {{
		/* Ensure jni wrapper library is loaded */
		ManagedBase.staticInit();
	}}
{class_body}
    private static native void writeArrayR32Impl(long native_object, float[] src_data, String channel);
    private static native void writeArrayR64Impl(long native_object, double[] src_data, String channel);
    private static native void writeArrayU8Impl(long native_object, byte[] src_data, String channel);
    private static native void writeArrayU16Impl(long native_object, short[] src_data, String channel);
    private static native void writeArrayU32Impl(long native_object, int[] src_data, String channel);
    private static native void writeArrayU64Impl(long native_object, long[] src_data, String channel);
    private static native void writeArrayBoolImpl(long native_object, boolean[] src_data, String channel);
    
    public static void writeArray(VDS vds, float[] src_data, String channel_name) {{
        writeArrayR32Impl(vds.getNativeObject(), src_data, channel_name);
    }}

    public static void writeArray(VDS vds, double[] src_data, String channel_name) {{
        writeArrayR64Impl(vds.getNativeObject(), src_data, channel_name);
    }}

    public static void writeArray(VDS vds, byte[] src_data, String channel_name) {{
        writeArrayU8Impl(vds.getNativeObject(), src_data, channel_name);
    }}

    public static void writeArray(VDS vds, short[] src_data, String channel_name) {{
        writeArrayU16Impl(vds.getNativeObject(), src_data, channel_name);
    }}
	
    public static void writeArray(VDS vds, int[] src_data, String channel_name) {{
        writeArrayU32Impl(vds.getNativeObject(), src_data, channel_name);
    }}

    public static void writeArray(VDS vds, long[] src_data, String channel_name) {{
        writeArrayU64Impl(vds.getNativeObject(), src_data, channel_name);
    }}
	
    public static void writeArray(VDS vds, boolean[] src_data, String channel_name) {{
        writeArrayBoolImpl(vds.getNativeObject(), src_data, channel_name);
    }}
}}
