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

import com.sequoiadb.base.SequoiadbConstants.PreferInstanceType;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.message.MsgOpCode;
import com.sequoiadb.message.request.*;
import com.sequoiadb.message.response.*;
import com.sequoiadb.net.ConfigOptions;
import com.sequoiadb.net.ConnectionTCPImpl;
import com.sequoiadb.net.IConnection;
import com.sequoiadb.net.ServerAddress;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.bson.types.Code;
import org.bson.util.JSON;

import java.io.Closeable;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.*;
import java.util.concurrent.atomic.AtomicLong;

/**
 * @class Sequoiadb
 * @brief Database operation interfaces of admin.
 */
public class Sequoiadb implements Closeable {
    private ServerAddress serverAddress;
    private IConnection connection;
    private String userName;
    private String password;
    private ByteOrder byteOrder = ByteOrder.BIG_ENDIAN;
    private long requestId;
    private long lastUseTime;

    // cache cs/cl name
    private Map<String, Long> nameCache = new HashMap<>();
    private static boolean enableCache = true;
    private static long cacheInterval = 300 * 1000;

    public final static int SDB_PAGESIZE_4K = 4096;
    public final static int SDB_PAGESIZE_8K = 8192;
    public final static int SDB_PAGESIZE_16K = 16384;
    public final static int SDB_PAGESIZE_32K = 32768;
    public final static int SDB_PAGESIZE_64K = 65536;
    /** 0 means using database's default pagesize, it 64k now */
    public final static int SDB_PAGESIZE_DEFAULT = 0;

    public final static int SDB_LIST_CONTEXTS = 0;
    public final static int SDB_LIST_CONTEXTS_CURRENT = 1;
    public final static int SDB_LIST_SESSIONS = 2;
    public final static int SDB_LIST_SESSIONS_CURRENT = 3;
    public final static int SDB_LIST_COLLECTIONS = 4;
    public final static int SDB_LIST_COLLECTIONSPACES = 5;
    public final static int SDB_LIST_STORAGEUNITS = 6;
    public final static int SDB_LIST_GROUPS = 7;
    public final static int SDB_LIST_STOREPROCEDURES = 8;
    public final static int SDB_LIST_DOMAINS = 9;
    public final static int SDB_LIST_TASKS = 10;
    public final static int SDB_LIST_CS_IN_DOMAIN = 11;
    public final static int SDB_LIST_CL_IN_DOMAIN = 12;

    public final static int SDB_SNAP_CONTEXTS = 0;
    public final static int SDB_SNAP_CONTEXTS_CURRENT = 1;
    public final static int SDB_SNAP_SESSIONS = 2;
    public final static int SDB_SNAP_SESSIONS_CURRENT = 3;
    public final static int SDB_SNAP_COLLECTIONS = 4;
    public final static int SDB_SNAP_COLLECTIONSPACES = 5;
    public final static int SDB_SNAP_DATABASE = 6;
    public final static int SDB_SNAP_SYSTEM = 7;
    public final static int SDB_SNAP_CATALOG = 8;

    public final static int FMP_FUNC_TYPE_INVALID = -1;
    public final static int FMP_FUNC_TYPE_JS = 0;
    public final static int FMP_FUNC_TYPE_C = 1;
    public final static int FMP_FUNC_TYPE_JAVA = 2;

    public final static String CATALOG_GROUP_NAME = "SYSCatalogGroup";

    void upsertCache(String name) {
        if (name == null)
            return;
        if (enableCache) {
            long current = System.currentTimeMillis();
            nameCache.put(name, current);
            String[] arr = name.split("\\.");
            if (arr.length > 1) {
                // extract cs name from cl full name and that
                // upsert cs name
                nameCache.put(arr[0], current);
            }
        }
    }

    void removeCache(String name) {
        if (name == null)
            return;
        String[] arr = name.split("\\.");
        if (arr.length == 1) {
            // when we come here, "name" is a cs name, so
            // we are going to remove the cache of the cs
            // and the cache of the cls

            // remove cs cache
            // name may be "foo.", it's a invalid name,
            // we don't want to remove anything,
            // so we use "name" but not "arr[0]" here
            nameCache.remove(name);
            Set<String> keySet = nameCache.keySet();
            List<String> list = new ArrayList<String>();
            for (String str : keySet) {
                String[] nameArr = str.split("\\.");
                if (nameArr.length > 1 && nameArr[0].equals(name))
                    list.add(str);
            }
            if (list.size() != 0) {
                for (String str : list)
                    nameCache.remove(str);
            }
        } else {
            // we are going to remove the cache of the cl
            nameCache.remove(name);
        }
    }

    boolean fetchCache(String name) {
        if (enableCache) {
            if (nameCache.containsKey(name)) {
                long lastUpdatedTime = nameCache.get(name);
                if ((System.currentTimeMillis() - lastUpdatedTime) > cacheInterval) {
                    nameCache.remove(name);
                    return false;
                } else {
                    return true;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /**
     * @fn initClient(ClientOptions options)
     * @brief Initialize the configuration options for client.
     * @param options the configuration options for client
     * @return void
     */
    public static void initClient(ClientOptions options) {
        enableCache = (options != null) ? options.getEnableCache() : true;
        cacheInterval = (options != null && options.getCacheInterval() >= 0) ? options.getCacheInterval() : 300 * 1000;
    }

    /**
     * @fn ServerAddress getServerAddress()
     * @brief Get the address of remote server.
     * @return ServerAddress
     */
    public ServerAddress getServerAddress() {
        return serverAddress;
    }

    /**
     * @fn void setServerAddress(ServerAddress serverAddress)
     * @brief Set the address of remote server.
     * @param serverAddress
     *            the serverAddress object of remote server
     */
    public void setServerAddress(ServerAddress serverAddress) {
        this.serverAddress = serverAddress;
    }

    /**
     * @fn boolean isEndianConvert()
     * @brief Judge the endian of the physical computer
     * @return Big-Endian for true while Little-Endian for false
     */
    public boolean isEndianConvert() {
        return byteOrder == ByteOrder.BIG_ENDIAN;
    }

    public long getLastUseTime() {
        return lastUseTime;
    }

    /**
     * @fn Sequoiadb(String username, String password)
     * @brief Constructor. The server address is "127.0.0.1 : 11810".
     * @param username the user's name of the account
     * @param password the password of the account
     * @exception com.sequoiadb.exception.BaseException
     *            "SDB_NETWORK" means network error,
     *            "SDB_INVALIDARG" means wrong address or the address don't map to the hosts table
     */
    @Deprecated
    public Sequoiadb(String username, String password) throws BaseException {
        // connect used default address
        serverAddress = new ServerAddress();
        ConfigOptions opts = new ConfigOptions();
        initConnection(opts);
        // authentication
        this.userName = username;
        this.password = password;
        auth();
    }

    /**
     * @fn Sequoiadb(String connString, String username, String password)
     * @brief Constructor.
     * @param connString
     *            remote server address "IP : Port" or "IP"(port is 50000)
     * @param username the user's name of the account
     * @param password the password of the account
     * @exception com.sequoiadb.exception.BaseException
     *            "SDB_NETWORK" means network error,
     *            "SDB_INVALIDARG" means wrong address or the address don't map to the hosts table
     */
    public Sequoiadb(String connString, String username, String password)
        throws BaseException {
        this(connString, username, password, null);
    }

    /**
     * @fn Sequoiadb(String connString, String username,
     *String password, ConfigOptions options)
     * @brief Constructor.
     * @param connString
     *            remote server address "IP : Port" or "IP"(port is 11810)
     * @param username the user's name of the account
     * @param password the password of the account
     * @param options the options for connection
     * @exception com.sequoiadb.exception.BaseException
     *            "SDB_NETWORK" means network error,
     *            "SDB_INVALIDARG" means wrong address or the address don't map to the hosts table
     */
    public Sequoiadb(String connString, String username, String password,
                     ConfigOptions options) throws BaseException {
        ConfigOptions opts = options;
        if (null == options)
            opts = new ConfigOptions();
        try {
            // connect
            serverAddress = new ServerAddress(connString);
            initConnection(opts);
        } catch (UnknownHostException e) {
            throw new BaseException(SDBError.SDB_NETWORK, connString, e);
        }
        // authentication
        this.userName = username;
        this.password = password;
        auth();
    }

    /**
     * @fn Sequoiadb(List<String> connStrings, String username, String password,
     *ConfigOptions options)
     * @brief Constructor, use a random valid address to connect to database.
     * @param connStrings The array of the coord's address
     * @param username the user's name of the account
     * @param password the password  of the account
     * @param options the options for connection
     * @exception com.sequoiadb.exception.BaseException
     *            "SDB_NETWORK" means network error,
     *            "SDB_INVALIDARG" means wrong address or the address don't map to the hosts table in local computer
     */
    public Sequoiadb(List<String> connStrings, String username, String password,
                     ConfigOptions options) throws BaseException {
        ConfigOptions opts = options;
        if (options == null) {
            opts = new ConfigOptions();
        }

        Iterator<String> tmpIter = connStrings.iterator();
        while (tmpIter.hasNext()) {
            String tmpStr = tmpIter.next();
            if (null == tmpStr) {
                tmpIter.remove();
            }
        }

        int size = connStrings.size();
        if (0 == size) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "Address list is empty");
        }
        Random random = new Random();
        int count = random.nextInt(size);
        int mark = count;
        do {
            count = ++count % size;
            String str = connStrings.get(count);
            try {
                // connect
                try {
                    serverAddress = new ServerAddress(str);
                    initConnection(opts);
                } catch (UnknownHostException e) {
                    throw new BaseException(SDBError.SDB_NETWORK, str);
                }
                // authentication
                this.userName = username;
                this.password = password;
                auth();
            } catch (BaseException e) {
                if (mark == count) {
                    throw new BaseException(SDBError.SDB_NET_CANNOT_CONNECT);
                }
                continue;
            }
            break;
        } while (mark != count);
    }

    /**
     * @fn Sequoiadb(String host, int port, String username, String password)
     * @brief Constructor.
     * @param host the address of coord
     * @param port the port of coord
     * @param username the user's name of the account
     * @param password the password  of the account
     * @exception com.sequoiadb.exception.BaseException
     *            "SDB_NETWORK" means network error,
     *            "SDB_INVALIDARG" means wrong address or the address don't map to the hosts table
     */
    public Sequoiadb(String host, int port, String username, String password)
        throws BaseException {
        this(host, port, username, password, null);
    }

    /**
     * @fn Sequoiadb(String host, int port, String username,
     *String password, ConfigOptions options)
     * @brief Constructor.
     * @param host the address of coord
     * @param port the port of coord
     * @param username the user's name of the account
     * @param password the password of the account
     * @exception com.sequoiadb.exception.BaseException
     *            "SDB_NETWORK" means network error,
     *            "SDB_INVALIDARG" means wrong address or the address don't map to the hosts table
     */
    public Sequoiadb(String host, int port,
                     String username, String password,
                     ConfigOptions options) throws BaseException {
        if (options == null) {
            options = new ConfigOptions();
        }
        try {
            serverAddress = new ServerAddress(host, port);
            initConnection(options);
        } catch (UnknownHostException e) {
            throw new BaseException(SDBError.SDB_NETWORK, host + ":" + port, e);
        }

        this.userName = username;
        this.password = password;
        auth();
    }

    /**
     * @fn auth()
     * @brief authentication
     */
    private void auth() {
        byteOrder = getSysInfo();

        AuthRequest request = new AuthRequest(userName, password, AuthRequest.AuthType.Verify);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            connection.close();
            throw new BaseException(SDBError.getSDBError(flag), "failed to auth, user is" + userName);
        }
    }

    /**
     * @fn void createUser(String username, String password)
     * @brief Add an user in current database.
     * @param username
     *            The connection user name
     * @param password
     *            The connection password
     */
    public void createUser(String username, String password) throws BaseException {
        if (username == null || username.length() == 0 || password == null) {
            throw new BaseException(SDBError.SDB_INVALIDARG);
        }

        AuthRequest request = new AuthRequest(username, password, AuthRequest.AuthType.CreateUser);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(SDBError.getSDBError(flag), "failed to create user " + username);
        }
    }

    /**
     * @fn void removeUser(String username, String password)
     * @brief Remove the spacified user from current database.
     * @param username
     *            The connection user name
     * @param password
     *            The connection password
     */
    public void removeUser(String username, String password) throws BaseException {
        AuthRequest request = new AuthRequest(username, password, AuthRequest.AuthType.DeleteUser);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(SDBError.getSDBError(flag), "failed to remove user " + username);
        }
    }

    /**
     * @fn void disconnect()
     * @brief Disconnect from the remote server.
     * @return void
     * @exception com.sequoiadb.exception.BaseException
     */
    public void disconnect() throws BaseException {
        if (connection == null || connection.isClosed()) {
            return;
        }
        try {
            releaseResource();
            DisconnectRequest request = new DisconnectRequest();
            sendRequest(request);
        } finally {
            connection.close();
        }
    }

    /**
     * @fn void releaseResource()
     * @brief Release the resource of the connection.
     * @return void
     * @exception com.sequoiadb.exception.BaseException
     * @since v1.2.6 && v2.2
     */
    public void releaseResource() {
        // let the receive buffer shrink to default value
        closeAllCursors();
    }

    /**
     * @fn boolean isClosed()
     * @brief Whether the socket has been closed or not.
     * @return return true when the socket has been
     * @since v1.2.6 && v2.2
     */
    public boolean isClosed() {
        if (connection == null)
            return true;
        return connection.isClosed();
    }

    /**
     * @fn boolean isValid()
     * @brief Send a test message to database to test whether the connection is valid or not.
     * @return if the connection is valid, return true
     * @exception com.sequoiadb.exception.BaseException
     */
    public boolean isValid() throws BaseException {
        // client not connect to database or client
        // disconnect from database
        if (connection == null || connection.isClosed())
            return false;
        try {
            killContext();
        } catch (BaseException e) {
            return false;
        }
        return true;
    }

    /**
     * @fn void changeConnectionOptions(ConfigOptions opts)
     * @brief Change the connection options.
     * @param opts
     *            The connection options
     * @exception com.sequoiadb.exception.BaseException
     */
    public void changeConnectionOptions(ConfigOptions opts)
        throws BaseException {
        connection.changeConnectionOptions(opts);
        auth();
    }

    /**
     * @fn void createCollectionSpace(String collectionSpaceName)
     * @brief Create the named collection space with default SDB_PAGESIZE_4K.
     * @param csName
     *            The collection space name
     * @exception com.sequoiadb.exception.BaseException
     */
    public CollectionSpace createCollectionSpace(String csName)
        throws BaseException {
        return createCollectionSpace(csName, SDB_PAGESIZE_DEFAULT);
    }

    /**
     * @fn CollectionSpace createCollectionSpace(String collectionSpaceName, int pageSize)
     * @brief Create collection space.
     * @param csName The name of collection space
     * @param pageSize The Page Size as below:
     * <ul>
     * <li> SDB_PAGESIZE_4K
     * <li> SDB_PAGESIZE_8K
     * <li> SDB_PAGESIZE_16K
     * <li> SDB_PAGESIZE_32K
     * <li> SDB_PAGESIZE_64K
     * <li> SDB_PAGESIZE_DEFAULT
     * </ul>
     * @return the newly created collection space object
     * @exception com.sequoiadb.exception.BaseException
     */
    public CollectionSpace createCollectionSpace(String csName, int pageSize)
        throws BaseException {
        BSONObject options = new BasicBSONObject();
        options.put("PageSize", pageSize);
        return createCollectionSpace(csName, options);
    }

    /**
     * @fn CollectionSpace createCollectionSpace(String csName, BSONObject options)
     * @brief Create collection space.
     * @param csName The name of collection space
     * @param options Contains configuration informations for create collection space. The options are as below:
     * <ul>
     * <li>PageSize    : Assign how large the page size is for the collection created in this collection space, default to be 64K
     * <li>Domain    : Assign which domain does current collection space belong to, it will belongs to the system domain if not assign this option
     * </ul>
     * @return the newly created collection space object
     * @exception com.sequoiadb.exception.BaseException
     */
    public CollectionSpace createCollectionSpace(String csName, BSONObject options)
        throws BaseException {
        if (csName == null || csName.length() == 0) {
            throw new BaseException(SDBError.SDB_INVALIDARG, csName);
        }
        if (isCollectionSpaceExist(csName)) {
            throw new BaseException(SDBError.SDB_DMS_CS_EXIST, csName);
        }

        BSONObject obj = new BasicBSONObject();
        obj.put(SequoiadbConstants.FIELD_NAME_NAME, csName);
        if (null != options) {
            obj.putAll(options);
        }

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_CS, obj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0)
            throw new BaseException(flag);
        upsertCache(csName);
        return new CollectionSpace(this, csName);
    }

    /**
     * @fn void dropCollectionSpace(String collectionSpaceName)
     * @brief Remove the named collection space.
     * @param csName
     *            The collection space name
     * @exception com.sequoiadb.exception.BaseException
     */
    public void dropCollectionSpace(String csName) throws BaseException {
        if (!isCollectionSpaceExist(csName)) {
            throw new BaseException(SDBError.SDB_DMS_CS_NOTEXIST, csName);
        }

        BSONObject options = new BasicBSONObject();
        options.put(SequoiadbConstants.FIELD_NAME_NAME, csName);

        AdminRequest request = new AdminRequest(AdminCommand.DROP_CS, options);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }

        removeCache(csName);
    }

    /*
     * @param csName The collection space name
     * @param options The control options:(Only take effect in coordinate nodes, can be null)
     *                <ul>
     *                <li>GroupID:int</li>
     *                <li>GroupName:String</li>
     *                <li>NodeID:int</li>
     *                <li>HostName:String</li>
     *                <li>svcname:String</li>
     *                <li>...</li>
     *                </ul>
     * @throws BaseException SDB_INVALIDARG, SDB_DMS_CS_NOTEXIST...
     * @since 2.8
     */
    public void loadCollectionSpace(String csName, BSONObject options) throws BaseException {
        if (csName == null || csName.length() == 0) {
            throw new BaseException(SDBError.SDB_INVALIDARG, csName);
        }
        if (isCollectionSpaceExist(csName)) {
            throw new BaseException(SDBError.SDB_DMS_CS_EXIST, csName);
        }

        BSONObject newOptions = new BasicBSONObject();
        newOptions.put(SequoiadbConstants.FIELD_NAME_NAME, csName);
        if (options != null) {
            newOptions.putAll(options);
        }

        AdminRequest request = new AdminRequest(AdminCommand.LOAD_CS, newOptions);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }

        upsertCache(csName);
    }

    /*
     * @param csName The collection space name
     * @param options The control options:(Only take effect in coordinate nodes, can be null)
     *                <ul>
     *                <li>GroupID:int</li>
     *                <li>GroupName:String</li>
     *                <li>NodeID:int</li>
     *                <li>HostName:String</li>
     *                <li>svcname:String</li>
     *                <li>...</li>
     *                </ul>
     * @throws BaseException SDB_INVALIDARG, SDB_DMS_CS_NOTEXIST...
     * @since 2.8
     */
    public void unloadCollectionSpace(String csName, BSONObject options) throws BaseException {
        if (csName == null || csName.length() == 0) {
            throw new BaseException(SDBError.SDB_INVALIDARG, csName);
        }
        if (!isCollectionSpaceExist(csName)) {
            throw new BaseException(SDBError.SDB_DMS_CS_NOTEXIST, csName);
        }

        BSONObject newOptions = new BasicBSONObject();
        newOptions.put(SequoiadbConstants.FIELD_NAME_NAME, csName);
        if (options != null) {
            newOptions.putAll(options);
        }

        AdminRequest request = new AdminRequest(AdminCommand.UNLOAD_CS, newOptions);
        SdbReply response = requestAndResponse(request);
        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }

        removeCache(csName);
    }

    /*
     * @param oldName The old collection space name
     * @param newName The new collection space name
     * @throws BaseException SDB_INVALIDARG, SDB_DMS_CS_NOTEXIST...
     * @since 2.8
     */
    public void renameCollectionSpace(String oldName, String newName) throws BaseException {
        if (oldName == null || oldName.length() == 0) {
            throw new BaseException(SDBError.SDB_INVALIDARG, oldName);
        }
        if (newName == null || newName.length() == 0) {
            throw new BaseException(SDBError.SDB_INVALIDARG, newName);
        }
        if (!isCollectionSpaceExist(oldName)) {
            throw new BaseException(SDBError.SDB_DMS_CS_NOTEXIST, oldName);
        }

        BSONObject matcher = new BasicBSONObject();
        matcher.put(SequoiadbConstants.FIELD_NAME_OLDNAME, oldName);
        matcher.put(SequoiadbConstants.FIELD_NAME_NEWNAME, newName);

        AdminRequest request = new AdminRequest(AdminCommand.RENAME_CS, matcher);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }

        removeCache(oldName);
        upsertCache(newName);
    }

    /**
     * @fn void sync(BSONObject options)
     * @brief sync the database
     * @param options The control options:(can be null)
     *                <ul>
     *                <li>
     *                    Deep:int
    Flush with deep mode or not. 1 in default.
    0 for non-deep mode,1 for deep mode,-1 means use the configuration with server
     *                </li>
     *                <li>
     *                    Block:boolean
    Flush with block mode or not. false in default.
     *                </li>
     *                <li>
     *                    CollectionSpace:String
    Specify the collectionspace to sync.
    If not set, will sync all the collection spaces and logs,
    otherwise, will only sync the collection space specified.
     *                </li>
     *                <li>
     *                Others:(Only take effect in coordinate nodes)
    GroupID:int,
    GroupName:String,
    NodeID:int,
    HostName:String,
    svcname:String
    ...
     *                </li>
     *                </ul>
     * @throws BaseException
     * @since 2.8
     */
    public void sync(BSONObject options) throws BaseException {
        AdminRequest request = new AdminRequest(AdminCommand.SYNC_DB, options);
        SdbReply response = requestAndResponse(request);
        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void sync()
     * @brief sync the whole database
     * @throws BaseException
     * @since 2.8
     */
    public void sync() throws BaseException {
        sync(null);
    }

    /**
     * @fn CollectionSpace getCollectionSpace(String csName)
     * @brief Get the named collection space.
     * @param csName
     *            The collection space name.
     * @return The collection space object.
     * @note If the collection space not exit, throw BaseException "SDB_DMS_CS_NOTEXIST".
     * @exception com.sequoiadb.exception.BaseException
     */
    public CollectionSpace getCollectionSpace(String csName)
        throws BaseException {
        // get cs object from cache
        if (fetchCache(csName)) {
            return new CollectionSpace(this, csName);
        }
        // get cs object from database
        // we don't need to update or remove cache here,
        // for "isCollectionSpaceExist" has do that
        if (isCollectionSpaceExist(csName)) {
            return new CollectionSpace(this, csName);
        } else {
            throw new BaseException(SDBError.SDB_DMS_CS_NOTEXIST, csName);
        }
    }

    /**
     * @fn boolean isCollectionSpaceExist(String csName)
     * @brief Verify the existence of collection space.
     * @param csName
     *            The collecion space name
     * @return True if existed or False if not existed
     * @exception com.sequoiadb.exception.BaseException
     */
    public boolean isCollectionSpaceExist(String csName) throws BaseException {
        BSONObject options = new BasicBSONObject();
        options.put(SequoiadbConstants.FIELD_NAME_NAME, csName);

        AdminRequest request = new AdminRequest(AdminCommand.TEST_CS, options);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag == 0) {
            upsertCache(csName);
            return true;
        } else if (flag == SDBError.SDB_DMS_CS_NOTEXIST.getErrorCode()) {
            removeCache(csName);
            return false;
        } else {
            throw new BaseException(flag, csName);
        }
    }

    /**
     * @fn DBCursor listCollectionSpaces()
     * @brief Get all the collecionspaces.
     * @return cursor of all collecionspace names
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor listCollectionSpaces() throws BaseException {
        return getList(SDB_LIST_COLLECTIONSPACES, null, null, null);
    }

    /**
     * @fn ArrayList<String> getCollectionSpaceNames()
     * @brief Get all the collecion space names
     * @return A list of all collecion space names
     * @exception com.sequoiadb.exception.BaseException
     */
    public ArrayList<String> getCollectionSpaceNames() throws BaseException {
        DBCursor cursor = getList(SDB_LIST_COLLECTIONSPACES, null, null, null);
        if (cursor == null) {
            return null;
        }
        ArrayList<String> colList = new ArrayList<String>();
        while (cursor.hasNext()) {
            colList.add(cursor.getNext().get("Name").toString());
        }
        return colList;
    }

    /**
     * @fn DBCursor listCollections()
     * @brief Get all the collections
     * @return dbCursor of all collecions
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor listCollections() throws BaseException {
        return getList(SDB_LIST_COLLECTIONS, null, null, null);
    }

    /**
     * @fn ArrayList<String> getCollectionNames()
     * @brief Get all the collection names
     * @return A list of all collecion names
     * @exception com.sequoiadb.exception.BaseException
     */
    public ArrayList<String> getCollectionNames() throws BaseException {
        DBCursor cursor = getList(SDB_LIST_COLLECTIONS, null, null, null);
        if (cursor == null) {
            return null;
        }
        ArrayList<String> colList = new ArrayList<String>();
        while (cursor.hasNext()) {
            colList.add(cursor.getNext().get("Name").toString());
        }
        return colList;
    }

    /**
     * @fn List<BSONObject> getStorageUnits()
     * @brief Get all the storage units
     * @return A list of all storage units
     * @exception com.sequoiadb.exception.BaseException
     */
    public ArrayList<String> getStorageUnits() throws BaseException {
        DBCursor cursor = getList(SDB_LIST_STORAGEUNITS, null, null, null);
        ArrayList<String> colList = new ArrayList<String>();
        while (cursor.hasNext()) {
            colList.add(cursor.getNext().get("Name").toString());
        }
        return colList;
    }

    /**
     * @fn void resetSnapshot()
     * @brief Reset the snapshot.
     * @return void
     * @exception com.sequoiadb.exception.BaseException
     */
    public void resetSnapshot() throws BaseException {
        AdminRequest request = new AdminRequest(AdminCommand.RESET_SNAPSHOT);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn DBCursor getList(int listType, BSONObject query, BSONObject selector,
    BSONObject orderBy)
     * @brief Get the informations of specified type.
     * @param listType The list type as below:
     *<dl>
     *<dt>Sequoiadb.SDB_LIST_CONTEXTS   : Get all contexts list
     *<dt>Sequoiadb.SDB_LIST_CONTEXTS_CURRENT        : Get contexts list for the current session
     *<dt>Sequoiadb.SDB_LIST_SESSIONS        : Get all sessions list
     *<dt>Sequoiadb.SDB_LIST_SESSIONS_CURRENT        : Get the current session
     *<dt>Sequoiadb.SDB_LIST_COLLECTIONS        : Get all collections list
     *<dt>Sequoiadb.SDB_LIST_COLLECTIONSPACES        : Get all collection spaces list
     *<dt>Sequoiadb.SDB_LIST_STORAGEUNITS        : Get storage units list
     *<dt>Sequoiadb.SDB_LIST_GROUPS        : Get replica group list ( only applicable in sharding env )
     *<dt>Sequoiadb.SDB_LIST_STOREPROCEDURES           : Get stored procedure list ( only applicable in sharding env )
     *<dt>Sequoiadb.SDB_LIST_DOMAINS        : Get all the domains list ( only applicable in sharding env )
     *<dt>Sequoiadb.SDB_LIST_TASKS        : Get all the running split tasks ( only applicable in sharding env )
     *</dl>
     * @param query The matching rule, match all the documents if null.
     * @param selector The selective rule, return the whole document if null.
     * @param orderBy The ordered rule, never sort if null.
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor getList(int listType, BSONObject query, BSONObject selector, BSONObject orderBy) throws BaseException {
        String command = getListCommand(listType);
        AdminRequest request = new AdminRequest(command, query, selector, orderBy, null);
        SdbReply response = requestAndResponse(request);

        int flags = response.getFlag();
        if (flags != 0) {
            if (flags == SDBError.SDB_DMS_EOC.getErrorCode()) {
                return null;
            } else {
                String msg = "query = " + query +
                    ", selector = " + selector +
                    ", orderBy = " + orderBy;
                throw new BaseException(SDBError.getSDBError(flags), msg);
            }
        }

        return new DBCursor(response, this);
    }

    /**
     * @fn void flushConfigure(BSONObject param)
     * @brief Flush the options to configuration file
     * @param options
     *            The param of flush, pass {"Global":true} or {"Global":false}
     *            In cluster environment, passing {"Global":true} will flush data's and catalog's configuration file,
     *            while passing {"Global":false} will flush coord's configuration file
     *            In stand-alone environment, both them have the same behaviour
     * @exception com.sequoiadb.exception.BaseException
     */
    public void flushConfigure(BSONObject options) throws BaseException {
        AdminRequest request = new AdminRequest(AdminCommand.EXPORT_CONFIG, options);
        SdbReply response = requestAndResponse(request);
        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void execUpdate(String sql)
     * @brief Execute sql in database.
     * @param sql the SQL command.
     * @exception com.sequoiadb.exception.BaseException
     */
    public void execUpdate(String sql) throws BaseException {
        SQLRequest request = new SQLRequest(sql);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag, sql);
        }
    }

    /**
     * @fn DBCursor exec(String sql)
     * @brief Execute sql in database.
     * @param sql the SQL command
     * @return the DBCursor of the result
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor exec(String sql) throws BaseException {
        SQLRequest request = new SQLRequest(sql);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            if (flag == SDBError.SDB_DMS_EOC.getErrorCode())
                return null;
            else {
                throw new BaseException(flag, sql);
            }
        }

        return new DBCursor(response, this);
    }

    /**
     * @fn DBCursor getSnapshot(int snapType, String matcher, String selector,
     *     String orderBy)
     * @brief Get snapshot of the database.
     * @param snapType The snapshot types are as below:
     * <dl>
     * <dt>Sequoiadb.SDB_SNAP_CONTEXTS   : Get all contexts' snapshot
     * <dt>Sequoiadb.SDB_SNAP_CONTEXTS_CURRENT        : Get the current context's snapshot
     * <dt>Sequoiadb.SDB_SNAP_SESSIONS        : Get all sessions' snapshot
     * <dt>Sequoiadb.SDB_SNAP_SESSIONS_CURRENT        : Get the current session's snapshot
     * <dt>Sequoiadb.SDB_SNAP_COLLECTIONS        : Get the collections' snapshot
     * <dt>Sequoiadb.SDB_SNAP_COLLECTIONSPACES        : Get the collection spaces' snapshot
     * <dt>Sequoiadb.SDB_SNAP_DATABASE        : Get database's snapshot
     * <dt>Sequoiadb.SDB_SNAP_SYSTEM        : Get system's snapshot
     * <dt>Sequoiadb.SDB_SNAP_CATALOG        : Get catalog's snapshot
     * <dt>Sequoiadb.SDB_LIST_GROUPS        : Get replica group list ( only applicable in sharding env )
     * <dt>Sequoiadb.SDB_LIST_STOREPROCEDURES           : Get stored procedure list ( only applicable in sharding env )
     * </dl>
     * @param matcher
     *            the matching rule, match all the documents if null
     * @param selector
     *            the selective rule, return the whole document if null
     * @param orderBy
     *            the ordered rule, never sort if null
     * @return the DBCursor instance of the result
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor getSnapshot(int snapType, String matcher, String selector,
                                String orderBy) throws BaseException {
        BSONObject ma = null;
        BSONObject se = null;
        BSONObject or = null;
        if (matcher != null) {
            ma = (BSONObject) JSON.parse(matcher);
        }
        if (selector != null) {
            se = (BSONObject) JSON.parse(selector);
        }
        if (orderBy != null) {
            or = (BSONObject) JSON.parse(orderBy);
        }

        return getSnapshot(snapType, ma, se, or);
    }

    /**
     * @fn DBCursor getSnapshot(int snapType, BSONObject matcher, BSONObject
     *     selector, BSONObject orderBy)
     * @brief Get snapshot of the database.
     * @param snapType The snapshot types are as below:
     * <dl>
     * <dt>Sequoiadb.SDB_SNAP_CONTEXTS   : Get all contexts' snapshot
     * <dt>Sequoiadb.SDB_SNAP_CONTEXTS_CURRENT        : Get the current context's snapshot
     * <dt>Sequoiadb.SDB_SNAP_SESSIONS        : Get all sessions' snapshot
     * <dt>Sequoiadb.SDB_SNAP_SESSIONS_CURRENT        : Get the current session's snapshot
     * <dt>Sequoiadb.SDB_SNAP_COLLECTIONS        : Get the collections' snapshot
     * <dt>Sequoiadb.SDB_SNAP_COLLECTIONSPACES        : Get the collection spaces' snapshot
     * <dt>Sequoiadb.SDB_SNAP_DATABASE        : Get database's snapshot
     * <dt>Sequoiadb.SDB_SNAP_SYSTEM        : Get system's snapshot
     * <dt>Sequoiadb.SDB_SNAP_CATALOG        : Get catalog's snapshot
     * <dt>Sequoiadb.SDB_LIST_GROUPS        : Get replica group list ( only applicable in sharding env )
     * <dt>Sequoiadb.SDB_LIST_STOREPROCEDURES           : Get stored procedure list ( only applicable in sharding env )
     * </dl>
     * @param matcher
     *            the matching rule, match all the documents if null
     * @param selector
     *            the selective rule, return the whole document if null
     * @param orderBy
     *            the ordered rule, never sort if null
     * @return the DBCursor instance of the result
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor getSnapshot(int snapType, BSONObject matcher,
                                BSONObject selector, BSONObject orderBy) throws BaseException {
        String command = getSnapshotCommand(snapType);

        AdminRequest request = new AdminRequest(command, matcher, selector, orderBy, null);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            if (flag == SDBError.SDB_DMS_EOC.getErrorCode()) {
                return null;
            } else {
                String msg = "matcher = " + matcher +
                    ", selector = " + selector +
                    ", orderBy = " + orderBy;
                throw new BaseException(SDBError.getSDBError(flag), msg);
            }
        }

        return new DBCursor(response, this);
    }

    private String getSnapshotCommand(int snapType) {
        switch (snapType) {
            case SDB_SNAP_CONTEXTS:
                return AdminCommand.SNAP_CONTEXTS;
            case SDB_SNAP_CONTEXTS_CURRENT:
                return AdminCommand.SNAP_CONTEXTS_CURRENT;
            case SDB_SNAP_SESSIONS:
                return AdminCommand.SNAP_SESSIONS;
            case SDB_SNAP_SESSIONS_CURRENT:
                return AdminCommand.SNAP_SESSIONS_CURRENT;
            case SDB_SNAP_COLLECTIONS:
                return AdminCommand.SNAP_COLLECTIONS;
            case SDB_SNAP_COLLECTIONSPACES:
                return AdminCommand.SNAP_COLLECTIONSPACES;
            case SDB_SNAP_DATABASE:
                return AdminCommand.SNAP_DATABASE;
            case SDB_SNAP_SYSTEM:
                return AdminCommand.SNAP_SYSTEM;
            case SDB_SNAP_CATALOG:
                return AdminCommand.SNAP_CATALOG;
            default:
                throw new BaseException(SDBError.SDB_INVALIDARG, String.format("Invalid snapshot type: %d", snapType));
        }
    }

    /**
     * @fn void beginTransaction()
     * @brief Begin the transaction.
     * @return void
     * @exception com.sequoiadb.exception.BaseException
     */
    public void beginTransaction() throws BaseException {
        TransactionRequest request = new TransactionRequest(TransactionRequest.TransactionType.Begin);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void commit()
     * @brief Commit the transaction.
     * @return void
     * @exception com.sequoiadb.exception.BaseException
     */
    public void commit() throws BaseException {
        TransactionRequest request = new TransactionRequest(TransactionRequest.TransactionType.Commit);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void rollback()
     * @brief Rollback the transaction.
     * @return void
     * @exception com.sequoiadb.exception.BaseException
     */
    public void rollback() throws BaseException {
        TransactionRequest request = new TransactionRequest(TransactionRequest.TransactionType.Rollback);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void crtJSProcedure ( String code )
     * @brief Create a store procedure.
     * @param code The code of store procedure
     * @exception com.sequoiadb.exception.BaseException
     */
    public void crtJSProcedure(String code) throws BaseException {
        if (null == code || code.equals("")) {
            throw new BaseException(SDBError.SDB_INVALIDARG, code);
        }

        BSONObject options = new BasicBSONObject();
        Code codeObj = new Code(code);
        options.put(SequoiadbConstants.FIELD_NAME_FUNC, codeObj);
        options.put(SequoiadbConstants.FMP_FUNC_TYPE, FMP_FUNC_TYPE_JS);

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_PROCEDURE, options);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void rmProcedure ( String name )
     * @brief Remove a store procedure.
     * @param name The name of store procedure to be removed
     * @exception com.sequoiadb.exception.BaseException
     */
    public void rmProcedure(String name) throws BaseException {
        if (null == name || name.equals("")) {
            throw new BaseException(SDBError.SDB_INVALIDARG, name);
        }

        BSONObject options = new BasicBSONObject();
        options.put(SequoiadbConstants.FIELD_NAME_FUNC, name);

        AdminRequest request = new AdminRequest(AdminCommand.REMOVE_PROCEDURE, options);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn DBCursor listProcedures ( BSONObject condition )
     * @brief List the store procedures.
     * @param condition The condition of list eg: {"name":"sum"}. return all if null
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor listProcedures(BSONObject condition) throws BaseException {
        return getList(SDB_LIST_STOREPROCEDURES, condition, null, null);
    }

    /**
     * @fn Sequoiadb.SptEvalResult evalJS ( String code )
     * @brief Eval javascript code.
     * @param code The javasript code
     * @return The result of the eval operation, including the return value type,
     *         the return data and the error message. If succeed to eval, error message is null,
     *         and we can extract the eval result from the return cursor and return type,
     *         if not, the return cursor and the return type are null, we can extract
     *         the error mssage for more detail. 
     * @exception com.sequoiadb.exception.BaseException
     */
    public Sequoiadb.SptEvalResult evalJS(String code) throws BaseException {
        if (code == null || code.equals("")) {
            throw new BaseException(SDBError.SDB_INVALIDARG);
        }

        SptEvalResult evalResult = new Sequoiadb.SptEvalResult();

        BSONObject newObj = new BasicBSONObject();
        Code codeObj = new Code(code);
        newObj.put(SequoiadbConstants.FIELD_NAME_FUNC, codeObj);
        newObj.put(SequoiadbConstants.FMP_FUNC_TYPE, FMP_FUNC_TYPE_JS);

        AdminRequest request = new AdminRequest(AdminCommand.EVAL, newObj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        // if something wrong with the eval operation, not throws exception here
        if (flag != 0) {
            if (response.getReturnedNum() > 0) {
                evalResult.errmsg = response.getResultSet().getNext();
            }
            return evalResult;
        } else {
            // get the return type of eval result
            if (response.getReturnedNum() > 0) {
                BSONObject obj = response.getResultSet().getNext();
                int typeValue = (Integer) obj.get(SequoiadbConstants.FIELD_NAME_RETYE);
                evalResult.returnType = Sequoiadb.SptReturnType.getTypeByValue(typeValue);
            }
            // set the return cursor
            evalResult.cursor = new DBCursor(response, this);
            return evalResult;
        }
    }

    /**
     * @fn void backupOffline ( BSONObject options )
     * @brief Backup the whole database or specifed replica group.
     * @param options Contains a series of backup configuration infomations. 
     *        Backup the whole cluster if null. The "options" contains 5 options as below. 
     *        All the elements in options are optional. 
     *        eg: {"GroupName":["rgName1", "rgName2"], "Path":"/opt/sequoiadb/backup", 
     *             "Name":"backupName", "Description":description, "EnsureInc":true, "OverWrite":true}
     *<ul>
     *<li>GroupID     : The id(s) of replica group(s) which to be backuped
     *<li>GroupName   : The name(s) of replica group(s) which to be backuped
     *<li>Name        : The name for the backup
     *<li>Path        : The backup path, if not assign, use the backup path assigned in the configuration file,
     *                  the path support to use wildcard(%g/%G:group name, %h/%H:host name, %s/%S:service name).
     *                  e.g.  {Path:"/opt/sequoiadb/backup/%g"}
     *<li>isSubDir    : Whether the path specified by paramer "Path" is a subdirectory of
     *                  the path specified in the configuration file, default to be false
     *<li>Prefix      : The prefix of name for the backup, default to be null. e.g. {Prefix:"%g_bk_"}
     *<li>EnableDateDir : Whether turn on the feature which will create subdirectory named to
     *                    current date like "YYYY-MM-DD" automatically, default to be false             
     *<li>Description : The description for the backup
     *<li>EnsureInc   : Whether turn on increment synchronization, default to be false
     *<li>OverWrite   : Whether overwrite the old backup file with the same name, default to be false
     *</ul>
     * @exception com.sequoiadb.exception.BaseException
     */
    public void backupOffline(BSONObject options) throws BaseException {
        AdminRequest request = new AdminRequest(AdminCommand.BACKUP_OFFLINE, options);
        SdbReply response = requestAndResponse(request);

        int flags = response.getFlag();
        if (flags != 0) {
            throw new BaseException(flags);
        }
    }

    /**
     * @fn DBCursor listBackup ( BSONObject options, BSONObject matcher,
    BSONObject selector, BSONObject orderBy )
     * @brief List the backups.
     * @param options Contains configuration infomations for remove backups, list all the backups in the default backup path if null.
     *        The "options" contains 3 options as below. All the elements in options are optional. 
     *        eg: {"GroupName":["rgName1", "rgName2"], "Path":"/opt/sequoiadb/backup", "Name":"backupName"}
     * <ul>
     * <li>GroupName   : Assign the backups of specifed replica groups to be list
     * <li>Path        : Assign the backups in specifed path to be list, if not assign, use the backup path asigned in the configuration file
     * <li>Name        : Assign the backups with specifed name to be list
     * </ul>
     * @param matcher The matching rule, return all the documents if null
     * @param selector The selective rule, return the whole document if null
     * @param orderBy The ordered rule, never sort if null
     * @return the DBCursor of the backup or null while having no backup infonation. 
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor listBackup(BSONObject options, BSONObject matcher,
                               BSONObject selector, BSONObject orderBy) throws BaseException {
        if (null != options) {
            for (String key : options.keySet()) {
                if (key.equals(SequoiadbConstants.FIELD_NAME_GROUPNAME)
                    || key.equals(SequoiadbConstants.FIELD_NAME_NAME)
                    || key.equals(SequoiadbConstants.FIELD_NAME_PATH)) {
                    continue;
                } else {
                    throw new BaseException(SDBError.SDB_INVALIDARG, key);
                }
            }
        }

        AdminRequest request = new AdminRequest(AdminCommand.LIST_BACKUP, matcher, selector, orderBy, options);
        SdbReply response = requestAndResponse(request);

        int flags = response.getFlag();
        if (flags != 0) {
            if (flags == SDBError.SDB_DMS_EOC.getErrorCode()) {
                return null;
            } else {
                String msg = "matcher = " + matcher +
                    ", selector = " + selector +
                    ", orderBy = " + orderBy +
                    ", options = " + options;
                throw new BaseException(SDBError.getSDBError(flags), msg);
            }
        }

        DBCursor cursor = new DBCursor(response, this);
        return cursor;
    }

    /**
     * @fn void removeBackup ( BSONObject options )
     * @brief Remove the backups.
     * @param options Contains configuration infomations for remove backups, remove all the backups in the default backup path if null.
     *                The "options" contains 3 options as below. All the elements in options are optional.
     *                eg: {"GroupName":["rgName1", "rgName2"], "Path":"/opt/sequoiadb/backup", "Name":"backupName"}
     *<ul>
     *<li>GroupName   : Assign the backups of specifed replica grouops to be remove
     *<li>Path        : Assign the backups in specifed path to be remove, if not assign, use the backup path asigned in the configuration file
     *<li>Name        : Assign the backups with specifed name to be remove
     *</ul>
     * @exception com.sequoiadb.exception.BaseException
     */
    public void removeBackup(BSONObject options) throws BaseException {
        if (null != options) {
            for (String key : options.keySet()) {
                if (key.equals(SequoiadbConstants.FIELD_NAME_GROUPNAME)
                    || key.equals(SequoiadbConstants.FIELD_NAME_NAME)
                    || key.equals(SequoiadbConstants.FIELD_NAME_PATH)) {
                    continue;
                } else {
                    throw new BaseException(SDBError.SDB_INVALIDARG);
                }
            }
        }

        AdminRequest request = new AdminRequest(AdminCommand.REMOVE_BACKUP, options);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn DBCursor listTasks ( BSONObject matcher, BSONObject selector,
     *		                    BSONObject orderBy, BSONObject hint )
     * @brief List the tasks.
     * @param matcher The matching rule, return all the documents if null
     * @param selector The selective rule, return the whole document if null
     * @param orderBy The ordered rule, never sort if null
     * @param hint
     *            Specified the index used to scan data. e.g. {"":"ageIndex"} means 
     *            using index "ageIndex" to scan data(index scan); 
     *            {"":null} means table scan. when hint is null, 
     *            database automatically match the optimal index to scan data.
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor listTasks(BSONObject matcher, BSONObject selector,
                              BSONObject orderBy, BSONObject hint) throws BaseException {
        return getList(SDB_LIST_TASKS, matcher, selector, orderBy);
    }

    /**
     * @fn DBCursor waitTasks (long[] taskIDs)
     * @brief Wait the tasks to finish.
     * @param taskIDs The array of task id
     * @exception com.sequoiadb.exception.BaseException
     */
    public void waitTasks(long[] taskIDs) throws BaseException {
        if (taskIDs == null || taskIDs.length == 0) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "taskIDs is empty or null");
        }

        // append argument:{ "TaskID": { "$in": [ 1, 2, 3 ] } }
        BSONObject newObj = new BasicBSONObject();
        BSONObject subObj = new BasicBSONObject();
        BSONObject list = new BasicBSONList();
        for (int i = 0; i < taskIDs.length; i++) {
            list.put(Integer.toString(i), taskIDs[i]);
        }
        subObj.put("$in", list);
        newObj.put(SequoiadbConstants.FIELD_NAME_TASKID, subObj);

        AdminRequest request = new AdminRequest(AdminCommand.WAIT_TASK, newObj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn DBCursor cancelTask ( long taskID, boolean isAsync )
     * @brief Cancel the specified task.
     * @param taskID The task id
     * @param isAsync The operation "cancel task" is async or not,
     *                "true" for async, "false" for sync. Default sync.
     * @exception com.sequoiadb.exception.BaseException
     */
    public void cancelTask(long taskID, boolean isAsync) throws BaseException {
        if (taskID <= 0) {
            String msg = "taskID = " + taskID + ", isAsync = " + isAsync;
            throw new BaseException(SDBError.SDB_INVALIDARG, msg);
        }

        BSONObject newObj = new BasicBSONObject();
        newObj.put(SequoiadbConstants.FIELD_NAME_TASKID, taskID);
        newObj.put(SequoiadbConstants.FIELD_NAME_ASYNC, isAsync);

        AdminRequest request = new AdminRequest(AdminCommand.CANCEL_TASK, newObj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void setSessionAttr( BSONObject options )
     * @brief Set the attributes of the current session.
     * @param options  The configuration options for the current session.The options are as below:
     * <ul>
     * <li>PreferedInstance   : indicate which instance to respond read request in current session.
     *                        eg:{"PreferedInstance":"m"/"M"/"s"/"S"/"a"/"A"/1-7}, prefer to choose "read and write instance"/"read only instance"/"anyone instance"/instance1-insatance7,
     *                        default to be {"PreferedInstance":"A"}, means would like to choose anyone instance to respond read request such as query.
     * </li>
     * </ul>
     * @note 1.Option "PreferedInstance" is used to choose which instance for querying in current session.When a new session is built,
     *         it works with default attribute {"PreferedInstance":"A"}. And it will keep the preferred instance for querying in current session
     *         until the session is closed or the data node which belongs to this instance is shut down.
     *       2.If a replica group only has 3 data notes, and we offer a configuraion option {"PreferedInstance":5},
     *         in most cases, it will choose the instance which node 2 is in, the formula is (5-1)%3+1. But, if the selected instance is a "read and write instance",
     *         it will choose next instance.
     *         when offer {"PreferedInstance":1-7}, it will choose "read only instance" first.
     *
     * @code
     *	Sequoiadb sdb = new Sequoiadb("ubuntu-dev1", 11810, "", ""); // when build object sdb, it means we start session 1
     *  sdb.setSessionAttr(new BasicBSONObject("PreferedInstance", 3)); // choose No.3 instance(assume it exist and it's not a r/w instance) for querying
     *  CollectionSpace cs = sdb.getCollectionSpace("foo");
     *  DBCollection cl = cs.getCollection("bar");
     *  cl.query(); // it will choose No.3 instance to query data in session 1
     *
     *  Sequoiadb sdb1 = new Sequoiadb("ubuntu-dev2", 11810, "", ""); // build another Sequoiadb object, and we start session 2
     *  sdb1.setSessionAttr(new BasicBSONObject("PreferedInstance", "M")); // choose r/w instance for querying in session 2
     *  CollectionSpace cs1 = sdb.getCollectionSpace("foo");
     *  DBCollection cl1 = cs1.getCollection("bar");
     *  cl1.query(); // it will choose r/w instance to query data in session 2
     *  cl.query(); // it will choose No.3 instance to query data in session 1
     *
     *  sdb.disconnect(); // close session 1
     *
     *  Sequoiadb sdb = new Sequoiadb("ubuntu-dev1", 11810, "", ""); // start session 3
     *  CollectionSpace cs = sdb.getCollectionSpace("foo");
     *  DBCollection cl = cs.getCollection("bar");
     *  cl.query(); // it will choose any instance to query data in session 3. Assuming it choise No.4 instance, when we qurey next time,
     *              // unless the node which belongs to NO.4 instance had shut down, it would choose No.4 instance again.
     *  cl.query(); // choose No.4 instance to query again
     * @endcode
     * @exception com.sequoiadb.exception.BaseException
     */
    public void setSessionAttr(BSONObject options) throws BaseException {
        if (null == options || options.isEmpty()) {
            return;
        }
        if (!options.containsField(SequoiadbConstants.FIELD_NAME_PREFERED_INSTANCE)) {
            throw new BaseException(SDBError.SDB_INVALIDARG, options.toString());
        }

        BSONObject newObj = new BasicBSONObject();
        Object value = options.get(SequoiadbConstants.FIELD_NAME_PREFERED_INSTANCE);
        int v;
        if (value instanceof Integer) {
            v = (Integer) value;
            if (v < 1 || v > 7) {
                throw new BaseException(SDBError.SDB_INVALIDARG, options.toString());
            }
        } else if (value instanceof String) {
            if (value.equals("M") || value.equals("m")) {
                v = PreferInstanceType.INS_MASTER.getCode();
            } else if (value.equals("S") || value.equals("s")) {
                v = PreferInstanceType.INS_SLAVE.getCode();
            } else if (value.equals("A") || value.equals("a")) {
                v = PreferInstanceType.INS_ANYONE.getCode();
            } else {
                throw new BaseException(SDBError.SDB_INVALIDARG, options.toString());
            }
        } else {
            throw new BaseException(SDBError.SDB_INVALIDARG, options.toString());
        }
        newObj.put(SequoiadbConstants.FIELD_NAME_PREFERED_INSTANCE, v);

        AdminRequest request = new AdminRequest(AdminCommand.SET_SESSION_ATTRIBUTE, newObj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn void closeAllCursors()
     * @brief Close all the cursors created in current connection, we can't use those cursors to get
     *        data again.
     * @return void
     * @exception com.sequoiadb.exception.BaseException
     */
    public void closeAllCursors() throws BaseException {
        InterruptRequest request = new InterruptRequest();
        sendRequest(request);
    }

    /**
     * @fn DBCursor listReplicaGroups()
     * @brief List all the replica group.
     * @return cursor of all collecionspace names
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor listReplicaGroups() throws BaseException {
        return getList(SDB_LIST_GROUPS, null, null, null);
    }

    /**
     * @fn boolean isDomainExist(String domainName)
     * @brief Verify the existence of domain.
     * @param domainName the name of domain
     * @return True if existed or False if not existed
     * @exception com.sequoiadb.exception.BaseException
     */
    public boolean isDomainExist(String domainName) throws BaseException {
        if (null == domainName || domainName.equals("")) {
            throw new BaseException(SDBError.SDB_INVALIDARG, domainName);
        }

        BSONObject matcher = new BasicBSONObject();
        matcher.put(SequoiadbConstants.FIELD_NAME_NAME, domainName);

        DBCursor cursor = getList(SDB_LIST_DOMAINS, matcher, null, null);
        return (null != cursor && cursor.hasNext());
    }

    /**
     * @fn Domain createDomain(String domainName, BSONObject options)
     * @brief Create a domain.
     * @param domainName The name of the creating domain
     * @param options The options for the domain. The options are as below:
     * <ul>
     * <li>Groups    : the list of the replica groups' names which the domain is going to contain.
     *                 eg: { "Groups": [ "group1", "group2", "group3" ] }
     *                 If this argument is not included, the domain will contain all replica groups in the cluster. 
     * <li>AutoSplit    : If this option is set to be true, while creating collection(ShardingType is "hash") in this domain,
     *                    the data of this collection will be split(hash split) into all the groups in this domain automatically.
     *                    However, it won't automatically split data into those groups which were add into this domain later.
     *                    eg: { "Groups": [ "group1", "group2", "group3" ], "AutoSplit: true" }
     * </ul>
     * @return the newly created collection space object
     * @exception com.sequoiadb.exception.BaseException
     */
    public Domain createDomain(String domainName, BSONObject options) throws BaseException {
        if (null == domainName || domainName.equals("")) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "domain name is empty or null");
        }
        if (isDomainExist(domainName))
            throw new BaseException(SDBError.SDB_CAT_DOMAIN_EXIST, domainName);

        BSONObject newObj = new BasicBSONObject();
        newObj.put(SequoiadbConstants.FIELD_NAME_NAME, domainName);
        if (null != options) {
            newObj.put(SequoiadbConstants.FIELD_NAME_OPTIONS, options);
        }

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_DOMAIN, newObj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
        return new Domain(this, domainName);
    }

    /**
     * @fn void dropDomain(String domainName)
     * @brief Drop a domain.
     * @param domainName the name of the domain
     * @exception com.sequoiadb.exception.BaseException
     */
    public void dropDomain(String domainName) throws BaseException {
        if (null == domainName || domainName.equals("")) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "domain name is empty or null");
        }

        BSONObject newObj = new BasicBSONObject();
        newObj.put(SequoiadbConstants.FIELD_NAME_NAME, domainName);

        AdminRequest request = new AdminRequest(AdminCommand.DROP_DOMAIN, newObj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    /**
     * @fn Domain getDomain(String domainName)
     * @brief Get the specified domain.
     * @param domainName the name of the domain
     * @return the Domain instance
     * @exception com.sequoiadb.exception.BaseException
     *            If the domain not exit, throw BaseException with the error type "SDB_CAT_DOMAIN_NOT_EXIST"
     */
    public Domain getDomain(String domainName) throws BaseException {
        if (isDomainExist(domainName)) {
            return new Domain(this, domainName);
        } else {
            throw new BaseException(SDBError.SDB_CAT_DOMAIN_NOT_EXIST, domainName);
        }
    }

    /**
     * @fn DBCursor listDomains(BSONObject matcher, BSONObject selector,
    BSONObject orderBy, BSONObject hint)
     * @brief List domains.
     * @param matcher the matching rule, return all the documents if null
     * @param selector the selective rule, return the whole document if null
     * @param orderBy the ordered rule, never sort if null
     * @param hint
     *            Specified the index used to scan data. e.g. {"":"ageIndex"} means 
     *            using index "ageIndex" to scan data(index scan); 
     *            {"":null} means table scan. when hint is null, 
     *            database automatically match the optimal index to scan data.
     * @exception com.sequoiadb.exception.BaseException
     */
    public DBCursor listDomains(BSONObject matcher, BSONObject selector,
                                BSONObject orderBy, BSONObject hint) throws BaseException {
        return getList(SDB_LIST_DOMAINS, matcher, selector, orderBy);
    }

    /**
     * @fn ArrayList<String> getReplicaGroupNames()
     * @brief Get all the replica groups' name.
     * @return A list of all the replica groups' names.
     * @exception com.sequoiadb.exception.BaseException
     */
    public ArrayList<String> getReplicaGroupNames() throws BaseException {
        DBCursor cursor = getList(SDB_LIST_GROUPS, null, null, null);
        if (cursor == null) {
            return null;
        }
        ArrayList<String> colList = new ArrayList<String>();
        while (cursor.hasNext()) {
            colList.add(cursor.getNext().get("GroupName").toString());
        }
        return colList;
    }

    /**
     * @fn List<String> getReplicaGroupsInfo()
     * @brief Get the infomations of the replica groups.
     * @return A list of informations of the replica groups.
     * @exception com.sequoiadb.exception.BaseException
     */
    public ArrayList<String> getReplicaGroupsInfo() throws BaseException {
        DBCursor cursor = getList(SDB_LIST_GROUPS, null, null, null);
        if (cursor == null) {
            return null;
        }
        ArrayList<String> colList = new ArrayList<String>();
        while (cursor.hasNext()) {
            colList.add(cursor.getNext().toString());
        }
        return colList;
    }

    /**
     * @fn ReplicaGroup getReplicaGroup(String rgName)
     * @brief Get replica group by name.
     * @param rgName
     *            replica group's name
     * @return A replica group object or null for not exit.
     * @exception com.sequoiadb.exception.BaseException
     */
    public ReplicaGroup getReplicaGroup(String rgName)
        throws BaseException {
        BSONObject rg = getDetailByName(rgName);
        if (rg == null) {
            return null;
        }
        return new ReplicaGroup(this, rgName);
    }

    /**
     * @fn ReplicaGroup getReplicaGroup(int rgId)
     * @brief Get replica group by id.
     * @param rgId
     *            replica group id
     * @return A replica group object or null for not exit.
     * @exception com.sequoiadb.exception.BaseException
     */
    public ReplicaGroup getReplicaGroup(int rgId) throws BaseException {
        BSONObject rg = getDetailById(rgId);
        if (rg == null) {
            return null;
        }
        return new ReplicaGroup(this, rgId);
    }

    /**
     * @fn ReplicaGroup createReplicaGroup(String rgName)
     * @brief Create replica group by name.
     * @param rgName
     *            replica group's name
     * @return A replica group object.
     * @exception com.sequoiadb.exception.BaseException
     */
    public ReplicaGroup createReplicaGroup(String rgName) throws BaseException {
        BSONObject rg = new BasicBSONObject();
        rg.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, rgName);

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_GROUP, rg);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag, rgName);
        }
        return getReplicaGroup(rgName);
    }

    /**
     * @fn void removeReplicaGroup(String rgName)
     * @brief Remove replica group by name.
     * @param rgName
     *            replica group's name
     * @exception com.sequoiadb.exception.BaseException
     */
    public void removeReplicaGroup(String rgName) throws BaseException {
        BSONObject rg = new BasicBSONObject();
        rg.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, rgName);

        AdminRequest request = new AdminRequest(AdminCommand.REMOVE_GROUP, rg);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag, rgName);
        }
    }

    private long getNextRequestId() {
        return requestId++;
    }

    /**
     * @fn void activateReplicaGroup(String rgName)
     * @brief Active replica group by name.
     * @param rgName
     *            replica group name
     * @exception com.sequoiadb.exception.BaseException
     */
    public void activateReplicaGroup(String rgName) throws BaseException {
        BSONObject rg = new BasicBSONObject();
        rg.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, rgName);

        AdminRequest request = new AdminRequest(AdminCommand.ACTIVE_GROUP, rg);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag, rgName);
        }
    }

    /**
     * @fn void createReplicaCataGroup(String hostName, int port, String dbPath,
     *     BSONObject configuration)
     * @brief Create the replica Catalog group with the given options.
     * @param hostName
     *            The host name
     * @param port
     *            The port
     * @param dbpath
     *            The database path
     * @param configure
     *            The configure options
     * @exception com.sequoiadb.exception.BaseException
     */
    public void createReplicaCataGroup(String hostName, int port,
                                       String dbPath, Map<String, String> configure) {
        BSONObject obj = new BasicBSONObject();
        obj.put(SequoiadbConstants.FIELD_NAME_HOST, hostName);
        obj.put(SequoiadbConstants.PMD_OPTION_SVCNAME, Integer.toString(port));
        obj.put(SequoiadbConstants.PMD_OPTION_DBPATH, dbPath);
        if (configure != null) {
            for (String key : configure.keySet()) {
                if (key.equals(SequoiadbConstants.FIELD_NAME_HOST)
                    || key.equals(SequoiadbConstants.PMD_OPTION_SVCNAME)
                    || key.equals(SequoiadbConstants.PMD_OPTION_DBPATH)) {
                    continue;
                }
                obj.put(key, configure.get(key));
            }
        }

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_CATALOG_GROUP, obj);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    private String getListCommand(int listType) {
        switch (listType) {
            case SDB_LIST_CONTEXTS:
                return AdminCommand.LIST_CONTEXTS;
            case SDB_LIST_CONTEXTS_CURRENT:
                return AdminCommand.LIST_CONTEXTS_CURRENT;
            case SDB_LIST_SESSIONS:
                return AdminCommand.LIST_SESSIONS;
            case SDB_LIST_SESSIONS_CURRENT:
                return AdminCommand.LIST_SESSIONS_CURRENT;
            case SDB_LIST_COLLECTIONS:
                return AdminCommand.LIST_COLLECTIONS;
            case SDB_LIST_COLLECTIONSPACES:
                return AdminCommand.LIST_COLLECTIONSPACES;
            case SDB_LIST_STORAGEUNITS:
                return AdminCommand.LIST_STORAGEUNITS;
            case SDB_LIST_GROUPS:
                return AdminCommand.LIST_GROUPS;
            case SDB_LIST_STOREPROCEDURES:
                return AdminCommand.LIST_PROCEDURES;
            case SDB_LIST_DOMAINS:
                return AdminCommand.LIST_DOMAINS;
            case SDB_LIST_TASKS:
                return AdminCommand.LIST_TASKS;
            case SDB_LIST_CS_IN_DOMAIN:
                return AdminCommand.LIST_CS_IN_DOMAIN;
            case SDB_LIST_CL_IN_DOMAIN:
                return AdminCommand.LIST_CL_IN_DOMAIN;
            default:
                throw new BaseException(SDBError.SDB_INVALIDARG, String.format("Invalid list type: %d", listType));
        }
    }

    String getUserName() {
        return userName;
    }

    String getPassword() {
        return password;
    }

    BSONObject getDetailByName(String name) throws BaseException {
        BSONObject condition = new BasicBSONObject();
        condition.put(SequoiadbConstants.FIELD_NAME_GROUPNAME, name);

        DBCursor cursor = getList(Sequoiadb.SDB_LIST_GROUPS, condition, null, null);
        if (cursor == null || !cursor.hasNext()) {
            return null;
        }
        return cursor.getNext();
    }

    BSONObject getDetailById(int id) throws BaseException {
        BSONObject condition = new BasicBSONObject();
        condition.put(SequoiadbConstants.FIELD_NAME_GROUPID, id);

        DBCursor cursor = getList(Sequoiadb.SDB_LIST_GROUPS, condition, null, null);
        if (cursor == null || !cursor.hasNext()) {
            return null;
        }
        return cursor.getNext();
    }

    private void initConnection(ConfigOptions options) throws BaseException {
        if (options == null) {
            throw new BaseException(SDBError.SDB_INVALIDARG, "options is null");
        }
        connection = new ConnectionTCPImpl(serverAddress, options);
        connection.connect();
    }

    private SysInfoResponse receiveSysInfoResponse() {
        SysInfoResponse response = new SysInfoResponse();
        byte[] lengthBytes = connection.receive(response.length());
        ByteBuffer buffer = ByteBuffer.wrap(lengthBytes);
        response.decode(buffer);
        return response;
    }

    private ByteBuffer receiveSdbResponse() {
        byte[] lengthBytes = connection.receive(4);
        int length = ByteBuffer.wrap(lengthBytes).order(byteOrder).getInt();

        byte[] bytes = new byte[length];
        System.arraycopy(lengthBytes, 0, bytes, 0, lengthBytes.length);
        connection.receive(bytes, 4, length - 4);
        ByteBuffer buffer = ByteBuffer.wrap(bytes).order(byteOrder);
        return buffer;
    }

    private void checkResponse(SdbRequest request, SdbResponse response) {
        if ((request.opCode() | MsgOpCode.RESP_MASK) != response.opCode()) {
            throw new BaseException(SDBError.SDB_UNKNOWN_MESSAGE,
                ("request=" + request.opCode() + " response=" + response.opCode()));
        }
    }

    private ByteBuffer encodeRequest(Request request) {
        ByteBuffer buffer = ByteBuffer.allocate(request.length());
        buffer.order(byteOrder);
        request.setRequestId(getNextRequestId());
        request.encode(buffer);
        return buffer;
    }

    private void sendRequest(Request request) {
        ByteBuffer buffer = encodeRequest(request);
        connection.send(buffer);
    }

    private <T extends SdbResponse> T decodeResponse(ByteBuffer buffer, Class<T> tClass) {
        T response;
        try {
            response = tClass.newInstance();
        } catch (Exception e) {
            throw new BaseException(SDBError.SDB_INVALIDARG, e);
        }
        response.decode(buffer);
        return response;
    }

    private ByteBuffer sendAndReceive(ByteBuffer request) {
        connection.send(request);
        lastUseTime = System.currentTimeMillis();
        return receiveSdbResponse();
    }

    <T extends SdbResponse> T requestAndResponse(SdbRequest request, Class<T> tClass) {
        ByteBuffer out = encodeRequest(request);
        ByteBuffer in = sendAndReceive(out);
        T response = decodeResponse(in, tClass);
        checkResponse(request, response);
        return response;
    }

    SdbReply requestAndResponse(SdbRequest request) {
        return requestAndResponse(request, SdbReply.class);
    }

    private ByteOrder getSysInfo() {
        SysInfoRequest request = new SysInfoRequest();
        sendRequest(request);

        SysInfoResponse response = receiveSysInfoResponse();

        return response.byteOrder();
    }

    private void killContext() {
        if (connection == null) {
            throw new BaseException(SDBError.SDB_NETWORK);
        }

        long[] contextIds = new long[]{-1};
        KillContextRequest request = new KillContextRequest(contextIds);
        SdbReply response = requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            throw new BaseException(flag);
        }
    }

    @Override
    public void close() throws BaseException {
        disconnect();
    }

    /**
     * @class SptEvalResult
     * @brief Class for executing stored procedure result.
     */
    public static class SptEvalResult {
        private SptReturnType returnType;
        private BSONObject errmsg;
        private DBCursor cursor;

        /**
         * @fn SptEvalResult ()
         * @brief Constructor.
         */
        public SptEvalResult() {
            returnType = null;
            errmsg = null;
            cursor = null;
        }

        /**
         * @fn setReturnType ()
         * @brief Set return type.
         */
        public void setReturnType(SptReturnType returnType) {
            this.returnType = returnType;
        }

        /**
         * @fn SptReturnType getReturnType ()
         * @brief Get return type.
         */
        public SptReturnType getReturnType() {
            return returnType;
        }

        /**
         * @fn setErrMsg ()
         * @brief Set error type.
         */
        public void setErrMsg(BSONObject errmsg) {
            this.errmsg = errmsg;
        }

        /**
         * @fn BSONObject getErrMsg ()
         * @brief Get error type.
         */
        public BSONObject getErrMsg() {
            return errmsg;
        }

        /**
         * @fn setCursor ()
         * @brief Set result cursor.
         */
        public void setCursor(DBCursor cursor) {
            this.cursor = cursor;
        }

        /**
         * @fn DBCursor getCursor ()
         * @brief Get result cursor.
         */
        public DBCursor getCursor() {
            return cursor;
        }
    }

    public enum SptReturnType {
        TYPE_VOID(0),
        TYPE_STR(1),
        TYPE_NUMBER(2),
        TYPE_OBJ(3),
        TYPE_BOOL(4),
        TYPE_RECORDSET(5),
        TYPE_CS(6),
        TYPE_CL(7),
        TYPE_RG(8),
        TYPE_RN(9);

        private int typeValue;

        SptReturnType(int typeValue) {
            this.typeValue = typeValue;
        }

        public int getTypeValue() {
            return typeValue;
        }

        public static SptReturnType getTypeByValue(int typeValue) {
            SptReturnType retType = null;
            for (SptReturnType rt : values()) {
                if (rt.getTypeValue() == typeValue) {
                    retType = rt;
                    break;
                }
            }
            return retType;
        }
    }
}
