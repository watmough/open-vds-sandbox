Interpolation
=============

Some of the methods to request data from VDS use floating-point indexes into the VDS and can interpolate between the closest samples.

Floating-point indexes
----------------------

The integer positions are the edges of each voxel, so the first voxel goes from 0.0 to 1.0 with the voxel center at 0.5. The sampled value
will be exactly equal to the original at the voxel center, regardless of the interpolation method used. The original reasoning for this
was to match OpenGL interpolation where each voxel is the same size when using nearest-neighbour interpolation, so the voxels at the edges
are not cut in half. This means that you should always add 0.5 to the integer idexes of a voxel when converting to floating-point indexes,
and you have to take care to transform the corners of a grid definition for a seismic survey to the centers of the voxels at the corners of the VDS.
Interpolation is only performed in the dimensions of the dimension group you are reading data from.

Interpolation methods
---------------------

We provide three main interpolation methods that are standard in image processing, nearest-neighbour, linear and cubic. In addition to this there
are two extra variants of linear interpolation, angular and triangular, that are situationally useful.

Nearest-neighbour interpolation will sample from the closest voxel center (0.5, 1.5, ...) to the sample position, the boundary where you go from sampling one voxel value to the next will be at the integer positions.

Linear interpolation will interpolate linearly between the voxel centers, so at position 1.0 you get the average of the first sample with voxel center at 0.5 and the second sample value with voxel center at 1.5.

Cubic interpolation will use one sample behind and two samples in front and compute the sample weights using a cubic Hermite spline, for more information see the Wikipedia aricle on
`bi-cubic interpolation https://en.wikipedia.org/wiki/Bicubic_interpolation`` (we will use tri-cubic interpolation when reading from a 3D dimension group).

Angular interpolation is the same as linear, but it will assume the values in the VDS wrap around the value range. This is particularly useful when the data is in degrees or radians and the value range is (0, 360), (0, 2*pi) or (-pi, +pi).

Triangular interpolation will interpolate linearly between the three nearest voxel centers, so the result will be similar to drawing a triangulated mesh with vertex interpolation.

NoValues and Interpolation
--------------------------

All the provided interpolation methods take care to set the weight of a NoValue sample to 0.0 so it will not affect the interpolated result. The remaining non-zero weights will be
normalized so they sum to 1.0. If all weights are 0 (i.e. the samples that would be interpolated are all NoValue), the result will be NoValue.
