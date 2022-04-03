/****************************************************************************
** Copyright 2021 The Open Group
** Copyright 2021 Bluware, Inc.
**
** Licensed under the Apache License, Version 2.0 (the "License");
** you may not use this file except in compliance with the License.
** You may obtain a copy of the License at
**
**   http://www.apache.org/licenses/LICENSE-2.0
**
** Unless required by applicable law or agreed to in writing, software
** distributed under the License is distributed on an "AS IS" BASIS,
** WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
** See the License for the specific language governing permissions and
** limitations under the License.
****************************************************************************/

#include "TraceInfo2DManager.h"

TraceInfo2DManagerStore::TraceInfo2DManagerStore(const TraceInfo2D* pData, int64_t count) : m_traceInfo(pData, pData + count)
{
}

void TraceInfo2DManagerStore::Clear()
{
  m_traceInfo.clear();
  ClearEnsembleNumberMap();
}

int TraceInfo2DManagerStore::Count()
{
  return static_cast<int>(m_traceInfo.size());
}

const TraceInfo2D& TraceInfo2DManagerStore::Get(int traceNumber)
{
  return m_traceInfo.at(traceNumber);
}

const TraceInfo2D& TraceInfo2DManagerStore::Front()
{
  return m_traceInfo.front();
}

int TraceInfo2DManagerStore::GetIndexOfEnsembleNumber(int ensembleNumber)
{
  if (m_ensembleToIndex.size() != m_traceInfo.size())
  {
    BuildEnsembleNumberMap();
  }

  const auto
    iter = m_ensembleToIndex.find(ensembleNumber);
  assert(iter != m_ensembleToIndex.end());
  if (iter == m_ensembleToIndex.end())
  {
    return 0;
  }

  return iter->second;
}

void TraceInfo2DManagerStore::BuildEnsembleNumberMap()
{
  m_ensembleToIndex.clear();
  for (int index = 0; index < static_cast<int>(m_traceInfo.size()); ++index)
  {
    m_ensembleToIndex.insert(std::make_pair(m_traceInfo[index].ensembleNumber, index));
  }
}

void TraceInfo2DManagerStore::ClearEnsembleNumberMap()
{
  m_ensembleToIndex.clear();
}

TraceInfo2DManagerImpl::TraceInfo2DManagerImpl(SEGY::Endianness endianness, double scaleOverride, const SEGY::HeaderField& coordinateScaleHeaderField, const SEGY::HeaderField& xCoordinateHeaderField, const SEGY::HeaderField& yCoordinateHeaderField, const SEGY::HeaderField& startTimeHeaderField, const SEGY::HeaderField& espnHeaderField, const SEGY::HeaderField& ensembleNumberHeaderField)
  : m_endianness(endianness), m_scaleOverride(scaleOverride), m_coordinateScaleHeaderField(coordinateScaleHeaderField), m_xCoordinateHeaderField(xCoordinateHeaderField), m_yCoordinateHeaderField(yCoordinateHeaderField), m_startTimeHeaderField(startTimeHeaderField), m_espnHeaderField(espnHeaderField), m_ensembleNumberHeaderField(ensembleNumberHeaderField), m_previousEnsembleNumber(-1)
{
}

void TraceInfo2DManagerImpl::Clear()
{
  TraceInfo2DManagerStore::Clear();
  m_previousEnsembleNumber = -1;
}

void TraceInfo2DManagerImpl::AddTraceInfo(const char* traceHeader, int64_t traceNumber)
{
  double coordScaleFactor = 1.0;

  if (m_scaleOverride == 0.0)
  {
    int scale = SEGY::ReadFieldFromHeader(traceHeader, m_coordinateScaleHeaderField, m_endianness);
    if (scale < 0)
    {
      coordScaleFactor = 1.0 / static_cast<double>(std::abs(scale));
    }
    else if (scale > 0)
    {
      coordScaleFactor = 1.0 * static_cast<double>(std::abs(scale));
    }
  }
  else
  {
    coordScaleFactor = m_scaleOverride;
  }

  TraceInfo2D traceInfo{};

  traceInfo.energySourcePointNumber = SEGY::ReadFieldFromHeader(traceHeader, m_espnHeaderField, m_endianness);
  traceInfo.ensembleNumber = SEGY::ReadFieldFromHeader(traceHeader, m_ensembleNumberHeaderField, m_endianness);
  traceInfo.x = coordScaleFactor * SEGY::ReadFieldFromHeader(traceHeader, m_xCoordinateHeaderField, m_endianness);
  traceInfo.y = coordScaleFactor * SEGY::ReadFieldFromHeader(traceHeader, m_yCoordinateHeaderField, m_endianness);
  traceInfo.startTime = SEGY::ReadFieldFromHeader(traceHeader, m_startTimeHeaderField, m_endianness);
  traceInfo.traceNumber = traceNumber;

  if (m_previousEnsembleNumber != traceInfo.ensembleNumber)
  {
    m_traceInfo.push_back(traceInfo);
    m_previousEnsembleNumber = traceInfo.ensembleNumber;

    // any modification to trace info collection should invalidate the ensemble number mapping
    ClearEnsembleNumberMap();
  }
}
