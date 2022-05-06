/****************************************************************************
** Copyright 2022 The Open Group
** Copyright 2022 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**     http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#ifndef WAVELETINVERSETRANSFORM_H
#define WAVELETINVERSETRANSFORM_H

#include "WaveletTypes.h"

namespace Wavelet {

void Wavelet_CreateTransformData(Wavelet_TransformData* transformData, IntVector3* bandSize, int* transformMask, int transformIterations);
#ifdef ENABLE_SSE_TRANSFORM
void WaveletTransform_InverseTransform_SSE(int32_t threadCount, float* tempBuffer, int32_t tempBufferSize, float* source, int32_t transformIterations, const IntVector3(&bandSize)[TRANSFORM_MAX_ITERATIONS + 1], const int32_t(&transformMask)[TRANSFORM_MAX_ITERATIONS], int32_t allocatedSizeX, int32_t allocatedSizeXY, uint32_t integerInfo);
#endif
void WaveletTransform_InverseTransform(float* source, int32_t transformIterations, const IntVector3(&bandSize)[TRANSFORM_MAX_ITERATIONS + 1], const int32_t(&transformMask)[TRANSFORM_MAX_ITERATIONS], int32_t allocatedSizeX, int32_t allocatedSizeXY, uint32_t integerInfo);

}

#endif
