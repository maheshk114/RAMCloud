/* Copyright (c) 2010 Stanford University
 *
 * Permission to use, copy, modify, and distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright
 * notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR(S) DISCLAIM ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL AUTHORS BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER
 * RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF
 * CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * \file Buffer.h Header file for the Buffer class.
 */

#ifndef RAMCLOUD_BUFFER_H
#define RAMCLOUD_BUFFER_H

#include <Common.h>

namespace RAMCloud {

/**
 * \class Buffer
 *
 * This class manages a logically linear array of bytes, which is implemented as
 * discontiguous chunks in memory. This class exists so that we can avoid copies
 * between the multiple layers of the RAMCloud system, by passing the Buffer
 * associated with memory regions instead of copying the regions themselves.
 */     
class Buffer {
  public:

    /**
     * This class provides a way to iterate over the chunks of a Buffer.
     * This should only be used by the low-level networking code, as Buffer
     * provides more convenient methods to access the Buffer for higher-level
     * code.
     *
     * \warning
     * The Buffer must not be modified during the lifetime of the iterator.
     */
    class Iterator {
      public:
        explicit Iterator(const Buffer& buffer);
        ~Iterator();
        bool isDone() const;
        void next();
        void* getData() const;
        uint32_t getLength() const;

      private:
        /**
         * The Buffer over which to iterate, as given to #Iterator().
         */
        const Buffer& buffer;

        /**
         * An index into the Buffer::chunks array. This starts at 0.
         */
        uint32_t chunkIndex;

      friend class BufferIteratorTest;
    };

    void prepend(void* src, uint32_t length);

    void append(void* src, uint32_t length);

    uint32_t peek(uint32_t offset, void** returnPtr);

    void* getRange(uint32_t offset, uint32_t length);

    uint32_t copy(uint32_t offset, uint32_t length, void* dest); // NOLINT

    /**
     * Returns the sum of the induvidual sizes of all the chunks composing this
     * Buffer.
     *
     * \return See above.
     */
    uint32_t totalLength() const { return totalLen; }

    /**
     * Return the number of chunks composing this Buffer.
     * Along with #Iterator, this is useful for networking code that is trying
     * to export the Buffer into a different format.
     */
    uint32_t numberChunks() const { return chunksUsed; }

    Buffer();

    Buffer(void* firstChunk, uint32_t firstChunkLen);

    ~Buffer();

  private:
    void allocateMoreChunks();

    void allocateMoreExtraBufs();

    uint32_t findChunk(uint32_t offset, uint32_t* chunkOffset);

    /**
     * A Buffer is an ordered collection of Chunks. Each induvidual chunk
     * represents a physically contiguous region of memory. When taking
     * together, an array of Chunks represent a logically contiguous memory
     * region, i.e., this Buffer.
     */
    struct Chunk {
        void *data;        // Pointer to the data represented by this Chunk.
        uint32_t len;      // The length of this Chunk in bytes.
    };

    /**
     * The initial size of the chunk array (see below). 10 should cover the vast
     * majority of Buffers. If not, we can increase this later.
     */
    static const uint32_t INITIAL_CHUNK_ARR_SIZE = 10;

    /**
     * The initial size of the extraBufs array. However, the extraBufs array is
     * only allocated when it is needed, ie, on the first call to getRange that
     * needs extra space.
     */
    static const uint32_t INITIAL_EXTRA_BUFS_ARR_SIZE = 10;

    uint32_t chunksUsed;      // The number of chunks that are currently in use.
                              // That is, the number of chunks that contain
                              // valid pointers to memory regions.
    uint32_t chunksAvail;     // The total number of chunks available at
                              // (*chunks). This is the number of chunks that we
                              // have allocated memory for.
    Chunk* chunks;            // The pointers and lengths of various chunks
                              // represented by this Buffer. Initially, we
                              // allocate INITIAL_CHUNK_ARR_SIZE of the above
                              // chunks, since this would be faster than using a
                              // vector<chunk>.
    uint32_t totalLen;        // The sum of the induvidual sizes of all the
                              // chunks currently in use.
    void **extraBufs;         // An array of pointers to memory that we allocate
                              // when we need to copy a range of bytes into
                              // contiguous memory, as part of getRange().
    uint32_t extraBufsAvail;  // The size of the extraBufs array.
    uint32_t extraBufsUsed;   // The number of extraBufs currently in use.

    friend class Iterator;
    friend class BufferTest;  // For CppUnit testing purposes.

    DISALLOW_COPY_AND_ASSIGN(Buffer);
};

}  // namespace RAMCloud

#endif  // RAMCLOUD_BUFFER_H
