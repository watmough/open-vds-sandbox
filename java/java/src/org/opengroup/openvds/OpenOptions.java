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
 * Base class for options for opening a VDS
 */
public class OpenOptions {

    public enum ConnectionType {
        AWS,
        Azure,
        AzurePresigned,
        Google,
        File,
        InMemory;
    }

    public int connectionType;

    public WaveletAdaptiveMode waveletAdaptiveMode = WaveletAdaptiveMode.BestQuality;
    public float waveletAdaptiveTolerance = 0.01f;
    public float waveletAdaptiveRatio = 1.0f;

    /**
     * Constructor.
     *
     * @param cType the connection type (see static members of this class)
     */
    protected OpenOptions(ConnectionType cType) {
        connectionType = cType.ordinal();
    }

    /**
     * Constructor.
     *
     * @param cType the connection type (see static members of this class)
     * @param wam wavelet adaptive method
     * @param wat wavelet adaptive tolerance
     * @param war wavelet adaptive ratio
     */
    protected OpenOptions(ConnectionType cType, WaveletAdaptiveMode wam, float wat, float war) {
        connectionType = cType.ordinal();
        waveletAdaptiveMode = wam;
        waveletAdaptiveTolerance = wat;
        waveletAdaptiveRatio = war;
    }
}
