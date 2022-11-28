import openvds
import openvds_test
import pytest

from CreateVds import *

def test_create_1bit_vds():
    name = "create_1bit_vds"
    size = (79, 79, 79)
    brickSize = openvds.VolumeDataLayoutDescriptor.BrickSize.BrickSize_64

    layoutDescriptor = openvds.VolumeDataLayoutDescriptor(brickSize,
                                                          0, 0, 4,
                                                          openvds.VolumeDataLayoutDescriptor.LODLevels.LODLevels_None,
                                                          openvds.VolumeDataLayoutDescriptor.Options.Options_None)
            
    axisDescriptors = [ openvds.VolumeDataAxisDescriptor(size[0], "X", "m", 0.0, 2000.0),
                        openvds.VolumeDataAxisDescriptor(size[1], "Y", "m", 0.0, 2000.0),
                        openvds.VolumeDataAxisDescriptor(size[2], "Z", "m", 0.0, 2000.0),
                      ]
    channelDescriptors = [ openvds.VolumeDataChannelDescriptor(openvds.VolumeDataFormat.Format_1Bit,
                                                               openvds.VolumeDataChannelDescriptor.Components.Components_1,
                                                               "Value", "", 0.0, ((size[2] * 3) * (size[1] * 2) * size[0]) - 1.0)
                         ]
    
    metaData = openvds.MetadataContainer()

    vds = openvds.create("inmemory://{}".format(name), "", layoutDescriptor, axisDescriptors, channelDescriptors, metaData)
    layout = openvds.getLayout(vds)
    
    def writePages(accessor, data):
        for c in range(accessor.getChunkCount()):
            page = accessor.createPage(c)
            buf = np.array(page.getWritableBuffer(), copy = False)
    #        print("shape {} strides {}".format(buf.shape, buf.strides))
            (min, max) = page.getMinMax()
    #        print("min {} max {}".format(min, max))
            buf[:,:,:] = np.packbits(data[min[2]:max[2],min[1]:max[1],min[0]:max[0]], axis=-1)
            page.release()
        accessor.commit()
    
    shape = (layout.getDimensionNumSamples(0), layout.getDimensionNumSamples(1), layout.getDimensionNumSamples(2))
    data = np.random.Generator(np.random.PCG64(seed=313)).integers(2, size=shape[0]*shape[1]*shape[2], dtype=np.bool_).reshape(shape)

    manager = openvds.getAccessManager(vds)
    accessor = manager.createVolumeDataPageAccessor(openvds.DimensionsND.Dimensions_012, 0, 0, 8, openvds.IVolumeDataAccessManager.AccessMode.AccessMode_Create, 1024)
    writePages(accessor, data)

    #req = manager.requestVolumeSubset(min=(0,0,0), max=(shape[0], shape[1], shape[2]))
    #result = req.data.reshape(shape)
    #assert np.array_equal(data, result)
    
    openvds.close(vds)
