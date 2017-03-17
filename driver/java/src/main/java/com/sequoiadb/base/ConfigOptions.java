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
 * @class ConfigOptions
 * @brief Database Connection Configuration Option
 */
public class ConfigOptions {
    private long maxAutoConnectRetryTime = 15000;
    private int connectTimeout = 10000;
    private int socketTimeout = 0;
    private boolean socketKeepAlive = false;
    private boolean useNagle = false;
    private boolean useSSL = false;

    /**
     * @return the socket timeout(milliseconds)(int)
     * @fn int getSocketTimeout()
     * @brief Get the socket timeout(milliseconds)
     */
    public int getSocketTimeout() {
        return socketTimeout;
    }

    /**
     * @param socketTimeout(int)
     * @fn void setSocketTimeout(int socketTimeout)
     * @brief Set the socket timeout(milliseconds)
     */
    public void setSocketTimeout(int socketTimeout) {
        this.socketTimeout = socketTimeout;
    }

    /**
     * @return the status(boolean)
     * @fn boolean getSocketKeepAlive()
     * @brief Get whether the socket keeps alive or not
     */
    public boolean getSocketKeepAlive() {
        return socketKeepAlive;
    }

    /**
     * @param socketKeepAlive the the status of socket(boolean)
     * @fn void setSocketKeepAlive(boolean socketKeepAlive)
     * @brief Set the status of socket
     */
    public void setSocketKeepAlive(boolean socketKeepAlive) {
        this.socketKeepAlive = socketKeepAlive;
    }

    /**
     * @return boolean
     * @fn boolean getUseNagle()
     * @brief Get whether use the Nagle Algorithm or not
     */
    public boolean getUseNagle() {
        return useNagle;
    }

    /**
     * @param useNagle(boolean)
     * @fn void setUseNagle(boolean useNagle)
     * @brief Set whether use the Nagle Algorithm or not
     */
    public void setUseNagle(boolean useNagle) {
        this.useNagle = useNagle;
    }

    /**
     * @return the connect timeout(int)
     * @fn int getConnectTimeout()
     * @brief Get the connect timeout(milliseconds)
     */
    public int getConnectTimeout() {
        return connectTimeout;
    }

    /**
     * @param connectTimeout(int)
     * @fn void setConnectTimeout(int connectTimeout)
     * @brief Set the connect timeout(milliseconds)
     */
    public void setConnectTimeout(int connectTimeout) {
        this.connectTimeout = connectTimeout;
    }

    /**
     * @return the max auto connect retry time(long)
     * @fn long getMaxAutoConnectRetryTime()
     * @brief Get the max auto connect retry time(milliseconds)
     */
    public long getMaxAutoConnectRetryTime() {
        return maxAutoConnectRetryTime;
    }

    /**
     * @param maxAutoConnectRetryTime(long)
     * @fn void setMaxAutoConnectRetryTime(long maxAutoConnectRetryTime)
     * @brief Set the max auto connect retry time(milliseconds)
     */
    public void setMaxAutoConnectRetryTime(long maxAutoConnectRetryTime) {
        this.maxAutoConnectRetryTime = maxAutoConnectRetryTime;
    }

    /**
     * @return boolean
     * @fn boolean getUseSSL()
     * @brief Get whether use the SSL or not
     * @since 1.12
     */
    public boolean getUseSSL() {
        return useSSL;
    }

    /**
     * @param useSSL(boolean)
     * @fn void setUseSSL(boolean useSSL)
     * @brief Set whether use the SSL or not
     * @since 1.12
     */
    public void setUseSSL(boolean useSSL) {
        this.useSSL = useSSL;
    }
}
