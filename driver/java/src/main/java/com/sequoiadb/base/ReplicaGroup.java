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
import org.bson.types.BasicBSONList;

import java.net.InetAddress;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.Random;

/**
 * @class ReplicaGroup
 * @brief Database operation interfaces of replica group.
 */
public class ReplicaGroup {
    private String name;
    private int id;
    private Sequoiadb sequoiadb;
    private boolean isCataRG;


    /**
     * @return the current replica group's Sequoiadb
     * @fn Sequoiadb getSequoiadb()
     * @brief Get current replica group's Sequoiadb.
     */
    public Sequoiadb getSequoiadb() {
        return sequoiadb;
    }

    /**
     * @return the current replica group's id
     * @fn int getId()
     * @brief Get current replica group's id.
     */
    public int getId() {
        return id;
    }

    /**
     * @return the current replica group's name
     * @fn String getGroupName()
     * @brief Get current replica group's name.
     */
    public String getGroupName() {
        return name;
    }

    ReplicaGroup(Sequoiadb sdb, int id) {
        this.sequoiadb = sdb;
        this.id = id;
        BSONObject group = sdb.getDetailById(id);
        this.name = group.get(SdbConstants.FIELD_NAME_GROUPNAME).toString();
        this.isCataRG = name.equals(Sequoiadb.CATALOG_GROUP_NAME);
    }

    ReplicaGroup(Sequoiadb sdb, String name) {
        this.sequoiadb = sdb;
        this.name = name;
        BSONObject group = sdb.getDetailByName(name);
        this.isCataRG = (name == Sequoiadb.CATALOG_GROUP_NAME);
        this.id = Integer.parseInt(group.get(
            SdbConstants.FIELD_NAME_GROUPID).toString());
    }

    /**
     * @param status Node.NodeStatus
     * @return the amount of the nodes with the specified status
     * @throws com.sequoiadb.exception.BaseException
     * @fn int getNodeNum(Node.NodeStatus status)
     * @brief Get the amount of the nodes with the specified status.
     * @deprecated Since v2.8, the status of node are invalid, never use this api again.
     */
    public int getNodeNum(Node.NodeStatus status) throws BaseException {
        BSONObject group = sequoiadb.getDetailById(id);
        try {
            Object obj = group.get(SdbConstants.FIELD_NAME_GROUP);
            if (obj == null) {
                return 0;
            }
            BasicBSONList list = (BasicBSONList) obj;
            return list.size();
        } catch (BaseException e) {
            throw e;
        } catch (Exception e) {
            throw new BaseException(SDBError.SDB_SYS, e);
        }
    }

    /**
     * @return the detail info
     * @throws com.sequoiadb.exception.BaseException
     * @fn BSONObject getDetail()
     * @brief Get detail info of current replicaGoup
     */
    public BSONObject getDetail() throws BaseException {
        return sequoiadb.getDetailById(id);
    }

    /**
     * @return the master node
     * @throws com.sequoiadb.exception.BaseException
     * @fn Node getMaster()
     * @brief Get the master node of current replica group.
     */
    public Node getMaster() throws BaseException {
        BSONObject group = sequoiadb.getDetailById(id);

        Object primaryNodeObj = group.get(SdbConstants.FIELD_NAME_PRIMARY);
        if (primaryNodeObj == null) {
            throw new BaseException(SDBError.SDB_CLS_NODE_NOT_EXIST);
        }
        Object groupInfoObj = group.get(SdbConstants.FIELD_NAME_GROUP);
        if (groupInfoObj == null) {
            return null;
        }

        BSONObject primaryData = null;
        Object nodeId;

        BasicBSONList nodeInfos = (BasicBSONList) groupInfoObj;
        for (Object nodeInfoObj : nodeInfos) {
            BSONObject nodeInfo = (BSONObject) nodeInfoObj;
            nodeId = nodeInfo.get(SdbConstants.FIELD_NAME_NODEID);
            if (nodeId == null) {
                throw new BaseException(SDBError.SDB_SYS);
            }
            if (nodeId.equals(primaryNodeObj)) {
                primaryData = nodeInfo;
                break;
            }
        }

        if (primaryData != null) {
            nodeId = primaryData.get(SdbConstants.FIELD_NAME_NODEID);
            String hostName = primaryData.get(
                SdbConstants.FIELD_NAME_HOST).toString();
            int port = getNodePort(primaryData);
            return new Node(hostName, port, Integer.parseInt(nodeId.toString()), this);
        }
        return null;
    }

    /**
     * @return the slave node
     * @throws com.sequoiadb.exception.BaseException
     * @fn Node getSlave()
     * @brief Get the random slave of current replica group.
     */
    public Node getSlave() throws BaseException {
        BSONObject group = sequoiadb.getDetailById(id);
        if (group == null) {
            return null;
        }

        Object primaryNodeObj = group.get(SdbConstants.FIELD_NAME_PRIMARY);
        if (primaryNodeObj == null) {
            throw new BaseException(SDBError.SDB_CLS_NODE_NOT_EXIST);
        }

        Object groupInfoObj = group.get(SdbConstants.FIELD_NAME_GROUP);
        if (groupInfoObj == null) {
            return null;
        }

        List<BSONObject> slaves = new ArrayList<>();
        BSONObject primaryData = null;

        BasicBSONList nodeInfos = (BasicBSONList) groupInfoObj;
        for (Object nodeInfoObj : nodeInfos) {
            BSONObject nodeInfo = (BSONObject) nodeInfoObj;
            Object nodeId = nodeInfo.get(SdbConstants.FIELD_NAME_NODEID);
            if (nodeId == null)
                throw new BaseException(SDBError.SDB_SYS);
            if (nodeId.equals(primaryNodeObj)) {
                primaryData = nodeInfo;
            } else {
                slaves.add(nodeInfo);
            }
        }

        if (slaves.size() != 0) {
            Random rand = new Random();
            BSONObject randNode = slaves.get(rand.nextInt(slaves.size()));
            int nodeId = Integer.parseInt(randNode.get(
                SdbConstants.FIELD_NAME_NODEID).toString());
            String hostName = randNode.get(SdbConstants.FIELD_NAME_HOST)
                .toString();
            int port = getNodePort(randNode);
            return new Node(hostName, port, nodeId, this);
        } else if (primaryData != null) {
            int nodeId = Integer.parseInt(primaryData.get(
                SdbConstants.FIELD_NAME_NODEID).toString());
            String hostName = primaryData.get(
                SdbConstants.FIELD_NAME_HOST).toString();
            int port = getNodePort(primaryData);
            return new Node(hostName, port, nodeId, this);
        } else {
            return null;
        }
    }

    /**
     * @param nodeName The name of the node
     * @return the specified node
     * @throws com.sequoiadb.exception.BaseException
     * @fn Node getNode(String nodeName)
     * @brief Get node by node's name (IP:PORT).
     */
    public Node getNode(String nodeName) throws BaseException {
        String[] temp = nodeName.split(":");
        if (temp.length != 2) {
            throw new BaseException(SDBError.SDB_INVALIDARG, nodeName);
        }

        BSONObject group = sequoiadb.getDetailById(id);
        if (group == null) {
            return null;
        }

        try {
            Object list = group.get(SdbConstants.FIELD_NAME_GROUP);
            if (list == null) {
                return null;
            }

            BasicBSONList nodeInfos = (BasicBSONList) list;
            if (nodeInfos.size() == 0) {
                return null;
            }

            for (Object nodeInfoObj : nodeInfos) {
                BSONObject nodeInfo = (BSONObject) nodeInfoObj;

                Object nodeId = nodeInfo.get(SdbConstants.FIELD_NAME_NODEID);
                Object hostName = nodeInfo.get(SdbConstants.FIELD_NAME_HOST);
                int port = getNodePort(nodeInfo);

                if (nodeId == null || hostName == null) {
                    throw new BaseException(SDBError.SDB_SYS);
                }

                String hostName2 = InetAddress.getByName(temp[0]).toString()
                    .split("/")[1];
                hostName = InetAddress.getByName(hostName.toString())
                    .toString().split("/")[1];
                if (hostName.equals(hostName2)
                    && port == Integer.parseInt((temp[1]))) {
                    return new Node(hostName2, port,
                        Integer.parseInt(nodeId.toString()), this);
                }
            }
        } catch (Exception e) {
            throw new BaseException(SDBError.SDB_SYS, nodeName);
        }
        return null;
    }

    /**
     * @param hostName host name
     * @param port     port
     * @return the Node object
     * @throws com.sequoiadb.exception.BaseException
     * @fn Node getNode(String hostName, int port)
     * @brief Get node by hostName and port.
     */
    public Node getNode(String hostName, int port) throws BaseException {
        BSONObject group = sequoiadb.getDetailById(id);
        try {
            Object list = group.get(SdbConstants.FIELD_NAME_GROUP);
            if (list == null) {
                return null;
            }

            BasicBSONList nodeInfos = (BasicBSONList) (list);
            if (nodeInfos.size() == 0) {
                return null;
            }

            for (Object obj : nodeInfos) {
                BSONObject nodeInfo = (BSONObject) obj;
                Object nodeIdObj = nodeInfo.get(SdbConstants.FIELD_NAME_NODEID);
                if (nodeIdObj == null) {
                    throw new BaseException(SDBError.SDB_SYS);
                }

                int nodeId = Integer.parseInt(nodeIdObj.toString());
                hostName = InetAddress.getByName(hostName).toString()
                    .split("/")[1];
                String hostName2 = InetAddress.getByName(
                    nodeInfo.get(SdbConstants.FIELD_NAME_HOST).toString())
                    .toString().split("/")[1];

                if (hostName2.equals(hostName)) {
                    int nodePort = getNodePort(nodeInfo);
                    if (nodePort == port) {
                        return new Node(hostName, port, nodeId, this);
                    }
                }
            }
        } catch (BaseException e) {
            throw e;
        } catch (Exception e) {
            throw new BaseException(SDBError.SDB_SYS, hostName + ":" + port, e);
        }
        return null;
    }

    /**
     * @param hostName  host name
     * @param port      port
     * @param configure configuration for this operation
     * @return the attach Node object
     * @throws com.sequoiadb.exception.BaseException
     * @fn Node attachNode(String hostName, int port,
     * BSONObject configure)
     * @brief Attach node.
     */
    public Node attachNode(String hostName, int port,
                           BSONObject configure) throws BaseException {
        BSONObject config = new BasicBSONObject();
        config.put(SdbConstants.FIELD_NAME_GROUPNAME, name);
        config.put(SdbConstants.FIELD_NAME_HOST, hostName);
        config.put(SdbConstants.PMD_OPTION_SVCNAME, Integer.toString(port));
        config.put(SdbConstants.FIELD_NAME_ONLY_ATTACH, true);
        if (configure != null) {
            for (String key : configure.keySet()) {
                if (key.equals(SdbConstants.FIELD_NAME_GROUPNAME)
                    || key.equals(SdbConstants.FIELD_NAME_HOST)
                    || key.equals(SdbConstants.PMD_OPTION_SVCNAME)
                    || key.equals(SdbConstants.FIELD_NAME_ONLY_ATTACH)) {
                    continue;
                }

                config.put(key, configure.get(key));
            }
        }

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_NODE, config);
        SdbReply response = sequoiadb.requestAndResponse(request);
        String msg = "node = " + hostName + ":" + port + ", configure = " + configure;
        sequoiadb.throwIfError(response, msg);
        return getNode(hostName, port);
    }

    /**
     * @param hostName  host name
     * @param port      port
     * @param configure configuration for this operation
     * @return void
     * @throws com.sequoiadb.exception.BaseException
     * @fn void detachNode(String hostName, int port,
     * BSONObject configure)
     * @brief Detach node.
     */
    public void detachNode(String hostName, int port,
                           BSONObject configure) throws BaseException {
        BSONObject config = new BasicBSONObject();
        config.put(SdbConstants.FIELD_NAME_GROUPNAME, name);
        config.put(SdbConstants.FIELD_NAME_HOST, hostName);
        config.put(SdbConstants.PMD_OPTION_SVCNAME,
            Integer.toString(port));
        config.put(SdbConstants.FIELD_NAME_ONLY_DETACH, true);
        if (configure != null) {
            for (String key : configure.keySet()) {
                if (key.equals(SdbConstants.FIELD_NAME_GROUPNAME)
                    || key.equals(SdbConstants.FIELD_NAME_HOST)
                    || key.equals(SdbConstants.PMD_OPTION_SVCNAME)
                    || key.equals(SdbConstants.FIELD_NAME_ONLY_DETACH)) {
                    continue;
                }

                config.put(key, configure.get(key));
            }
        }

        AdminRequest request = new AdminRequest(AdminCommand.REMOVE_NODE, config);
        SdbReply response = sequoiadb.requestAndResponse(request);
        String msg = "node = " + hostName + ":" + port + ", configure = " + configure;
        sequoiadb.throwIfError(response, msg);
    }

    /**
     * @param hostName  host name
     * @param port      port
     * @param dbPath    the path for node
     * @param configure configuration for this operation
     * @return the created Node object
     * @throws com.sequoiadb.exception.BaseException
     * @fn Node createNode(String hostName, int port, String dbPath,
     * Map<String, String> configure)
     * @brief Create node.
     * @deprecated we have override this api by passing a "BSONObject" instead of a "Map"
     */
    public Node createNode(String hostName, int port, String dbPath,
                           Map<String, String> configure) throws BaseException {
        BSONObject obj = new BasicBSONObject();
        if (configure != null) {
            for (String key : configure.keySet()) {
                obj.put(key, configure.get(key));
            }
        }
    	return createNode(hostName, port, dbPath, obj);
    }

    /**
     * @param hostName  host name
     * @param port      port
     * @param dbPath    the path for node
     * @param configure configuration for this operation
     * @return the created Node object
     * @throws com.sequoiadb.exception.BaseException
     * @fn Node createNode(String hostName, int port, String dbPath,
     * BSONObject configure)
     * @brief Create node.
     */
    public Node createNode(String hostName, int port, String dbPath,
                           BSONObject configure) throws BaseException {
        BSONObject config = new BasicBSONObject();
        config.put(SdbConstants.FIELD_NAME_GROUPNAME, name);
        config.put(SdbConstants.FIELD_NAME_HOST, hostName);
        config.put(SdbConstants.PMD_OPTION_SVCNAME,
            Integer.toString(port));
        config.put(SdbConstants.PMD_OPTION_DBPATH, dbPath);
        if (configure != null && !configure.isEmpty()) {
            for (String key : configure.keySet()) {
                if (key.equals(SdbConstants.FIELD_NAME_GROUPNAME)
                    || key.equals(SdbConstants.FIELD_NAME_HOST)
                    || key.equals(SdbConstants.PMD_OPTION_SVCNAME)) {
                    continue;
                }
                config.put(key, configure.get(key));
            }
        }

        AdminRequest request = new AdminRequest(AdminCommand.CREATE_NODE, config);
        SdbReply response = sequoiadb.requestAndResponse(request);
        String msg = "node = " + hostName + ":" + port +
            ", dbPath = " + dbPath +
            ", configure = " + configure;
        sequoiadb.throwIfError(response, msg);
        return getNode(hostName, port);
    }

    /**
     * @param hostName  host name
     * @param port      port
     * @param configure configuration for this operation
     * @throws com.sequoiadb.exception.BaseException
     * @fn void removeNode(String hostName, int port,
     * BSONObject configure)
     * @brief Remove node.
     */
    public void removeNode(String hostName, int port,
                           BSONObject configure) throws BaseException {
        BSONObject config = new BasicBSONObject();
        config.put(SdbConstants.FIELD_NAME_GROUPNAME, name);
        config.put(SdbConstants.FIELD_NAME_HOST, hostName);
        config.put(SdbConstants.PMD_OPTION_SVCNAME,
            Integer.toString(port));
        if (configure != null) {
            for (String key : configure.keySet()) {
                if (key.equals(SdbConstants.FIELD_NAME_GROUPNAME)
                    || key.equals(SdbConstants.FIELD_NAME_HOST)
                    || key.equals(SdbConstants.PMD_OPTION_SVCNAME)) {
                    continue;
                }
                config.put(key, configure.get(key));
            }
        }

        AdminRequest request = new AdminRequest(AdminCommand.REMOVE_NODE, config);
        SdbReply response = sequoiadb.requestAndResponse(request);
        String msg = "node = " + hostName + ":" + port +
            ", configure = " + configure;
        sequoiadb.throwIfError(response, msg);
    }

    /**
     * @return void
     * @throws com.sequoiadb.exception.BaseException
     * @fn void start()
     * @brief Start current replica group.
     */
    public void start() throws BaseException {
        BSONObject groupName = new BasicBSONObject();
        groupName.put(SdbConstants.FIELD_NAME_GROUPNAME, name);

        AdminRequest request = new AdminRequest(AdminCommand.ACTIVE_GROUP, groupName);
        SdbReply response = sequoiadb.requestAndResponse(request);
        sequoiadb.throwIfError(response, name);
    }

    /**
     * @return void
     * @throws com.sequoiadb.exception.BaseException
     * @fn void stop()
     * @brief Stop current replica group.
     */
    public void stop() throws BaseException {
        BSONObject groupName = new BasicBSONObject();
        groupName.put(SdbConstants.FIELD_NAME_GROUPNAME, name);

        AdminRequest request = new AdminRequest(AdminCommand.SHUTDOWN_GROUP, groupName);
        SdbReply response = sequoiadb.requestAndResponse(request);
        sequoiadb.throwIfError(response, name);
    }

    /**
     * @return true is while false is not
     * @fn boolean isCatalog()
     * @brief Judge whether current replicaGroup is catalog replica group or not.
     */
    public boolean isCatalog() {
        return isCataRG;
    }

    private int getNodePort(BSONObject node) {
        if (node == null) {
            throw new BaseException(SDBError.SDB_SYS, "invalid information of node");
        }
        Object services = node.get(SdbConstants.FIELD_NAME_GROUPSERVICE);
        if (services == null)
            throw new BaseException(SDBError.SDB_SYS, node.toString());
        BasicBSONList serviceInfos = (BasicBSONList) services;
        if (serviceInfos.size() == 0)
            throw new BaseException(SDBError.SDB_CLS_NODE_NOT_EXIST);
        int port = -1;
        for (Object obj : serviceInfos) {
            BSONObject service = (BSONObject) obj;
            if (service.get(SdbConstants.FIELD_NAME_SERVICETYPE)
                .toString().equals("0")) {
                port = Integer.parseInt(service.get(
                    SdbConstants.FIELD_NAME_SERVICENAME).toString());
                break;
            }
        }
        if (port == -1) {
            throw new BaseException(SDBError.SDB_SYS, node.toString());
        }
        return port;
    }
}
