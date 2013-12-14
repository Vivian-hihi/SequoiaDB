using System.Collections.Generic;
using System;
using SequoiaDB.Bson;

/** \namespace SequoiaDB
 *  \brief SequoiaDB Driver for C#.Net
 *  \author Hetiu Lin
 */
namespace SequoiaDB
{
    /** \class DBCollection
     *  \brief Database operation interfaces of collection
     */
    public class DBCollection
   {
        private string name;
        private string collectionFullName;
        private CollectionSpace collSpace;
        private IConnection connection;
        internal bool isBigEndian = false;

        private readonly Logger logger = new Logger("DBCollection");

        /** \property Name
         *  \brief Return the name of current collection
         *  \return The collection name
         */
        public string Name
        {
            get { return name; }
        }

        /** \property CollSpace
         *  \ brief Return the Collection Space handle of current collection
         *  \return CollectionSpace object
         */
        public CollectionSpace CollSpace
        {
            get { return collSpace; }
        }

        internal DBCollection(CollectionSpace cs, string name)
        {
            this.name = name;
            this.collSpace = cs;
            this.collectionFullName = cs.Name + "." + name;
            this.connection = cs.SequoiaDB.Connection;
            this.isBigEndian = cs.isBigEndian;
        }

        /* \fn void Rename(string newName)
         *  \brief Rename the collection
         *  \param newName The new collection name
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        /*
        public void Rename(string newName)
        {
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.RENAME_CMD + " "
                             + SequoiadbConstants.COLLECTION;
            BsonDocument matcher = new BsonDocument();
            matcher.Add(SequoiadbConstants.FIELD_COLLECTIONSPACE, collSpace.Name);
            matcher.Add(SequoiadbConstants.FIELD_OLDNAME, name);
            matcher.Add(SequoiadbConstants.FIELD_NEWNAME, newName);

            BsonDocument dummyObj = new BsonDocument();
            SDBMessage rtn = AdminCommand(command, matcher, dummyObj, dummyObj, dummyObj, -1, -1);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
            else
            {
                this.name = newName;
                this.collectionFullName = collSpace.Name + "." + newName;
            }
        }
        */

        /** \fn void Split(string sourceGroupName, string destGroupName,
                           BsonDocument splitCondition, BsonDocument splitEndCondition)
         *  \brief Split the collection from one group into another group by range
         *  \param sourceGroupName The source group
         *  \param destGroupName The destination group
         *  \param splitCondition The split condition
         *  \param splitEndCondition The split end condition or null
         *		eg:If we create a collection with the option {ShardingKey:{"age":1},ShardingType:"Hash",Partition:2^10},
    	 *		we can fill {age:30} as the splitCondition, and fill {age:60} as the splitEndCondition. when split, 
    	 *		the targe group will get the records whose age's hash value are in [30,60). If splitEndCondition is null,
    	 *		they are in [30,max).
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void Split(string sourceGroupName, string destGroupName,
            BsonDocument splitCondition, BsonDocument splitEndCondition)
        {
            // check argument
            if ((null == sourceGroupName || sourceGroupName.Equals("")) ||
                (null == destGroupName || destGroupName.Equals("")) ||
                null == splitCondition) {
                    throw new BaseException("SDB_INVALIDARG");
            }
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SPLIT_CMD;
            BsonDocument matcher = new BsonDocument();
            matcher.Add(SequoiadbConstants.FIELD_NAME, collectionFullName);
            matcher.Add(SequoiadbConstants.FIELD_SOURCE, sourceGroupName);
            matcher.Add(SequoiadbConstants.FIELD_TARGET, destGroupName);
            matcher.Add(SequoiadbConstants.FIELD_SPLITQUERY, splitCondition);
            if(null != splitEndCondition)
                matcher.Add(SequoiadbConstants.FIELD_SPLITENDQUERY, splitEndCondition);

            BsonDocument dummyObj = new BsonDocument();
            SDBMessage rtn = AdminCommand(command, matcher, dummyObj, dummyObj, dummyObj, -1, -1);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void Split(string sourceGroupName, string destGroupName, double percent)
         *  \brief Split the collection from one group into another group by percent
         *  \param sourceGroupName The source group
         *  \param destGroupName The destination group
         *  \param percent percent The split percent, Range:(0.0, 100.0]
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void Split(string sourceGroupName, string destGroupName, double percent)
        {
            // check argument
            if ((null == sourceGroupName || sourceGroupName.Equals("")) ||
                (null == destGroupName || destGroupName.Equals("")) ||
                (percent <= 0.0 || percent > 100.0))
            {
                throw new BaseException("SDB_INVALIDARG");
            }
            string command = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.SPLIT_CMD;
            BsonDocument matcher = new BsonDocument();
            matcher.Add(SequoiadbConstants.FIELD_NAME, collectionFullName);
            matcher.Add(SequoiadbConstants.FIELD_SOURCE, sourceGroupName);
            matcher.Add(SequoiadbConstants.FIELD_TARGET, destGroupName);
            matcher.Add(SequoiadbConstants.FIELD_SPLITPERCENT, percent);

            BsonDocument dummyObj = new BsonDocument();
            SDBMessage rtn = AdminCommand(command, matcher, dummyObj, dummyObj, dummyObj, -1, -1);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn ObjectId Insert(BsonDocument insertor)
         *  \brief Insert a document into current collection
         *  \param insertor The Bson document of insertor, can't be null
         *  \return ObjectId
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public ObjectId Insert(BsonDocument insertor) 
        {
            if (insertor == null)
                throw new BaseException("SDB_INVALIDARG");
            SDBMessage sdbMessage = new SDBMessage();
            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.CollectionFullName = collectionFullName;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            sdbMessage.Insertor = insertor;

            ObjectId objId;
            BsonValue tmp;
            if (insertor.TryGetValue(SequoiadbConstants.OID, out tmp))
                objId = tmp.AsObjectId;
            else
            {
                objId = ObjectId.GenerateNewId();
                insertor.Add(SequoiadbConstants.OID, objId);
            }

            byte[] request = SDBMessageHelper.BuildInsertRequest(sdbMessage, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);

            return objId;
        }

        /** \fn void BulkInsert(List<BsonDocument> insertor, int flag)
         *  \brief Insert a bulk of bson objects into current collection
         *  \param insertor The Bson document of insertor list, can't be null
         *  \param flag FLG_INSERT_CONTONDUP or 0
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void BulkInsert(List<BsonDocument> insertor, int flag)
        {
            if ( insertor == null || insertor.Count == 0 )
                throw new BaseException("SDB_INVALIDARG");
            SDBMessage sdbMessage = new SDBMessage();
            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.CollectionFullName = collectionFullName;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            if (flag != 0 && flag != SDBConst.FLG_INSERT_CONTONDUP)
                throw new BaseException(flag);
            sdbMessage.Flags = flag;
            sdbMessage.Insertor = insertor[0];

            byte[] request = SDBMessageHelper.BuildInsertRequest(sdbMessage, isBigEndian);

            for (int count = 1; count < insertor.Count; count++)
            {
                request = SDBMessageHelper.AppendInsertMsg(request, insertor[count], isBigEndian);
            }
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void Delete(BsonDocument matcher)
         *  \brief Delete the matching document of current collection
         *  \param matcher The matching condition
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void Delete(BsonDocument matcher)
        {
            Delete(matcher, new BsonDocument());
        }

        /** \fn void Delete(BsonDocument matcher, BsonDocument hint)
         *  \brief Delete the matching document of current collection
         *  \param matcher The matching condition
         *  \param hint Hint
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void Delete(BsonDocument matcher, BsonDocument hint)
        {
            SDBMessage sdbMessage = new SDBMessage();
            BsonDocument dummyObj = new BsonDocument();
            if (matcher == null)
                matcher = dummyObj;
            if (hint == null)
                hint = dummyObj;        

            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.Flags = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;

            sdbMessage.CollectionFullName = collectionFullName;
            sdbMessage.RequestID = 0;
            sdbMessage.Matcher = matcher;
            sdbMessage.Hint = hint;

            byte[] request = SDBMessageHelper.BuildDeleteRequest(sdbMessage, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
            {
                throw new BaseException(flags);
            }
       }

        /** \fn void Update(DBQuery query)
         *  \brief Update the document of current collection
         *  \param query DBQuery with matching condition, updating rule and hint
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         *  \note It won't work to update the "ShardingKey" field, but the other fields take effect
         */
        public void Update(DBQuery query) 
        {
            _Update(0, query.Matcher, query.Modifier, query.Hint);
        }

        /** \fn void Update(BsonDocument matcher, BsonDocument modifier, BsonDocument hint)
         *  \brief Update the document of current collection
         *  \param matcher The matching condition
         *  \param modifier The updating rule
         *  \param hint Hint
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         *  \note It won't work to update the "ShardingKey" field, but the other fields take effect
         */
        public void Update(BsonDocument matcher, BsonDocument modifier, BsonDocument hint) 
        {
            _Update(0, matcher, modifier, hint);
        }

        /** \fn void Upsert(BsonDocument matcher, BsonDocument modifier, BsonDocument hint)
         *  \brief Update the document of current collection, insert if no matching
         *  \param matcher The matching condition
         *  \param modifier The updating rule
         *  \param hint Hint
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         *  \note It won't work to upsert the "ShardingKey" field, but the other fields take effect
         */
        public void Upsert(BsonDocument matcher, BsonDocument modifier, BsonDocument hint)
        {
            _Update(SequoiadbConstants.FLG_UPDATE_UPSERT, matcher, modifier, hint);
        }

        /** \fn DBCursor Query()
         *  \brief Find all documents of current collection
         *  \return The DBCursor of matching documents or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor Query()
        {
            return Query(null, null, null, null, 0, -1);
        }

        /** \fn DBCursor Query(DBQuery query) 
         *  \brief Find documents of current collection with DBQuery
         *  \param query DBQuery with matching condition, selector, order rule, hint, SkipRowsCount and ReturnRowsCount
         *  \return The DBCursor of matching documents or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor Query(DBQuery query)
        {
            BsonDocument dummyObj = new BsonDocument();
            BsonDocument matcher = query.Matcher;
            BsonDocument selector = query.Selector;
            BsonDocument orderBy = query.OrderBy;
            BsonDocument hint = query.Hint;
            long skipRows = query.SkipRowsCount;
            long returnRows = query.ReturnRowsCount;

            return Query(matcher, selector, orderBy, hint, skipRows, returnRows);
        }

        /** \fn DBCursor Query(BsonDocument query, BsonDocument selector, BsonDocument orderBy, BsonDocument hint)
         *  \brief Find documents of current collection
         *  \param query The matching condition
         *  \param selector The selective rule
         *  \param orderBy the ordered rule
         *  \param hint One of the indexs in current collection, using default index to query if not provided
         *           eg:{"":"ageIndex"}
         *  \return The DBCursor of matching documents or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor Query(BsonDocument query, BsonDocument selector, BsonDocument orderBy, BsonDocument hint)
        {
            return Query(query, selector, orderBy, hint, 0, -1);
        }

        /** \fn DBCursor Query(BsonDocument query, BsonDocument selector, BsonDocument orderBy, BsonDocument hint, 
         *   long skipRows, long returnRows) 
         *  \brief Find documents of current collection
         *  \param query The matching condition
         *  \paramselector The selective rule
         *  \param orderBy The ordered rule
         *  \param hint One of the indexs in current collection, using default index to query if not provided
         *           eg:{"":"ageIndex"}
         *  \param skipRows Skip the first numToSkip documents, default is 0
         *  \param returnRows Only return numToReturn documents, default is -1 for returning all results
         *  \return The DBCursor of matching documents or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor Query(BsonDocument query, BsonDocument selector, BsonDocument orderBy, BsonDocument hint,
            long skipRows, long returnRows)
        {
            BsonDocument dummyObj = new BsonDocument();
            if ( query == null )
                query = dummyObj;
            if (selector == null)
                selector = dummyObj;
            if (orderBy == null)
                orderBy = dummyObj;
            if (hint == null)
                hint = dummyObj;
            SDBMessage rtnSDBMessage = AdminCommand(collectionFullName, query, selector,
                orderBy, hint, skipRows, returnRows);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                if (flags == SequoiadbConstants.SDB_DMS_EOC)
                    return null;
                else
                {
                    if (logger.IsDebugEnabled)
                        logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                    throw new BaseException(flags);
                }

            return new DBCursor(rtnSDBMessage, this);
        }

        /** \fn DBCursor GetIndexes()
         *  \brief Get all the indexes of current collection
         *  \return A cursor of all indexes or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor GetIndexes() 
        {
            string commandString = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.GET_INXES;
            BsonDocument dummyObj = new BsonDocument();
            BsonDocument obj = new BsonDocument();
            obj.Add(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);

            SDBMessage rtn = AdminCommand(commandString, dummyObj, dummyObj, dummyObj, obj, -1, -1);

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

        /** \fn DBCursor GetIndex(string name)
         *  \brief Get the named index of current collection
         *  \param name The index name
         *  \return A index, if not exist then return null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor GetIndex(string name)
        {
            if (name == null)
                return GetIndexes();
            string commandString = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.GET_INXES;
            BsonDocument dummyObj = new BsonDocument();
            BsonDocument obj = new BsonDocument();
            BsonDocument conndition = new BsonDocument();
            obj.Add(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
            conndition.Add(SequoiadbConstants.IXM_INDEXDEF + "." + SequoiadbConstants.IXM_NAME,
                    name);

            SDBMessage rtn = AdminCommand(commandString, conndition, dummyObj, dummyObj, obj, -1, -1);

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

        /** \fn void CreateIndex(string name, BsonDocument key, bool isUnique, bool isEnforced)
         *  \brief Create a index with name and key
         *  \param name The index name
         *  \param key The index key
         *  \param isUnique Whether the index elements are unique or not
         *  \param isEnforced Whether the index is enforced unique.
         *                    This element is meaningful when isUnique is group to true.
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void CreateIndex(string name, BsonDocument key, bool isUnique, bool isEnforced) 
        {
            string commandString = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.CREATE_INX;
            BsonDocument obj = new BsonDocument();
            BsonDocument dummyObj = new BsonDocument();
            BsonDocument createObj = new BsonDocument();
            obj.Add(SequoiadbConstants.IXM_NAME, name);
            obj.Add(SequoiadbConstants.IXM_KEY, key);
            obj.Add(SequoiadbConstants.IXM_UNIQUE, isUnique);
            obj.Add(SequoiadbConstants.IXM_ENFORCED, isEnforced);
            createObj.Add(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
            createObj.Add(SequoiadbConstants.FIELD_INDEX, obj);

            SDBMessage rtn = AdminCommand(commandString, createObj, dummyObj, dummyObj, dummyObj, -1, -1);

            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn void DropIndex(string name)
         *  \brief Remove the named index of current collection
         *  \param name The index name
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public void DropIndex(string name) 
        {
            string commandString = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.DROP_INX;
            BsonDocument dummyObj = new BsonDocument();
            BsonDocument dropObj = new BsonDocument();
            BsonDocument index = new BsonDocument();
            index.Add("", name);
            dropObj.Add(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
            dropObj.Add(SequoiadbConstants.FIELD_INDEX, index);
            SDBMessage rtn = AdminCommand(commandString, dropObj, dummyObj, dummyObj, dummyObj, -1, -1);
            int flags = rtn.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        /** \fn long GetCount(BsonDocument condition)
         *  \brief Get the count of matching documents in current collection
         *  \param condition The matching rule
         *  \return The count of matching documents
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
        */
        public long GetCount(BsonDocument condition)
        {
            string commandString = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.GET_COUNT;
            BsonDocument dummyObj = new BsonDocument();
            BsonDocument hint = new BsonDocument();
            if (condition == null)
                condition = dummyObj;
            hint.Add(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
            SDBMessage rtnSDBMessage = AdminCommand(commandString, condition, dummyObj, dummyObj, hint, -1, -1);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);

            List<BsonDocument> rtn = GetMoreCommand(rtnSDBMessage);
            return rtn[0][SequoiadbConstants.FIELD_TOTAL].AsInt64;
        }
		
        /** \fn DBCursor Aggregate(List<BsonDocument> obj)
         *  \brief Execute aggregate operation in specified collection
         *  \param insertor The array of bson objects, can't be null
         *  \return The DBCursor of result or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor Aggregate(List<BsonDocument> obj)
        {
            if ( obj == null || obj.Count == 0 )
                throw new BaseException("SDB_INVALIDARG");
            SDBMessage sdbMessage = new SDBMessage();
            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.CollectionFullName = collectionFullName;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            sdbMessage.Flags = 0;
            sdbMessage.Insertor = obj[0];

            byte[] request = SDBMessageHelper.BuildAggrRequest(sdbMessage, isBigEndian);

            for (int count = 1; count < obj.Count; count++)
            {
                request = SDBMessageHelper.AppendAggrMsg(request, obj[count], isBigEndian);
            }
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                if (flags == SequoiadbConstants.SDB_DMS_EOC)
                    return null;
                else
                {
                    if (logger.IsDebugEnabled)
                        logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                    throw new BaseException(flags);
                }

            return new DBCursor(rtnSDBMessage, this);
        }

        /** \fn DBCursor GetQueryMeta(BsonDocument query, BsonDocument orderBy, BsonDocument hint, 
         *                            long skipRows, long returnRows) 
         *  \brief Get the index blocks' or data blocks' infomations for concurrent query
         *  \param query The matching condition, return the whole range of index blocks if not provided
         *           eg:{"age":{"$gt":25},"age":{"$lt":75}}
         *  \param orderBy The ordered rule, result set is unordered if not provided
         *  \param hint hint One of the indexs in current collection, using default index to query if not provided
         *           eg:{"":"ageIndex"}
         *  \param skipRows Skip the first numToSkip documents, default is 0
         *  \param returnRows Only return numToReturn documents, default is -1 for returning all results
         *  \return The DBCursor of matching infomations or null
         *  \exception SequoiaDB.BaseException
         *  \exception System.Exception
         */
        public DBCursor GetQueryMeta(BsonDocument query, BsonDocument orderBy, BsonDocument hint,
                                     long skipRows, long returnRows)
        {
            BsonDocument dummyObj = new BsonDocument();
            if (query == null)
                query = dummyObj;
            if (orderBy == null)
                orderBy = dummyObj;
            if (hint == null)
                hint = dummyObj;
            if (returnRows == 0)
                returnRows = -1;
            string commandString = SequoiadbConstants.ADMIN_PROMPT + SequoiadbConstants.GET_QUERYMETA;
            BsonDocument hint1 = new BsonDocument();
            hint1.Add(SequoiadbConstants.FIELD_COLLECTION, collectionFullName);
            SDBMessage rtnSDBMessage = AdminCommand(commandString, query, hint, orderBy,
                                                     hint1, skipRows, returnRows);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                if (flags == SequoiadbConstants.SDB_DMS_EOC)
                    return null;
                else
                {
                    if (logger.IsDebugEnabled)
                        logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                    throw new BaseException(flags);
                }
            return new DBCursor(rtnSDBMessage, this);
        }

        private void _Update(int flag, BsonDocument matcher, BsonDocument modifier, BsonDocument hint)
        {
            if ( modifier == null )
                throw new BaseException("SDB_INVALIDARG");
            BsonDocument dummyObj = new BsonDocument();
            if (matcher == null)
                matcher = dummyObj;
            if (hint == null)
                hint = dummyObj;
            SDBMessage sdbMessage = new SDBMessage();

            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.Flags = flag;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.CollectionFullName = collectionFullName;
            sdbMessage.RequestID = 0;
            sdbMessage.Matcher = matcher;
            sdbMessage.Modifier = modifier;
            sdbMessage.Hint = hint;

            byte[] request = SDBMessageHelper.BuildUpdateRequest(sdbMessage, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);
            int flags = rtnSDBMessage.Flags;
            if (flags != 0)
                throw new BaseException(flags);
        }

        private SDBMessage AdminCommand(string command, BsonDocument query, BsonDocument selector, BsonDocument orderBy,
            BsonDocument hint, long skipRows, long returnRows) 
        {
            SDBMessage sdbMessage = new SDBMessage();

            sdbMessage.CollectionFullName = command;
            sdbMessage.Version = 0;
            sdbMessage.W = 0;
            sdbMessage.Padding = 0;
            sdbMessage.Flags = 0;
            sdbMessage.NodeID = SequoiadbConstants.ZERO_NODEID;
            sdbMessage.RequestID = 0;
            sdbMessage.SkipRowsCount = skipRows;
            sdbMessage.ReturnRowsCount = returnRows;
            sdbMessage.Matcher = query;
            sdbMessage.Selector = selector;
            sdbMessage.OrderBy = orderBy;
            sdbMessage.Hint = hint;

            byte[] request = SDBMessageHelper.BuildQueryRequest(sdbMessage, isBigEndian);
            connection.SendMessage(request);
            SDBMessage rtnSDBMessage = SDBMessageHelper.MsgExtractReply(connection.ReceiveMessage(isBigEndian), isBigEndian);

            return rtnSDBMessage;
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
                    if (flags == SequoiadbConstants.SDB_DMS_EOC)
                        hasMore = false;
                    else
                    {
                        if (logger.IsDebugEnabled)
                            logger.Debug("Return flags==>" + String.Format("0:X", flags) + "<==");
                        throw new BaseException(flags);
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
   }
}
