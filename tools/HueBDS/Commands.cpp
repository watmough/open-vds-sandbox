/****************************************************************************
** Copyright 2022 The Open Group
** Copyright 2022 Bluware, Inc.
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

#include "Commands.h"


int Command::argumentCount[] = { 0, 1, 2 };

bool ScanVDSFile(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, int argumentCount, char *arguments[])
{
  if (file->GetFileType() != FILETYPE_VDS_LAYER)
  {
    printf("File is not a VDS layer\n");
    return false;
  }

  int
    fileMetadataLength = file->GetFileMetadataLength();


  if (fileMetadataLength != sizeof(VDSLayerMetadata) && fileMetadataLength != sizeof(VDSLayerMetadataWaveletAdaptive))
  {
    printf("File metadata has the wrong length (%d, expected %d or %d)\n", file->GetFileMetadataLength(), (int)sizeof(VDSLayerMetadata), (int)sizeof(VDSLayerMetadataWaveletAdaptive));
    return false;
  }

  bool
    isWaveletAdaptive = (fileMetadataLength == sizeof(VDSLayerMetadataWaveletAdaptive));

  int
    chunkMetadataLength = isWaveletAdaptive ? sizeof(VDSWaveletAdaptiveLevelsChunkMetadata) : sizeof(VDSChunkMetadata);

  if (file->GetChunkMetadataLength() != chunkMetadataLength)
  {
    printf("Chunk metadata has the wrong length (%d, expected %d)\n", file->GetChunkMetadataLength(), chunkMetadataLength);
    return false;
  }

  VDSLayerMetadataWaveletAdaptive
    fileMetadata;

  file->ReadFileMetadata(&fileMetadata);

  static const uint64_t UNKNOWN = 0ULL;
  static const uint64_t CONSTANT = 0x0101010100000000ULL;
  static const uint64_t NOVALUE = ~0ULL;
  static const uint64_t MASK = 0xFFFFFFFF00000000ULL;

  int
    nChunkCount = file->GetChunkCount(),
    validChunks = 0,
    constantChunks = 0,
    noValueChunks = 0;

  int64_t
    nTotalSize = 0;

  int64_t
    anAccumulatedAdaptiveLevelSizes[VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS] = { 0 };

  bool
    isInInvalidRange = false;

  std::vector<Range_c>
    lcRange;

  Range_c
    cRange;

  for (int iChunk = 0; iChunk < nChunkCount; iChunk++)
  {
    bool
      success;

    IndexEntry
      indexEntry;

    VDSWaveletAdaptiveLevelsChunkMetadata
      metadata;

    memset(&metadata, 0, sizeof(VDSWaveletAdaptiveLevelsChunkMetadata));

    success = file->ReadIndexEntry(iChunk, &indexEntry, &metadata);

    if (!success)
    {
      printf("Failed to read index entry for chunk %d: %s\n", iChunk, dataStore->GetErrorMessage());
      return false;
    }

    nTotalSize += indexEntry.m_length;

    uint64_t
      chunkHash = metadata.m_hash;

    if (metadata.m_hash != UNKNOWN)
    {
      validChunks++;

      if (isInInvalidRange)
      {
        cRange._iRangeEnd = iChunk - 1;
        lcRange.push_back(cRange);
        isInInvalidRange = false;
      }
    }
    else {
      if (!isInInvalidRange)
      {
        cRange._iRangeStart = iChunk;
        isInInvalidRange = true;
      }
    }

    if ((chunkHash & MASK) == CONSTANT)
    {
      constantChunks++;
    }
    else if (chunkHash == NOVALUE)
    {
      noValueChunks++;
      constantChunks++;
    }

    if (isWaveletAdaptive)
    {
      int32_t
        adaptiveLevelSizes[VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS];

      DecodeWaveletAdaptiveLevelsMetadata(adaptiveLevelSizes, indexEntry.m_length, metadata.m_levels);

      for (int level = 0; level < VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS; level++)
      {
        anAccumulatedAdaptiveLevelSizes[level] += adaptiveLevelSizes[level];
      }
    }
  }

  if (isInInvalidRange)
  {
    cRange._iRangeEnd = nChunkCount - 1;
    lcRange.push_back(cRange);
  }

  if (noValueChunks > 0)
  {
    printf("Total size %lld in %d chunks, %d valid chunks, %d constant chunks(%d NoValue)\n", (long long int)nTotalSize, nChunkCount, validChunks, constantChunks, noValueChunks);
  }
  else
  {
    printf("Total size %lld in %d chunks, %d valid chunks, %d constant chunks\n", (long long int)nTotalSize, nChunkCount, validChunks, constantChunks);
  }

  if (fileMetadata.m_validChunkCount != validChunks)
  {
    printf("Inconsistent file metadata (%d valid chunks versus %d actually valid)\n", fileMetadata.m_validChunkCount, validChunks);
  }

  if (isWaveletAdaptive)
  {
    for (int level = 0; level < VDSWaveletAdaptiveLevelsChunkMetadata::WAVELET_ADAPTIVE_LEVELS; level++)
    {
      if (anAccumulatedAdaptiveLevelSizes[level] != fileMetadata.m_adaptiveLevelSizes[level])
      {
        printf("Inconsistent file metadata (size of wavelet adaptive level %d is %lld versus %lld actually accumulated)\n", level, (long long)fileMetadata.m_adaptiveLevelSizes[level], (long long)anAccumulatedAdaptiveLevelSizes[level]);
        break;
      }
    }
  }

  if (!lcRange.empty())
  {
    for (int iRange = 0; iRange < (int)lcRange.size(); ++iRange)
    {
      int pageIndexStart = file->GetIndexPageIndexForChunk(lcRange[iRange]._iRangeStart),
        pageIndexEnd = file->GetIndexPageIndexForChunk(lcRange[iRange]._iRangeEnd),
        entryIndexStart = file->GetIndexEntryIndexForChunk(lcRange[iRange]._iRangeStart),
        entryIndexEnd = file->GetIndexEntryIndexForChunk(lcRange[iRange]._iRangeEnd);

      printf("Invalid range: [%d, %d], ", lcRange[iRange]._iRangeStart, lcRange[iRange]._iRangeEnd);
      printf("PageIndexStart: %d, PageIndexEnd: %d, EntryIndexStart: %d, EntryIndexEnd: %d\n", pageIndexStart, pageIndexEnd, entryIndexStart, entryIndexEnd);
    }
  }

  if (fileMetadata.m_validChunkCount != validChunks)
  {
    return false;
  }

  return true;
}

bool RenameFile(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, int argumentCount, char *arguments[])
{
  bool
    success = dataStore->EnableWriting();

  if (!success)
  {
    printf("Failed to enable writing for Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  file->Rename(arguments[0]);

  success = file->Commit();
  if (!success)
  {
    printf("Failed to commit changes to Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  return true;
}

bool AddFile(HueBulkDataStore *dataStore, int argumentCount, char *arguments[])
{
  bool
    success = dataStore->EnableWriting();

  if (!success)
  {
    printf("Failed to enable writing for Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  char fileTypeChars[4] = { ' ', ' ', ' ', ' ' };

  int fileTypeArgumentLength = (int)strlen(arguments[3]);

  if (fileTypeArgumentLength > 4)
  {
    printf("File type can be at most 4 characters\n");
    return false;
  }

  for (int i = 0; i < fileTypeArgumentLength; i++) fileTypeChars[i] = arguments[3][i];

  int
    fileType = DATASTORE_FILETYPE(fileTypeChars[0], fileTypeChars[1], fileTypeChars[2], fileTypeChars[3]);

  int
    metadataLength = 0;

  void
    *metadata = NULL;

  // Check if metadata provided
  if (argumentCount > 5)
  {
    success = ParseMetadata(NULL, &metadataLength, arguments[5], fileType);

    if (!success)
    {
      printf("ERROR: Couldn't parse file metadata\n");
      return false;
    }

    metadata = alloca(metadataLength);
    memset(metadata, 0, metadataLength);

    success = ParseMetadata(metadata, &metadataLength, arguments[5], fileType);
    assert(success);
  }

  HueBulkDataStore::FileInterface
    *file = dataStore->AddFile(arguments[0], atoi(arguments[1]), atoi(arguments[2]), fileType, atoi(arguments[4]), metadataLength, false);

  if (!file)
  {
    printf("Failed to add file %s to Hue bulk data store file: %s\n", arguments[0], dataStore->GetErrorMessage());
    return false;
  }

  if (metadataLength > 0)
  {
    file->WriteFileMetadata(metadata);
  }

  success = file->Commit();
  if (!success)
  {
    printf("Failed to commit changes to Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  return true;
}

bool RemoveFile(HueBulkDataStore *dataStore, int argumentCount, char *arguments[])
{
  bool
    success = dataStore->EnableWriting();

  if (!success)
  {
    printf("Failed to enable writing for Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  success = dataStore->RemoveFile(arguments[0]);

  if (!success)
  {
    printf("Failed to remove Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  return true;
}

bool CheckDataStore(HueBulkDataStore *dataStore, int argumentCount, char *arguments[])
{
  bool
    success = dataStore->BuildExtentAllocator();

  if (!success)
  {
    printf("Invalid Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  ExtentAllocator
    &extentAllocator = dataStore->GetExtentAllocator();

  int64_t
    totalFreeSize = 0,
    totalChunkSize = 0,
    totalIndexSize = 0,
    totalOtherSize = 0;

  for (ExtentAllocator::Extents::iterator next = extentAllocator.m_extents.begin(), it = next++; next != extentAllocator.m_extents.end(); it = next++)
  {
    int64_t
      extentOffset = it->first,
      extentSize = next->first - it->first;

    ExtentAllocator::Extent &
      extent = it->second;

    char
      combinedExtentDescription[32];

    const char *
      extentDescription;

    switch (extent.m_extentType)
    {
    case ExtentAllocator::ChunkExtent:
      sprintf(combinedExtentDescription, "%d %s", extent.m_combinedExtents, extent.m_combinedExtents == 1 ? "chunk" : "chunks");
      extentDescription = combinedExtentDescription;
      break;

    case ExtentAllocator::IndexPageExtent:
      sprintf(combinedExtentDescription, "%d %s", extent.m_combinedExtents, extent.m_combinedExtents == 1 ? "indexpage" : "indexpages");
      extentDescription = combinedExtentDescription;
      break;

    case ExtentAllocator::FreeExtent:                  extentDescription = "Free";            break;
    case ExtentAllocator::DataStoreHeaderExtent:       extentDescription = "DataStoreHeader"; break;
    case ExtentAllocator::FileTableExtent:             extentDescription = "FileTable";       break;
    case ExtentAllocator::PageDirectoryExtent:         extentDescription = "PageDirectory";   break;
    case ExtentAllocator::MixedExtent:                 extentDescription = "Mixed";           break;
    default: assert(0 && "Illegal extent type"); extentDescription = "Illegal";         break;
    }

    switch (extent.m_extentType)
    {
    case ExtentAllocator::ChunkExtent:     totalChunkSize += extentSize; break;
    case ExtentAllocator::IndexPageExtent: totalIndexSize += extentSize; break;
    case ExtentAllocator::FreeExtent:      totalFreeSize += extentSize; break;
    default:                         totalOtherSize += extentSize; break;
    }

    char
      referencesDescription[100];

    if (extent.m_referenceCount > 1)
    {
      sprintf(referencesDescription, ", referenced %d times", extent.m_referenceCount);
    }
    else
    {
      referencesDescription[0] = '\0';
    }

    printf("%-15s size: %8lld at offset: %10lld%s\n", extentDescription, (long long)extentSize, (long long)extentOffset, referencesDescription);
  }

  printf("Total chunk size:  %10lld\n", (long long)totalChunkSize);
  printf("Total index size:  %10lld\n", (long long)totalIndexSize);
  printf("Total other size:  %10lld\n", (long long)totalOtherSize);
  printf("Unallocated space: %10lld\n", (long long)totalFreeSize);
  return true;
}

bool ListDataStore(HueBulkDataStore *dataStore, int argumentCount, char *arguments[])
{
  int fileCount = dataStore->GetFileCount();
  for (int file = 0; file < fileCount; file++)
  {
    HueBulkDataStore::FileInterface
      *fileInterface = dataStore->OpenFile(dataStore->GetFileName(file));

    if (!fileInterface)
    {
      printf("Error opening file #%d: %s\n", file, dataStore->GetErrorMessage());
      continue;
    }

    int
      chunkCount = fileInterface->GetChunkCount();

    char
      chunkInfo[100];

    if (chunkCount > 1)
    {
      sprintf(chunkInfo, " (%d %s)", chunkCount, chunkCount == 1 ? "chunk" : "chunks");
    }
    else
    {
      chunkInfo[0] = '\0';
    }

    int
      revisionNumber = fileInterface->GetRevisionNumber();

    char
      revisionInfo[100];

    if (revisionNumber > 0)
    {
      sprintf(revisionInfo, " (revision %d)", revisionNumber);
    }
    else
    {
      revisionInfo[0] = '\0';
    }

    int
      metadataLength = fileInterface->GetFileMetadataLength();

    std::string
      metadataInfo;

    if (metadataLength)
    {
      void
        *metadata = alloca(metadataLength);

      fileInterface->ReadFileMetadata(metadata);

      metadataInfo = " ";
      metadataInfo.append(FormatMetadata(metadata, metadataLength, fileInterface->GetFileType()));
    }

    printf("%s%s%s%s\n", dataStore->GetFileName(file), chunkInfo, revisionInfo, metadataInfo.c_str());

    dataStore->CloseFile(fileInterface);
  }
  return true;
}

bool Export(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, const char *outputFileName, int startChunk, int endChunk)
{
  FILE
    *outputFile = fopen(outputFileName, "wb");

  if (!outputFile)
  {
    printf("ERROR: Couldn't open %s for writing\n", outputFileName);
    return false;
  }

  std::string
    chunkfileName(outputFileName);

  chunkfileName.append(".chunks");

  FILE
    *chunkFile = fopen(chunkfileName.c_str(), "wb");

  if (!chunkFile)
  {
    printf("ERROR: Couldn't open %s for writing\n", chunkfileName.c_str());
    return false;
  }

  int
    metadataLength = file->GetChunkMetadataLength();

  void
    *metadata = (metadataLength == 0) ? NULL : alloca(metadataLength);

  for (int chunk = startChunk; chunk < endChunk; chunk++)
  {
    long long
      offset = 0;

    IndexEntry
      indexEntry;

    bool
      read = file->ReadIndexEntry(chunk, &indexEntry, metadata);

    if (!read)
    {
      printf("ERROR: Couldn't read index entry for chunk %d\n", chunk);
      return false;
    }

    if (indexEntry.m_length > 0)
    {
      HueBulkDataStore::Buffer
        *buffer = dataStore->ReadChunkData(indexEntry);

      if (!buffer)
      {
        printf("ERROR: Couldn't read chunk %d\n", chunk);
        return false;
      }

      offset = ftello(outputFile);

      size_t
        written = fwrite(buffer->Data(), buffer->Size(), 1, outputFile);

      HueBulkDataStore::ReleaseBuffer(buffer);

      if (written != 1)
      {
        printf("ERROR: Couldn't write chunk %d to file %s\n", chunk, outputFileName);
        return false;
      }
    }

    std::string
      metadataInfo;

    if (metadataLength)
    {
      metadataInfo = ", metadata: ";
      metadataInfo.append(FormatMetadata(metadata, metadataLength));
    }

    fprintf(chunkFile, "offset: %lld, length: %d%s\n", offset, indexEntry.m_length, metadataInfo.c_str());
  }

  fclose(chunkFile);
  fclose(outputFile);
  return true;
}

bool ExportFile(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, int argumentCount, char *arguments[])
{
  int
    chunkCount = file->GetChunkCount();

  const char *
    outputFileName = file->GetFileName();

  if (argumentCount == 1)
  {
    outputFileName = arguments[0];
  }

  return Export(dataStore, file, outputFileName, 0, chunkCount);
}

bool ExportChunks(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, int argumentCount, char *arguments[])
{
  char *
    endchunkarg = NULL;

  int
    chunk = strtol(arguments[0], &endchunkarg, 10),
    endChunk = chunk + 1;

  if (*endchunkarg == '-')
  {
    endChunk = strtol(endchunkarg + 1, NULL, 10);
  }

  const char *
    outputFileName = file->GetFileName();

  if (argumentCount == 2)
  {
    outputFileName = arguments[1];
  }

  return Export(dataStore, file, outputFileName, chunk, endChunk);
}

bool Import(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, const char *inputFileName, int startChunk, int endChunk)
{
  bool
    success = dataStore->EnableWriting();

  if (!success)
  {
    printf("Failed to enable writing for Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  FILE
    *inputFile = fopen(inputFileName, "rb");

  if (!inputFile)
  {
    printf("ERROR: Couldn't open %s for reading\n", inputFileName);
    return false;
  }

  std::string
    chunkfileName(inputFileName);

  chunkfileName.append(".chunks");

  FILE
    *chunkFile = fopen(chunkfileName.c_str(), "rb");

  if (!chunkFile)
  {
    printf("ERROR: Couldn't open %s for reading\n", chunkfileName.c_str());
    return false;
  }

  int
    metadataLength = file->GetChunkMetadataLength();

  void
    *metadata = (metadataLength == 0) ? NULL : alloca(metadataLength);

  for (int chunk = startChunk; chunk < endChunk; chunk++)
  {
    long long
      offset = 0;

    int
      size = 0;

    int
      items = fscanf(chunkFile, "offset: %lld, length: %d", &offset, &size);

    if (items != 2)
    {
      printf("ERROR: Couldn't read chunk %d definition from %s\n", chunk, chunkfileName.c_str());
      return false;
    }

    if (metadataLength)
    {
      fscanf(chunkFile, ", metadata: ");

      char
        formattedMetadata[1000];

      fgets(formattedMetadata, 1000, chunkFile);

      success = ParseMetadata(metadata, &metadataLength, formattedMetadata);

      if (!success)
      {
        printf("ERROR: Couldn't parse metadata of chunk %d definition from %s\n", chunk, chunkfileName.c_str());
        return false;
      }
    }

    void
      *buffer = NULL;

    if (size > 0)
    {
      buffer = malloc(size);

      if (fread(buffer, size, 1, inputFile) != 1)
      {
        printf("ERROR: Couldn't read chunk %d from file %s\n", chunk, inputFileName);
        free(buffer);
        return false;
      }
    }

    if (buffer)
    {
      success = file->WriteChunk(chunk, buffer, size, metadata);
    }
    else
    {
      success = file->WriteChunkMetadata(chunk, metadata);
    }

    if (!success)
    {
      printf("ERROR: Couldn't write chunk %d\n", chunk);
      free(buffer);
      return false;
    }

    free(buffer);
  }

  fclose(chunkFile);
  fclose(inputFile);

  success = file->Commit();
  if (!success)
  {
    printf("Failed to commit changes to Hue bulk data store file: %s\n", dataStore->GetErrorMessage());
    return false;
  }

  return true;
}

bool ImportFile(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, int argumentCount, char *arguments[])
{
  int
    chunkCount = file->GetChunkCount();

  const char *
    inputFileName = file->GetFileName();

  if (argumentCount == 1)
  {
    inputFileName = arguments[0];
  }

  return Import(dataStore, file, inputFileName, 0, chunkCount);
}

bool ImportChunks(HueBulkDataStore *dataStore, HueBulkDataStore::FileInterface *file, int argumentCount, char *arguments[])
{
  char *
    endchunkarg = NULL;

  int
    chunk = strtol(arguments[0], &endchunkarg, 10),
    endChunk = chunk + 1;

  if (*endchunkarg == '-')
  {
    endChunk = strtol(endchunkarg + 1, NULL, 10);
  }

  const char *
    inputFileName = file->GetFileName();

  if (argumentCount == 2)
  {
    inputFileName = arguments[1];
  }

  return Import(dataStore, file, inputFileName, chunk, endChunk);
}
