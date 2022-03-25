/*
 * Copyright 2022 Bluware, Inc.
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


#ifndef NDVEC_H_INCLUDED
#define NDVEC_H_INCLUDED

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <cassert>

namespace ndvec 
{

template<int DIMENSIONS, typename INDEX_TYPE=int>
struct NDVec
{
  INDEX_TYPE Val[DIMENSIONS];

  size_t
  _size(INDEX_TYPE index) const {
    return (index < DIMENSIONS) ? Val[index] * _size(index + 1) : 1;
  }


  // Return the product of all components
  size_t
  size() const
  {
    return _size(0);
  }

  static NDVec create(INDEX_TYPE const* array, size_t arraySize) {
    NDVec v;
    assert(arraySize >= DIMENSIONS);
    for (int i = 0; i < DIMENSIONS; ++i) {
      v.Val[i] = array[i];
    }
    return v;
  }

  // Set all components to the same value
  NDVec& init(INDEX_TYPE val) {
    for (int i = 0; i < DIMENSIONS; ++i) {
      Val[i] = val;
    }
    return *this;
  }

  // From a shape vector, compute the pitch/modulus for each dimension
  // The dimension 0 modulus will always be 1.
  NDVec to_pitch() const {
    NDVec p;
    INDEX_TYPE pitch = 1;
    for (int i = 0; i < DIMENSIONS; ++i) {
      p.Val[i] = pitch;
      pitch *= Val[i];
    }
    return p;
  }

  NDVec scale(INDEX_TYPE factor) const {
    NDVec s;
    for (int i = 0; i < DIMENSIONS; ++i) {
      s.Val[i] = Val[i] * factor;
    }
    return s;
  }

  NDVec add(NDVec const& rhs) const {
    NDVec s;
    for (int i = 0; i < DIMENSIONS; ++i) {
      s.Val[i] = Val[i] + rhs.Val[i];
    }
    return s;
  }

  NDVec sub(NDVec const& rhs) const {
    NDVec s;
    for (int i = 0; i < DIMENSIONS; ++i) {
      s.Val[i] = Val[i] - rhs.Val[i];
    }
    return s;
  }

};

template<int DIMENSIONS, typename INDEX_TYPE=int>
NDVec<DIMENSIONS, INDEX_TYPE> operator+ (NDVec<DIMENSIONS, INDEX_TYPE> const& lhs, NDVec<DIMENSIONS, INDEX_TYPE> const& rhs) { 
  return lhs.add(rhs);
}

template<int DIMENSIONS, typename INDEX_TYPE=int>
NDVec<DIMENSIONS, INDEX_TYPE> operator- (NDVec<DIMENSIONS, INDEX_TYPE> const& lhs, NDVec<DIMENSIONS, INDEX_TYPE> const& rhs) { 
  return lhs.sub(rhs);
}

template<int DIMENSIONS, typename INDEX_TYPE=int>
std::ostream& operator<<(std::ostream& os, NDVec<DIMENSIONS, INDEX_TYPE> const& ndvec)
{
  os << "[ ";
  for (int i = 0; i < DIMENSIONS; ++i) {
    if (i > 0) os << ", ";
    os << ndvec.Val[i];
  }
  os << " ]";
  return os;
}

// Iterates over an N-dimensional shape to produce all coordinates/offsets 
// with the dimension 0 coordinate increasing the fastest.
// To iterate over a sub-space, an offset position and pitch vector may be
// specified.
template<int DIMENSIONS, typename INDEX_TYPE=int>
struct NDIterator
{
  using Vec_t = NDVec<DIMENSIONS, INDEX_TYPE>;

  Vec_t m_Pos;
  Vec_t m_Shape;
  Vec_t m_Pitch;
  size_t m_Index;
  size_t m_Size;
  size_t m_Offset;
  size_t m_BaseOffset;

  NDIterator(Vec_t const& shape) : m_Pos(), m_Shape(shape), m_Pitch(shape.to_pitch()), m_Index(0), m_Size(shape.size()), m_Offset(0), m_BaseOffset(0) {
  }

  NDIterator(Vec_t const& shape, Vec_t const& pitch) : m_Pos(), m_Shape(shape), m_Pitch(pitch), m_Index(0), m_Size(shape.size()), m_Offset(0), m_BaseOffset(0) {
  }

  NDIterator(Vec_t const& shape, Vec_t const& pitch, Vec_t const& offsetpos) : m_Pos(), m_Shape(shape), m_Pitch(pitch), m_Index(0), m_Size(shape.size()), m_Offset(0), m_BaseOffset(0) {
    m_Offset = m_BaseOffset = posToOffset(offsetpos);
  }

  bool operator == (NDIterator const& rhs) const {
    return rhs.m_Index == m_Index;
  }

  bool operator != (NDIterator const& rhs) const {
    return rhs.m_Index != m_Index;
  }

  size_t posToOffset(Vec_t pos) const {
    assert(m_Pitch.Val[0] == 1);
    size_t offset = pos.Val[0];
    for (int i = 1; i < DIMENSIONS; ++i) {
      offset = offset + m_Pitch.Val[i] * pos.Val[i];
    }
    return offset;
  }

  NDIterator& invalidate() {
    m_Index = m_Size;
    return *this;
  }

  NDIterator& advance() {
    if (m_Index < m_Size) {
      ++m_Index;
      if (m_Pos.Val[0] < m_Shape.Val[0] - 1) {
        m_Pos.Val[0]++;
        m_Offset++;
      } else {
        for (int i = 1; i < DIMENSIONS; ++i) {
          if (m_Pos.Val[i] < m_Shape.Val[i] - 1) {
            m_Pos.Val[i]++;
            while (i > 0) {
              --i;
              m_Pos.Val[i] = 0;
            }
            break;
          }
        }
        m_Offset = posToOffset(m_Pos) + m_BaseOffset;
      }
    }
    return *this;
  }

  NDIterator& operator++ () {
    return advance();
  }

  bool valid() const {
    return m_Index < m_Size;
  }

  Vec_t const& pos() const {
    return m_Pos;
  }

  size_t index() const {
    return m_Index;
  }

  size_t offset() const {
    return m_Offset;
  }

  size_t size() const {
    return m_Size;
  }

  size_t operator* () const {
    return offset();
  }
};

} // namespace ndvec

#endif
