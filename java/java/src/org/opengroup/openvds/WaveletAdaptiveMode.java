package org.opengroup.openvds;

/**
 * Wavelet adaptative mode, used to select quality when querying samples on compressed data.
 */
public enum WaveletAdaptiveMode {
    BestQuality, ///< The best quality available data is loaded (this is the only setting which will load lossless data).
    Tolerance,   ///< An adaptive level closest to the global compression tolerance is selected when loading wavelet compressed data.
    Ratio        ///< An adaptive level closest to the global compression ratio is selected when loading wavelet compressed data.
}
