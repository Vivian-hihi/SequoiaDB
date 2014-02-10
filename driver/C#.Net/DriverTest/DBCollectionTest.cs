using SequoiaDB;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using SequoiaDB.Bson;
using System.Collections.Generic;

namespace DriverTest
{
    
    
    [TestClass()]
    public class DBCollectionTest
    {
        private TestContext testContextInstance;
        private static Config config = null;

        Sequoiadb sdb = null;
        CollectionSpace cs = null;
        DBCollection coll = null;
        //string csName = "testfoo";
        //string cName = "testbar";
        string csName = "testfoo";
        string cName = "testbar";

        public TestContext TestContext
        {
            get
            {
                return testContextInstance;
            }
            set
            {
                testContextInstance = value;
            }
        }

        #region 附加测试特性
        [ClassInitialize()]
        public static void SequoiadbInitialize(TestContext testContext)
        {
            if (config == null)
                config = new Config(); 
        }

        [TestInitialize()]
        public void MyTestInitialize()
        {
            try
            {
                sdb = new Sequoiadb(config.conf.Coord.Address);
                sdb.Connect(config.conf.UserName, config.conf.Password);
                if (sdb.IsCollectionSpaceExist(csName))
                    sdb.DropCollectionSpace(csName);
                cs = sdb.CreateCollectionSpace(csName);
                coll = cs.CreateCollection(cName);
            }
            catch (BaseException e)
            {
                Console.WriteLine("Failed to Initialize in DBCollectionTest, ErrorType = {0}", e.ErrorType);
                Environment.Exit(0);
            }
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            sdb.DropCollectionSpace(csName);
            sdb.Disconnect();
        }
        #endregion


        /// <summary>
        ///Testing for Insert
        ///</summary>
        [TestMethod()]
        public void InsertTest()
        {
            // insert
            BsonDocument insertor = new BsonDocument();
            string date = DateTime.Now.ToString();
            insertor.Add("operation", "Insert");
            insertor.Add("date", date);
            ObjectId id = coll.Insert(insertor);

            BsonDocument matcher = new BsonDocument();
            DBQuery query = new DBQuery();
            matcher.Add("date", date);
            query.Matcher = matcher;
            DBCursor cursor = coll.Query(query);
            Assert.IsNotNull(cursor);
            BsonDocument bson = cursor.Next();
            Assert.IsNotNull(bson);
            Assert.IsTrue(id.Equals(bson["_id"].AsObjectId));
        }

        /// <summary>
        ///Testing for DBCollection Operation: insert, delete, update, query
        ///</summary>
        [TestMethod()]
        public void OperationTest()
        {
            // Insert
            BsonDocument insertor = new BsonDocument();
            insertor.Add("Last Name", "Lin");
            insertor.Add("First Name", "Hetiu");
            insertor.Add("Address", "SYSU");
            BsonDocument sInsertor = new BsonDocument();
            sInsertor.Add("Phone", "10086");
            sInsertor.Add("EMail", "hetiu@yahoo.com.cn");
            insertor.Add("Contact", sInsertor);
            ObjectId insertID = coll.Insert(insertor);
            Assert.IsNotNull(insertID);

            // Update
            DBQuery query = new DBQuery();
            BsonDocument updater = new BsonDocument();
            BsonDocument matcher = new BsonDocument();
            BsonDocument modifier = new BsonDocument();
            updater.Add("Age", 25);
            modifier.Add("$set", updater);
            matcher.Add("First Name", "Hetiu");
            query.Matcher = matcher;
            query.Modifier = modifier;
            coll.Update(query);

            // Query
            DBCursor cursor = coll.Query(query);
            Assert.IsNotNull(cursor);
            BsonDocument bson = cursor.Next();
            Assert.IsNotNull(bson);
            Assert.IsTrue(bson["First Name"].AsString.Equals("Hetiu"));
            Assert.IsTrue(bson["Age"].AsInt32.Equals(25));

            // Delete
            BsonDocument drop = new BsonDocument();
            drop.Add("Last Name", "Lin");
            coll.Delete(drop);
            query.Matcher = drop;
            cursor = coll.Query(query);
            Assert.IsNotNull(cursor);
            bson = cursor.Next();
            Assert.IsNull(bson);
        }

        /// <summary>
        ///Testing for Query
        ///</summary>
        [TestMethod()]
        public void QueryTest()
        {
            for (int i = 0; i < 10; ++i)
            {
                string date = DateTime.Now.ToString();
                BsonDocument insertor = new BsonDocument();
                insertor.Add("operation", "Query");
                insertor.Add("date", date);
                coll.Insert(insertor);
            }
            BsonDocument matcher = new BsonDocument();
            DBQuery query = new DBQuery();
            matcher.Add("operation", "Query");
            query.Matcher = matcher;
            query.ReturnRowsCount = 5;
            query.SkipRowsCount = 5;
            DBCursor cursor = coll.Query(query);
            Assert.IsNotNull(cursor);
            int count = 0;
            while (cursor.Next() != null)
            {
                ++count;
                BsonDocument bson = cursor.Current();
                Assert.IsNotNull(bson);
            }
            Assert.IsTrue(count == 5);
        }

        /// <summary>
        ///Testing for Indexes: create index, get indexes, drop index
        ///</summary>
        [TestMethod()]
        public void IndexesTest()
        {
            // Insert
            BsonDocument insertor = new BsonDocument();
            insertor.Add("Last Name", "Lin");
            insertor.Add("First Name", "Hetiu");
            insertor.Add("Address", "SYSU");
            BsonDocument sInsertor = new BsonDocument();
            sInsertor.Add("Phone", "10086");
            sInsertor.Add("EMail", "hetiu@yahoo.com.cn");
            insertor.Add("Contact", sInsertor);
            ObjectId insertID = coll.Insert(insertor);
            Assert.IsNotNull(insertID);

            // Create Index
            BsonDocument key = new BsonDocument();
            key.Add("Last Name", 1);
            key.Add("First Name", 1);
            string name = "index name";
            coll.CreateIndex(name, key, false, false);

            // Get Indexes
            DBCursor cursor = coll.GetIndex(name);
            Assert.IsNotNull(cursor);
            BsonDocument index = cursor.Next();
            Assert.IsNotNull(index);
            Assert.IsTrue(index["IndexDef"].AsBsonDocument["name"].AsString.Equals("index name"));
            
            // Drop Index
            coll.DropIndex(name);
            cursor = coll.GetIndex(name);
            Assert.IsNotNull(cursor);
            index = cursor.Next();
            Assert.IsNull(index);
        }

        [TestMethod()]
        public void GetCountTest()
        {
            for (int i = 0; i < 10; ++i)
            {
                string date = DateTime.Now.ToString();
                BsonDocument insertor = new BsonDocument();
                insertor.Add("operation", "GetCount");
                insertor.Add("date", date);
                coll.Insert(insertor);
            }

            BsonDocument condition = new BsonDocument();
            condition.Add("operation", "GetCount");
            long count = coll.GetCount(condition);
            Assert.IsTrue(count == 10);
        }

        [TestMethod()]
        public void BulkInsertTest()
        {
            List<BsonDocument> insertor = new List<BsonDocument>();
            for (int i = 0; i < 10; i++)
            {
                BsonDocument obj = new BsonDocument();
                obj.Add("operation", "BulkInsert");
                obj.Add("date", DateTime.Now.ToString());
                insertor.Add(obj);
            }
            coll.BulkInsert(insertor, 0);
            BsonDocument condition = new BsonDocument();
            condition.Add("operation", "BulkInsert");
            long count = coll.GetCount(condition);
            Assert.IsTrue(count == 10);
        }

        [TestMethod()]
        public void UpsertTest()
        {
            BsonDocument updater = new BsonDocument();
            BsonDocument matcher = new BsonDocument();
            BsonDocument modifier = new BsonDocument();
            BsonDocument hint = new BsonDocument();
            BsonDocument tempMatcher = new BsonDocument();
            DBQuery query = new DBQuery();
            matcher.Add("operation", new BsonDocument("$et", "Upsert"));
            query.Matcher = matcher;

            long count = coll.GetCount(matcher);
            Assert.IsTrue(count == 0);

            // update but insert
            updater.Add("operation", "Upsert");
            updater.Add("type", "Insert");
            modifier.Add("$set", updater);
            tempMatcher.Add("type",new BsonDocument("$et","Insert"));
            coll.Upsert(matcher, modifier, hint);
            count = coll.GetCount(tempMatcher);
            Assert.IsTrue(count == 1);
            DBCursor cursor = coll.Query(query);
            Assert.IsNotNull(cursor);
            BsonDocument rtn = cursor.Next();
            Assert.IsNotNull(rtn);
            Assert.IsTrue(rtn["type"].AsString.Equals("Insert"));

            // update
            updater = new BsonDocument();
            updater.Add("type", "Update");
            modifier = new BsonDocument();
            modifier.Add("$set", updater);
            coll.Upsert(matcher, modifier, hint);

            cursor = coll.Query(query);
            Assert.IsNotNull(cursor);
            count = coll.GetCount(matcher);
            Assert.IsTrue(count == 1);
            rtn = cursor.Next();
            Assert.IsTrue(rtn["type"].AsString.Equals("Update"));
        }

        [TestMethod()]
        public void InsertChineseTest()
        {
            BsonDocument insertor = new BsonDocument();
            insertor.Add("姓名", "林海涛");
            insertor.Add("年龄", 24);
            insertor.Add("id", 2001);

            // an embedded bson object
            BsonDocument phone = new BsonDocument
                {
                    {"0", "10086"},
                    {"1", "10000"}
                };

            insertor.Add("电话", phone);

            ObjectId id = coll.Insert(insertor);

            BsonDocument matcher = new BsonDocument();
            DBQuery query = new DBQuery();
            matcher.Add("姓名", "林海涛");
            query.Matcher = matcher;
            DBCursor cursor = coll.Query(query);
            Assert.IsNotNull(cursor);
            BsonDocument rtn = cursor.Next();
            Assert.IsNotNull(rtn);
            Assert.IsTrue(id.Equals(rtn["_id"].AsObjectId));
        }
/*
        [TestMethod()]
        public void RenameTest()
        {
            for (int i = 0; i < 10; ++i)
            {
                string date = DateTime.Now.ToString();
                BsonDocument insertor = new BsonDocument();
                insertor.Add("operation", "Rename");
                insertor.Add("date", date);
                coll.Insert(insertor);
            }

            coll.Rename("RenameCL");
            BsonDocument condition = new BsonDocument();
            condition.Add("operation", "Rename");
            long count = coll.GetCount(condition);
            Assert.IsTrue(count == 10);    
        }
*/

/*
        [TestMethod()]
        public void SplitTest()
        {
            //string srcGroup = config.conf.Sets[0].SetName;
            //string destGroup = config.conf.Sets[1].SetName;
            string oRGName = "group1";
            string nRGName = config.conf.Groups[0].GroupName;
            string node1 = "ubuntu-dev1:51000";
            string node2 = config.conf.Groups[0].Nodes[0].HostName + ":"
                           + config.conf.Groups[0].Nodes[0].Port.ToString();
            //string node2 = config.conf.Sets[1].Nodes[0].HostName + ":"
            //               + config.conf.Sets[1].Nodes[0].Port.ToString();
            string hostName = config.conf.Groups[0].Nodes[0].HostName;
            int port = config.conf.Groups[0].Nodes[0].Port;
            string csName = "SplitCS";
            string cName = "SplitCL";
            string cFullName = csName+"."+cName;

            // test wether we are in cluster environment or not
            try
            { 
                sdb.GetList(SDBConst.SDB_LIST_GROUPS);
            }
            catch(Exception ex)
            {
                ex.ToString();
                return;
            }
            // create rg
            ReplicaGroup rg = sdb.GetReplicaGroup(nRGName);
            if (rg == null)
                rg = sdb.CreateReplicaGroup(nRGName);
            // create node
            ReplicaNode node = rg.GetNode(hostName, port);
            if (node == null)
            {
                string dbpath = config.conf.Groups[0].Nodes[0].DBPath;
                Dictionary<string, string> map = new Dictionary<string, string>();
                map.Add("diaglevel", config.conf.Groups[0].Nodes[0].DiagLevel);
                node = rg.CreateNode(hostName, port, dbpath, map);
            }
            rg.Start();
            // create cs
            CollectionSpace cs1 = sdb.GetCollecitonSpace(csName);
            if (cs1 == null)
                cs1 = sdb.CreateCollectionSpace(csName);
            // create cl
            DBCollection coll1 = cs1.GetCollection(cName);
            if (coll1 == null)
            {
                BsonDocument shardingKey = new BsonDocument();
                shardingKey.Add("operation", 1);
                BsonDocument options = new BsonDocument();
                options.Add("ShardingKey", shardingKey);
                string s = options.ToString();
                coll1 = cs1.CreateCollection(cName, options);
                ;
            }
            // prepare record
            for (int i = 0; i < 10; ++i)
            {
                string date = DateTime.Now.ToString();
                BsonDocument insertor = new BsonDocument();
                insertor.Add("operation", "Split1");
                insertor.Add("date", date);
                coll1.Insert(insertor);
            }
            for (int i = 0; i < 10; ++i)
            {
                string date = DateTime.Now.ToString();
                BsonDocument insertor = new BsonDocument();
                insertor.Add("operation", "Split2");
                insertor.Add("date", date);
                coll1.Insert(insertor);
            }
            // get src and dest group
            string srcGroup = null;
            string destGroup = null;
            Sequoiadb cdb = new Sequoiadb(config.conf.Catalog.Address);
            cdb.Connect(config.conf.UserName, config.conf.Password);
            CollectionSpace scs = cdb.GetCollecitonSpace("SYSCAT");
            DBCollection scl = scs.GetCollection("SYSCOLLECTIONS");
            //BsonDocument query = new BsonDocument{ {"Name": cFullName} };
            //BsonDocument selector = new BsonDocument{ {"":"CataInfo"} };
            BsonDocument query = new BsonDocument();
            query.Add("Name", cFullName);
            BsonDocument selector = new BsonDocument();
            selector.Add("", "CataInfo");
            DBCursor cursor = scl.Query(query, null, null, null);
            BsonDocument obj = cursor.Next();
            BsonArray array = obj["CataInfo"].AsBsonArray;
            for( int i=0; i<array.Count; i++)
            {
                BsonDocument subObj = array[i].AsBsonDocument;
                srcGroup = subObj["GroupName"].AsString;
                if(srcGroup != null)
                    break;
            }
            if (srcGroup == null)
            {
                System.Console.WriteLine("Can't find the source group.");
                Assert.IsTrue(0 == 1);
            }
            if(srcGroup.Equals(nRGName))
               destGroup = oRGName;
            else
               destGroup = nRGName;
            // build up split condition
            BsonDocument condition = new BsonDocument();
            condition.Add("operation", "Split2");
            // split
            coll1.Split(srcGroup, destGroup, condition, null);
            // check up
            Sequoiadb sdb2 = null;
            CollectionSpace cs2 = null;
            DBCollection coll2 = null;
            if(destGroup == oRGName)
            {
                sdb2 = sdb.GetReplicaGroup(destGroup).GetNode(node1).Connect(config.conf.UserName, config.conf.Password);
                cs2 = sdb2.GetCollecitonSpace(csName);
                // sleep 1s
                System.Threading.Thread.Sleep(3000);
                coll2 = cs2.GetCollection(cName);
                long num = coll2.GetCount(condition);
                Assert.IsTrue(num == 10);
            }
            else if (destGroup == nRGName)
            {
                sdb2 = sdb.GetReplicaGroup(destGroup).GetNode(node2).Connect(config.conf.UserName, config.conf.Password);
                cs2 = sdb2.GetCollecitonSpace(csName);
                // sleep 1s
                System.Threading.Thread.Sleep(3000);
                coll2 = cs2.GetCollection(cName);
                long num = coll2.GetCount(condition);
                Assert.IsTrue( num == 10);
            }
            else
            {
                System.Console.WriteLine("Something wrong.");
                Assert.IsTrue( 0 == 1);
            }

            sdb.DropCollectionSpace(csName);
            sdb.RemoveReplicaGroup(nRGName);
            sdb2.Disconnect();
        }
 */
        [TestMethod()]
        public void AggregateTest()
        {
		    String[] command = new String[2];
		    command[0] = "{$match:{status:\"A\"}}";
		    command[1] = "{$group:{_id:\"$cust_id\",total:{$sum:\"$amount\"}}}";
		    String[] record = new String[4];
		    record[0] = "{cust_id:\"A123\",amount:500,status:\"A\"}";
	        record[1] = "{cust_id:\"A123\",amount:250,status:\"A\"}";
	        record[2] = "{cust_id:\"B212\",amount:200,status:\"A\"}";
	        record[3] = "{cust_id:\"A123\",amount:300,status:\"D\"}";
	        // insert record into database
		    for ( int i=0; i<record.Length; i++)
		    {
			    BsonDocument obj = new BsonDocument();
			    obj = BsonDocument.Parse(record[i]);
                Console.WriteLine("Record is: "+obj.ToString());
			    coll.Insert(obj);
		    }
            List<BsonDocument> list = new List<BsonDocument>();
		    for ( int i=0; i<command.Length; i++)
		    {
                BsonDocument obj = new BsonDocument();
                obj = BsonDocument.Parse(command[i]);
			    list.Add(obj);
		    }

            DBCursor cursor = coll.Aggregate(list);
            int count = 0;
            while ( null != cursor.Next() )
            {
                Console.WriteLine("Result is: "+cursor.Current().ToString());
                String str = cursor.Current().ToString();
                count++ ;
            }
            Assert.IsTrue(2 == count);
        }

        [TestMethod()]
        public void GetQueryMetaTest()
        {
            try{
            // create cl
            DBCollection coll2 = null;
            string cName2 = "testbar2";
            if (cs.IsCollectionExist(cName2))
                cs.DropCollection(cName);
            coll2 = cs.CreateCollection(cName2);
            // create index
            coll2.CreateIndex("ageIndex", new BsonDocument("age", -1), false, false);
            // prepare record
            Random ro = new Random();
            int recordNum = 10000;
            List<BsonDocument> insertor = new List<BsonDocument>();
            for (int i = 0; i < recordNum; i++)
            {
                BsonDocument obj = new BsonDocument();
                obj.Add("Id",i);
                obj.Add("age", ro.Next(0, 100));
                obj.Add("date", DateTime.Now.ToString());
                insertor.Add(obj);
            }
            coll2.BulkInsert(insertor, 0);

            // TODO:

            // query
            BsonDocument subobj = new BsonDocument();
            BsonDocument query = new BsonDocument();
            query.Add("age", subobj);
            subobj.Add("$gt", 1);
            subobj.Add("$lt", 99);
            // hint
            BsonDocument hint = new BsonDocument();
            hint.Add("", "ageIndex");
            // orderBy
            BsonDocument orderBy = new BsonDocument();
            orderBy.Add("Indexblocks", 1);
            // execute getQueryMeta
            DBCursor cursor = coll2.GetQueryMeta(query, orderBy, null, 0, -1);
            DBCursor datacursor = null ;
            long count = 0;
            while (cursor.Next() != null)
            {
                BsonDocument temp = new BsonDocument();
                temp = cursor.Current();
                BsonDocument h = new BsonDocument();
                if ( temp.Contains("Indexblocks") && temp["Indexblocks"].IsBsonArray )
                {
                     h.Add("Indexblocks", temp["Indexblocks"].AsBsonArray);
                }
                datacursor = coll2.Query(null, null, null, h, 0, -1);
                while ( datacursor.Next() != null )
                {
                    count++;
                }
            }
            Assert.IsTrue(recordNum == count);
            }catch (BaseException e)
            {
                Console.WriteLine(e.ErrorType);
                return;
            }
        }
    }
}
