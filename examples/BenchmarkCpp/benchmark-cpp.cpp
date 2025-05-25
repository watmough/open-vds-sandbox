//
// Benchmark for OpenVDS in C++
//

#include "running-stats.hpp"

#include <chrono>
#include <cstdint>
#include <fmt/format.h>
#include <string>

using namespace std;

#include <OpenVDS/OpenVDS.h>
#include <OpenVDS/VolumeDataLayout.h>
#include <OpenVDS/VolumeDataAccess.h>


// Options for Benchmark

//  DATA
//  * Full volume by range of brick sizes
//  * Full volume by il/xl/x slices
//  * Random mix of full il/xl/sample slices
//  * Random positions, random size cubes

//  VDS QUERIES
//  * RequestVolumeSamples ... consider
//  * RequestVolumeSubset
//  * 

//  EXECUTION
//  * Single-threaded client
//  * Multi-threaded client with #n threads
//  * Single-threaded with coroutines (poll completions)
//  * Multi-threaded with coroutines


void vds_info(string file, int64_t chunks_to_read) {

  // open the vds
    // open a vds
    OpenVDS::Error error;
    OpenVDS::VDSHandle vds_handle = OpenVDS::Open(file, error);
    if(error.code != 0)
    {
      throw std::runtime_error(fmt::format("%s: Unable to open VDS {} with error {}.", file, error.string));
    }

  // get VDS access manager
  OpenVDS::VolumeDataAccessManager vds_access_manager = OpenVDS::GetAccessManager(vds_handle);

  // get the layout, so we can query the dimensions
  const OpenVDS::VolumeDataLayout *vds_layout = vds_access_manager.GetVolumeDataLayout();

  // print the vds dimensions
  uint32_t ijklmn[6] = {0,0,0,0,0,0};
  fmt::print("\nVolume axes / dimensions:\n");
  for (int dim=0; dim<vds_layout->GetDimensionality(); ++dim) {

      // get VDS axis descriptor from layout
      OpenVDS::VolumeDataAxisDescriptor axisDescriptor = vds_layout->GetAxisDescriptor(dim);
      ijklmn[dim] = axisDescriptor.GetNumSamples();
      fmt::print("  Dimension {}  {}  {} - {} ({})\n", dim, axisDescriptor.GetName(),
                  axisDescriptor.GetCoordinateMin(), axisDescriptor.GetCoordinateMax(), axisDescriptor.GetNumSamples());
  }

  // get the chunk count
  auto chunk_count = vds_access_manager.GetVDSChunkCount(OpenVDS::Dimensions_012, /*lod*/0, /*channel*/0);
  chunks_to_read = !chunks_to_read ? chunk_count : min(chunk_count, chunks_to_read);
  fmt::print("VDS chunk count is {} Reading {} chunks.\n\n", chunk_count, chunks_to_read);

  // get volume data page accessor
  auto page_accessor = vds_access_manager.CreateVolumeDataPageAccessor(OpenVDS::Dimensions_012,
    /*lod*/0, /*channel*/0, /*maxPages*/16, OpenVDS::VolumeDataAccessManager::AccessMode_ReadOnly);

  // time to fetch chunks
  running_stats chunk_time_stats;

  // get the chunk sizes and data
  int voxel_min[6], voxel_max[6];
  for (uint32_t chunk=0; chunk<chunks_to_read; ++chunk) {

    // start a timer
    auto t_start = chrono::high_resolution_clock::now();

    // get each chunk dimension
    page_accessor->GetChunkMinMax(chunk, voxel_min, voxel_max);

    // print the 3 dimensions
    fmt::print("chunk {}  il {:3}-{:3}  xl {:3}-{:3} samples {:3}-{:3} ... ", chunk,
                voxel_min[2], voxel_max[2], voxel_min[1], voxel_max[1], voxel_min[0], voxel_max[0]);

    // read the page
    OpenVDS::VolumeDataPage *page = page_accessor->ReadPage(chunk);
    int size[6], pitch[6];
    const void *buffer = page->GetBuffer(size, pitch);
    fmt::print("size {} x {} x {}  pitch {} x {} x {}\n",
                size[2], size[1], size[0], pitch[2], pitch[1], pitch[0]);

    // capture time to get (upto) 128x128x128 chunk from open vds
    auto t_stop = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = t_stop - t_start;
    chunk_time_stats.add(duration.count());

    // // calculate page stats
    // const float *float_data = (float*)buffer;
    // running_stats chunk_stats;
    // for (uint32_t idx{0}; idx<size[0]*size[1]*size[2]; ++idx) {
    //   chunk_stats.add(float_data[idx]);
    // }
    // fmt::print("{}: chunk {} mean {:.3} std dev {:.3} over {} samples.\n", __FUNCTION__, chunk,
    //     chunk_stats.mean(), chunk_stats.std_dev(), chunk_stats.sample_count());

    // release used page
    page->Release();
  }

  // print the chunk data retrieval stats
  auto chunk_stat_count = chunk_time_stats.sample_count();
  auto chunk_total_data = 128*128*128*sizeof(float) * chunk_stat_count / 1024 / 1024;
  auto chunk_total_time = chunk_stat_count * chunk_time_stats.mean() / 1000;
  fmt::print("Fetched {} 128x128x128 chunks in {:.3} sec, mean {:.3} std.dev. {:.3} msec per chunk. {:.3} MBytes/sec.\n",
              chunk_stat_count, chunk_total_time, chunk_time_stats.mean(), chunk_time_stats.std_dev(), chunk_total_data/chunk_total_time);

}

int main(int argc, const char *argv[]) {

  if (argc<2) {
    fmt::print("usage: {} vds-file [ChunksToRead]\n");
    fmt::print("       Test VDS access speed by different methods.\n\n");
    std::exit(1);
  }

  // default to reading all chunks
  uint32_t chunks_to_read{0};
  if (argc==3) {
    chunks_to_read = atoi(argv[2]);
  }

  string filename(argv[1]);
  vds_info(filename, chunks_to_read);

  return 0;
}
