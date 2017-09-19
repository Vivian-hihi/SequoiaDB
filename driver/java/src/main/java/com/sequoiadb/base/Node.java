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

/**
 * @class Node
 * @brief Database operation interfaces of node.
 */
public class Node {
    private String hostName;
    private int port;
    private String nodeName;
    private int id;
    private ReplicaGroup rg;
    private Sequoiadb sequoiadb;

    Node(String hostName, int port, int nodeId, ReplicaGroup rg) {
        this.rg = rg;
        this.hostName = hostName;
        this.port = port;
        this.nodeName = hostName + ":" + port;
        this.id = nodeId;
    }

    /*!
     * enum Node::NodeStatus
     */
    public enum NodeStatus {
        SDB_NODE_ALL(1),
        SDB_NODE_ACTIVE(2),
        SDB_NODE_INACTIVE(3),
        SDB_NODE_UNKNOWN(4);
        private final int key;

        NodeStatus(int key) {
            this.key = key;
        }

        public int getKey() {
            return key;
        }

        public static NodeStatus getByKey(int key) {
            NodeStatus nodeStatus = NodeStatus.SDB_NODE_ALL;
            for (NodeStatus status : NodeStatus.values()) {
                if (status.getKey() == key) {
                    nodeStatus = status;
                    break;
                }
            }
            return nodeStatus;
        }
    }

    /**
     * @return Current node's id.
     * @fn int getNodeId()
     * @brief Get current node's id.
     */
    public int getNodeId() {
        return id;
    }

    /**
     * @return Current node's parent replica group.
     * @fn ReplicaGroup getReplicaGroup()
     * @brief Get current node's parent replica group.
     */
    public ReplicaGroup getReplicaGroup() {
        return rg;
    }

    /**
     * @return void
     * @throws com.sequoiadb.exception.BaseException
     * @fn void disconnect()
     * @brief Disconnect from current node.
     */
    public void disconnect() throws BaseException {
        sequoiadb.disconnect();
    }

    /**
     * @return The Sequoiadb object of current node.
     * @throws com.sequoiadb.exception.BaseException
     * @fn Sequoiadb connect ()
     * @brief Connect to current node with the same username and password.
     */
    public Sequoiadb connect() throws BaseException {
        if (sequoiadb != null && !sequoiadb.isClosed()) {
            sequoiadb.close();
        }
        sequoiadb = new Sequoiadb(hostName, port, rg.getSequoiadb().getUserName(),
            rg.getSequoiadb().getPassword());
        return sequoiadb;
    }

    /**
     * @param username user name
     * @param password pass word
     * @return The Sequoiadb object of current node.
     * @throws com.sequoiadb.exception.BaseException
     * @fn Sequoiadb connect(String username, String password)
     * @brief Connect to current node with username and password.
     */
    public Sequoiadb connect(String username, String password) throws BaseException {
        if (sequoiadb != null && !sequoiadb.isClosed()) {
            sequoiadb.close();
        }
        sequoiadb = new Sequoiadb(hostName, port, username, password);
        return sequoiadb;
    }

    /**
     * @return The Sequoiadb object of current node.
     * @fn Sequoiadb getSdb()
     * @brief Get the Sequoiadb of current node.
     */
    public Sequoiadb getSdb() {
        return sequoiadb;
    }

    /**
     * @return Hostname of current node.
     * @fn String getHostName()
     * @brief Get the hostname of current node.
     */
    public String getHostName() {
        return hostName;
    }

    /**
     * @return The port of current node.
     * @fn int getPort()
     * @brief Get the port of current node.
     */
    public int getPort() {
        return port;
    }

    /**
     * @return The name of current node.
     * @fn String getNodeName()
     * @brief Get the name of current node.
     */
    public String getNodeName() {
        return nodeName;
    }

    /**
     * @return The status of current node.
     * @throws com.sequoiadb.exception.BaseException
     * @fn NodeStatus getStatus()
     * @brief Get the status of current node.
     * @deprecated Since v2.8, the status of node are invalid, never use this api again.
     */
    public NodeStatus getStatus() throws BaseException {
        BSONObject obj = new BasicBSONObject();
        obj.put(SdbConstants.FIELD_NAME_GROUPID, rg.getId());
        obj.put(SdbConstants.FIELD_NAME_NODEID, id);

        AdminRequest request = new AdminRequest(AdminCommand.SNAP_DATABASE, obj);
        SdbReply response = rg.getSequoiadb().requestAndResponse(request);

        int flag = response.getFlag();
        if (flag != 0) {
            if (flag == SDBError.SDB_NET_CANNOT_CONNECT.getErrorCode()) {
                return NodeStatus.SDB_NODE_INACTIVE;
            } else {
                rg.getSequoiadb().throwIfError(response);
            }
        }
        return NodeStatus.SDB_NODE_ACTIVE;
    }

    /**
     * @return void
     * @throws com.sequoiadb.exception.BaseException
     * @fn void start()
     * @brief Start current node in database.
     */
    public void start() throws BaseException {
        startStop(true);
    }

    /**
     * @return void
     * @throws com.sequoiadb.exception.BaseException
     * @fn void stop()
     * @brief Stop current node in database.
     */
    public void stop() throws BaseException {
        startStop(false);
    }

    private void startStop(boolean start) {
        BSONObject config = new BasicBSONObject();
        config.put(SdbConstants.FIELD_NAME_HOST, hostName);
        config.put(SdbConstants.PMD_OPTION_SVCNAME, Integer.toString(port));

        String cmd = start ? AdminCommand.STARTUP_NODE : AdminCommand.SHUTDOWN_NODE;
        AdminRequest request = new AdminRequest(cmd, config);
        SdbReply response = rg.getSequoiadb().requestAndResponse(request);
        String msg = "node = " + hostName + ":" + port;
        rg.getSequoiadb().throwIfError(response, msg);
    }
}
