using System;
using SequoiaDB.Bson;
using System.Collections.Generic;

/** \namespace SequoiaDB
 *  \brief SequoiaDB Driver for C#.Net
 *  \author Hetiu Lin
 */
namespace SequoiaDB
{
    /** \class Shard
     *  \brief Database operation interfaces of shard.This class takes the place of class "replicaGroup".
     *  \note We use concept "shard" instead of "replica group",
     *        and change the class name "ReplicaGroup" to "Shard".
     */
    public class Shard
    {
        private int shardID = -1;
        private string shardName = null;
        private bool isCatalog = false;
        internal bool isBigEndian = false;
        private Sequoiadb sdb = null;

        internal Shard(Sequoiadb sdb, string shardName, int shardID)
        {
            this.sdb = sdb;
            this.shardName = shardName;
            this.shardID = shardID;
            isCatalog = shardName.Equals(SequoiadbConstants.CATALOG_GROUP);
            isBigEndian = sdb.isBigEndian;
        }

        /** \property SequoiaDB
         *  \brief Return the sequoiadb handle of current shard 
         *  \return The Sequoiadb object
         */
        public Sequoiadb SequoiaDB
        {
            get { return sdb; }
        }

        /** \property ShardName
         *  \brief Return the name of current shard
         *  \return The shard name
         */
        public string ShardName
        {
            get { return shardName; }
        }

        /** \property ShardID
         *  \brief Return the shard ID of current shard
         *  \return The shard ID
         */
        public int ShardID
        {
            get { return shardID; }
        }

        /** \property IsCatalog
         *  \brief Verify the role of current shard
         *  \return True if is catalog shard or False if not
         */
        public bool IsCatalog
        {
            get { return isCatalog; }
        }

        /** \fn bool Stop()
         *  \brief Stop the current node
         *  \return True if succeed or False if fail
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public bool Stop()
        {
            bool start = false;
            return StopStart(start);
        }

        /** \fn bool Start()
         *  \brief Start the current node
         *  \return True if succeed or False if fail
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public bool Start()
        {
            bool start = true;
            return StopStart(start);
        }

        /** \fn int GetNodeNum( SDBConst.NodeStatus status)
         *  \brief Get the count of node with given status
         *  \param status The specified status as below:
         *  
         *      SDB_NODE_ALL
         *      SDB_NODE_ACTIVE
         *      SDB_NODE_INACTIVE
         *      SDB_NODE_UNKNOWN
         *  \return The count of node
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public int GetNodeNum(SDBConst.NodeStatus status)
        {
            try
            {
                int total = 0;
                BsonDocument detail = GetDetail();
                if (detail[SequoiadbConstants.FIELD_GROUP].IsBsonArray)
                {
                    BsonArray nodes = detail[SequoiadbConstants.FIELD_GROUP].AsBsonArray;
                    total = nodes.Count;
                    //foreach (BsonDocument node in nodes)
                    //{
                    //    Node rnode = ExtractNode(node);
                    //    SDBConst.NodeStatus sta = rnode.GetStatus();
                    //    if (SDBConst.NodeStatus.SDB_NODE_ALL == status || rnode.GetStatus() == status)
                    //        ++total;
                    //}
                }
                return total;
            }
            catch (KeyNotFoundException)
            {
                throw new BaseException("SDB_CLS_NODE_NOT_EXIST");
            }
            catch (FormatException)
            {
                throw new BaseException("SDB_INVALIDARG");
            }
        }

        /** \fn BsonDocument GetDetail()
         *  \brief Get the detail information of current shard
         *  \return The detail information in BsonDocument object
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public BsonDocument GetDetail()
        {
            BsonDocument matcher = new BsonDocument();
            BsonDocument dummyobj = new BsonDocument();
            matcher.Add(SequoiadbConstants.FIELD_GROUPNAME, shardName);
            matcher.Add(SequoiadbConstants.FIELD_GROUPID, shardID);
            DBCursor cursor = sdb.GetList(SDBConst.SDB_LIST_SHARDS, matcher, dummyobj, dummyobj);
            if (cursor != null)
            {
                BsonDocument detail = cursor.Next();
                if (detail != null)
                    return detail;
                else
                    throw new BaseException("SDB_CLS_GRP_NOT_EXIST");
            }
            else
                throw new BaseException("SDB_SYS");
        }

        /** \fn Node CreateNode(string hostName, int port, string dbpath,
                               Dictionary<string, string> map)
         *  \brief Create the node
         *  \param hostName The host name of node
         *  \param port The port of node
         *  \param dbpath The database path of node
         *  \param map The other configure information of node
         *  \return The Node object
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public Node CreateNode(string hostName, int port, string dbpath,
                               Dictionary<string, string> map)
        {
            if (hostName == null || port < 0 || port < 0 || port > 65535 ||
                dbpath == null)
                throw new BaseException("SDB_INVALIDARG");
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.CREATE_CMD + " "
                             + SequoiadbConstants.NODE;
            BsonDocument configuration = new BsonDocument();
            configuration.Add(SequoiadbConstants.FIELD_GROUPNAME, shardName);
            map.Remove(SequoiadbConstants.FIELD_GROUPNAME);
            configuration.Add(SequoiadbConstants.FIELD_HOSTNAME, hostName);
            map.Remove(SequoiadbConstants.FIELD_HOSTNAME);
            configuration.Add(SequoiadbConstants.SVCNAME, port.ToString());
            map.Remove(SequoiadbConstants.SVCNAME);
            configuration.Add(SequoiadbConstants.DBPATH, dbpath);
            map.Remove(SequoiadbConstants.DBPATH);
            Dictionary<string, string>.Enumerator it = map.GetEnumerator();
            while (it.MoveNext())
                configuration.Add(it.Current.Key, it.Current.Value);
            BsonDocument dummyObj = new BsonDocument();
            SDBMessage rtn = AdminCommand(command, configuration, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
            else
                return GetNode(hostName, port);
        }

        /** \fn void RemoveNode(string hostName, int port,
                       BsonDocument configure)
         *  \brief Remove the specified node
         *  \param hostName The host name of node
         *  \param port The port of node
         *  \param configure The configurations for the node
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void RemoveNode(string hostName, int port,
                               BsonDocument configure)
        {
            if (hostName == null || port < 0 || port < 0 || port > 65535)
                throw new BaseException("SDB_INVALIDARG");
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.REMOVE_CMD + " "
                 + SequoiadbConstants.NODE;
            BsonDocument config = new BsonDocument();
            config.Add(SequoiadbConstants.FIELD_GROUPNAME, shardName);
            config.Add(SequoiadbConstants.FIELD_HOSTNAME, hostName);
            config.Add(SequoiadbConstants.SVCNAME, Convert.ToString(port));
            if (configure != null)
            {
                foreach (string key in configure.Names)
                {
                    if (key.Equals(SequoiadbConstants.FIELD_GROUPNAME) ||
                        key.Equals(SequoiadbConstants.FIELD_HOSTNAME) ||
                        key.Equals(SequoiadbConstants.SVCNAME))
                    {
                        continue;
                    }
                    config.Add(configure.GetElement(key));
                }
            }
            BsonDocument dummyObj = new BsonDocument();
            SDBMessage rtn = AdminCommand(command, config, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn Node GetMaster()
         *  \brief Get the master Node of current shard
         *  \return The fitted Node or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public Node GetMaster()
        {
            int primaryNode = -1;
            try
            {
                BsonDocument detail = GetDetail();
                if (!detail[SequoiadbConstants.FIELD_PRIMARYNODE].IsInt32)
                    throw new BaseException("SDB_SYS");
                primaryNode = detail[SequoiadbConstants.FIELD_PRIMARYNODE].AsInt32;
                if (!detail[SequoiadbConstants.FIELD_GROUP].IsBsonArray)
                    throw new BaseException("SDB_SYS");
                BsonArray nodes = detail[SequoiadbConstants.FIELD_GROUP].AsBsonArray;
                foreach (BsonDocument node in nodes)
                {
                    if (!node[SequoiadbConstants.FIELD_NODEID].IsInt32)
                        throw new BaseException("SDB_SYS");
                    int nodeID = node[SequoiadbConstants.FIELD_NODEID].AsInt32;
                    if (nodeID == primaryNode)
                    {
                        return ExtractNode(node);
                    }
                }
                return null;
            }
            catch (KeyNotFoundException)
            {
                return null;
            }
            catch (FormatException)
            {
                throw new BaseException("SDB_INVALIDARG");
            }
        }

        /** \fn Node GetSlave()
         *  \brief Get the slave Node of current shard
         *  \return The fitted Node or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public Node GetSlave()
        {
            int primaryID = -1;
            List<BsonDocument> nodeList = new List<BsonDocument>();
            BsonDocument primaryNode = null;
            try
            {
                BsonDocument detail = GetDetail();
                if (!detail[SequoiadbConstants.FIELD_PRIMARYNODE].IsInt32)
                    throw new BaseException("SDB_CLS_NODE_NOT_EXIST");
                primaryID = detail[SequoiadbConstants.FIELD_PRIMARYNODE].AsInt32;
                if (!detail[SequoiadbConstants.FIELD_GROUP].IsBsonArray)
                    throw new BaseException("SDB_SYS");
                BsonArray nodes = detail[SequoiadbConstants.FIELD_GROUP].AsBsonArray;
                foreach (BsonDocument node in nodes)
                {
                    if (!node[SequoiadbConstants.FIELD_NODEID].IsInt32)
                        throw new BaseException("SDB_SYS");
                    int nodeID = node[SequoiadbConstants.FIELD_NODEID].AsInt32;
                    if (nodeID != primaryID)
                        nodeList.Add(node);
                    else
                        primaryNode = node;
                }
                if (nodeList.Count > 0)
                {
                    Random rnd = new Random();
                    int slaveID = rnd.Next() % nodeList.Count;
                    return ExtractNode(nodeList[slaveID]);
                }
                else
                    return ExtractNode(primaryNode);
            }
            catch (KeyNotFoundException)
            {
                return null;
            }
            catch (FormatException)
            {
                throw new BaseException("SDB_INVALIDARG");
            }
        }

        /** \fn Node GetNode(string nodeName)
         *  \brief Get the Node by node name
         *  \param nodeName The node name
         *  \return The fitted Node or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public Node GetNode(string nodeName)
        {
            try
            {
                if (!nodeName.Contains(SequoiadbConstants.NODE_NAME_SERVICE_SEP))
                    throw new BaseException("SDB_INVALIDARG");
                string[] hostname = nodeName.Split(SequoiadbConstants.NODE_NAME_SERVICE_SEP[0]);
                if (hostname[1].Equals(string.Empty))
                    throw new BaseException("SDB_INVALIDARG");
                return GetNode(hostname[0].Trim(), int.Parse(hostname[1].Trim()));
            }
            catch (FormatException)
            {
                throw new BaseException("SDB_INVALIDARG");
            }
        }

        /** \fn Node GetNode(string hostName, int port)
         *  \brief Get the Node by host name and port
         *  \param hostName The host name
         *  \param port The port
         *  \return The fitted Node or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public Node GetNode(string hostName, int port)
        {
            try
            {
                BsonDocument detail = GetDetail();
                if (!detail[SequoiadbConstants.FIELD_GROUP].IsBsonArray)
                    throw new BaseException("SDB_SYS");
                BsonArray nodes = detail[SequoiadbConstants.FIELD_GROUP].AsBsonArray;
                foreach (BsonDocument node in nodes)
                {
                    if (!node[SequoiadbConstants.FIELD_HOSTNAME].IsString)
                        throw new BaseException("SDB_SYS");
                    string hostname = node[SequoiadbConstants.FIELD_HOSTNAME].AsString;
                    if (hostname.Equals(hostName))
                    {
                        Node rn = ExtractNode(node);
                        if (rn.Port == port)
                            return rn;
                    }
                }
                return null;
            }
            catch (KeyNotFoundException)
            {
                return null;
            }
        }

        private SDBMessage AdminCommand(string command, BsonDocument arg1, BsonDocument arg2,
                                        BsonDocument arg3, BsonDocument arg4)
        {
            IConnection connection = sdb.Connection;
            SDBMessage sdbMessage = new SDBMessage();

            sdbMessage.Matcher = arg1;
            sdbMessage.Selector = arg2;
            sdbMessage.OrderBy = arg3;
            sdbMessage.Hint = arg4;
            sdbMessage.CollectionFullName = command;
            sdbMessage.Flags = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            sdbMessage.SkipRowsCount = -1;
            sdbMessage.ReturnRowsCount = -1;

            byte[] request = SDBMessageHelper.BuildQueryRequest(sdbMessage, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

            return rtnSDBMessage;
        }

        private Node ExtractNode(BsonDocument node)
        {
            try
            {
                if (!node[SequoiadbConstants.FIELD_HOSTNAME].IsString)
                    throw new BaseException("SDB_SYS");
                string hostName = node[SequoiadbConstants.FIELD_HOSTNAME].AsString;
                if (!node[SequoiadbConstants.FIELD_NODEID].IsInt32)
                    throw new BaseException("SDB_SYS");
                int nodeID = node[SequoiadbConstants.FIELD_NODEID].AsInt32;
                if (!node[SequoiadbConstants.FIELD_SERVICE].IsBsonArray)
                    throw new BaseException("SDB_SYS");
                BsonArray svcs = node[SequoiadbConstants.FIELD_SERVICE].AsBsonArray;
                foreach (BsonDocument svc in svcs)
                {
                    if (!svc[SequoiadbConstants.FIELD_SERVICE_TYPE].IsInt32)
                        throw new BaseException("SDB_SYS");
                    int type = svc[SequoiadbConstants.FIELD_SERVICE_TYPE].AsInt32;
                    if (0 == type)
                    {
                        if (!svc[SequoiadbConstants.FIELD_NAME].IsString)
                            throw new BaseException("SDB_SYS");
                        string svcname = svc[SequoiadbConstants.FIELD_NAME].AsString;
                        return new Node(this, hostName, int.Parse(svcname), nodeID);
                    }
                }
                return null;
            }
            catch (KeyNotFoundException)
            {
                throw new BaseException("SDB_SYS");
            }
            catch (FormatException)
            {
                throw new BaseException("SDB_INVALIDARG");
            }
        }

        private bool StopStart(bool start)
        {
            string command = start ? SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.ACTIVE_CMD + " "
                                    + SequoiadbConstants.GROUP :
                                   SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SHUTDOWN_CMD + " "
                                    + SequoiadbConstants.GROUP;
            BsonDocument configuration = new BsonDocument();
            configuration.Add(SequoiadbConstants.FIELD_GROUPNAME, shardName);
            configuration.Add(SequoiadbConstants.FIELD_GROUPID, shardID);
            BsonDocument dummyObj = new BsonDocument();
            SDBMessage rtn = AdminCommand(command, configuration, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags != 0)
                return false;
            else
                return true;
        }
    }
}
