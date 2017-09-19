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
import com.sequoiadb.exception.SDBError;
import com.sequoiadb.message.request.AdminRequest;
import com.sequoiadb.message.response.SdbReply;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;

import java.util.ArrayList;
import java.util.List;

/**
 * @class CollectionSpace
 * @brief Database operation interfaces of collection space.
 */
public class CollectionSpace {
    private String name;
    private Sequoiadb sequoiadb;

    /**
     * @return The collection space name
     * @fn String getName()
     * @brief Return the name of current collection space.
     */
    public String getName() {
        return name;
    }

    /**
     * @return Sequoiadb object
     * @fn Sequoiadb getSequoiadb()
     * @brief Return the Sequoiadb instance of current collection space belong to.
     */
    public Sequoiadb getSequoiadb() {
        return sequoiadb;
    }

    /**
     * @param sequoiadb Sequoiadb handle
     * @param name      Collection space name
     * @fn CollectionSpace(Sequoiadb sequoiadb, String name)
     * @brief Constructor
     */
    CollectionSpace(Sequoiadb sequoiadb, String name) {
        this.name = name;
        this.sequoiadb = sequoiadb;
    }

    /**
     * @param collectionName The collection name
     * @return The collection object or null for collection not exist
     * @throws com.sequoiadb.exception.BaseException
     * @fn DBCollection getCollection(String collectionName)
     * @brief Get the named collection
     */
    public DBCollection getCollection(String collectionName)
        throws BaseException {
        // get cl from cache
        String collectionFullName = name + "." + collectionName;
        if (sequoiadb.fetchCache(collectionFullName)) {
            return new DBCollection(sequoiadb, this, collectionName);
        }

        // no need to upsert/remove cache here,
        // for "isCollectionExist" has do that
        if (isCollectionExist(collectionName)) {
            return new DBCollection(sequoiadb, this, collectionName);
        } else {
            throw new BaseException(SDBError.SDB_DMS_NOTEXIST, collectionName);
        }
    }

    /**
     * @param colName The collection name
     * @return True if collection existed or False if not existed
     * @throws com.sequoiadb.exception.BaseException
     * @fn boolean isCollectionExist(String colName)
     * @brief Verify the existence of collection in current collection space
     */
    public boolean isCollectionExist(String colName) throws BaseException {
        String collectionFullName = name + "." + colName;

        BSONObject obj = new BasicBSONObject();
        obj.put(SdbConstants.FIELD_NAME_NAME, collectionFullName);

        AdminRequest request = new AdminRequest(AdminCommand.TEST_CL, obj);
        SdbReply response = sequoiadb.requestAndResponse(request);

        int flag = response.getFlag();
        if (flag == 0) {
            sequoiadb.upsertCache(collectionFullName);
            return true;
        } else if (flag == SDBError.SDB_DMS_NOTEXIST.getErrorCode()) {
            sequoiadb.removeCache(collectionFullName);
            return false;
        } else {
            sequoiadb.throwIfError(response);
            return false; // make compiler happy
        }
    }

    /**
     * @return A list of collection names
     * @throws com.sequoiadb.exception.BaseException
     * @fn List<String> getCollectionNames()
     * @brief Get all the collection names of current collection space
     */
    public List<String> getCollectionNames() throws BaseException {
        List<String> collectionNames = new ArrayList<String>();
        List<String> colNames = sequoiadb.getCollectionNames();
        if ((colNames != null) && (colNames.size() != 0)) {
            for (String col : colNames) {
                if (col.startsWith(name + ".")) {
                    collectionNames.add(col);
                }
            }
        }
        return collectionNames;
    }

    /**
     * @param collectionName The collection name
     * @throws com.sequoiadb.exception.BaseException
     * @fn void createCollection(String collectionName)
     * @brief Create the named collection in current collection space
     */
    public DBCollection createCollection(String collectionName)
        throws BaseException {
        return createCollection(collectionName, null);
    }

    /**
     * @param collectionName The collection name
     * @param options        The options for creating collection, including
     *                       "ShardingKey", "ReplSize", "IsMainCL" and "Compressed" informations,
     *                       no options, if null
     * @return the created DBCollection
     * @throws com.sequoiadb.exception.BaseException
     * @fn DBCollection createCollection(String collectionName, BSONObject
     * options)
     * @brief Create collection by options
     */
    public DBCollection createCollection(String collectionName,
                                         BSONObject options) {
        if (isCollectionExist(collectionName)) {
            throw new BaseException(SDBError.SDB_DMS_EXIST, collectionName);
        }

        String collectionFullName = name + "." + collectionName;

        BSONObject obj = new BasicBSONObject();
        obj.put(SdbConstants.FIELD_NAME_NAME, collectionFullName);
        if (options != null) {
            obj.putAll(options);
        }

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_CL, obj);
        SdbReply response = sequoiadb.requestAndResponse(request);
        String msg = "collection = " + collectionFullName + ", options = " + options;
        sequoiadb.throwIfError(response, msg);
        sequoiadb.upsertCache(collectionFullName);
        return new DBCollection(sequoiadb, this, collectionName);
    }

    /**
     * @return void
     * @throws com.sequoiadb.exception.BaseException
     * @fn void drop()
     * @brief Drop current collectionSpace
     * @see com.sequoiadb.base.Sequoiadb.dropCollectionSpace
     * @deprecated the method will be deprecated in version 2.x, use Sequoiadb.dropCollectionSpace instead
     */
    public void drop() throws BaseException {
        sequoiadb.dropCollectionSpace(this.name);
    }

    /**
     * @param collectionName The collection name
     * @throws com.sequoiadb.exception.BaseException
     * @fn void dropCollection(String collectionName)
     * @brief Remove the named collection of current collection space
     */
    public void dropCollection(String collectionName) throws BaseException {
        if (!isCollectionExist(collectionName)) {
            throw new BaseException(SDBError.SDB_DMS_NOTEXIST, collectionName);
        }

        String collectionFullName = name + "." + collectionName;

        BSONObject obj = new BasicBSONObject();
        obj.put(SdbConstants.FIELD_NAME_NAME, collectionFullName);

        AdminRequest request = new AdminRequest(AdminCommand.DROP_CL, obj);
        SdbReply response = sequoiadb.requestAndResponse(request);
        sequoiadb.throwIfError(response, collectionName);
        sequoiadb.removeCache(collectionFullName);
    }
}
