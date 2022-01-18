.. _vds_disk_storage_format:

===================
Disk Storage Format
===================

This document describes the file format used to store VDS files on disk. The file format is a generic container that can
be used to store a variety of "objects" within a single file on disk. As such, it is also possible to store non-VDS data
using this file format. For example, the Bluware HueSpace engine uses this on-disk format for  properties and shape
data.

The format is chunk-based, and allows for sparse files. It is intended to be only a container format with minimal
knowledge of the semantics of the stored data.

High-level Requirements
-----------------------

- The container file can only be modified from a single process at a time.
  Concurrent read-only access by multiple processes is supported.
- Atomic updates: a crash, full disk, etc. must never leave behind a corrupt file.
- It must be possible to group multiple updates into a transaction. Nested transactions are not supported.
- Fast direct reads/writes from/to any position within the file.
- Metadata stored for chunk must be extensible [e.g. VDSs need min/max values in the chunk].
- Individual chunks within the same file may be of different sizes.
- Support for file versioning and delta compression.

File Structure
--------------

The overall organization is a container that has a number of named files consisting of a number of chunks. For each file
there is an index table which contains the file offset and size of each chunk. Additional metadata can be stored per
chunk (e.g. hash values for de-duplication of data, min/max values of the data in a chunk). The index table is divided
into pages so it is not necessary to read or write the entire index when updating the file.

Header
^^^^^^

A valid VDS file starts with a 12-byte string which reads ``HueDataStore``.

.. code-block:: c++

  struct DataStoreHeader
  {
    char                magic[12]; /* "HueDataStore" */
    int32_t             version;   /* (major << 16) + minor */
    int64_t             file_table_offset;
    int32_t             file_table_num_entries;
    int32_t             file_table_name_length;
  };

File Table Entry
^^^^^^^^^^^^^^^^

Each file table entry describes the number of chunks in the file, the layout of the chunk index table and where to find
the page table for the file. A UTF-8 encoded file name with the length described in the file header follows the
information about the file. The file table name length must be a multiple of 8 to ensure alignment of the file table
entries. The file names are assumed to be padded with NUL characters up to the file table name length, but need not be
NUL terminated.

.. code-block:: c++

  struct FileTableEntry
  {
      int64_t             head_page_directory_offset;
      int32_t             head_num_chunks;
      int32_t             head_revision_number;
      int32_t             index_page_num_entries;
      int32_t             file_type; // file type four-CC 
                                     // (four ASCII-characters)
      int32_t             chunk_metadata_length;
      int16_t             file_metadata_length;
      char                file_name[/* file_table_name_length */];
  };

All file table entries have the same number of bytes reserved for the file name.

Index Entry
^^^^^^^^^^^

Each chunk in the file has an index entry associated with it. This ensures that basic information about the chunk can be
kept in memory. The chunk index makes it possible to do partial updates of files where each chunk may change size as a
result of compression.

.. code-block:: c++

  struct IndexEntry
  {
      int64_t             offset;
      int32_t             size;
      int32_t             reserved;
      char                metadata[/* chunk_metadata_length */];
  };

Index Page
^^^^^^^^^^

In order to support atomic updates of files, the index entries are grouped into pages. This makes atomic updates of the
index entries possible without rewriting the entire index. The number of index entries per page is found in the file
table entry field ``index_page_num_entries``. Because the index pages do not have a header, it is possible to write a
contiguous index and only decide how to break it up into pages when writing the page directory. If the number of chunks
in the file is not a multiple of the number of entries per page, the last index page must be zero-padded.

Page Directory
^^^^^^^^^^^^^^

At the ``page_directory_offset`` we find a small header and an array of ``int64_t`` file offsets where each index page
starts. The number of index pages is the number of chunks divided by the number of indexes per page (rounded up to the
nearest integer). The last index page only has as many entries as there are remaining chunks in the file. If the offset
for a page is zero, that means the page has not been written to the file yet, this ensures that is reasonably efficient
to have a sparse file.

In addition to this, the page directory provides a basic versioning scheme by being able to link to previous versions of
the page directory. The revision number must be decreasing for previous versions, and cannot be negative.

.. code-block:: c++

  struct PageDirectory
  {
      int64_t             previous_page_directory_offset;
      int32_t             previous_num_chunks;
      int32_t             previous_revision_number;
      char                metadata[/* file_metadata_length */];
      int64_t             index_page_offsets[/* num_index_pages */];
  };
