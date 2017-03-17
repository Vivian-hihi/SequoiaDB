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

import java.io.InputStream;
import java.io.OutputStream;

/**
 * @class DBLob
 * @brief Operation interfaces of DBLob.
 */
public interface DBLob {
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
     * @fn ObjectId getID()
     * @brief get the lob's id
     * @return the lob's id
     */
    ObjectId getID();

    /**
     * @fn long getSize()
     * @brief get the size of lob
     * @return the lob's size
     */
    long getSize();

    /**
     * @fn long getCreateTime()
     * @brief get the create time of lob
     * @return the lob's create time
     */
    long getCreateTime();

    /**
     * @fn void write( InputStream in )
     * @brief Writes bytes from the input stream to this lob.
     * @param       in   the input stream.
     * @exception com.sequoiadb.exception.BaseException
     * @note user need to close the input stream
     */
    void write(InputStream in) throws BaseException;

    /**
     * @fn void write( byte[] b )
     * @brief Writes <code>b.length</code> bytes from the specified
     *              byte array to this lob. 
     * @param       b   the data.
     * @exception com.sequoiadb.exception.BaseException
     */
    void write(byte[] b) throws BaseException;

    /**
     * @fn void write( byte[] b, int off, int len )
     * @brief Writes <code>len</code> bytes from the specified
     *              byte array starting at offset <code>off</code> to this lob. 
     * @param       b   the data.
     * @param       off the start offset in the data.
     * @param       len the number of bytes to write.
     * @exception com.sequoiadb.exception.BaseException
     */
    void write(byte[] b, int off, int len) throws BaseException;

    /**
     * @fn void read( OutputStream out )
     * @brief Reads the content to the output stream.
     * @param       out   the output stream.
     * @exception com.sequoiadb.exception.BaseException
     * @note user need to close the output stream
     */
    void read(OutputStream out) throws BaseException;

    /**
     * @fn int read( byte[] b )
     * @brief Reads up to <code>b.length</code> bytes of data from this lob into
     *              an array of bytes. 
     * @param       b   the buffer into which the data is read.
     * @return the total number of bytes read into the buffer, or <code>-1</code> if
     *              there is no more data because the end of the file has been 
     *              reached, or <code>0</code> if <code>b.length</code> is Zero.
     * @exception com.sequoiadb.exception.BaseException
     */
    int read(byte[] b) throws BaseException;

    /**
     * @fn int read( byte[] b, int off, int len )
     * @brief Reads up to <code>len</code> bytes of data from this lob into
     *              an array of bytes.
     * @param       b   the buffer into which the data is read.
     * @param       off the start offset in the destination array <code>b</code>.
     * @param       len the maximum number of bytes read.
     * @return the total number of bytes read into the buffer, or <code>-1</code> if
     *              there is no more data because the end of the file has been 
     *              reached, or <code>0</code> if <code>len</code> is Zero.
     * @exception com.sequoiadb.exception.BaseException
     */
    int read(byte[] b, int off, int len) throws BaseException;

    /**
     * @fn seek(long size, int seekType)
     * @brief change the read position of the lob. The new position is
     *              obtained by adding size to the position specified by 
     *              seekType. If seekType is set to SDB_LOB_SEEK_SET, 
     *              SDB_LOB_SEEK_CUR, or SDB_LOB_SEEK_END, the offset is 
     *              relative to the start of the lob, the current position 
     *              of lob, or the end of lob.
     * @param       size the adding size.
     * @param       seekType  SDB_LOB_SEEK_SET/SDB_LOB_SEEK_CUR/SDB_LOB_SEEK_END
     * @exception com.sequoiadb.exception.BaseException.
     */
    void seek(long size, int seekType) throws BaseException;

    /**
     * @fn close()
     * @brief close the lob
     * @param       null
     * @exception com.sequoiadb.exception.BaseException
     */
    void close() throws BaseException;
}

