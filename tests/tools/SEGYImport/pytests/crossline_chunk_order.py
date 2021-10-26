"""
Testing/prototyping code for ordering chunks for crossline-sorted data.
"""
import math
import time
from contextlib import redirect_stdout
from timeit import timeit

import openvds


def create_3d_vds():
    layout_descriptor = openvds.VolumeDataLayoutDescriptor(openvds.VolumeDataLayoutDescriptor.BrickSize.BrickSize_64,
                                                          0, 0, 4,
                                                          openvds.VolumeDataLayoutDescriptor.LODLevels.LODLevels_None,
                                                          openvds.VolumeDataLayoutDescriptor.Options.Options_None)
    compression_method = openvds.CompressionMethod(0)
    compression_tolerance = 0.01

    sample_samples = 500
    crossline_samples = 800
    inline_samples = 350

    axis_descriptors = [
        openvds.VolumeDataAxisDescriptor(sample_samples, openvds.KnownAxisNames.sample(), openvds.KnownUnitNames.millisecond(), 0.0,
                                         (sample_samples - 1) * 4.0),
        openvds.VolumeDataAxisDescriptor(crossline_samples, openvds.KnownAxisNames.crossline(), openvds.KnownUnitNames.unitless(), 2000.0,
                                         2000.0 + (crossline_samples - 1) * 4),
        openvds.VolumeDataAxisDescriptor(inline_samples, openvds.KnownAxisNames.inline(), openvds.KnownUnitNames.unitless(), 1000.0,
                                         1000.0 + (inline_samples - 1) * 2),
        ]
    channel_descriptors = [openvds.VolumeDataChannelDescriptor(openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                              openvds.VolumeDataChannelDescriptor.Components.Components_1,
                                                              "Amplitude", openvds.KnownUnitNames.millisecond(),
                                                              -20000.0, 20000.0)
                          ]

    metaData = openvds.MetadataContainer()
    metaData.setMetadataDoubleVector2(openvds.KnownMetadata.surveyCoordinateSystemOrigin().category,
                                      openvds.KnownMetadata.surveyCoordinateSystemOrigin().name, (1234.0, 4321.0))

    vds = openvds.create("C:\\temp\\SEGY\\t\\crossline_test.vds", "", layout_descriptor, axis_descriptors, channel_descriptors, metaData,
                         compression_method, compression_tolerance)

    return vds


def create_4d_vds():
    layout_descriptor = openvds.VolumeDataLayoutDescriptor(openvds.VolumeDataLayoutDescriptor.BrickSize.BrickSize_64,
                                                           0, 0, 4,
                                                           openvds.VolumeDataLayoutDescriptor.LODLevels.LODLevels_None,
                                                           openvds.VolumeDataLayoutDescriptor.Options.Options_None)
    compression_method = openvds.CompressionMethod(0)
    compression_tolerance = 0.01

    sample_samples = 500
    trace_samples = 100
    crossline_samples = 800
    inline_samples = 350

    axis_descriptors = [
        openvds.VolumeDataAxisDescriptor(sample_samples, openvds.KnownAxisNames.sample(), openvds.KnownUnitNames.millisecond(), 0.0,
                                         (sample_samples - 1) * 4.0),
        openvds.VolumeDataAxisDescriptor(trace_samples, "Trace (offset)", openvds.KnownUnitNames.unitless(), 1.0,
                                         trace_samples + 1.0),
        openvds.VolumeDataAxisDescriptor(crossline_samples, openvds.KnownAxisNames.crossline(), openvds.KnownUnitNames.unitless(), 2000.0,
                                         2000.0 + (crossline_samples - 1) * 4),
        openvds.VolumeDataAxisDescriptor(inline_samples, openvds.KnownAxisNames.inline(), openvds.KnownUnitNames.unitless(), 1000.0,
                                         1000.0 + (inline_samples - 1) * 2),
        ]
    channel_descriptors = [openvds.VolumeDataChannelDescriptor(openvds.VolumeDataChannelDescriptor.Format.Format_R32,
                                                               openvds.VolumeDataChannelDescriptor.Components.Components_1,
                                                               "Amplitude", openvds.KnownUnitNames.millisecond(),
                                                               -20000.0, 20000.0)
                          ]

    metaData = openvds.MetadataContainer()
    metaData.setMetadataDoubleVector2(openvds.KnownMetadata.surveyCoordinateSystemOrigin().category,
                                      openvds.KnownMetadata.surveyCoordinateSystemOrigin().name, (1234.0, 4321.0))

    vds = openvds.create("C:\\temp\\SEGY\\t\\crossline_test.vds", "", layout_descriptor, axis_descriptors, channel_descriptors, metaData,
                         compression_method, compression_tolerance)

    return vds


def create_vds_and_iterate_crossline_chunks_3d():
    vds = create_3d_vds()

    manager = openvds.getAccessManager(vds)
    accessor = manager.createVolumeDataPageAccessor(openvds.DimensionsND.Dimensions_012, 0, 0, 8,
                                                    openvds.IVolumeDataAccessManager.AccessMode.AccessMode_Create, 1024)

    sample_axis_chunk_count = 0
    crossline_axis_chunk_count = 0
    inline_axis_chunk_count = 0

    for chunk_num in range(accessor.getChunkCount()):
        chunk_min, _ = accessor.getChunkMinMax(chunk_num)
        if chunk_min[0] == 0 and chunk_min[1] == 0:
            inline_axis_chunk_count += 1
        if chunk_min[0] == 0 and chunk_min[2] == 0:
            crossline_axis_chunk_count += 1
        if chunk_min[1] == 0 and chunk_min[2] == 0:
            sample_axis_chunk_count += 1

    print()
    print("Gen 1 3D VDS chunk iteration")
    print()
    print(f"sample axis chunk count:     {sample_axis_chunk_count}")
    print(f"crossline axis chunk count:  {crossline_axis_chunk_count}")
    print(f"inline axis chunk count:     {inline_axis_chunk_count}")
    print(f"total chunks:                {accessor.getChunkCount()}")

    chunks_visited = set()

    for chunk_sequence in range(accessor.getChunkCount()):
        # convert sequence number to chunk number for crossline-sorted input

        # xl_chunk_coord = chunk_sequence // (inline_axis_chunk_count * sample_axis_chunk_count)
        # il_chunk_coord = (chunk_sequence % (inline_axis_chunk_count * sample_axis_chunk_count)) // sample_axis_chunk_count
        # sample_chunk_coord = chunk_sequence % sample_axis_chunk_count

        working, sample_chunk_coord = divmod(chunk_sequence, sample_axis_chunk_count)
        xl_chunk_coord, il_chunk_coord = divmod(working, inline_axis_chunk_count)

        chunk_num = il_chunk_coord * crossline_axis_chunk_count * sample_axis_chunk_count + xl_chunk_coord * sample_axis_chunk_count + sample_chunk_coord
        chunk_min, _ = accessor.getChunkMinMax(chunk_num)
        print(f"{chunk_sequence:3}  {chunk_num:3}  ({chunk_min[0]:3}, {chunk_min[1]:3}, {chunk_min[2]:3})")
        chunks_visited.add(chunk_num)

    print()
    print(f"Visited chunk count:  {len(chunks_visited)}")


def create_vds_and_iterate_crossline_chunks_4d():
    vds = create_4d_vds()

    manager = openvds.getAccessManager(vds)
    accessor = manager.createVolumeDataPageAccessor(openvds.DimensionsND.Dimensions_012, 0, 0, 8,
                                                    openvds.IVolumeDataAccessManager.AccessMode.AccessMode_Create, 1024)

    sample_axis_chunk_count = 0
    trace_axis_chunk_count = 0
    crossline_axis_chunk_count = 0
    inline_axis_chunk_count = 0

    for chunk_num in range(accessor.getChunkCount()):
        chunk_min, _ = accessor.getChunkMinMax(chunk_num)
        if chunk_min[0] == 0 and chunk_min[1] == 0 and chunk_min[2] == 0:
            inline_axis_chunk_count += 1
        if chunk_min[0] == 0 and chunk_min[1] == 0 and chunk_min[3] == 0:
            crossline_axis_chunk_count += 1
        if chunk_min[0] == 0 and chunk_min[2] == 0 and chunk_min[3] == 0:
            trace_axis_chunk_count += 1
        if chunk_min[1] == 0 and chunk_min[2] == 0 and chunk_min[3] == 0:
            sample_axis_chunk_count += 1

    print()
    print("Gen 1 4D VDS chunk iteration")
    print()
    print(f"sample axis chunk count:     {sample_axis_chunk_count}")
    print(f"trace axis chunk count:      {trace_axis_chunk_count}")
    print(f"crossline axis chunk count:  {crossline_axis_chunk_count}")
    print(f"inline axis chunk count:     {inline_axis_chunk_count}")
    print(f"total chunks:                {accessor.getChunkCount()}")

    chunks_visited = set()

    for chunk_sequence in range(accessor.getChunkCount()):
        # convert sequence number to chunk number for 4D crossline-sorted input

        working, sample_chunk_coord = divmod(chunk_sequence, sample_axis_chunk_count)
        working, trace_chunk_coord = divmod(working, trace_axis_chunk_count)
        xl_chunk_coord, il_chunk_coord = divmod(working, inline_axis_chunk_count)

        chunk_num = il_chunk_coord * crossline_axis_chunk_count * trace_axis_chunk_count * sample_axis_chunk_count\
                    + xl_chunk_coord * trace_axis_chunk_count * sample_axis_chunk_count\
                    + trace_chunk_coord * sample_axis_chunk_count\
                    + sample_chunk_coord
        chunk_min, _ = accessor.getChunkMinMax(chunk_num)
        print(f"{chunk_sequence:5}  {chunk_num:5}  ({chunk_min[0]:3}, {chunk_min[1]:3}, {chunk_min[2]:3}, {chunk_min[3]:3})")
        chunks_visited.add(chunk_num)

    print()
    print(f"Visited chunk count:  {len(chunks_visited)}")


def iterate_crossline_chunks_universal(vds_handle):
    layout = openvds.getLayout(vds_handle)
    dimensionality = layout.getDimensionality()
    assert dimensionality == 3 or dimensionality == 4

    manager = openvds.getAccessManager(vds_handle)
    accessor = manager.createVolumeDataPageAccessor(openvds.DimensionsND.Dimensions_012, 0, 0, 8,
                                                    openvds.IVolumeDataAccessManager.AccessMode.AccessMode_Create, 1024)

    dim_chunk_counts = [0, 0, 0, 0]

    for chunk_num in range(accessor.getChunkCount()):
        chunk_min, _ = accessor.getChunkMinMax(chunk_num)
        for i in range(len(dim_chunk_counts)):
            is_zero = True
            j = 0
            while is_zero and j < len(dim_chunk_counts):
                if j != i:
                    is_zero = chunk_min[j] == 0
                j += 1
            if is_zero:
                dim_chunk_counts[i] += 1

    for i in range(len(dim_chunk_counts)):
        print(f"dim{i} axis chunk count:     {dim_chunk_counts[i]}")
    print()
    print(f"total chunks:    accessor:  {accessor.getChunkCount()}  counts product:  {math.prod(dim_chunk_counts)}")

    pitches = [1, 1, 1, 1]
    for i in range(1, len(dim_chunk_counts)):
        pitches[i] = pitches[i - 1] * dim_chunk_counts[i - 1]

    chunks_visited = set()

    for chunk_sequence in range(accessor.getChunkCount()):
        # convert sequence number to chunk number for 4D crossline-sorted input

        chunk_coords = [0, 0, 0, 0]
        if dimensionality == 3:
            working, chunk_coords[0] = divmod(chunk_sequence, dim_chunk_counts[0])
            chunk_coords[1], chunk_coords[2] = divmod(working, dim_chunk_counts[2])  # 2 not 1 because we're working in crossline-major space
        elif dimensionality == 4:
            working, chunk_coords[0] = divmod(chunk_sequence, dim_chunk_counts[0])
            working, chunk_coords[1] = divmod(working, dim_chunk_counts[1])
            chunk_coords[2], chunk_coords[3] = divmod(working, dim_chunk_counts[3])  # 3 not 2 because we're working in crossline-major space
        else:
            raise ValueError(f"Invalid dimensionality:  {dimensionality}")

        chunk_num = 0
        for i in range(len(chunk_coords)):
            chunk_num += chunk_coords[i] * pitches[i]

        chunk_min, _ = accessor.getChunkMinMax(chunk_num)
        print(
            f"{chunk_sequence:5}  {chunk_num:5}  ({chunk_min[0]:3}, {chunk_min[1]:3}, {chunk_min[2]:3}, {chunk_min[3]:3})")
        chunks_visited.add(chunk_num)

    print()
    print(f"Visited chunk count:  {len(chunks_visited)}")


def create_3d_vds_and_iterate_universal():
    vds = create_3d_vds()

    print()
    print("Gen 2 (universal) 3D VDS chunk iteration")
    print()

    iterate_crossline_chunks_universal(vds)


def create_4d_vds_and_iterate_universal():
    vds = create_4d_vds()

    print()
    print("Gen 2 (universal) 4D VDS chunk iteration")
    print()

    iterate_crossline_chunks_universal(vds)


if __name__ == "__main__":
    is_poststack = False

    if is_poststack:
        create_vds_and_iterate_crossline_chunks_3d()
        create_3d_vds_and_iterate_universal()
    else:
        with open("C:\\temp\\SEGY\\t\\chunk_order.txt", "w") as f:
            with redirect_stdout(f):
                start = time.perf_counter()
                create_vds_and_iterate_crossline_chunks_4d()
                stop1 = time.perf_counter()
                create_4d_vds_and_iterate_universal()
                stop2 = time.perf_counter()

        print()
        print(f"Gen 1 elapsed:  {stop1 - start}")
        print(f"Gen 2 elapsed:  {stop2 - stop1}")
