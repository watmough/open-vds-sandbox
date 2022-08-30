//=============================================================================
// <copyright>
// Copyright (c) 2021 Bluware Inc. All rights reserved.
//
// All rights are reserved. Reproduction or transmission in whole or in part,
// in any form or by any means, electronic, mechanical or otherwise,
// is prohibited without the prior written permission of the copyright owner.
// </copyright>
//=============================================================================

#include <OpenVDS/IJKCoordinateTransformer.h>
#include <OpenVDS/KnownMetadata.h>
#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/IO/IOManager.h>
#include <OpenVDS/IO/IOManagerInMemory.h>
#include <OpenVDS/GlobalMetadataCommon.h>

//#include "../utils/GenerateVDS.h"

#include <fmt/format.h>

#include <OpenVDS/VDS/VDS.h>

#include "gtest/gtest.h"

static const double IJKCoordinateTransformerTests_TestMaxDelta = 0.00000001;

TEST(IJKCoordinateTransformerTests, TestBasicFunctionality)
{
  OpenVDS::IJKGridDefinition
    ijkGridDefinition(OpenVDS::DoubleVector3(100, 200, 300), OpenVDS::DoubleVector3(1, 0, 0), OpenVDS::DoubleVector3(0, 2, 0), OpenVDS::DoubleVector3(0, 0, 3));
  OpenVDS::IntVector3
    ijkSize(10, 20, 30);
  OpenVDS::IntVector3
    ijkToVoxelDimensionMap021(0, 2, 1);
  OpenVDS::DoubleVector3
    ijkAnnotationStart(1000, 2000, 3000),
    ijkAnnotationEnd(1500, 2500, 3500);

  OpenVDS::VDSIJKGridDefinition
    vdsIJKGridDefinition(ijkGridDefinition, OpenVDS::IntVector3(1, 2, 0));

  {
    // Contructor with IJK grid definition and size; default IJK to voxel dimension map 2/1/0, annotations not defined
    OpenVDS::IJKCoordinateTransformer
      ijkCoordinateTransformer(ijkGridDefinition, ijkSize);

    ASSERT_EQ(ijkGridDefinition, ijkCoordinateTransformer.IJKGrid());
    ASSERT_EQ(OpenVDS::IntVector3(2, 1, 0), ijkCoordinateTransformer.IJKToVoxelDimensionMap());
    ASSERT_FALSE(ijkCoordinateTransformer.AnnotationsDefined());
  }

  {
    // Contructor with IJK grid definition, size, and IJK to voxel dimension map; annotations not defined
    OpenVDS::IJKCoordinateTransformer
      ijkCoordinateTransformer(ijkGridDefinition, ijkSize, ijkToVoxelDimensionMap021);

    ASSERT_EQ(ijkGridDefinition, ijkCoordinateTransformer.IJKGrid());
    ASSERT_EQ(OpenVDS::IntVector3(0, 2, 1), ijkCoordinateTransformer.IJKToVoxelDimensionMap());
    ASSERT_FALSE(ijkCoordinateTransformer.AnnotationsDefined());
  }

  {
    // Contructor with VDS IJK grid definition (includes IJK to voxel dimension map) and size; annotations not defined
    OpenVDS::IJKCoordinateTransformer
      ijkCoordinateTransformer(vdsIJKGridDefinition, ijkSize);

    ASSERT_EQ(ijkGridDefinition, ijkCoordinateTransformer.IJKGrid());
    ASSERT_EQ(OpenVDS::IntVector3(1, 2, 0), ijkCoordinateTransformer.IJKToVoxelDimensionMap());
    ASSERT_FALSE(ijkCoordinateTransformer.AnnotationsDefined());
  }

  {
    // Contructor with IJK grid definition, size, annotations; default IJK to voxel dimension map 2/1/0
    OpenVDS::IJKCoordinateTransformer
      ijkCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd);

    ASSERT_EQ(ijkGridDefinition, ijkCoordinateTransformer.IJKGrid());
    ASSERT_EQ(OpenVDS::IntVector3(2, 1, 0), ijkCoordinateTransformer.IJKToVoxelDimensionMap());
    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(ijkAnnotationStart, ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(ijkAnnotationEnd, ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  {
    // Contructor with everything defined directly
    OpenVDS::IJKCoordinateTransformer
      ijkCoordinateTransformer(ijkGridDefinition, ijkSize, ijkToVoxelDimensionMap021, ijkAnnotationStart, ijkAnnotationEnd);

    ASSERT_EQ(ijkGridDefinition, ijkCoordinateTransformer.IJKGrid());
    ASSERT_EQ(OpenVDS::IntVector3(0, 2, 1), ijkCoordinateTransformer.IJKToVoxelDimensionMap());
    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(ijkAnnotationStart, ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(ijkAnnotationEnd, ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  {
    // Contructor with VDS IJK grid definition (includes IJK to voxel dimension map), size, annotations
    OpenVDS::IJKCoordinateTransformer
      ijkCoordinateTransformer(vdsIJKGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd);

    ASSERT_EQ(ijkGridDefinition, ijkCoordinateTransformer.IJKGrid());
    ASSERT_EQ(OpenVDS::IntVector3(1, 2, 0), ijkCoordinateTransformer.IJKToVoxelDimensionMap());
    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(ijkAnnotationStart, ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(ijkAnnotationEnd, ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  {
    OpenVDS::IJKCoordinateTransformer
      ijkCoordinateTransformer(ijkGridDefinition, ijkSize);

    // IJK ranges
    ASSERT_FALSE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(0, 0, 0)));
    ASSERT_EQ(0, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, 0)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(-1, 0, 0)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKPositionOutOfRange(OpenVDS::DoubleVector3(-0.01, 0, 0)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(-1, 0, 0)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(-0.01, 0, 0)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(0, -1, 0)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKPositionOutOfRange(OpenVDS::DoubleVector3(0, -0.01, 0)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(0, -1, 0)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(0, -0.01, 0)));
    
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(0, 0, -1)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, -0.01)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, -1)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, -0.01)));

    ASSERT_FALSE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(9, 19, 29)));
    ASSERT_EQ(0, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(9, 19, 29)));
    
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(10, 19, 29)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKPositionOutOfRange(OpenVDS::DoubleVector3(9.01, 19, 29)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(10, 19, 29)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(9.01, 19, 29)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(9, 20, 29)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKPositionOutOfRange(OpenVDS::DoubleVector3(9, 19.01, 29)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(9, 20, 29)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(9, 19.01, 29)));
    
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKIndexOutOfRange(OpenVDS::IntVector3(9, 19, 30)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsIJKPositionOutOfRange(OpenVDS::DoubleVector3(9, 19, 29.01)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(9, 19, 30)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfIJKPositionOutOfRange(OpenVDS::DoubleVector3(9, 19, 29.01)));

    // Voxel ranges (default IJK to voxel dimension map 2/1/0)
    ASSERT_FALSE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(0, 0, 0)));
    ASSERT_EQ(0, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, 0)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(0, 0, -1)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, -0.01)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, -1)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(0, 0, -0.01)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(0, -1, 0)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelPositionOutOfRange(OpenVDS::DoubleVector3(0, -0.01, 0)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(0, -1, 0)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(0, -0.01, 0)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(-1, 0, 0)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelPositionOutOfRange(OpenVDS::DoubleVector3(-0.01, 0, 0)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(-1, 0, 0)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(-0.01, 0, 0)));

    ASSERT_FALSE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(29, 19, 9)));
    ASSERT_EQ(0, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29, 19, 9)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(29, 19, 10)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29, 19, 9.01)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29, 19, 10)));
    ASSERT_EQ(3, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29, 19, 9.01)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(29, 20, 9)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29, 19.01, 9)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29, 20, 9)));
    ASSERT_EQ(2, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29, 19.01, 9)));

    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelIndexOutOfRange(OpenVDS::IntVector3(30, 19, 9)));
    ASSERT_TRUE(ijkCoordinateTransformer.IsVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29.01, 19, 9)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(30, 19, 9)));
    ASSERT_EQ(1, ijkCoordinateTransformer.ErrorCodeIfVoxelPositionOutOfRange(OpenVDS::DoubleVector3(29.01, 19, 9)));
  }
}

TEST(IJKCoordinateTransformerTests, TestIJKToWorld)
{
  OpenVDS::IJKGridDefinition
    ijkGridDefinition(OpenVDS::DoubleVector3(100, 200, 300), OpenVDS::DoubleVector3(1, 0, 0), OpenVDS::DoubleVector3(0, 2, 0), OpenVDS::DoubleVector3(0, 0, 3));
  OpenVDS::IntVector3
    ijkSize(10, 20, 30);

  OpenVDS::IJKCoordinateTransformer
    ijkCoordinateTransformer(ijkGridDefinition, ijkSize);

  ASSERT_EQ(OpenVDS::DoubleVector3(100, 200, 300), ijkCoordinateTransformer.IJKIndexToWorld(OpenVDS::IntVector3(0, 0, 0)));
  ASSERT_EQ(OpenVDS::DoubleVector3(109, 238, 387), ijkCoordinateTransformer.IJKIndexToWorld(OpenVDS::IntVector3(9, 19, 29)));
  ASSERT_EQ(OpenVDS::DoubleVector3(104, 210, 318), ijkCoordinateTransformer.IJKIndexToWorld(OpenVDS::IntVector3(4, 5, 6)));

  ASSERT_EQ(OpenVDS::IntVector3(0, 0, 0), ijkCoordinateTransformer.WorldToIJKIndex(OpenVDS::DoubleVector3(100, 200, 300)));
  ASSERT_EQ(OpenVDS::IntVector3(9, 19, 29), ijkCoordinateTransformer.WorldToIJKIndex(OpenVDS::DoubleVector3(109, 238, 387)));
  ASSERT_EQ(OpenVDS::IntVector3(4, 5, 6), ijkCoordinateTransformer.WorldToIJKIndex(OpenVDS::DoubleVector3(104, 210, 318)));

  ASSERT_NEAR(100.1, ijkCoordinateTransformer.IJKPositionToWorld(OpenVDS::DoubleVector3(0.1, 0.2, 0.3)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(200.4, ijkCoordinateTransformer.IJKPositionToWorld(OpenVDS::DoubleVector3(0.1, 0.2, 0.3)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(300.9, ijkCoordinateTransformer.IJKPositionToWorld(OpenVDS::DoubleVector3(0.1, 0.2, 0.3)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(0.1, ijkCoordinateTransformer.WorldToIJKPosition(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.2, ijkCoordinateTransformer.WorldToIJKPosition(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.3, ijkCoordinateTransformer.WorldToIJKPosition(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(104.9, ijkCoordinateTransformer.IJKPositionToWorld(OpenVDS::DoubleVector3(4.9, 4.8, 4.7)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(209.6, ijkCoordinateTransformer.IJKPositionToWorld(OpenVDS::DoubleVector3(4.9, 4.8, 4.7)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(314.1, ijkCoordinateTransformer.IJKPositionToWorld(OpenVDS::DoubleVector3(4.9, 4.8, 4.7)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(4.9, ijkCoordinateTransformer.WorldToIJKPosition(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.8, ijkCoordinateTransformer.WorldToIJKPosition(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.7, ijkCoordinateTransformer.WorldToIJKPosition(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).Z, IJKCoordinateTransformerTests_TestMaxDelta);
}

TEST(IJKCoordinateTransformerTests, TestIJKToAnnotation)
{
  OpenVDS::IJKGridDefinition
    ijkGridDefinition(OpenVDS::DoubleVector3(100, 200, 300), OpenVDS::DoubleVector3(1, 0, 0), OpenVDS::DoubleVector3(0, 2, 0), OpenVDS::DoubleVector3(0, 0, 3));
  OpenVDS::IntVector3
    ijkSize(10, 20, 30);
  OpenVDS::DoubleVector3
    ijkAnnotationStart(1000, 2000, 3000),
    ijkAnnotationEnd(1500, 2500, 3500);

  OpenVDS::IJKCoordinateTransformer
    ijkCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd);

  ASSERT_EQ(OpenVDS::DoubleVector3(1000, 2000, 3000), ijkCoordinateTransformer.IJKIndexToAnnotation(OpenVDS::IntVector3(0, 0, 0)));
  ASSERT_EQ(OpenVDS::DoubleVector3(1500, 2500, 3500), ijkCoordinateTransformer.IJKIndexToAnnotation(OpenVDS::IntVector3(9, 19, 29)));

  ASSERT_NEAR(1222.2222222222222, ijkCoordinateTransformer.IJKIndexToAnnotation(OpenVDS::IntVector3(4, 5, 6)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2131.5789473684210, ijkCoordinateTransformer.IJKIndexToAnnotation(OpenVDS::IntVector3(4, 5, 6)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3103.4482758620690, ijkCoordinateTransformer.IJKIndexToAnnotation(OpenVDS::IntVector3(4, 5, 6)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_EQ(OpenVDS::IntVector3(0, 0, 0), ijkCoordinateTransformer.AnnotationToIJKIndex(OpenVDS::DoubleVector3(1000, 2000, 3000)));
  ASSERT_EQ(OpenVDS::IntVector3(9, 19, 29), ijkCoordinateTransformer.AnnotationToIJKIndex(OpenVDS::DoubleVector3(1500, 2500, 3500)));
  ASSERT_EQ(OpenVDS::IntVector3(4, 5, 6), ijkCoordinateTransformer.AnnotationToIJKIndex(OpenVDS::DoubleVector3(1222.2222222222222, 2131.5789473684210, 3103.4482758620690)));

  ASSERT_NEAR(1005.555555555556, ijkCoordinateTransformer.IJKPositionToAnnotation(OpenVDS::DoubleVector3(0.1, 0.2, 0.3)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2005.263157894737, ijkCoordinateTransformer.IJKPositionToAnnotation(OpenVDS::DoubleVector3(0.1, 0.2, 0.3)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3005.172413793103, ijkCoordinateTransformer.IJKPositionToAnnotation(OpenVDS::DoubleVector3(0.1, 0.2, 0.3)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(0.1, ijkCoordinateTransformer.AnnotationToIJKPosition(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.2, ijkCoordinateTransformer.AnnotationToIJKPosition(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.3, ijkCoordinateTransformer.AnnotationToIJKPosition(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(1272.222222222222, ijkCoordinateTransformer.IJKPositionToAnnotation(OpenVDS::DoubleVector3(4.9, 4.8, 4.7)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2126.315789473684, ijkCoordinateTransformer.IJKPositionToAnnotation(OpenVDS::DoubleVector3(4.9, 4.8, 4.7)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3081.034482758621, ijkCoordinateTransformer.IJKPositionToAnnotation(OpenVDS::DoubleVector3(4.9, 4.8, 4.7)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(4.9, ijkCoordinateTransformer.AnnotationToIJKPosition(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.8, ijkCoordinateTransformer.AnnotationToIJKPosition(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.7, ijkCoordinateTransformer.AnnotationToIJKPosition(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).Z, IJKCoordinateTransformerTests_TestMaxDelta);
}

TEST(IJKCoordinateTransformerTests, TestIJKToVoxel)
{
  OpenVDS::IJKGridDefinition
    ijkGridDefinition(OpenVDS::DoubleVector3(100, 200, 300), OpenVDS::DoubleVector3(1, 0, 0), OpenVDS::DoubleVector3(0, 2, 0), OpenVDS::DoubleVector3(0, 0, 3));
  OpenVDS::IntVector3
    ijkSize(10, 20, 30);

  OpenVDS::IJKCoordinateTransformer
    ijkCoordinateTransformer(ijkGridDefinition, ijkSize);

  ASSERT_EQ(OpenVDS::IntVector3(0, 0, 0), ijkCoordinateTransformer.IJKIndexToVoxelIndex(OpenVDS::IntVector3(0, 0, 0)));
  ASSERT_EQ(OpenVDS::IntVector3(29, 19, 9), ijkCoordinateTransformer.IJKIndexToVoxelIndex(OpenVDS::IntVector3(9, 19, 29)));
  ASSERT_EQ(OpenVDS::IntVector3(6, 5, 4), ijkCoordinateTransformer.IJKIndexToVoxelIndex(OpenVDS::IntVector3(4, 5, 6)));

  ASSERT_EQ(OpenVDS::IntVector3(0, 0, 0), ijkCoordinateTransformer.VoxelIndexToIJKIndex(OpenVDS::IntVector3(0, 0, 0)));
  ASSERT_EQ(OpenVDS::IntVector3(9, 19, 29), ijkCoordinateTransformer.VoxelIndexToIJKIndex(OpenVDS::IntVector3(29, 19, 9)));
  ASSERT_EQ(OpenVDS::IntVector3(4, 5, 6), ijkCoordinateTransformer.VoxelIndexToIJKIndex(OpenVDS::IntVector3(6, 5, 4)));

  ASSERT_EQ(OpenVDS::DoubleVector3(0.3, 0.2, 0.1), ijkCoordinateTransformer.IJKPositionToVoxelPosition(OpenVDS::DoubleVector3(0.1, 0.2, 0.3)));

  ASSERT_EQ(OpenVDS::DoubleVector3(0.1, 0.2, 0.3), ijkCoordinateTransformer.VoxelPositionToIJKPosition(OpenVDS::DoubleVector3(0.3, 0.2, 0.1)));

  ASSERT_EQ(OpenVDS::DoubleVector3(4.7, 4.8, 4.9), ijkCoordinateTransformer.IJKPositionToVoxelPosition(OpenVDS::DoubleVector3(4.9, 4.8, 4.7)));

  ASSERT_EQ(OpenVDS::DoubleVector3(4.9, 4.8, 4.7), ijkCoordinateTransformer.VoxelPositionToIJKPosition(OpenVDS::DoubleVector3(4.7, 4.8, 4.9)));
}

TEST(IJKCoordinateTransformerTests, TestWorldToAnnotation)
{
  OpenVDS::IJKGridDefinition
    ijkGridDefinition(OpenVDS::DoubleVector3(100, 200, 300), OpenVDS::DoubleVector3(1, 0, 0), OpenVDS::DoubleVector3(0, 2, 0), OpenVDS::DoubleVector3(0, 0, 3));
  OpenVDS::IntVector3
    ijkSize(10, 20, 30);
  OpenVDS::DoubleVector3
    ijkAnnotationStart(1000, 2000, 3000),
    ijkAnnotationEnd(1500, 2500, 3500);

  OpenVDS::IJKCoordinateTransformer
    ijkCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd);

  // Via IJK (0, 0, 0)
  ASSERT_EQ(OpenVDS::DoubleVector3(1000, 2000, 3000), ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(100, 200, 300)));
  // Via IJK (9, 19, 29)
  ASSERT_EQ(OpenVDS::DoubleVector3(1500, 2500, 3500), ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(109, 238, 387)));
  // Via IJK (4, 5, 6)
  ASSERT_NEAR(1222.2222222222222, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(104, 210, 318)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2131.5789473684210, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(104, 210, 318)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3103.4482758620690, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(104, 210, 318)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  // Via IJK (0, 0, 0)
  ASSERT_EQ(OpenVDS::DoubleVector3(100, 200, 300), ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1000, 2000, 3000)));
  // Via IJK (9, 19, 29)
  ASSERT_EQ(OpenVDS::DoubleVector3(109, 238, 387), ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1500, 2500, 3500)));
  // Via IJK (4, 5, 6)
  ASSERT_NEAR(104.0, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1222.2222222222222, 2131.5789473684210, 3103.4482758620690)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(210.0, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1222.2222222222222, 2131.5789473684210, 3103.4482758620690)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(318.0, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1222.2222222222222, 2131.5789473684210, 3103.4482758620690)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  // Via IJK (0.1, 0.2, 0.3)
  ASSERT_NEAR(1005.555555555556, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2005.263157894737, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3005.172413793103, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(100.1, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(200.4, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(300.9, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  // Via IJK (4.9, 4.8, 4.7)
  ASSERT_NEAR(1272.222222222222, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2126.315789473684, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3081.034482758621, ijkCoordinateTransformer.WorldToAnnotation(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(104.9, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(209.6, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(314.1, ijkCoordinateTransformer.AnnotationToWorld(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).Z, IJKCoordinateTransformerTests_TestMaxDelta);
}

TEST(IJKCoordinateTransformerTests, TestWorldToVoxel)
{
  OpenVDS::IJKGridDefinition
    ijkGridDefinition(OpenVDS::DoubleVector3(100, 200, 300), OpenVDS::DoubleVector3(1, 0, 0), OpenVDS::DoubleVector3(0, 2, 0), OpenVDS::DoubleVector3(0, 0, 3));
  OpenVDS::IntVector3
    ijkSize(10, 20, 30);

  OpenVDS::IJKCoordinateTransformer
    ijkCoordinateTransformer(ijkGridDefinition, ijkSize);

  ASSERT_EQ(OpenVDS::DoubleVector3(100, 200, 300), ijkCoordinateTransformer.VoxelIndexToWorld(OpenVDS::IntVector3(0, 0, 0)));
  ASSERT_EQ(OpenVDS::DoubleVector3(109, 238, 387), ijkCoordinateTransformer.VoxelIndexToWorld(OpenVDS::IntVector3(29, 19, 9)));
  ASSERT_EQ(OpenVDS::DoubleVector3(104, 210, 318), ijkCoordinateTransformer.VoxelIndexToWorld(OpenVDS::IntVector3(6, 5, 4)));

  ASSERT_EQ(OpenVDS::IntVector3(0, 0, 0), ijkCoordinateTransformer.WorldToVoxelIndex(OpenVDS::DoubleVector3(100, 200, 300)));
  ASSERT_EQ(OpenVDS::IntVector3(29, 19, 9), ijkCoordinateTransformer.WorldToVoxelIndex(OpenVDS::DoubleVector3(109, 238, 387)));
  ASSERT_EQ(OpenVDS::IntVector3(6, 5, 4), ijkCoordinateTransformer.WorldToVoxelIndex(OpenVDS::DoubleVector3(104, 210, 318)));

  ASSERT_NEAR(100.1, ijkCoordinateTransformer.VoxelPositionToWorld(OpenVDS::DoubleVector3(0.3, 0.2, 0.1)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(200.4, ijkCoordinateTransformer.VoxelPositionToWorld(OpenVDS::DoubleVector3(0.3, 0.2, 0.1)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(300.9, ijkCoordinateTransformer.VoxelPositionToWorld(OpenVDS::DoubleVector3(0.3, 0.2, 0.1)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(0.3, ijkCoordinateTransformer.WorldToVoxelPosition(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.2, ijkCoordinateTransformer.WorldToVoxelPosition(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.1, ijkCoordinateTransformer.WorldToVoxelPosition(OpenVDS::DoubleVector3(100.1, 200.4, 300.9)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(104.9, ijkCoordinateTransformer.VoxelPositionToWorld(OpenVDS::DoubleVector3(4.7, 4.8, 4.9)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(209.6, ijkCoordinateTransformer.VoxelPositionToWorld(OpenVDS::DoubleVector3(4.7, 4.8, 4.9)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(314.1, ijkCoordinateTransformer.VoxelPositionToWorld(OpenVDS::DoubleVector3(4.7, 4.8, 4.9)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(4.7, ijkCoordinateTransformer.WorldToVoxelPosition(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.8, ijkCoordinateTransformer.WorldToVoxelPosition(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.9, ijkCoordinateTransformer.WorldToVoxelPosition(OpenVDS::DoubleVector3(104.9, 209.6, 314.1)).Z, IJKCoordinateTransformerTests_TestMaxDelta);
}

TEST(IJKCoordinateTransformerTests, TestAnnotationToVoxel)
{
  OpenVDS::IJKGridDefinition
    ijkGridDefinition(OpenVDS::DoubleVector3(100, 200, 300), OpenVDS::DoubleVector3(1, 0, 0), OpenVDS::DoubleVector3(0, 2, 0), OpenVDS::DoubleVector3(0, 0, 3));
  OpenVDS::IntVector3
    ijkSize(10, 20, 30);
  OpenVDS::DoubleVector3
    ijkAnnotationStart(1000, 2000, 3000),
    ijkAnnotationEnd(1500, 2500, 3500);

  OpenVDS::IJKCoordinateTransformer
    ijkCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd);

  ASSERT_EQ(OpenVDS::DoubleVector3(1000, 2000, 3000), ijkCoordinateTransformer.VoxelIndexToAnnotation(OpenVDS::IntVector3(0, 0, 0)));
  ASSERT_EQ(OpenVDS::DoubleVector3(1500, 2500, 3500), ijkCoordinateTransformer.VoxelIndexToAnnotation(OpenVDS::IntVector3(29, 19, 9)));

  ASSERT_NEAR(1222.2222222222222, ijkCoordinateTransformer.VoxelIndexToAnnotation(OpenVDS::IntVector3(6, 5, 4)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2131.5789473684210, ijkCoordinateTransformer.VoxelIndexToAnnotation(OpenVDS::IntVector3(6, 5, 4)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3103.4482758620690, ijkCoordinateTransformer.VoxelIndexToAnnotation(OpenVDS::IntVector3(6, 5, 4)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_EQ(OpenVDS::IntVector3(0, 0, 0), ijkCoordinateTransformer.AnnotationToVoxelIndex(OpenVDS::DoubleVector3(1000, 2000, 3000)));
  ASSERT_EQ(OpenVDS::IntVector3(29, 19, 9), ijkCoordinateTransformer.AnnotationToVoxelIndex(OpenVDS::DoubleVector3(1500, 2500, 3500)));
  ASSERT_EQ(OpenVDS::IntVector3(6, 5, 4), ijkCoordinateTransformer.AnnotationToVoxelIndex(OpenVDS::DoubleVector3(1222.2222222222222, 2131.5789473684210, 3103.4482758620690)));

  ASSERT_NEAR(1005.555555555556, ijkCoordinateTransformer.VoxelPositionToAnnotation(OpenVDS::DoubleVector3(0.3, 0.2, 0.1)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2005.263157894737, ijkCoordinateTransformer.VoxelPositionToAnnotation(OpenVDS::DoubleVector3(0.3, 0.2, 0.1)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3005.172413793103, ijkCoordinateTransformer.VoxelPositionToAnnotation(OpenVDS::DoubleVector3(0.3, 0.2, 0.1)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(0.3, ijkCoordinateTransformer.AnnotationToVoxelPosition(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.2, ijkCoordinateTransformer.AnnotationToVoxelPosition(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(0.1, ijkCoordinateTransformer.AnnotationToVoxelPosition(OpenVDS::DoubleVector3(1005.555555555556, 2005.263157894737, 3005.172413793103)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(1272.222222222222, ijkCoordinateTransformer.VoxelPositionToAnnotation(OpenVDS::DoubleVector3(4.7, 4.8, 4.9)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(2126.315789473684, ijkCoordinateTransformer.VoxelPositionToAnnotation(OpenVDS::DoubleVector3(4.7, 4.8, 4.9)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(3081.034482758621, ijkCoordinateTransformer.VoxelPositionToAnnotation(OpenVDS::DoubleVector3(4.7, 4.8, 4.9)).Z, IJKCoordinateTransformerTests_TestMaxDelta);

  ASSERT_NEAR(4.7, ijkCoordinateTransformer.AnnotationToVoxelPosition(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).X, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.8, ijkCoordinateTransformer.AnnotationToVoxelPosition(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).Y, IJKCoordinateTransformerTests_TestMaxDelta);
  ASSERT_NEAR(4.9, ijkCoordinateTransformer.AnnotationToVoxelPosition(OpenVDS::DoubleVector3(1272.222222222222, 2126.315789473684, 3081.034482758621)).Z, IJKCoordinateTransformerTests_TestMaxDelta);
}

const int AXIS0_SIZE = 12;
const float AXIS0_MIN = 123;
const float AXIS0_MAX = 321;

const int AXIS1_SIZE = 34;
const float AXIS1_MIN = 456;
const float AXIS1_MAX = 654;

const int AXIS2_SIZE = 56;
const float AXIS2_MIN = 789;
const float AXIS2_MAX = 987;

static OpenVDS::VDSHandle generateVDS(const std::string &vds_name, const char *(&axis_names)[3])
{
  int negativeMargin = 4;
  int positiveMargin = 4;
  int brickSize2DMultiplier = 4;
  auto lodLevels = OpenVDS::VolumeDataLayoutDescriptor::LODLevels_None;
  auto layoutOptions = OpenVDS::VolumeDataLayoutDescriptor::Options_None;
  OpenVDS::VolumeDataLayoutDescriptor layoutDescriptor(OpenVDS::VolumeDataLayoutDescriptor::BrickSize_32, negativeMargin, positiveMargin, brickSize2DMultiplier, lodLevels, layoutOptions);

  std::vector<OpenVDS::VolumeDataAxisDescriptor> axisDescriptors;
  axisDescriptors.emplace_back(AXIS0_SIZE, axis_names[0], "", AXIS0_MIN, AXIS0_MAX);
  axisDescriptors.emplace_back(AXIS1_SIZE, axis_names[1], "", AXIS1_MIN, AXIS1_MAX);
  axisDescriptors.emplace_back(AXIS2_SIZE, axis_names[2], "", AXIS2_MIN, AXIS2_MAX);

  std::vector<OpenVDS::VolumeDataChannelDescriptor> channelDescriptors;
  float rangeMin = -0.1234f;
  float rangeMax = 0.1234f;
  float intScale = 1.0f;
  float intOffset = 0.0f;
  channelDescriptors.emplace_back(OpenVDS::VolumeDataFormat::Format_R32, OpenVDS::VolumeDataComponents::Components_1, AMPLITUDE_ATTRIBUTE_NAME, "", rangeMin, rangeMax, OpenVDS::VolumeDataMapping::Direct, 1, OpenVDS::VolumeDataChannelDescriptor::Default, 0.f, intScale, intOffset);

  OpenVDS::MetadataContainer metadataContainer;
  metadataContainer.SetMetadataDoubleVector2(KNOWNMETADATA_SURVEYCOORDINATESYSTEM, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_ORIGIN, OpenVDS::DoubleVector2(0, 0));
  metadataContainer.SetMetadataDoubleVector2(KNOWNMETADATA_SURVEYCOORDINATESYSTEM, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_INLINESPACING, OpenVDS::DoubleVector2(1, 0));
  metadataContainer.SetMetadataDoubleVector2(KNOWNMETADATA_SURVEYCOORDINATESYSTEM, KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_CROSSLINESPACING, OpenVDS::DoubleVector2(0, 1));

  metadataContainer.SetMetadataDoubleVector3(KNOWNMETADATA_SURVEYCOORDINATESYSTEM, KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_ORIGIN3D, OpenVDS::DoubleVector3(0, 0, 0));
  metadataContainer.SetMetadataDoubleVector3(KNOWNMETADATA_SURVEYCOORDINATESYSTEM, KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_I_STEPVECTOR, OpenVDS::DoubleVector3(1, 0, 0));
  metadataContainer.SetMetadataDoubleVector3(KNOWNMETADATA_SURVEYCOORDINATESYSTEM, KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_J_STEPVECTOR, OpenVDS::DoubleVector3(0, 1, 0));
  metadataContainer.SetMetadataDoubleVector3(KNOWNMETADATA_SURVEYCOORDINATESYSTEM, KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_K_STEPVECTOR, OpenVDS::DoubleVector3(0, 0, -1));

  OpenVDS::Error error;
  std::string in_memory_name = fmt::format("inmemory://{}", vds_name);
  auto handle = OpenVDS::Create(in_memory_name, std::string(""), layoutDescriptor, axisDescriptors, channelDescriptors, metadataContainer, error);
  OpenVDS::VolumeDataAccessManager accessManager = OpenVDS::GetAccessManager(handle);

  //Generate Layerstatus
  std::shared_ptr<OpenVDS::VolumeDataPageAccessor> pageAccessor = accessManager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012, 0, 0, 100, OpenVDS::VolumeDataAccessManager::AccessMode_Create);
  //ASSERT_TRUE(pageAccessor);

  pageAccessor->Commit();
  accessManager.Flush(error);
  pageAccessor.reset();
  OpenVDS::Close(handle);

  return OpenVDS::Open(in_memory_name, std::string(""), error);
}

TEST(IJKCoordinateTransformerTests, TestVDSIJKCoordinateTransformerFromMetadata)
{
  // Set metadata used for IJK origin, unit steps
  // Survey annotations: inline-> I, crossline->J, time->K
  {
    const char* axisnames[] = {
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_TIME,
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_CROSSLINE,
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_INLINE
    };
    OpenVDS::ScopedVDSHandle handle(generateVDS("time_corssline_inline", axisnames));
    auto layout = OpenVDS::GetLayout(handle);

    OpenVDS::IJKCoordinateTransformer ijkCoordinateTransformer(layout);

    ASSERT_EQ(OpenVDS::IntVector3(2, 1, 0), ijkCoordinateTransformer.IJKToVoxelDimensionMap());

    ASSERT_EQ(OpenVDS::IntVector3(AXIS2_SIZE, AXIS1_SIZE, AXIS0_SIZE), ijkCoordinateTransformer.IJKSize());

    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS2_MIN, AXIS1_MIN, AXIS0_MIN), ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS2_MAX, AXIS1_MAX, AXIS0_MAX), ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  {
    const char* axisnames[] = {
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_INLINE,
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_TIME,
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_CROSSLINE
    };
    OpenVDS::ScopedVDSHandle handle(generateVDS("inline_time_corssline", axisnames));
    auto layout = OpenVDS::GetLayout(handle);

    OpenVDS::IJKCoordinateTransformer ijkCoordinateTransformer(layout);

    ASSERT_EQ(OpenVDS::IntVector3(0, 2, 1), ijkCoordinateTransformer.IJKToVoxelDimensionMap());

    ASSERT_EQ(OpenVDS::IntVector3(AXIS0_SIZE, AXIS2_SIZE, AXIS1_SIZE), ijkCoordinateTransformer.IJKSize());

    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MIN, AXIS2_MIN, AXIS1_MIN), ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MAX, AXIS2_MAX, AXIS1_MAX), ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  {
    const char* axisnames[] = {
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_INLINE,
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_CROSSLINE,
KNOWNMETADATA_SURVEYCOORDINATE_INLINECROSSLINE_AXISNAME_TIME
    };
    OpenVDS::ScopedVDSHandle handle(generateVDS("inline_corssline_time", axisnames));
    auto layout = OpenVDS::GetLayout(handle);

    OpenVDS::IJKCoordinateTransformer ijkCoordinateTransformer(layout);

    ASSERT_EQ(OpenVDS::IntVector3(0, 1, 2), ijkCoordinateTransformer.IJKToVoxelDimensionMap());

    ASSERT_EQ(OpenVDS::IntVector3(AXIS0_SIZE, AXIS1_SIZE, AXIS2_SIZE), ijkCoordinateTransformer.IJKSize());

    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MIN, AXIS1_MIN, AXIS2_MIN), ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MAX, AXIS1_MAX, AXIS2_MAX), ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  // IJK annotations map directly
  {
    const char* axisnames[] = {
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_K,
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_J,
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_I
    };
    OpenVDS::ScopedVDSHandle handle(generateVDS("kji", axisnames));
    auto layout = OpenVDS::GetLayout(handle);

    OpenVDS::IJKCoordinateTransformer ijkCoordinateTransformer(layout);

    ASSERT_EQ(OpenVDS::IntVector3(2, 1, 0), ijkCoordinateTransformer.IJKToVoxelDimensionMap());

    ASSERT_EQ(OpenVDS::IntVector3(AXIS2_SIZE, AXIS1_SIZE, AXIS0_SIZE), ijkCoordinateTransformer.IJKSize());

    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS2_MIN, AXIS1_MIN, AXIS0_MIN), ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS2_MAX, AXIS1_MAX, AXIS0_MAX), ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  {
    const char* axisnames[] = {
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_I,
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_K,
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_J
    };
    OpenVDS::ScopedVDSHandle handle(generateVDS("ikj", axisnames));
    auto layout = OpenVDS::GetLayout(handle);

    OpenVDS::IJKCoordinateTransformer ijkCoordinateTransformer(layout);

    ASSERT_EQ(OpenVDS::IntVector3(0, 2, 1), ijkCoordinateTransformer.IJKToVoxelDimensionMap());

    ASSERT_EQ(OpenVDS::IntVector3(AXIS0_SIZE, AXIS2_SIZE, AXIS1_SIZE), ijkCoordinateTransformer.IJKSize());

    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MIN, AXIS2_MIN, AXIS1_MIN), ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MAX, AXIS2_MAX, AXIS1_MAX), ijkCoordinateTransformer.IJKAnnotationEnd());
  }

  {
    const char* axisnames[] = {
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_I,
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_J,
KNOWNMETADATA_SURVEYCOORDINATE_3DIJK_AXISNAME_K
    };
    OpenVDS::ScopedVDSHandle handle(generateVDS("ikj", axisnames));
    auto layout = OpenVDS::GetLayout(handle);

    OpenVDS::IJKCoordinateTransformer ijkCoordinateTransformer(layout);

    ASSERT_EQ(OpenVDS::IntVector3(0, 1, 2), ijkCoordinateTransformer.IJKToVoxelDimensionMap());

    ASSERT_EQ(OpenVDS::IntVector3(AXIS0_SIZE, AXIS1_SIZE, AXIS2_SIZE), ijkCoordinateTransformer.IJKSize());

    ASSERT_TRUE(ijkCoordinateTransformer.AnnotationsDefined());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MIN, AXIS1_MIN, AXIS2_MIN), ijkCoordinateTransformer.IJKAnnotationStart());
    ASSERT_EQ(OpenVDS::DoubleVector3(AXIS0_MAX, AXIS1_MAX, AXIS2_MAX), ijkCoordinateTransformer.IJKAnnotationEnd());
  }
}

