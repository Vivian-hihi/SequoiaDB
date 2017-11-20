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

/**
 * Connection configuration option for Sequoiadb.
 */
public class ConfigOptions {
    private long maxAutoConnectRetryTime = 15000;
    private int connectTimeout = 10000;
    private int socketTimeout = 0;
    private boolean socketKeepAlive = false;
    private boolean useNagle = false;
    private boolean useSSL = false;

    /**
     * Get the socket timeout time.
     * @return the socket timeout time(milliseconds)
     */
    public int getSocketTimeout() {
        return socketTimeout;
    }

    /**
     * Set the socket timeout time.
     * @param socketTimeout The socket timeout time(milliseconds).
     */
    public void setSocketTimeout(int socketTimeout) {
        this.socketTimeout = socketTimeout;
    }

    /**
     * Get whether the socket keeps alive or not.
     * @return True if keep alive and false if not.
     */
    public boolean getSocketKeepAlive() {
        return socketKeepAlive;
    }

    /**
     * Set the keep alive status of socket
     * @param socketKeepAlive the the status of socket
     */
    public void setSocketKeepAlive(boolean socketKeepAlive) {
        this.socketKeepAlive = socketKeepAlive;
    }

    /**
     * Get whether use the Nagle algorithm or not.
     * @return True if use nagle and false if not.
     */
    public boolean getUseNagle() {
        return useNagle;
    }

    /**
     * Set whether use the Nagle algorithm or not.
     * @param useNagle whether use the Nagle algorithm or not
     */
    public void setUseNagle(boolean useNagle) {
        this.useNagle = useNagle;
    }

    /**
     * Get the connect timeout time.
     * @return the connect timeout(milliseconds)
     */
    public int getConnectTimeout() {
        return connectTimeout;
    }

    /**
     * Set the connect timeout time.
     * @param connectTimeout connect timeout time(milliseconds)
     */
    public void setConnectTimeout(int connectTimeout) {
        this.connectTimeout = connectTimeout;
    }

    /**
     * Get the max auto connect retry time
     * @return the max auto connect retry time(milliseconds)
     */
    public long getMaxAutoConnectRetryTime() {
        return maxAutoConnectRetryTime;
    }

    /**
     * Set the max auto connect retry time.
     * @param maxAutoConnectRetryTime the max auto connect retry time(milliseconds)
     */
    public void setMaxAutoConnectRetryTime(long maxAutoConnectRetryTime) {
        this.maxAutoConnectRetryTime = maxAutoConnectRetryTime;
    }

    /**
     * Get whether use the SSL or not.
     * @return True if use SSL and false if not.
     * @since 1.12
     */
    public boolean getUseSSL() {
        return useSSL;
    }

    /**
     * Set whether use the SSL or not.
     * @param useSSL whether use the SSL or not
     * @since 1.12
     */
    public void setUseSSL(boolean useSSL) {
        this.useSSL = useSSL;
    }
}
