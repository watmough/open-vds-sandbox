/****************************************************************************
** Copyright 2019 The Open Group
** Copyright 2019 Bluware, Inc.
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

#include "VolumeDataRegion.h"

#include <OpenVDS/VolumeDataLayout.h>

#include "VolumeDataChunk.h"
#include "VolumeDataChannelMapping.h"

#include <assert.h>
#include <algorithm>

namespace OpenVDS
{
int64_t VolumeDataRegion::GetNumChunksInRegion() const
{
  return m_chunksInRegion;
}

int64_t VolumeDataRegion::GetChunkIndexInRegion(int64_t chunkInRegion) const
{
  assert(chunkInRegion >= 0 && chunkInRegion < m_chunksInRegion);

  int64_t chunkIndex = 0;

  for(int dimension = int(ArraySize(m_chunkMin)) - 1; dimension >= 0; dimension--)
  {
    chunkIndex += (chunkInRegion / m_modulo[dimension] + m_chunkMin[dimension]) * m_layerModulo[dimension];
    chunkInRegion %= m_modulo[dimension];
  }

  return chunkIndex;
}

void VolumeDataRegion::GetChunksInRegion(std::vector<VolumeDataChunk>* volumeDataChunks, bool append) const
{
  if (!append)
  {
    volumeDataChunks->clear();
  }

  int chunksInRegion = (int)GetNumChunksInRegion();
  if(!chunksInRegion) return;

  volumeDataChunks->reserve(volumeDataChunks->size() + chunksInRegion);

  for(int chunkInRegion = 0; chunkInRegion < chunksInRegion; chunkInRegion++)
  {
    volumeDataChunks->push_back(m_volumeDataLayer->GetChunkFromIndex(GetChunkIndexInRegion(chunkInRegion)));
  }
}

bool VolumeDataRegion::IsChunkInRegion(VolumeDataChunk const &volumeDataChunk) const
{
  if(//volumeDataChunk.GetVDS() == _gVDS &&
     volumeDataChunk.layer == m_volumeDataLayer)
  {
    IndexArray indexArray;

    m_volumeDataLayer->ChunkIndexToIndexArray(volumeDataChunk.index, indexArray);
    for(int dimension = 0; dimension < ArraySize(indexArray); dimension++)
    {
      if(indexArray[dimension] < m_chunkMin[dimension] ||
         indexArray[dimension] > m_chunkMax[dimension])
      {
        return false;
      }
    }
    return true;
  }
  return false;
}

VolumeDataRegion::VolumeDataRegion(VolumeDataLayer const &volumeDataLayer, const IndexArray &min, const IndexArray &max, bool snapVoxelMax)
  : m_volumeDataLayer(&volumeDataLayer)
{
  const VolumeDataChannelMapping *volumeDataChannelMapping = volumeDataLayer.GetVolumeDataChannelMapping();

  int64_t modulo = 1;
  const int LOD = volumeDataLayer.GetLOD();

  for(int dimension = 0; dimension < ArraySize(m_chunkMin); dimension++)
  {
    int voxelMin = min[dimension];
    int voxelMax = max[dimension];

    // Snap voxel max to the minimum needed to ensure that the LOD size of the subset is the same as the given subset
    if(snapVoxelMax)
    {
      voxelMax = voxelMin + ((voxelMax - voxelMin - 1) & ~((1 << LOD) - 1)) + 1;
      assert(GetLODSize(min[dimension], max[dimension], LOD) == GetLODSize(voxelMin, voxelMax, LOD));
      assert(GetLODSize(min[dimension], max[dimension], LOD) == GetLODSize(voxelMin, voxelMax - 1, LOD) + 1);
    }

    if(volumeDataChannelMapping)
    {
      m_chunkMin[dimension] = volumeDataChannelMapping->GetMappedChunkIndexFromVoxel(volumeDataLayer.GetPrimaryChannelLayer(), voxelMin, dimension);
      m_chunkMax[dimension] = volumeDataChannelMapping->GetMappedChunkIndexFromVoxel(volumeDataLayer.GetPrimaryChannelLayer(), voxelMax - 1, dimension);
    }
    else
    {
      m_chunkMin[dimension] = volumeDataLayer.VoxelToIndex(voxelMin, dimension);
      m_chunkMax[dimension] = volumeDataLayer.VoxelToIndex(voxelMax - 1, dimension);
    }

    m_layerModulo[dimension] = volumeDataLayer.m_modulo[dimension];
    m_modulo[dimension] = modulo;
    modulo *= m_chunkMax[dimension] - m_chunkMin[dimension] + 1;

    assert(m_chunkMin[dimension] <= m_chunkMax[dimension]);
  }

  m_chunksInRegion = modulo;
}

VolumeDataRegion::VolumeDataRegion(VolumeDataLayer const &volumeDataLayer, const IndexArray &chunkMin, const IndexArray &chunkMax, struct FromChunkMinMax)
  : m_volumeDataLayer(&volumeDataLayer)
{
  int64_t modulo = 1;

  for(int dimension = 0; dimension < ArraySize(m_chunkMin); dimension++)
  {
    m_chunkMin[dimension] = chunkMin[dimension];
    m_chunkMax[dimension] = chunkMax[dimension];

    m_layerModulo[dimension] = volumeDataLayer.m_modulo[dimension];
    m_modulo[dimension] = modulo;
    modulo *= m_chunkMax[dimension] - m_chunkMin[dimension] + 1;

    assert(m_chunkMin[dimension] <= m_chunkMax[dimension]);
  }

  m_chunksInRegion = modulo;
}

VolumeDataRegion VolumeDataRegion::VolumeDataRegionOverlappingChunk(VolumeDataLayer const &volumeDataLayer, VolumeDataChunk const &volumeDataChunk, const IndexArray &offset)
{
  IndexArray min;
  IndexArray max;

  const VolumeDataLayer *targetLayer = volumeDataChunk.layer;

  assert(volumeDataLayer.GetVolumeDataChannelMapping() == targetLayer->GetVolumeDataChannelMapping() && "VolumeDataRegionOverlappingChunk() doesn't work between layers with different mappings");

  targetLayer->GetChunkMinMax(volumeDataChunk.index, min, max, false);

  IndexArray validMin;
  IndexArray validMax;

  for (int dimension = 0; dimension < ArraySize(validMin); dimension++)
  {
    int neededExtraValidVoxelsNegative = 0,
        neededExtraValidVoxelsPositive = 0;

    // Do we have a render margin in this dimension?
    if (DimensionGroupUtil::IsDimensionInGroup(targetLayer->GetOriginalDimensionGroup(), dimension))
    {
      // Can we copy from source's render margin in this dimension?
      if (DimensionGroupUtil::IsDimensionInGroup(volumeDataLayer.GetOriginalDimensionGroup(), dimension))
      {
        neededExtraValidVoxelsNegative = std::max(0, targetLayer->GetNegativeRenderMargin() - volumeDataLayer.GetNegativeRenderMargin());
        neededExtraValidVoxelsPositive = std::max(0, targetLayer->GetPositiveRenderMargin() - volumeDataLayer.GetPositiveRenderMargin());
      }
      else
      {
        neededExtraValidVoxelsNegative = targetLayer->GetNegativeRenderMargin();
        neededExtraValidVoxelsPositive = targetLayer->GetPositiveRenderMargin();
      }
    }

    neededExtraValidVoxelsNegative += targetLayer->GetNegativeMargin(dimension) - volumeDataLayer.GetNegativeMargin(dimension);
    neededExtraValidVoxelsPositive += targetLayer->GetPositiveMargin(dimension) - volumeDataLayer.GetPositiveMargin(dimension);

    validMin[dimension] = min[dimension] - offset[dimension] - std::max(0, neededExtraValidVoxelsNegative),
    validMax[dimension] = max[dimension] - offset[dimension] + std::max(0, neededExtraValidVoxelsPositive);

    // Limit the valid area so it doesn't extend into the border
    validMin[dimension] = std::max(validMin[dimension], targetLayer->GetDimensionFirstSample(dimension));
    validMax[dimension] = std::min(validMax[dimension], targetLayer->GetDimensionFirstSample(dimension) + targetLayer->GetDimensionNumSamples(dimension));
  }

  return VolumeDataRegion(volumeDataLayer, validMin, validMax);
}

}
