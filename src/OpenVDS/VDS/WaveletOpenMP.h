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

#ifndef WAVELETOPENMP_H
#define WAVELETOPENMP_H

#include <algorithm>

#if defined(_OPENMP)
  #include <omp.h>
#else
  #ifndef omp_get_max_threads
    #define omp_get_max_threads() 1
  #endif
  #ifndef omp_get_thread_num
    #define omp_get_thread_num() 0
  #endif
  #ifndef omp_get_num_procs
    #define omp_get_num_procs() 1
  #endif
#endif

#define WAVELET_OPENMP_SSE_THREAD_COUNT 4
#define WAVELET_OPENMP_MEMORY_THREAD_COUNT 2

namespace OpenVDS
{

inline int Wavelet_GetEffectiveOpenMPThreadCount(int wantedThreadCount)
{
  return std::max(1, std::min(omp_get_num_procs() - 2, wantedThreadCount));
}

}

#endif
