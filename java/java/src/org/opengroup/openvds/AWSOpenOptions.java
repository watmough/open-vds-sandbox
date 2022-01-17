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

/**
 * Options for opening a VDS in AWS (Amazon Web Services) cloud computing
 * platform.
 */
public class AWSOpenOptions extends OpenOptions {

    public String bucket;
    public String key;
    public String region;
    public String endPointOverride;
    public String accessKeyId;
    public String secretKey;
    public String sessionToken;
    public String expiration;
    public String logFilenamePrefix;
    public String logLevel;
    public int connectionTimeoutMs = 3000;
    public int requestTimeoutMs = 6000;
    public boolean disableInitApi = false;

    /**
     * Default constructor.
     */
    public AWSOpenOptions() {
        super(ConnectionType.AWS);
    }

    /**
     * Constructor.
     *
     * @param pBucket           the bucket of the VDS
     * @param pKey              the key prefix of the VDS
     * @param pRegion           the region of the bucket of the VDS
     * @param pEndpointOverride This parameter allows to override the endpoint url
     */
    public AWSOpenOptions(String pBucket, String pKey, String pRegion, String pEndpointOverride) {
        super(ConnectionType.AWS);
        bucket = pBucket;
        key = pKey;
        region = pRegion;
        endPointOverride = pEndpointOverride;
    }

    /**
     * Constructor.
     *
     * @param pBucket           the bucket of the VDS
     * @param pKey              the key prefix of the VDS
     * @param pRegion           the region of the bucket of the VDS
     * @param pEndpointOverride This parameter allows to override the endpoint url
     * @apram wam wavelet adaptive method
     * @param wat wavelet adaptive tolerance
     * @param war wavelet adaptive ratio
     */
    public AWSOpenOptions(String pBucket, String pKey, String pRegion, String pEndpointOverride, WaveletAdaptiveMode wam, float wat, float war) {
        super(ConnectionType.AWS, wam, wat, war);
        bucket = pBucket;
        key = pKey;
        region = pRegion;
        endPointOverride = pEndpointOverride;
    }

    /**
     * Constructor.
     *
     * @param pBucket the bucket of the VDS
     * @param pKey    the key prefix of the VDS
     * @param pRegion the region of the bucket of the VDS
     */
    public AWSOpenOptions(String pBucket, String pKey, String pRegion) {
        this(pBucket, pKey, pRegion, null);
    }

    /**
     * Constructor with adaptive wavelet parameters
     *
     * @param pBucket the bucket of the VDS
     * @param pKey    the key prefix of the VDS
     * @param pRegion the region of the bucket of the VDS
     * @apram wam wavelet adaptive method
     * @param wat wavelet adaptive tolerance
     * @param war wavelet adaptive ratio
     */
    public AWSOpenOptions(String pBucket, String pKey, String pRegion, WaveletAdaptiveMode wam, float wat, float war) {
        this(pBucket, pKey, pRegion, null, wam, wat, war);
    }
}

