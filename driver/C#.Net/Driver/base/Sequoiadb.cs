using System.Collections.Generic;
using System.Security.Cryptography;
using System.Text;
using System;
using SequoiaDB.Bson;

/** \namespace SequoiaDB
 *  \brief SequoiaDB Driver for C#.Net
 *  \author Hetiu Lin
 */
namespace SequoiaDB
{
    /** \class Sequoiadb
     *  \brief Database operation interfaces of admin
     */
    public class Sequoiadb
   {
        private readonly Logger logger = new Logger("Sequoiadb");

        private ServerAddress serverAddress = null;
        private IConnection connection = null;
        internal bool isBigEndian = false ;

        public IConnection Connection
        {
            get { return connection; }
        }

        /** \property ServerAddr
         *  \brief Return or group the remote server address
         *  \return The ServerAddress
         */
        public ServerAddress ServerAddr
        {
            get { return serverAddress; }
            set { serverAddress = value; }
        }

        /** \fn Sequoiadb()
         *  \brief Default Constructor
         *  
         * Server address "127.0.0.1 : 50000"
         */
        public Sequoiadb()
        {
            serverAddress = new ServerAddress();
        }

        /** \fn Sequoiadb(string connString)
         *  \brief Constructor
         *  \param connString Remote server address "IP : Port" or "IP"(port is 50000)
         */
        public Sequoiadb(string connString)
        {
            serverAddress = new ServerAddress(connString);
        }

        /** \fn Sequoiadb(string host, int port)
         *  \brief Constructor
         *  \param addr IP address
         *  \param port Port
         */
        public Sequoiadb(string host, int port)
        {
            serverAddress = new ServerAddress(host, port);
        }

        /** \fn void Connect()
         *  \brief Connect to remote Sequoiadb database server
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void Connect()
        {
            Connect("", "");
        }

        /** \fn void Connect(string username, string password)
         *  \brief Connect to remote Sequoiadb database server
         *  \username Sequoiadb connection user name
         *  \password Sequoiadb connection password
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void Connect(string username, string password)
        {
            if (username == null)
                username = "";
            if (password == null)
                password = "";
            if (connection == null)
            {
                ConfigOptions options = new ConfigOptions();
                try
                {
                    connection = new ConnectionTCPImpl(serverAddress, options);
                    connection.Connect();
                }
                catch (System.Exception e)
                {
                    connection = null;
                    throw e;
                }
                isBigEndian = RequestSysInfo();
                MD5 md5 = MD5.Create();
                byte[] data = md5.ComputeHash(Encoding.Default.GetBytes(password));
                StringBuilder builder = new StringBuilder();
                for ( int i = 0; i < data.Length; i++ )
                    builder.Append(data[i].ToString("x2"));
                byte[] request = SDBMessageHelper.BuildAuthMsg(0, username, builder.ToString(), isBigEndian);
                connection.SendMessage(request);
                SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

                int flags = rtnSDBMessage.Flags;
                if (flags != 0)
                    throw new BaseException(flags);
            }
        }

        /** \fn Disconnect()
         *  \brief Disconnect the remote server
         *  \exception System.Exception
         */
        public void Disconnect()
        {
            byte[] request = SDBMessageHelper.BuildDisconnectRequest(isBigEndian);
            connection.SendMessage(request);
            connection.Close();
            connection = null;
        }

        /** \fn void CreateUser(string username, string password)
         *  \brief Add an user in current database
         *  \username Sequoiadb connection user name
         *  \password Sequoiadb connection password
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void CreateUser(string username, string password)
        {
            if (username == null || password == null)
                throw new BaseException("SDB_INVALIDARG");
            MD5 md5 = MD5.Create();
            byte[] data = md5.ComputeHash(Encoding.Default.GetBytes(password));
            StringBuilder builder = new StringBuilder();
            for (int i = 0; i < data.Length; i++)
                builder.Append(data[i].ToString("x2"));
            byte[] request = SDBMessageHelper.BuildAuthCrtMsg(0, username, builder.ToString(), isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void RemoveUser(string username, string password)
         *  \brief Remove the user from current database
         *  \username Sequoiadb connection user name
         *  \password Sequoiadb connection password
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void RemoveUser(string username, string password)
        {
            if (username == null || password == null)
                throw new BaseException("SDB_INVALIDARG");
            MD5 md5 = MD5.Create();
            byte[] data = md5.ComputeHash(Encoding.Default.GetBytes(password));
            StringBuilder builder = new StringBuilder();
            for (int i = 0; i < data.Length; i++)
                builder.Append(data[i].ToString("x2"));
            byte[] request = SDBMessageHelper.BuildAuthDelMsg(0, username, builder.ToString(), isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void TransactionBegin()
         *  \brief Begin the database transaction
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void TransactionBegin()
        {
            byte[] request = SDBMessageHelper.BuildTransactionRequest(Operation.OP_TRANS_BEGIN, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void TransactionCommit()
         *  \brief Commit the database transaction
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void TransactionCommit()
        {
            byte[] request = SDBMessageHelper.BuildTransactionRequest(Operation.OP_TRANS_COMMIT, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void TransactionRollback()
         *  \brief Rollback the database transaction
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void TransactionRollback()
        {
            byte[] request = SDBMessageHelper.BuildTransactionRequest(Operation.OP_TRANS_ROLLBACK, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void ChangeConnectionOptions(ConfigOptions opts)
         *  \brief Change the connection options
         *  \param opts The connection options
         *  \exception System.Exception
         */
        public void ChangeConnectionOptions(ConfigOptions opts)
        {
            connection.ChangeConfigOptions(opts);
        }

        /** \fn void CreateCollectionSpace(string csName)
         *  \brief Create the named collection space with default SDB_PAGESIZE_4K
         *  \param csName The collection space name
         *  \return The collection space handle
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public CollectionSpace CreateCollectionSpace(string csName) 
        {
            return CreateCollectionSpace(csName, SDBConst.SDB_PAGESIZE_DEFAULT);
        }

        /** \fn void CreateCollectionSpace(string csName, int pageSize)
         *  \brief Create the named collection space
         *  \param csName The collection space name
         *  \param pageSize The Page Size as below
         *  
         *        SDB_PAGESIZE_4K
         *        SDB_PAGESIZE_8K
         *        SDB_PAGESIZE_16K
         *        SDB_PAGESIZE_32K
         *        SDB_PAGESIZE_64K
         *        SDB_PAGESIZE_DEFAULT
         *  \return The collection space handle
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public CollectionSpace CreateCollectionSpace(string csName, int pageSize)
        {
            if (csName == null ||
                pageSize != SDBConst.SDB_PAGESIZE_4K &&
                pageSize != SDBConst.SDB_PAGESIZE_8K &&
                pageSize != SDBConst.SDB_PAGESIZE_16K &&
                pageSize != SDBConst.SDB_PAGESIZE_32K &&
                pageSize != SDBConst.SDB_PAGESIZE_64K)
            {
                throw new BaseException("SDB_INVALIDARG");
            }

            SDBMessage rtn = CreateCS(csName, pageSize);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);

            return new CollectionSpace(this, csName.Trim());
        }

        /** \fn void DropCollectionSpace(string csName)
         *  \brief Remove the named collection space
         *  \param csName The collection space name
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void DropCollectionSpace(string csName) 
        {
            SDBMessage rtn = AdminCommand(SequoiadbConstants.DROP_CMD, SequoiadbConstants.COLSPACE, csName);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn CollectionSpace GetCollecitonSpace(string csName)
         *  \brief Get the named collection space
         *  \param csName The collection space name
         *  \return The CollecionSpace handle
         *  \note If collection space not exit, throw BaseException
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public CollectionSpace GetCollecitonSpace(string csName) 
        {
            if (IsCollectionSpaceExist(csName))
                return new CollectionSpace(this, csName.Trim());
            else
                throw new BaseException("SDB_DMS_CS_NOTEXIST") ;
        }

        /** \fn bool IsCollectionSpaceExist(string csName)
         *  \brief Verify the existence of collection space
         *  \param csName The collecion space name
         *  \return True if existed or False if not existed
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public bool IsCollectionSpaceExist(string csName)
        {
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.TEST_CMD + " "
                             + SequoiadbConstants.COLSPACE;
            BsonDocument condition = new BsonDocument();
            BsonDocument dummyObj = new BsonDocument();
            condition.Add(SequoiadbConstants.FIELD_NAME, csName);
            SDBMessage rtn = AdminCommand(command, condition, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags == 0)
                return true;
            else if (flags == (int)Errors.errors.SDB_DMS_CS_NOTEXIST)
                return false;
            else
                throw new BaseException(flags);
        }
        /** \fn DBCursor ListCollectionSpaces()
         *  \brief List all the collecion space
         *  \rerurn A DBCursor of all the collection space or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor ListCollectionSpaces()
        {
            return GetList(SDBConst.SDB_LIST_COLLECTIONSPACES);
        }

        /** \fn DBCursor ListCollections()
         *  \brief List all the collecion space
         *  \rerurn A DBCursor of all the collection or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor ListCollections()
        {
            return GetList(SDBConst.SDB_LIST_COLLECTIONS);
        }

        /** \fn DBCursor Exec(string sql)
         *  \brief Executing SQL command
         *  \param sql SQL command
         *  \return The DBCursor of matching documents or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor Exec(string sql)
        {
            SDBMessage sdbMessage = new SDBMessage();
            sdbMessage.RequestID = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;

            byte[] request = SDBMessageHelper.BuildSqlMsg(sdbMessage, sql, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
            {
                if (flags == SequoiadbConstants.SDB_DMS_EOC)
                    return null;
                else
                {
                    if (logger.IsDebugEnabled)
                        logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                    throw new BaseException(flags);
                }
            }
            if (  null == rtnSDBMessage.ContextIDList ||
                  rtnSDBMessage.ContextIDList.Count != 1 ||
                  rtnSDBMessage.ContextIDList[0] == -1 )
                  return null ;
            return new DBCursor(rtnSDBMessage, this);
        }

        /** \fn void ExecUpdate(string sql)
         *  \brief Executing SQL command for updating
         *  \param sql SQL command
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void ExecUpdate(string sql)
        {
            SDBMessage sdbMessage = new SDBMessage();
            sdbMessage.RequestID = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;

            byte[] request = SDBMessageHelper.BuildSqlMsg(sdbMessage, sql, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn DBCursor ListReplicaGroups()
         *  \brief Get all the groups
         *  \return A cursor of all the groups
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor ListReplicaGroups()
        {
            BsonDocument dummyObj = new BsonDocument();
            return GetList(SDBConst.SDB_LIST_GROUPS, dummyObj, dummyObj, dummyObj);
        }

        /** \fn ReplicaGroup GetReplicaGroup(string groupName)
         *  \brief Get the ReplicaGroup by name
         *  \param groupName The group name
         *  \return The fitted ReplicaGroup or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public ReplicaGroup GetReplicaGroup(string groupName)
        {
            BsonDocument matcher = new BsonDocument();
            BsonDocument dummyobj = new BsonDocument();
            matcher.Add(SequoiadbConstants.FIELD_GROUPNAME, groupName);
            DBCursor cursor = GetList(SDBConst.SDB_LIST_GROUPS, matcher, dummyobj, dummyobj);
            if (cursor != null)
            {
                BsonDocument detail = cursor.Next();
                if (detail != null)
                {
                    try
                    {
                        if (!detail[SequoiadbConstants.FIELD_GROUPID].IsInt32)
                            throw new BaseException("SDB_SYS");
                        int groupID = detail[SequoiadbConstants.FIELD_GROUPID].AsInt32;
                        return new ReplicaGroup(this, groupName, groupID);
                    }
                    catch (KeyNotFoundException)
                    {
                        throw new BaseException("SDB_SYS");
                    }
                }
                else
                    return null;
            }
            else
                throw new BaseException("SDB_SYS");
        }

        /** \fn ReplicaGroup GetReplicaGroup(int groupID)
         *  \brief Get the ReplicaGroup by ID
         *  \param groupID The group ID
         *  \return The fitted ReplicaGroup or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public ReplicaGroup GetReplicaGroup(int groupID)
        {
            BsonDocument matcher = new BsonDocument();
            BsonDocument dummyobj = new BsonDocument();
            matcher.Add(SequoiadbConstants.FIELD_GROUPID, groupID);
            DBCursor cursor = GetList(SDBConst.SDB_LIST_GROUPS, matcher, dummyobj, dummyobj);
            if (cursor != null)
            {
                BsonDocument detail = cursor.Next();
                if (detail != null)
                    try
                    {
                        if (!detail[SequoiadbConstants.FIELD_GROUPNAME].IsString)
                            throw new BaseException("SDB_SYS");
                        string groupName = detail[SequoiadbConstants.FIELD_GROUPNAME].AsString;
                        return new ReplicaGroup(this, groupName, groupID);
                    }
                    catch (KeyNotFoundException)
                    {
                        throw new BaseException("SDB_SYS");
                    }
                else
                    return null;
            }
            else
                throw new BaseException("SDB_SYS");
        }

        /** \fn ReplicaGroup CreateReplicaGroup(string groupName)
         *  \brief Create the ReplicaGroup with given name
         *  \param groupName The group name
         *  \return The ReplicaGroup has been created succefully
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public ReplicaGroup CreateReplicaGroup(string groupName)
        {
            if (groupName == null)
                throw new BaseException("SDB_INVALIDARG");
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.CREATE_CMD + " "
                             + SequoiadbConstants.GROUP;
            BsonDocument condition = new BsonDocument();
            condition.Add(SequoiadbConstants.FIELD_GROUPNAME, groupName);
            BsonDocument dummyObj = new BsonDocument();

            SDBMessage rtn = AdminCommand(command, condition, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
            else
                return GetReplicaGroup(groupName);
        }

        /** \fn ReplicaGroup RemoveReplicaGroup(string groupName)
         *  \brief Remove the ReplicaGroup with given name
         *  \param groupName The group name
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         *  \note We can't remove a replica group which has data
         */
        public void RemoveReplicaGroup(string groupName)
        {
            if (groupName == null)
                throw new BaseException("SDB_INVALIDARG");
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.REMOVE_CMD + " "
                             + SequoiadbConstants.GROUP;
            BsonDocument condition = new BsonDocument();
            condition.Add(SequoiadbConstants.FIELD_GROUPNAME, groupName);
            BsonDocument dummyObj = new BsonDocument();

            SDBMessage rtn = AdminCommand(command, condition, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }
        /** \fn void CreateReplicaCataGroup(string hostName, int port, string dbpath,
                                            BsonDocument configure) 
         *  \brief Create the Replica Catalog Group with given options
         *  \param hostName The host name
         *  \param port The port
         *  \param dbpath The database path
         *  \param configure The configure options
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void CreateReplicaCataGroup(string hostName, int port, string dbpath,
                                            BsonDocument configure)
        {
            if (hostName == null || port == 0 || dbpath == null)
                throw new BaseException("SDB_INVALIDARG");
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.CREATE_CMD + " "
                             + SequoiadbConstants.CATALOG + " " + SequoiadbConstants.GROUP;
            BsonDocument condition = new BsonDocument();
            condition.Add(SequoiadbConstants.FIELD_HOSTNAME, hostName);
            condition.Add(SequoiadbConstants.SVCNAME, port.ToString());
            condition.Add(SequoiadbConstants.DBPATH, dbpath);
            if (configure != null)
            {
                IEnumerator<BsonElement> it = configure.GetEnumerator();
                while (it.MoveNext())
                {
                    BsonElement e = it.Current;
                    if (e.Name == SequoiadbConstants.FIELD_HOSTNAME ||
                         e.Name == SequoiadbConstants.SVCNAME ||
                         e.Name == SequoiadbConstants.DBPATH)
                        continue;
                    condition.Add(e.Name, e.Value.AsString);
                }
            }

            BsonDocument dummyObj = new BsonDocument();

            SDBMessage rtn = AdminCommand(command, condition, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);    
        }

        /** \fn ReplicaGroup ActivateReplicaGroup(string groupName)
         *  \brief Activate the ReplicaGroup with given name
         *  \param groupName The group name
         *  \return The ReplicaGroup has been activated if succeed or null if fail
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public ReplicaGroup ActivateReplicaGroup(string groupName)
        {
            ReplicaGroup group = GetReplicaGroup(groupName);
            bool result = group.Start();
            if (result)
                return group;
            else
                return null;
        }

        /** \fn DBCursor GetSnapshot(int snapType, BsonDocument matcher, BsonDocument selector,
                                          BsonDocument orderBy)
         *  \brief Get the snapshots of specified type
         *  \param snapType The specified type as below:
         *  
         *      SDB_SNAP_CONTEXTS
         *      SDB_SNAP_CONTEXTS_CURRENT
         *      SDB_SNAP_SESSIONS
         *      SDB_SNAP_SESSIONS_CURRENT
         *      SDB_SNAP_COLLECTIONS
         *      SDB_SNAP_COLLECTIONSPACES
         *      SDB_SNAP_DATABASE
         *      SDB_SNAP_SYSTEM
         *  \param matcher The matching condition or null
         *  \param selector The selective rule or null
         *  \param orderBy The ordered rule or null
         *  \return A DBCursor of all the fitted objects or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor GetSnapshot(int snapType, BsonDocument matcher, BsonDocument selector,
                                          BsonDocument orderBy)
        {
            string command = null;
            switch (snapType)
            {
                case SDBConst.SDB_SNAP_CONTEXTS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.CONTEXTS;
                    break;
                case SDBConst.SDB_SNAP_CONTEXTS_CURRENT:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.CONTEXTS_CUR;
                    break;
                case SDBConst.SDB_SNAP_SESSIONS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.SESSIONS;
                    break;
                case SDBConst.SDB_SNAP_SESSIONS_CURRENT:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.SESSIONS_CUR;
                    break;
                case SDBConst.SDB_SNAP_COLLECTIONS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.COLLECTIONS;
                    break;
                case SDBConst.SDB_SNAP_COLLECTIONSPACES:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.COLSPACES;
                    break;
                case SDBConst.SDB_SNAP_DATABASE:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.DATABASE;
                    break;
                case SDBConst.SDB_SNAP_SYSTEM:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.SYSTEM;
                    break;
                case SDBConst.SDB_SNAP_CATALOG:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " " +
                           SequoiadbConstants.CATA;
                    break;
                default:
                    throw new BaseException("SDB_INVALIDARG");
            }

            BsonDocument dummyObj = new BsonDocument();
            if (matcher == null)
                matcher = dummyObj;
            if (selector == null)
                selector = dummyObj;
            if (orderBy == null)
                orderBy = dummyObj;
            SDBMessage rtn = AdminCommand(command, matcher, selector, orderBy, dummyObj);

            int flags = rtn.Flags;
            if (flags != 0)
                if (flags == SequoiadbConstants.SDB_DMS_EOC)
                    return null;
                else
                {
                    if (logger.IsDebugEnabled)
                        logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                    throw new BaseException(flags);
                }

            return new DBCursor(rtn, this);
        }

        /** \fn DBCursor GetList(int listType)
         *  \brief Get the informations of specified type
         *  \param listType The specified type as below:
         *  
         *      SDB_LIST_CONTEXTS
         *      SDB_LIST_CONTEXTS_CURRENT
         *      SDB_LIST_SESSIONS
         *      SDB_LIST_SESSIONS_CURRENT
         *      SDB_LIST_COLLECTIONS
         *      SDB_LIST_COLLECTIONSPACES
         *      SDB_LIST_STORAGEUNITS
         *      SDB_LIST_GROUPS
         *  \return A DBCursor of all the fitted objects or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor GetList(int listType)
        {
            BsonDocument dummyObj = new BsonDocument();
            return GetList(listType, dummyObj, dummyObj, dummyObj);
        }

        /** \fn DBCursor GetList(int listType, BsonDocument matcher, BsonDocument selector,
                                          BsonDocument orderBy)
         *  \brief Get the informations of specified type
         *  \param listType The specified type as below:
         *  
         *      SDB_LIST_CONTEXTS
         *      SDB_LIST_CONTEXTS_CURRENT
         *      SDB_LIST_SESSIONS
         *      SDB_LIST_SESSIONS_CURRENT
         *      SDB_LIST_COLLECTIONS
         *      SDB_LIST_COLLECTIONSPACES
         *      SDB_LIST_STORAGEUNITS
         *      SDB_LIST_GROUPS
         *  \param matcher The matching condition or null
         *  \param selector The selective rule or null
         *  \param orderBy The ordered rule or null
         *  \return A DBCursor of all the fitted objects or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor GetList(int listType, BsonDocument matcher, BsonDocument selector,
                                          BsonDocument orderBy)
        {
            string command = null;
            switch (listType)
            {
                case SDBConst.SDB_LIST_CONTEXTS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " +
                           SequoiadbConstants.CONTEXTS;
                    break;
                case SDBConst.SDB_LIST_CONTEXTS_CURRENT:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " +
                           SequoiadbConstants.CONTEXTS_CUR;
                    break;
                case SDBConst.SDB_LIST_SESSIONS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " +
                           SequoiadbConstants.SESSIONS;
                    break;
                case SDBConst.SDB_LIST_SESSIONS_CURRENT:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " +
                           SequoiadbConstants.SESSIONS_CUR;
                    break;
                case SDBConst.SDB_LIST_COLLECTIONS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " + 
                           SequoiadbConstants.COLLECTIONS;
                    break;
                case SDBConst.SDB_LIST_COLLECTIONSPACES:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " + 
                           SequoiadbConstants.COLSPACES;
                    break;
                case SDBConst.SDB_LIST_STORAGEUNITS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " + 
                           SequoiadbConstants.STOREUNITS;
                    break;
                case SDBConst.SDB_LIST_GROUPS:
                    command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.LIST_CMD + " " + 
                           SequoiadbConstants.GROUPS;
                    break;
                default:
                    throw new BaseException("SDB_INVALIDARG");
            }

            BsonDocument dummyObj = new BsonDocument();
            if (matcher == null)
                matcher = dummyObj;
            if (selector == null)
                selector = dummyObj;
            if (orderBy == null)
                orderBy = dummyObj;
            SDBMessage rtn = AdminCommand(command, matcher, selector, orderBy, dummyObj);

            int flags = rtn.Flags;
            if (flags != 0)
                if (flags == SequoiadbConstants.SDB_DMS_EOC)
                    return null;
                else
                {
                    if (logger.IsDebugEnabled)
                        logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                    throw new BaseException(flags);
                }

            return new DBCursor(rtn, this);
        }

        /** \fn void ResetSnapshot( BsonDocument matcher )
         *  \brief Reset the snapshot
         *  \param matcher The matching condition 
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void ResetSnapshot( BsonDocument matcher )
        {
            BsonDocument dummyObj = new BsonDocument();
            if (matcher == null)
                matcher = dummyObj;
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SNAP_CMD + " "
                             + SequoiadbConstants.RESET;
            SDBMessage rtn = AdminCommand(command, matcher, dummyObj, dummyObj, dummyObj);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        private SDBMessage CreateCS(string csName, int pageSize)
        {
            string commandString = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.CREATE_CMD + " " + SequoiadbConstants.COLSPACE;
            BsonDocument cObj = new BsonDocument();
            BsonDocument dummyObj = new BsonDocument();
            SDBMessage sdbMessage = new SDBMessage();

            cObj.Add(SequoiadbConstants.FIELD_NAME, csName);
            cObj.Add(SequoiadbConstants.FIELD_PAGESIZE, pageSize);
            sdbMessage.Matcher = cObj;
            sdbMessage.CollectionFullName = commandString;

            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.Flags = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            sdbMessage.SkipRowsCount = 0;
            sdbMessage.ReturnRowsCount = -1;
            sdbMessage.Selector = dummyObj;
            sdbMessage.OrderBy = dummyObj;
            sdbMessage.Hint = dummyObj;

            byte[] request = SDBMessageHelper.BuildQueryRequest(sdbMessage, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

            return rtnSDBMessage;
        }

        private SDBMessage AdminCommand(string cmdType, string contextType, string contextName)
        {
            BsonDocument dummyObj = new BsonDocument();
            SDBMessage sdbMessage = new SDBMessage();
            string commandString = SequoiadbConstants.ADMIN_PROMPT + cmdType + " " + contextType;
            if (!(cmdType.Equals(SequoiadbConstants.LIST_CMD) || cmdType.Equals(SequoiadbConstants.SNAP_CMD)))
            {
                BsonDocument cObj = new BsonDocument();
                cObj.Add(SequoiadbConstants.FIELD_NAME, contextName);
                sdbMessage.Matcher = cObj;
            }
            else
                sdbMessage.Matcher = dummyObj;
            sdbMessage.CollectionFullName = commandString;

            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.Flags = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            sdbMessage.SkipRowsCount = -1;
            sdbMessage.ReturnRowsCount = -1;
            sdbMessage.Selector = dummyObj;
            sdbMessage.OrderBy = dummyObj;
            sdbMessage.Hint = dummyObj;

            byte[] request = SDBMessageHelper.BuildQueryRequest(sdbMessage, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

            return rtnSDBMessage;
        }

        private SDBMessage AdminCommand(string command, BsonDocument matcher, BsonDocument selector,
                                        BsonDocument orderBy, BsonDocument hint)
        {
            SDBMessage sdbMessage = new SDBMessage();
            sdbMessage.CollectionFullName = command;
            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.Flags = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            sdbMessage.SkipRowsCount = 0;
            sdbMessage.ReturnRowsCount = -1;
            sdbMessage.Matcher = matcher;
            sdbMessage.Selector = selector;
            sdbMessage.OrderBy = orderBy;
            sdbMessage.Hint = hint;

            byte[] request = SDBMessageHelper.BuildQueryRequest(sdbMessage, isBigEndian);
            if(connection == null)
                throw new BaseException("SDB_NETWORK");
            connection.SendMessage(request);
            SDBMessage rtn = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

            return rtn;
        }

        private List<BsonDocument> GetMoreCommand(SDBMessage rtnSDBMessage)
        {
            ulong requestID = rtnSDBMessage.RequestID;
            List<long> contextIDs = rtnSDBMessage.ContextIDList;
            List<BsonDocument> fullList = new List<BsonDocument>();
            bool hasMore = true;
            while (hasMore)
            {
                SDBMessage sdbMessage = new SDBMessage();
                sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
                sdbMessage.ContextIDList = contextIDs;
                sdbMessage.RequestID = requestID;
                sdbMessage.ReturnRowsCount2 = -1;

                byte[] request = SDBMessageHelper.BuildGetMoreRequest(sdbMessage, isBigEndian);
                connection.SendMessage(request);
                rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

                int flags = rtnSDBMessage.Flags;
                if (flags != 0)
                {
                    if (flags == SequoiadbConstants.SDB_DMS_EOC)
                        hasMore = false;
                    else
                    {
                        if (logger.IsDebugEnabled)
                            logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                        throw new BaseException(flags);
                    }
                }
                else
                {
                    requestID = rtnSDBMessage.RequestID;
                    List<BsonDocument> objList = rtnSDBMessage.ObjectList;
                    fullList.AddRange(objList);
                }
            }
            return fullList;
        }

        private bool RequestSysInfo()
        {
            byte[] request = SDBMessageHelper.BuildSysInfoRequest();
            connection.SendMessage(request);
            int osType = 0 ;
            return SDBMessageHelper.ExtractSysInfoReply(connection.ReceiveMessage(128), ref osType );
        }
   }
}
