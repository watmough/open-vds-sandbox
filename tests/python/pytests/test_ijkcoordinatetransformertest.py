import openvds

import pytest

IJKCoordinateTransformerTests_TestMaxDelta = 0.00000001

def test_basic_functionality():
  ijkGridDefinition = openvds.IJKGridDefinition((100, 200, 300), (1, 0, 0), (0, 2, 0), (0, 0, 3))
  ijkSize = (10, 20, 30)
  ijkToVoxelDimensionMap021 = (0, 2, 1)
  ijkAnnotationStart = (1000, 2000, 3000)
  ijkAnnotationEnd = (1500, 2500, 3500)

  # Contructor with IJK grid definition and size; default IJK to voxel dimension map 2/1/0, annotations not defined
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize)

  assert (2, 1, 0) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert ijkGridDefinition == ijkCoordinateTransformer.IJKGrid
  assert not ijkCoordinateTransformer.annotationsDefined

  # Contructor with IJK grid definition, size, and IJK to voxel dimension map; annotations not defined
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize, ijkToVoxelDimensionMap021)

  assert ijkGridDefinition == ijkCoordinateTransformer.IJKGrid
  assert (0, 2, 1) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert not ijkCoordinateTransformer.annotationsDefined

  # Contructor with IJK grid definition, size, annotations; default IJK to voxel dimension map 2/1/0
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd)

  assert ijkGridDefinition == ijkCoordinateTransformer.IJKGrid
  assert (2, 1, 0) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert ijkCoordinateTransformer.annotationsDefined
  assert ijkAnnotationStart == ijkCoordinateTransformer.IJKAnnotationStart
  assert ijkAnnotationEnd == ijkCoordinateTransformer.IJKAnnotationEnd

  # Contructor with everything defined directly
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize, ijkToVoxelDimensionMap021, ijkAnnotationStart, ijkAnnotationEnd)

  assert ijkGridDefinition == ijkCoordinateTransformer.IJKGrid
  assert (0, 2, 1) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert ijkCoordinateTransformer.annotationsDefined
  assert ijkAnnotationStart == ijkCoordinateTransformer.IJKAnnotationStart
  assert ijkAnnotationEnd == ijkCoordinateTransformer.IJKAnnotationEnd

  # IJK ranges
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize)

  assert not ijkCoordinateTransformer.isIJKIndexOutOfRange((0, 0, 0))

  assert ijkCoordinateTransformer.isIJKIndexOutOfRange((-1, 0, 0))
  assert ijkCoordinateTransformer.isIJKPositionOutOfRange((-0.01, 0, 0))

  assert ijkCoordinateTransformer.isIJKIndexOutOfRange((0, -1, 0))
  assert ijkCoordinateTransformer.isIJKPositionOutOfRange((0, -0.01, 0))
    
  assert ijkCoordinateTransformer.isIJKIndexOutOfRange((0, 0, -1))
  assert ijkCoordinateTransformer.isIJKPositionOutOfRange((0, 0, -0.01))

  assert not ijkCoordinateTransformer.isIJKIndexOutOfRange((9, 19, 29))
    
  assert ijkCoordinateTransformer.isIJKIndexOutOfRange((10, 19, 29))
  assert ijkCoordinateTransformer.isIJKPositionOutOfRange((9.01, 19, 29))

  assert ijkCoordinateTransformer.isIJKIndexOutOfRange((9, 20, 29))
  assert ijkCoordinateTransformer.isIJKPositionOutOfRange((9, 19.01, 29))
    
  assert ijkCoordinateTransformer.isIJKIndexOutOfRange((9, 19, 30))
  assert ijkCoordinateTransformer.isIJKPositionOutOfRange((9, 19, 29.01))

  # Voxel ranges (default IJK to voxel dimension map 2/1/0)
  assert not ijkCoordinateTransformer.isVoxelIndexOutOfRange((0, 0, 0))

  assert ijkCoordinateTransformer.isVoxelIndexOutOfRange((0, 0, -1))
  assert ijkCoordinateTransformer.isVoxelPositionOutOfRange((0, 0, -0.01))

  assert ijkCoordinateTransformer.isVoxelIndexOutOfRange((0, -1, 0))
  assert ijkCoordinateTransformer.isVoxelPositionOutOfRange((0, -0.01, 0))

  assert ijkCoordinateTransformer.isVoxelIndexOutOfRange((-1, 0, 0))
  assert ijkCoordinateTransformer.isVoxelPositionOutOfRange((-0.01, 0, 0))

  assert not ijkCoordinateTransformer.isVoxelIndexOutOfRange((29, 19, 9))

  assert ijkCoordinateTransformer.isVoxelIndexOutOfRange((29, 19, 10))
  assert ijkCoordinateTransformer.isVoxelPositionOutOfRange((29, 19, 9.01))

  assert ijkCoordinateTransformer.isVoxelIndexOutOfRange((29, 20, 9))
  assert ijkCoordinateTransformer.isVoxelPositionOutOfRange((29, 19.01, 9))

  assert ijkCoordinateTransformer.isVoxelIndexOutOfRange((30, 19, 9))
  assert ijkCoordinateTransformer.isVoxelPositionOutOfRange((29.01, 19, 9))

def test_ijk_to_world():

  ijkGridDefinition = openvds.IJKGridDefinition((100, 200, 300), (1, 0, 0), (0, 2, 0), (0, 0, 3))
  ijkSize = (10, 20, 30)
  
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize)

  assert (100, 200, 300) == ijkCoordinateTransformer.IJKIndexToWorld((0, 0, 0))
  assert (109, 238, 387) == ijkCoordinateTransformer.IJKIndexToWorld((9, 19, 29))
  assert (104, 210, 318) == ijkCoordinateTransformer.IJKIndexToWorld((4, 5, 6))

  assert (0, 0, 0) == ijkCoordinateTransformer.worldToIJKIndex((100, 200, 300))
  assert (9, 19, 29) == ijkCoordinateTransformer.worldToIJKIndex((109, 238, 387))
  assert (4, 5, 6) == ijkCoordinateTransformer.worldToIJKIndex((104, 210, 318))

  assert abs(100.1 - ijkCoordinateTransformer.IJKPositionToWorld((0.1, 0.2, 0.3))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(200.4 - ijkCoordinateTransformer.IJKPositionToWorld((0.1, 0.2, 0.3))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(300.9 - ijkCoordinateTransformer.IJKPositionToWorld((0.1, 0.2, 0.3))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(0.1 - ijkCoordinateTransformer.worldToIJKPosition((100.1, 200.4, 300.9))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.2 - ijkCoordinateTransformer.worldToIJKPosition((100.1, 200.4, 300.9))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.3 - ijkCoordinateTransformer.worldToIJKPosition((100.1, 200.4, 300.9))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(104.9 - ijkCoordinateTransformer.IJKPositionToWorld((4.9, 4.8, 4.7))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(209.6 - ijkCoordinateTransformer.IJKPositionToWorld((4.9, 4.8, 4.7))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(314.1 - ijkCoordinateTransformer.IJKPositionToWorld((4.9, 4.8, 4.7))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(4.9 - ijkCoordinateTransformer.worldToIJKPosition((104.9, 209.6, 314.1))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.8 - ijkCoordinateTransformer.worldToIJKPosition((104.9, 209.6, 314.1))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.7 - ijkCoordinateTransformer.worldToIJKPosition((104.9, 209.6, 314.1))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

def test_ijk_to_annotation():

  ijkGridDefinition = openvds.IJKGridDefinition((100, 200, 300), (1, 0, 0), (0, 2, 0), (0, 0, 3))
  ijkSize = (10, 20, 30)
  ijkAnnotationStart = (1000, 2000, 3000)
  ijkAnnotationEnd = (1500, 2500, 3500)

  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd)

  assert (1000, 2000, 3000) == ijkCoordinateTransformer.IJKIndexToAnnotation((0, 0, 0))
  assert (1500, 2500, 3500) == ijkCoordinateTransformer.IJKIndexToAnnotation((9, 19, 29))

  assert abs(1222.2222222222222 - ijkCoordinateTransformer.IJKIndexToAnnotation((4, 5, 6))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2131.5789473684210 - ijkCoordinateTransformer.IJKIndexToAnnotation((4, 5, 6))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3103.4482758620690 - ijkCoordinateTransformer.IJKIndexToAnnotation((4, 5, 6))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert (0, 0, 0) ==  ijkCoordinateTransformer.annotationToIJKIndex((1000, 2000, 3000))
  assert (9, 19, 29) == ijkCoordinateTransformer.annotationToIJKIndex((1500, 2500, 3500))
  assert (4, 5, 6) == ijkCoordinateTransformer.annotationToIJKIndex((1222.2222222222222, 2131.5789473684210, 3103.4482758620690))

  assert abs(1005.555555555556 - ijkCoordinateTransformer.IJKPositionToAnnotation((0.1, 0.2, 0.3))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2005.263157894737 - ijkCoordinateTransformer.IJKPositionToAnnotation((0.1, 0.2, 0.3))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3005.172413793103 - ijkCoordinateTransformer.IJKPositionToAnnotation((0.1, 0.2, 0.3))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(0.1 - ijkCoordinateTransformer.annotationToIJKPosition((1005.555555555556, 2005.263157894737, 3005.172413793103))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.2 - ijkCoordinateTransformer.annotationToIJKPosition((1005.555555555556, 2005.263157894737, 3005.172413793103))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.3 - ijkCoordinateTransformer.annotationToIJKPosition((1005.555555555556, 2005.263157894737, 3005.172413793103))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(1272.222222222222 - ijkCoordinateTransformer.IJKPositionToAnnotation((4.9, 4.8, 4.7))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2126.315789473684 - ijkCoordinateTransformer.IJKPositionToAnnotation((4.9, 4.8, 4.7))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3081.034482758621 - ijkCoordinateTransformer.IJKPositionToAnnotation((4.9, 4.8, 4.7))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(4.9 - ijkCoordinateTransformer.annotationToIJKPosition((1272.222222222222, 2126.315789473684, 3081.034482758621))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.8 - ijkCoordinateTransformer.annotationToIJKPosition((1272.222222222222, 2126.315789473684, 3081.034482758621))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.7 - ijkCoordinateTransformer.annotationToIJKPosition((1272.222222222222, 2126.315789473684, 3081.034482758621))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

def test_ijk_to_voxel():

  ijkGridDefinition = openvds.IJKGridDefinition((100, 200, 300), (1, 0, 0), (0, 2, 0), (0, 0, 3))
  ijkSize = (10, 20, 30)

  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize)

  assert (0, 0, 0) == ijkCoordinateTransformer.IJKIndexToVoxelIndex((0, 0, 0))
  assert (29, 19, 9) == ijkCoordinateTransformer.IJKIndexToVoxelIndex((9, 19, 29))
  assert (6, 5, 4) == ijkCoordinateTransformer.IJKIndexToVoxelIndex((4, 5, 6))

  assert (0, 0, 0) == ijkCoordinateTransformer.voxelIndexToIJKIndex((0, 0, 0))
  assert (9, 19, 29) == ijkCoordinateTransformer.voxelIndexToIJKIndex((29, 19, 9))
  assert (4, 5, 6) == ijkCoordinateTransformer.voxelIndexToIJKIndex((6, 5, 4))

  assert (0.3, 0.2, 0.1) == ijkCoordinateTransformer.IJKPositionToVoxelPosition((0.1, 0.2, 0.3))

  assert (0.1, 0.2, 0.3) == ijkCoordinateTransformer.voxelPositionToIJKPosition((0.3, 0.2, 0.1))

  assert (4.7, 4.8, 4.9) == ijkCoordinateTransformer.IJKPositionToVoxelPosition((4.9, 4.8, 4.7))

  assert (4.9, 4.8, 4.7) == ijkCoordinateTransformer.voxelPositionToIJKPosition((4.7, 4.8, 4.9))

def test_world_to_annotation():

  ijkGridDefinition = openvds.IJKGridDefinition((100, 200, 300), (1, 0, 0), (0, 2, 0), (0, 0, 3))
  ijkSize = (10, 20, 30)
  ijkAnnotationStart = (1000, 2000, 3000)
  ijkAnnotationEnd = (1500, 2500, 3500)

  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd)

  # Via IJK (0, 0, 0)
  assert (1000, 2000, 3000) == ijkCoordinateTransformer.worldToAnnotation((100, 200, 300))
  # Via IJK (9, 19, 29)
  assert (1500, 2500, 3500) == ijkCoordinateTransformer.worldToAnnotation((109, 238, 387))
  # Via IJK (4, 5, 6)
  assert abs(1222.2222222222222 - ijkCoordinateTransformer.worldToAnnotation((104, 210, 318))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2131.5789473684210 - ijkCoordinateTransformer.worldToAnnotation((104, 210, 318))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3103.4482758620690 - ijkCoordinateTransformer.worldToAnnotation((104, 210, 318))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  # Via IJK (0, 0, 0)
  assert (100, 200, 300) == ijkCoordinateTransformer.annotationToWorld((1000, 2000, 3000))
  # Via IJK (9, 19, 29)
  assert (109, 238, 387) == ijkCoordinateTransformer.annotationToWorld((1500, 2500, 3500))
  # Via IJK (4, 5, 6)
  assert abs(104.0 - ijkCoordinateTransformer.annotationToWorld((1222.2222222222222, 2131.5789473684210, 3103.4482758620690))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(210.0 - ijkCoordinateTransformer.annotationToWorld((1222.2222222222222, 2131.5789473684210, 3103.4482758620690))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(318.0 - ijkCoordinateTransformer.annotationToWorld((1222.2222222222222, 2131.5789473684210, 3103.4482758620690))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  # Via IJK (0.1, 0.2, 0.3)
  assert abs(1005.555555555556 - ijkCoordinateTransformer.worldToAnnotation((100.1, 200.4, 300.9))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2005.263157894737 - ijkCoordinateTransformer.worldToAnnotation((100.1, 200.4, 300.9))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3005.172413793103 - ijkCoordinateTransformer.worldToAnnotation((100.1, 200.4, 300.9))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(100.1 - ijkCoordinateTransformer.annotationToWorld((1005.555555555556, 2005.263157894737, 3005.172413793103))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(200.4 - ijkCoordinateTransformer.annotationToWorld((1005.555555555556, 2005.263157894737, 3005.172413793103))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(300.9 - ijkCoordinateTransformer.annotationToWorld((1005.555555555556, 2005.263157894737, 3005.172413793103))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  # Via IJK (4.9, 4.8, 4.7)
  assert abs(1272.222222222222 - ijkCoordinateTransformer.worldToAnnotation((104.9, 209.6, 314.1))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2126.315789473684 - ijkCoordinateTransformer.worldToAnnotation((104.9, 209.6, 314.1))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3081.034482758621 - ijkCoordinateTransformer.worldToAnnotation((104.9, 209.6, 314.1))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(104.9 - ijkCoordinateTransformer.annotationToWorld((1272.222222222222, 2126.315789473684, 3081.034482758621))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(209.6 - ijkCoordinateTransformer.annotationToWorld((1272.222222222222, 2126.315789473684, 3081.034482758621))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(314.1 - ijkCoordinateTransformer.annotationToWorld((1272.222222222222, 2126.315789473684, 3081.034482758621))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

def test_world_to_voxel():

  ijkGridDefinition = openvds.IJKGridDefinition((100, 200, 300), (1, 0, 0), (0, 2, 0), (0, 0, 3))
  ijkSize = (10, 20, 30)

  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize)

  assert (100, 200, 300) == ijkCoordinateTransformer.voxelIndexToWorld((0, 0, 0))
  assert (109, 238, 387) == ijkCoordinateTransformer.voxelIndexToWorld((29, 19, 9))
  assert (104, 210, 318) == ijkCoordinateTransformer.voxelIndexToWorld((6, 5, 4))

  assert (0, 0, 0) == ijkCoordinateTransformer.worldToVoxelIndex((100, 200, 300))
  assert (29, 19, 9) == ijkCoordinateTransformer.worldToVoxelIndex((109, 238, 387))
  assert (6, 5, 4) == ijkCoordinateTransformer.worldToVoxelIndex((104, 210, 318))

  assert abs(100.1 - ijkCoordinateTransformer.voxelPositionToWorld((0.3, 0.2, 0.1))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(200.4 - ijkCoordinateTransformer.voxelPositionToWorld((0.3, 0.2, 0.1))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(300.9 - ijkCoordinateTransformer.voxelPositionToWorld((0.3, 0.2, 0.1))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(0.3 - ijkCoordinateTransformer.worldToVoxelPosition((100.1, 200.4, 300.9))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.2 - ijkCoordinateTransformer.worldToVoxelPosition((100.1, 200.4, 300.9))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.1 - ijkCoordinateTransformer.worldToVoxelPosition((100.1, 200.4, 300.9))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(104.9 - ijkCoordinateTransformer.voxelPositionToWorld((4.7, 4.8, 4.9))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(209.6 - ijkCoordinateTransformer.voxelPositionToWorld((4.7, 4.8, 4.9))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(314.1 - ijkCoordinateTransformer.voxelPositionToWorld((4.7, 4.8, 4.9))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(4.7 - ijkCoordinateTransformer.worldToVoxelPosition((104.9, 209.6, 314.1))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.8 - ijkCoordinateTransformer.worldToVoxelPosition((104.9, 209.6, 314.1))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.9 - ijkCoordinateTransformer.worldToVoxelPosition((104.9, 209.6, 314.1))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

def test_annotation_to_voxel():

  ijkGridDefinition = openvds.IJKGridDefinition((100, 200, 300), (1, 0, 0), (0, 2, 0), (0, 0, 3))
  ijkSize = (10, 20, 30)
  ijkAnnotationStart = (1000, 2000, 3000)
  ijkAnnotationEnd = (1500, 2500, 3500)

  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(ijkGridDefinition, ijkSize, ijkAnnotationStart, ijkAnnotationEnd)

  assert (1000, 2000, 3000) == ijkCoordinateTransformer.voxelIndexToAnnotation((0, 0, 0))
  assert (1500, 2500, 3500) == ijkCoordinateTransformer.voxelIndexToAnnotation((29, 19, 9))

  assert abs(1222.2222222222222 - ijkCoordinateTransformer.voxelIndexToAnnotation((6, 5, 4))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2131.5789473684210 - ijkCoordinateTransformer.voxelIndexToAnnotation((6, 5, 4))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3103.4482758620690 - ijkCoordinateTransformer.voxelIndexToAnnotation((6, 5, 4))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert (0, 0, 0) == ijkCoordinateTransformer.annotationToVoxelIndex((1000, 2000, 3000))
  assert (29, 19, 9) == ijkCoordinateTransformer.annotationToVoxelIndex((1500, 2500, 3500))
  assert (6, 5, 4) == ijkCoordinateTransformer.annotationToVoxelIndex((1222.2222222222222, 2131.5789473684210, 3103.4482758620690))

  assert abs(1005.555555555556 - ijkCoordinateTransformer.voxelPositionToAnnotation((0.3, 0.2, 0.1))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2005.263157894737 - ijkCoordinateTransformer.voxelPositionToAnnotation((0.3, 0.2, 0.1))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3005.172413793103 - ijkCoordinateTransformer.voxelPositionToAnnotation((0.3, 0.2, 0.1))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(0.3 - ijkCoordinateTransformer.annotationToVoxelPosition((1005.555555555556, 2005.263157894737, 3005.172413793103))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.2 - ijkCoordinateTransformer.annotationToVoxelPosition((1005.555555555556, 2005.263157894737, 3005.172413793103))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(0.1 - ijkCoordinateTransformer.annotationToVoxelPosition((1005.555555555556, 2005.263157894737, 3005.172413793103))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(1272.222222222222 - ijkCoordinateTransformer.voxelPositionToAnnotation((4.7, 4.8, 4.9))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(2126.315789473684 - ijkCoordinateTransformer.voxelPositionToAnnotation((4.7, 4.8, 4.9))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(3081.034482758621 - ijkCoordinateTransformer.voxelPositionToAnnotation((4.7, 4.8, 4.9))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

  assert abs(4.7 - ijkCoordinateTransformer.annotationToVoxelPosition((1272.222222222222, 2126.315789473684, 3081.034482758621))[0]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.8 - ijkCoordinateTransformer.annotationToVoxelPosition((1272.222222222222, 2126.315789473684, 3081.034482758621))[1]) < IJKCoordinateTransformerTests_TestMaxDelta
  assert abs(4.9 - ijkCoordinateTransformer.annotationToVoxelPosition((1272.222222222222, 2126.315789473684, 3081.034482758621))[2]) < IJKCoordinateTransformerTests_TestMaxDelta

AXIS0_SIZE = 12
AXIS0_MIN = 123
AXIS0_MAX = 321

AXIS1_SIZE = 34
AXIS1_MIN = 456
AXIS1_MAX = 654

AXIS2_SIZE = 56
AXIS2_MIN = 789
AXIS2_MAX = 987

def generateVDS(vds_name, axis_names):
  layoutDescriptor = openvds.VolumeDataLayoutDescriptor(openvds.VolumeDataLayoutDescriptor.BrickSize.BrickSize_64,
                                                        0, 0, 4,
                                                        openvds.VolumeDataLayoutDescriptor.LODLevels.LODLevels_None,
                                                        openvds.VolumeDataLayoutDescriptor.Options.Options_None)
          
  axisDescriptors = [openvds.VolumeDataAxisDescriptor(AXIS0_SIZE, axis_names[0], "m", AXIS0_MIN, AXIS0_MAX),
                      openvds.VolumeDataAxisDescriptor(AXIS1_SIZE, axis_names[1], "m", AXIS1_MIN, AXIS1_MAX),
                      openvds.VolumeDataAxisDescriptor(AXIS2_SIZE, axis_names[2], "m", AXIS2_MIN, AXIS2_MAX),]
  channelDescriptors = [openvds.VolumeDataChannelDescriptor(openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                             openvds.VolumeDataChannelDescriptor.Components.Components_1,
                                                             "Value", "", 0.0, (300.0 * 200.0 * 100.0) - 1.0)]
  
  metaData = openvds.MetadataContainer()
  metaData.setMetadataDoubleVector2(openvds.KnownMetadata.surveyCoordinateSystemOrigin().category, openvds.KnownMetadata.surveyCoordinateSystemOrigin().name, (1234.0, 4321.0))
  metaData.setMetadataDoubleVector2(openvds.KnownMetadata.categorySurveyCoordinateSystem(), openvds.KnownMetadata.surveyCoordinateSystemOrigin().getName(), (0, 0))
  metaData.setMetadataDoubleVector2(openvds.KnownMetadata.categorySurveyCoordinateSystem(), openvds.KnownMetadata.surveyCoordinateSystemInlineSpacing().getName(), (1, 0))
  metaData.setMetadataDoubleVector2(openvds.KnownMetadata.categorySurveyCoordinateSystem(), openvds.KnownMetadata.surveyCoordinateSystemCrosslineSpacing().getName(), (0, 1))

  metaData.setMetadataDoubleVector3(openvds.KnownMetadata.categorySurveyCoordinateSystem(), openvds.KnownMetadata.surveyCoordinateSystemIJKOrigin().getName(), (0, 0, 0))
  metaData.setMetadataDoubleVector3(openvds.KnownMetadata.categorySurveyCoordinateSystem(), openvds.KnownMetadata.surveyCoordinateSystemIStepVector().getName(), (1, 0, 0))
  metaData.setMetadataDoubleVector3(openvds.KnownMetadata.categorySurveyCoordinateSystem(), openvds.KnownMetadata.surveyCoordinateSystemJStepVector().getName(), (0, 1, 0))
  metaData.setMetadataDoubleVector3(openvds.KnownMetadata.categorySurveyCoordinateSystem(), openvds.KnownMetadata.surveyCoordinateSystemKStepVector().getName(), (0, 0, -1))

  vds_url = "inmemory://{}".format(vds_name)
  vds = openvds.create(vds_url, "", layoutDescriptor, axisDescriptors, channelDescriptors, metaData)
  layout = openvds.getLayout(vds)
  
  manager = openvds.getAccessManager(vds)
  accessor = manager.createVolumeDataPageAccessor(openvds.DimensionsND.Dimensions_012, 0, 0, 8, openvds.VolumeDataAccessManager.AccessMode.AccessMode_Create, 1024)
  accessor.commit()
  manager.manager.flushUploadQueue()
  manager.manager.destroyVolumeDataPageAccessor(accessor)
  openvds.close(vds)

  return openvds.open(vds_url, "")
    
def test_vds_get_ijk_coordinate_transformer_from_metadata():
  vds = generateVDS("time_cossline_inline", ("Time", "Crossline", "Inline"))
  layout = openvds.getLayout(vds)
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(layout)
  assert (2, 1, 0) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert (AXIS2_SIZE, AXIS1_SIZE, AXIS0_SIZE) == ijkCoordinateTransformer.IJKSize
  assert ijkCoordinateTransformer.annotationsDefined
  assert (AXIS2_MIN, AXIS1_MIN, AXIS0_MIN) == ijkCoordinateTransformer.IJKAnnotationStart
  assert (AXIS2_MAX, AXIS1_MAX, AXIS0_MAX) == ijkCoordinateTransformer.IJKAnnotationEnd

  vds = generateVDS("inline_time_cossline", ("Inline", "Time", "Crossline"))
  layout = openvds.getLayout(vds)
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(layout)
  print(ijkCoordinateTransformer.IJKToVoxelDimensionMap)
  assert (0, 2, 1) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert (AXIS0_SIZE, AXIS2_SIZE, AXIS1_SIZE) == ijkCoordinateTransformer.IJKSize
  assert ijkCoordinateTransformer.annotationsDefined
  assert (AXIS0_MIN, AXIS2_MIN, AXIS1_MIN) == ijkCoordinateTransformer.IJKAnnotationStart
  assert (AXIS0_MAX, AXIS2_MAX, AXIS1_MAX) == ijkCoordinateTransformer.IJKAnnotationEnd

  vds = generateVDS("inline_crossline_time", ("Inline", "Crossline", "Time"))
  layout = openvds.getLayout(vds)
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(layout)
  assert (0, 1, 2) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert (AXIS0_SIZE, AXIS1_SIZE, AXIS2_SIZE) == ijkCoordinateTransformer.IJKSize
  assert ijkCoordinateTransformer.annotationsDefined
  assert (AXIS0_MIN, AXIS1_MIN, AXIS2_MIN) == ijkCoordinateTransformer.IJKAnnotationStart
  assert (AXIS0_MAX, AXIS1_MAX, AXIS2_MAX) == ijkCoordinateTransformer.IJKAnnotationEnd

  # IJK annotations map directly
  vds = generateVDS("kji", ("K", "J", "I"))
  layout = openvds.getLayout(vds)
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(layout)
  assert (2, 1, 0) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert (AXIS2_SIZE, AXIS1_SIZE, AXIS0_SIZE) == ijkCoordinateTransformer.IJKSize
  assert ijkCoordinateTransformer.annotationsDefined
  assert (AXIS2_MIN, AXIS1_MIN, AXIS0_MIN) == ijkCoordinateTransformer.IJKAnnotationStart
  assert (AXIS2_MAX, AXIS1_MAX, AXIS0_MAX) == ijkCoordinateTransformer.IJKAnnotationEnd

  vds = generateVDS("ikj", ("I", "K", "J"))
  layout = openvds.getLayout(vds)
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(layout)
  assert (0, 2, 1) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert (AXIS0_SIZE, AXIS2_SIZE, AXIS1_SIZE) == ijkCoordinateTransformer.IJKSize
  assert ijkCoordinateTransformer.annotationsDefined
  assert (AXIS0_MIN, AXIS2_MIN, AXIS1_MIN) == ijkCoordinateTransformer.IJKAnnotationStart
  assert (AXIS0_MAX, AXIS2_MAX, AXIS1_MAX) == ijkCoordinateTransformer.IJKAnnotationEnd

  vds = generateVDS("ijk", ("I", "J", "K"))
  layout = openvds.getLayout(vds)
  ijkCoordinateTransformer = openvds.IJKCoordinateTransformer(layout)
  assert (0, 1, 2) == ijkCoordinateTransformer.IJKToVoxelDimensionMap
  assert (AXIS0_SIZE, AXIS1_SIZE, AXIS2_SIZE) == ijkCoordinateTransformer.IJKSize
  assert ijkCoordinateTransformer.annotationsDefined
  assert (AXIS0_MIN, AXIS1_MIN, AXIS2_MIN) == ijkCoordinateTransformer.IJKAnnotationStart
  assert (AXIS0_MAX, AXIS1_MAX, AXIS2_MAX) == ijkCoordinateTransformer.IJKAnnotationEnd
