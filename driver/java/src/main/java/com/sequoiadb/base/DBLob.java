/*
 * Copyright 2017 SequoiaDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/

package com.sequoiadb.base;

import com.sequoiadb.exception.BaseException;
import org.bson.types.ObjectId;

import java.io.Closeable;
import java.io.InputStream;
import java.io.OutputStream;

/**
 * @class DBLob
 * @brief Operation interfaces of DBLob.
 */
public interface DBLob extends Closeable {
    /**
     * @memberof SDB_LOB_SEEK_SET 0
     * @brief Change the position from the beginning of lob
     */
    int SDB_LOB_SEEK_SET = 0;

    /**
     * @memberof SDB_LOB_SEEK_CUR 1
     * @brief Change the position from the current position of lob
     */
    int SDB_LOB_SEEK_CUR = 1;

    /**
     * @memberof SDB_LOB_SEEK_END 2
     * @brief Change the position from the end of lob
     */
    int SDB_LOB_SEEK_END = 2;

    /**
     * @memberof SDB_LOB_READ
     * @brief LOB open mode for reading
     */
    int SDB_LOB_READ = 0x00000004;

    /**
     * @memberof SDB_LOB_WRITE
     * @brief LOB open mode for writing
     */
    int SDB_LOB_WRITE = 0x00000008;

    /**
     * @return the lob's id
     * @fn ObjectId getID()
     * @brief get the lob's id
     */
    ObjectId getID();

    /**
     * @return the lob's size
     * @fn long getSize()
     * @brief get the size of lob
     */
    long getSize();

    /**
     * @return the lob's create time
     * @fn long getCreateTime()
     * @brief get the create time of lob
     */
    long getCreateTime();

    /**
     * @return the lob's last modification time
     * @fn long getModificationTime()
     * @brief get the last modification time of lob
     */
    long getModificationTime();

    /**
     * @param in the input stream.
     * @throws com.sequoiadb.exception.BaseException
     * @fn void write( InputStream in )
     * @brief Writes bytes from the input stream to this lob.
     * @note user need to close the input stream
     */
    void write(InputStream in) throws BaseException;

    /**
     * @param b the data.
     * @throws com.sequoiadb.exception.BaseException
     * @fn void write( byte[] b )
     * @brief Writes <code>b.length</code> bytes from the specified
     * byte array to this lob.
     */
    void write(byte[] b) throws BaseException;

    /**
     * @param b   the data.
     * @param off the start offset in the data.
     * @param len the number of bytes to write.
     * @throws com.sequoiadb.exception.BaseException
     * @fn void write( byte[] b, int off, int len )
     * @brief Writes <code>len</code> bytes from the specified
     * byte array starting at offset <code>off</code> to this lob.
     */
    void write(byte[] b, int off, int len) throws BaseException;

    /**
     * @param out the output stream.
     * @throws com.sequoiadb.exception.BaseException
     * @fn void read( OutputStream out )
     * @brief Reads the content to the output stream.
     * @note user need to close the output stream
     */
    void read(OutputStream out) throws BaseException;

    /**
     * @param b the buffer into which the data is read.
     * @return the total number of bytes read into the buffer, or <code>-1</code> if
     * there is no more data because the end of the file has been
     * reached, or <code>0</code> if <code>b.length</code> is Zero.
     * @throws com.sequoiadb.exception.BaseException
     * @fn int read( byte[] b )
     * @brief Reads up to <code>b.length</code> bytes of data from this lob into
     * an array of bytes.
     */
    int read(byte[] b) throws BaseException;

    /**
     * @param b   the buffer into which the data is read.
     * @param off the start offset in the destination array <code>b</code>.
     * @param len the maximum number of bytes read.
     * @return the total number of bytes read into the buffer, or <code>-1</code> if
     * there is no more data because the end of the file has been
     * reached, or <code>0</code> if <code>len</code> is Zero.
     * @throws com.sequoiadb.exception.BaseException
     * @fn int read( byte[] b, int off, int len )
     * @brief Reads up to <code>len</code> bytes of data from this lob into
     * an array of bytes.
     */
    int read(byte[] b, int off, int len) throws BaseException;

    /**
     * @param size the adding size.
     * @param seekType SDB_LOB_SEEK_SET/SDB_LOB_SEEK_CUR/SDB_LOB_SEEK_END
     * @throws com.sequoiadb.exception.BaseException.
     * @fn seek(long size, int seekType)
     * @brief change the read or write position of the lob. The new position is
     * obtained by adding size to the position specified by
     * seekType. If seekType is set to SDB_LOB_SEEK_SET,
     * SDB_LOB_SEEK_CUR, or SDB_LOB_SEEK_END, the offset is
     * relative to the start of the lob, the current position
     * of lob, or the end of lob.
     */
    void seek(long size, int seekType) throws BaseException;

    /**
     * @param offset lock start position
     * @param length lock length
     * @throws com.sequoiadb.exception.BaseException.
     * @fn lock(long offset, long length)
     * @brief lock LOB section for write mode
     */
    void lock(long offset, long length) throws BaseException;

    /**
     * @param offset lock start position
     * @param length lock length
     * @throws com.sequoiadb.exception.BaseException.
     * @fn lockAndSeek(long offset, long length)
     * @brief lock LOB section for write mode and seek to the offset position
     */
    void lockAndSeek(long offset, long length) throws BaseException;

    /**
     * @param null
     * @throws com.sequoiadb.exception.BaseException
     * @fn close()
     * @brief close the lob
     */
    void close() throws BaseException;
}

