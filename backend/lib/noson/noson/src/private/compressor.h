/*
 *      Copyright (C) 2016 Jean-Luc Barriere
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation; either version 3, or (at your option)
 *  any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this library; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 51 Franklin Street, Fifth Floor, Boston,
 *  MA 02110-1301 USA
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#ifndef COMPRESSOR_H
#define COMPRESSOR_H

#include "local_config.h"

#include <cstddef> // for size_t
#include <cstring> // for memcpy

namespace NSROOT
{

  class Compressor
  {
  public:
    typedef int(*STREAM_READER)(void *handle, void *buf, int sz);

    Compressor(STREAM_READER reader, void *handle, int level = -1);
    Compressor(const char *input, size_t len, int level = -1);
    virtual ~Compressor();

    /**
     * @brief More data can be read from output stream.
     * @return Output status
     */
    bool HasOutputData() { return !m_stop; }

    /**
     * @brief Output stream is completed.
     * @return Output stream status
     */
    bool IsCompleted();

    /**
     * @brief Data cannot be read from input.
     * @return Input status
     */
    bool HasBufferError();

    /**
     * @brief Data error occurred from stream.
     * @return stream status
     */
    bool HasStreamError();

    /**
     * @brief Copy data from output stream to the given pointer until size limit.
     * @param buf pointer to copy data
     * @param len max length of data
     * @return lenght of copied data
     */
    size_t ReadOutput(char *buf, size_t len);

    /**
     * @brief Fetch next chunk of data from output stream.
     *        No copy of data is performed and result can be used as is.
     * @param data pointer to const data pointer
     * @return the byte count available from the pointer to const data
     */
    size_t FetchOutput(const char **data);

  private:
    int m_status;
    int m_flush;
    bool m_stop;
    size_t m_chunk_size;

    const enum
    {
      MEM_BUFFER,
      FCB_READER
    } m_type_in;

    size_t m_input_len;
    const char *m_input;

    STREAM_READER m_rstream;
    void *m_rstream_hdl;
    char *m_rstream_buf;

    char *m_output;
    size_t m_output_pos;
    size_t m_output_len;

    void *_opaque;

    static int _init(void *zp, void *out, size_t len, int level);
    size_t NextChunk();
  };


  class Decompressor
  {
  public:
    typedef int(*STREAM_READER)(void *handle, void *buf, int sz);

    Decompressor(STREAM_READER reader, void *handle);
    Decompressor(const char *input, size_t len);
    virtual ~Decompressor();

    /**
     * @brief More data can be read from output stream.
     * @return Output status
     */
    bool HasOutputData() { return !m_stop; }

    /**
     * @brief Output stream is completed.
     * @return Output stream status
     */
    bool IsCompleted();

    /**
     * @brief Data cannot be read from input.
     * @return Input status
     */
    bool HasBufferError();

    /**
     * @brief Data error occurred from stream.
     * @return stream status
     */
    bool HasStreamError();

    /**
     * @brief Copy data from output stream to the given pointer until size limit.
     * @param buf pointer to copy data
     * @param len max length of data
     * @return lenght of copied data
     */
    size_t ReadOutput(char *buf, size_t len);

    /**
     * @brief Fetch next chunk of data from output stream.
     *        No copy of data is performed and result can be used as is.
     * @param data pointer to const data pointer
     * @return the byte count available from the pointer to const data
     */
    size_t FetchOutput(const char **data);

  private:
    int m_status;
    bool m_stop;
    size_t m_chunk_size;

    const enum
    {
      MEM_BUFFER,
      FCB_READER
    } m_type_in;

    size_t m_input_len;
    const char *m_input;

    STREAM_READER m_rstream;
    void *m_rstream_hdl;
    char *m_rstream_buf;

    char *m_output;
    size_t m_output_pos;
    size_t m_output_len;

    void *_opaque;

    static int _init(void *zp, void *out, size_t len);
    size_t NextChunk();
  };

}

#endif /* COMPRESSOR_H */

