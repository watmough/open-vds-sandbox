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

#pragma once

#include "SEGYUtils/SEGYFileInfo.h"
#include <unordered_map>

struct TraceInfo2D
{
  int       energySourcePointNumber;
  int       ensembleNumber;
  double    x;
  double    y;
  int       startTime;
  int64_t   traceNumber;
};

class TraceInfo2DManager
{
public:
  virtual ~TraceInfo2DManager() = default;

  virtual void Clear() {}

  virtual void AddTraceInfo(const char* traceHeader, int64_t traceNumber) {}

  virtual int Count() { return 0; }

  virtual const TraceInfo2D& Get(int traceNumber) { return m_placeHolder; }

  virtual const TraceInfo2D& Front() { return m_placeHolder; }

  virtual int GetIndexOfEnsembleNumber(int ensembleNumber) { return 0; }

private:
  TraceInfo2D m_placeHolder{};
};

class TraceInfo2DManagerStore : public TraceInfo2DManager
{
public:
  TraceInfo2DManagerStore() = default;

  TraceInfo2DManagerStore(const TraceInfo2D* pData, int64_t count);

  void Clear() override;

  int Count() override;

  const TraceInfo2D& Get(int traceNumber) override;

  const TraceInfo2D& Front() override;

  int GetIndexOfEnsembleNumber(int ensembleNumber) override;

protected:
  std::vector<TraceInfo2D>  m_traceInfo;
  std::unordered_map<int, int>
                            m_ensembleToIndex;

  void BuildEnsembleNumberMap();

  void ClearEnsembleNumberMap();
};

class TraceInfo2DManagerImpl : public TraceInfo2DManagerStore
{
public:
  TraceInfo2DManagerImpl(SEGY::Endianness endianness, double scaleOverride, const SEGY::HeaderField& coordinateScaleHeaderField, const SEGY::HeaderField& xCoordinateHeaderField, const SEGY::HeaderField& yCoordinateHeaderField, const SEGY::HeaderField& startTimeHeaderField, const SEGY::HeaderField& espnHeaderField, const SEGY::HeaderField& ensembleNumberHeaderField);

  void Clear() override;

  void AddTraceInfo(const char* traceHeader, int64_t traceNumber) override;

private:
  SEGY::Endianness          m_endianness;
  double                    m_scaleOverride;
  SEGY::HeaderField         m_coordinateScaleHeaderField;
  SEGY::HeaderField         m_xCoordinateHeaderField;
  SEGY::HeaderField         m_yCoordinateHeaderField;
  SEGY::HeaderField         m_startTimeHeaderField;
  SEGY::HeaderField         m_espnHeaderField;
  SEGY::HeaderField         m_ensembleNumberHeaderField;

  int                       m_previousEnsembleNumber;
};