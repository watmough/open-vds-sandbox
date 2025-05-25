# BenchmarkCpp

This is a helper to facilitate evaluating OpenVDS performance in a number of scenarios.

## Running with Valgrind / Calgrind

The two options in the command-line below `[-atstart=no] [--instr-atstart=no]` are optional, dependent on 
whether you wish to rebuild OpenVDS with the suggested changes to instrument only the specific wavelet 
decompression pieces.

```
#include <valgrind/callgrind.h>

...

  CALLGRIND_START_INSTRUMENTATION;
  CALLGRIND_TOGGLE_COLLECT;


  if (!wavelet.Decompress(true, -1, -1, -1, &startThreshold, &threshold, dataBlock.Format, valueRange, integerScale, integerOffset, isUseNoValue, noValue, &isAnyNoValue, &waveletNoValue, isNormalize, nDecompressLevel, isLossless, nCompressedAdaptiveDataSize, dataBlock, target, errorCode, errorString))
    return false;


  CALLGRIND_TOGGLE_COLLECT;
  CALLGRIND_STOP_INSTRUMENTATION;

```

'''
valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes --collect
[-atstart=no] [--instr-atstart=no] ./build/examples/BenchmarkCpp/benchmark-cpp ~/Shared/Example_VDS/<wavelet_compressed>.vds 13
```
