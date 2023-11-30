/*
 * Copyright 2023 SequoiaDB Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 */

package com.sequoiadb.base;

import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;

import java.util.Objects;

/**
 * The charset option for Sequoiadb Connection.
 */
public class ClientCharset {
    private final static ClientCharsetEnum DEFAULT_CHARSET = ClientCharsetEnum.UTF_8;
    private ClientCharsetEnum clientCharset;
    private ClientCharsetEnum resultsCharset;

    /**
     * Create an instance of ClientCharset, default charset is UTF-8.
     */
    public ClientCharset() {
        this.clientCharset = DEFAULT_CHARSET;
        this.resultsCharset = DEFAULT_CHARSET;
    }

    /**
     * Sets the charset for encoding request message.
     *
     * @param charset the charset
     */
    public void setClientCharset(final ClientCharsetEnum charset) {
        if (charset == null) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "charset cannot be null");
        }
        this.clientCharset = charset;
    }

    /**
     * Sets the charset for decoding response message.
     *
     * @param charset the charset
     */
    public void setResultsCharset(final ClientCharsetEnum charset) {
        if (charset == null) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "charset cannot be null");
        }
        this.resultsCharset = charset;
    }

    /**
     * Sets the charset for encoding request message and decoding response message.
     *
     * @param charset the charset
     */
    public void setCharsets(final ClientCharsetEnum charset) {
        if (charset == null) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "charset cannot be null");
        }
        this.clientCharset = charset;
        this.resultsCharset = charset;
    }

    /**
     * Gets the charset for encoding request message.
     *
     * @return the charset
     */
    public ClientCharsetEnum getClientCharset() {
        return clientCharset;
    }

    /**
     * Gets the charset for decoding response message.
     *
     * @return the charset
     */
    public ClientCharsetEnum getResultsCharset() {
        return resultsCharset;
    }

    @Override
    public boolean equals(final Object o) {
        if (this == o) {
            return true;
        }
        if (o == null || getClass() != o.getClass()) {
            return false;
        }
        final ClientCharset that = (ClientCharset) o;

        return Objects.equals(clientCharset, that.clientCharset)
                && Objects.equals(resultsCharset, that.resultsCharset);
    }

    @Override
    public int hashCode() {
        return Objects.hash(clientCharset, resultsCharset);
    }

    @Override
    public String toString() {
        return "ClientCharset{" +
                "clientCharset=" + clientCharset +
                ", resultCharset=" + resultsCharset +
                '}';
    }
}
