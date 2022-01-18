/*
 * Copyright 2019 The Open Group
 * Copyright 2019 INT, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.opengroup.openvds;

import java.io.IOException;

public class OpenVDS extends VdsHandle {
    private static native long cpOpenAWS(String bucket, String key, String region, String endPointOverride, String accessKeyId, String secretKey,
                                         String sessionToken, String expiration,
                                         String logFilenamePrefix, String loglevel, int connectionTimeoutMs, int requestTimeoutMs, boolean disableInitApi,
                                         int waveletAdaptiveMode, float waveletAdaptiveTolerance, float waveletAdaptiveRatio) throws IOException;

    private static native long cpOpenAzure(String pConnectionString, String pContainer, String pBlob,
                                           int pParallelismFactor, int pMaxExecutionTime,
                                           int waveletAdaptiveMode, float waveletAdaptiveTolerance, float waveletAdaptiveRatio) throws IOException;

    private static native long cpOpenGoogle(String bucket, String pathPrefix, int waveletAdaptiveMode, float waveletAdaptiveTolerance, float waveletAdaptiveRatio) throws IOException;

    private static native long cpOpenAzurePresigned(String baseUrl, String urlSuffix, int waveletAdaptiveMode, float waveletAdaptiveTolerance, float waveletAdaptiveRatio) throws IOException;

    private static native long cpOpenConnection(String url, String connectionString) throws IOException;

    private static native long cpOpenConnectionWithAdaptiveCompressionRatio(String url, String connectionString, float adaptiveRatio) throws IOException;

    private static native long cpOpenConnectionWithAdaptiveCompressionTolerance(String url, String connectionString, float adaptiveTolerance) throws IOException;

    private static native boolean cpIsCompressionMethodSupported(int compressionMethod);

    private static native long cpOpenVDSFile(String filePath, int waveletAdaptiveMode, float waveletAdaptiveTolerance, float waveletAdaptiveRatio) throws IOException;

    private static native long cpCreate(OpenOptions options, VolumeDataLayoutDescriptor layoutDescriptor,
                                        VolumeDataAxisDescriptor[] axisDescriptors,
                                        VolumeDataChannelDescriptor[] channelDescriptors,
                                        MetadataContainer mdContainer, VdsError error) throws IOException;

    private static native long cpCreateAzure(String pConnectionString, String pContainer, String pBlob,
                                             int pParallelismFactor, int pMaxExecutionTime,
                                             VolumeDataLayoutDescriptor ld, VolumeDataAxisDescriptor[] vda,
                                             VolumeDataChannelDescriptor[] vdc, MetadataReadAccess md, int compressionMethod, float compressionTolerance) throws IOException;

    private static native long cpCreateVDSFile(String pFilePath,
                                             VolumeDataLayoutDescriptor ld, VolumeDataAxisDescriptor[] vda,
                                             VolumeDataChannelDescriptor[] vdc, MetadataReadAccess md, int compressionMethod, float compressionTolerance) throws IOException;

    private static native long cpCreateAws(String bucket, String key, String region, String endPointOverride, String accessKeyId, String secretKey, String sessionToken, String expiration,
                                           String logFilenamePrefix, String loglevel, int connectionTimeoutMs, int requestTimeoutMs, boolean disableInitApi,
                                           VolumeDataLayoutDescriptor ld, VolumeDataAxisDescriptor[] vda,
                                           VolumeDataChannelDescriptor[] vdc, MetadataReadAccess md, int compressionMethod, float compressionTolerance) throws IOException;
    
    private static native long cpCreateGoogle(String bucket, String key,
                                              VolumeDataLayoutDescriptor ld, VolumeDataAxisDescriptor[] vda,
                                              VolumeDataChannelDescriptor[] vdc, MetadataReadAccess md, int compressionMethod, float compressionTolerance) throws IOException;
    
    private static native long cpCreateAzurePresigned(String baseUrl, String urlSuffix,
                                                      VolumeDataLayoutDescriptor ld, VolumeDataAxisDescriptor[] vda,
                                                      VolumeDataChannelDescriptor[] vdc, MetadataReadAccess md, int compressionMethod, float compressionTolerance) throws IOException;

    private static native long cpCreateConnection(String url, String connectionString,
                                                  VolumeDataLayoutDescriptor ld, VolumeDataAxisDescriptor[] vda,
                                                  VolumeDataChannelDescriptor[] vdc, MetadataReadAccess md, int compressionMethod, float compressionTolerance) throws IOException;

    private static native void cpWriteData_r64(JniPointer ptr, double[] src_data, String channel);
    private static native void cpWriteData_r32(JniPointer ptr, float[] src_data, String channel);
    private static native void cpWriteData_u32(JniPointer ptr, int[] src_data, String channel);
    private static native void cpWriteData_u8(JniPointer ptr, byte[] src_data, String channel);
    private static native void cpWriteData_bool(JniPointer ptr, boolean[] src_data, String channel);


    private OpenVDS(long handle, boolean ownHandle) {
        super(handle, ownHandle);
    }

    public static OpenVDS open(AWSOpenOptions o) throws IOException {
        if (o == null) throw new IllegalArgumentException("open option can't be null");
        return new OpenVDS(cpOpenAWS(o.bucket, o.key, o.region, o.endPointOverride, o.accessKeyId, o.secretKey, o.sessionToken, o.expiration,
                o.logFilenamePrefix, o.logLevel, o.connectionTimeoutMs, o.requestTimeoutMs, o.disableInitApi,
                o.waveletAdaptiveMode.ordinal(), o.waveletAdaptiveTolerance, o.waveletAdaptiveRatio), true);
    }

    public static OpenVDS open(AzureOpenOptions o) throws IOException {
        if (o == null) throw new IllegalArgumentException("open option can't be null");
        return new OpenVDS(cpOpenAzure(o.connectionString, o.container, o.blob, o.parallelism_factor, o.max_execution_time,
                o.waveletAdaptiveMode.ordinal(), o.waveletAdaptiveTolerance, o.waveletAdaptiveRatio), true);
    }

    public static OpenVDS open(AzurePresignedOpenOptions o) throws IOException {
        if (o == null) throw new IllegalArgumentException("open option can't be null");
        return new OpenVDS(cpOpenAzurePresigned(o.baseUrl, o.urlSuffix, o.waveletAdaptiveMode.ordinal(), o.waveletAdaptiveTolerance, o.waveletAdaptiveRatio), true);
    }

    public static OpenVDS open(GoogleOpenOptions o) throws IOException {
        if (o == null) throw new IllegalArgumentException("open option can't be null");
        return new OpenVDS(cpOpenGoogle(o.bucket, o.pathPrefix, o.waveletAdaptiveMode.ordinal(), o.waveletAdaptiveTolerance, o.waveletAdaptiveRatio), true);
    }

    public static OpenVDS open(String url, String connectionString) throws IOException {
        if ("".equals(url)) throw new IllegalArgumentException("url can't be empty");
        return new OpenVDS(cpOpenConnection(url, connectionString), true);
    }

    public static OpenVDS openWithAdaptiveCompressionRatio(String url, String connectionString, float adaptiveRatio) throws IOException {
        if ("".equals(url)) throw new IllegalArgumentException("url can't be empty");
        return new OpenVDS(cpOpenConnectionWithAdaptiveCompressionRatio(url, connectionString, adaptiveRatio), true);
    }

    public static OpenVDS openWithAdaptiveCompressionTolerance(String url, String connectionString, float adaptiveTolerance) throws IOException {
        if ("".equals(url)) throw new IllegalArgumentException("url can't be empty");
        return new OpenVDS(cpOpenConnectionWithAdaptiveCompressionTolerance(url, connectionString, adaptiveTolerance), true);
    }

    public static OpenVDS open(VDSFileOpenOptions o) throws IOException {
        if (o == null) throw new IllegalArgumentException("open option can't be null");
        return new OpenVDS(cpOpenVDSFile(o.filePath, o.waveletAdaptiveMode.ordinal(), o.waveletAdaptiveTolerance, o.waveletAdaptiveRatio), true);
    }

    private static void validateCreateArguments(VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IllegalArgumentException {

        if (ld == null) {
            throw new IllegalArgumentException("VolumeDataLayoutDescriptor can't be null");
        }

        if (vda == null || java.util.Arrays.stream(vda).allMatch(a -> a == null)) {
            throw new IllegalArgumentException("VolumeDataLayoutDescriptor or its elements can't be null");
        }

        if (vdc == null || java.util.Arrays.stream(vdc).allMatch(a -> a == null)) {
            throw new IllegalArgumentException("VolumeDataChannelDescriptor or its elements can't be null");
        }

        if (md == null) {
            throw new IllegalArgumentException("MetadataReadAccess can't be null");
        }
    }

    private static <OpenOpt> void validateCreateArguments(OpenOpt o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IllegalArgumentException {

        if (o == null) {
            throw new IllegalArgumentException("open option can't be null");
        }
        validateCreateArguments(ld, vda, vdc, md);
    }

    public static boolean isCompressionMethodSupported(CompressionMethod compressionMethod) {
      return cpIsCompressionMethodSupported(compressionMethod.ordinal());
    }

    public static OpenVDS create(AzureOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateAzure(o.connectionString, o.container, o.blob, 
                                         o.parallelism_factor, o.max_execution_time,
                                         ld, vda, vdc, md, 0, 0), true);
    }

    public static OpenVDS create(AzureOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md, CompressionMethod compressionMethod, float compressionTolerance) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateAzure(o.connectionString, o.container, o.blob, 
                                         o.parallelism_factor, o.max_execution_time,
                                         ld, vda, vdc, md, compressionMethod.ordinal(), compressionTolerance), true);
    }

    public static OpenVDS create(VDSFileOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);
        return new OpenVDS(cpCreateVDSFile(o.filePath, ld, vda, vdc, md, 0, 0), true);
    }

    public static OpenVDS create(VDSFileOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md, CompressionMethod compressionMethod, float compressionTolerance) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);
        return new OpenVDS(cpCreateVDSFile(o.filePath, ld, vda, vdc, md, compressionMethod.ordinal(), compressionTolerance), true);
    }

    public static OpenVDS create(AWSOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateAws(o.bucket, o.key, o.region, o.endPointOverride, o.accessKeyId, o.secretKey, o.sessionToken, o.expiration,
                o.logFilenamePrefix, o.logLevel, o.connectionTimeoutMs, o.requestTimeoutMs, o.disableInitApi,
                                         ld, vda, vdc, md, 0, 0), true);
    }

    public static OpenVDS create(AWSOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md, CompressionMethod compressionMethod, float compressionTolerance) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateAws(o.bucket, o.key, o.region, o.endPointOverride, o.accessKeyId, o.secretKey, o.sessionToken, o.expiration,
                o.logFilenamePrefix, o.logLevel, o.connectionTimeoutMs, o.requestTimeoutMs, o.disableInitApi,
                ld, vda, vdc, md, compressionMethod.ordinal(), compressionTolerance), true);
    }

    public static OpenVDS create(GoogleOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateGoogle(o.bucket, o.pathPrefix, 
                                         ld, vda, vdc, md, 0, 0), true);
    }

    public static OpenVDS create(GoogleOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md, CompressionMethod compressionMethod, float compressionTolerance) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateGoogle(o.bucket, o.pathPrefix, 
                                         ld, vda, vdc, md, compressionMethod.ordinal(), compressionTolerance), true);
    }

    public static OpenVDS create(AzurePresignedOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateAzurePresigned(o.baseUrl, o.urlSuffix,
                                         ld, vda, vdc, md, 0, 0), true);
    }

    public static OpenVDS create(AzurePresignedOpenOptions o, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md, CompressionMethod compressionMethod, float compressionTolerance) throws IOException {
        validateCreateArguments(o, ld, vda, vdc, md);

        return new OpenVDS(cpCreateAzurePresigned(o.baseUrl, o.urlSuffix,
                                         ld, vda, vdc, md, compressionMethod.ordinal(), compressionTolerance), true);
    }

    public static OpenVDS create(String url, String connectionString, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md) throws IOException {
        if (url == null) {
            throw new IllegalArgumentException("url can't be null");
        }

        validateCreateArguments(ld, vda, vdc, md);

        return new OpenVDS(cpCreateConnection(url, connectionString,
                                         ld, vda, vdc, md, 0, 0), true);
    }

    public static OpenVDS create(String url, String connectionString, VolumeDataLayoutDescriptor ld,
                                 VolumeDataAxisDescriptor[] vda, VolumeDataChannelDescriptor[] vdc,
                                 MetadataReadAccess md, CompressionMethod compressionMethod, float compressionTolerance) throws IOException {
        if (url == null) {
            throw new IllegalArgumentException("url can't be null");
        }

        validateCreateArguments(ld, vda, vdc, md);

        return new OpenVDS(cpCreateConnection(url, connectionString,
                                         ld, vda, vdc, md, compressionMethod.ordinal(), compressionTolerance), true);
    }

    public static void writeArray(VdsHandle handle, double[] src_data, String channel_name) {
        cpWriteData_r64(handle, src_data, channel_name);
    }

    public static void writeArray(VdsHandle handle, float[] src_data, String channel_name) {
        cpWriteData_r32(handle, src_data, channel_name);
    }

    public static void writeArray(VdsHandle handle, int[] src_data, String channel_name) {
        cpWriteData_u32(handle, src_data, channel_name);
    }

    public static void writeArray(VdsHandle handle, byte[] src_data, String channel_name) {
        cpWriteData_u8(handle, src_data, channel_name);
    }

    public static void writeArray(VdsHandle handle, boolean[] src_data, String channel_name) {
        cpWriteData_bool(handle, src_data, channel_name);
    }
}
