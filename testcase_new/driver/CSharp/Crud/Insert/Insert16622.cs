using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SequoiaDB;
using CSharp.TestCommon;
using SequoiaDB.Bson;


namespace CSharp.Crud.Insert
{
    /**
     * TestCase : seqDB-16622
     * test interface:   public BsonDocument Insert(BsonDocument record, int flags)
     *                   public BsonDocument Insert(List<BsonDocument> recordList, int flags)
     * author:  chensiqin
     * date:    2018/11/19
     * version: 1.0
    */

    [TestClass]
    public class Insert16622
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;

        private string clName1 = "cl16622_1";
        private string clName2 = "cl16622_2";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
            cs = sdb.GetCollecitonSpace(SdbTestBase.csName);
        }

        [TestMethod]
        public void Test16622()
        {
            TestInsert16622_1();
            TestInsert16622_2();
        }

        [TestCleanup()]
        public void TearDown()
        {
            if (sdb != null)
            {
                sdb.Disconnect();
            }
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " end  : " + this.GetType().ToString());
        }


        //BsonDocument Insert(BsonDocument record, int flags)
        public void TestInsert16622_1()
        {
            DBCollection cl = cs.CreateCollection(clName1);
            BsonDocument record = new BsonDocument();
            record.Add("_id", 1);
            record.Add("num", 1);
            cl.Insert(record);
            try
            {
                cl.Insert(record, 0);
            }
            catch (BaseException e)
            {
                Assert.AreEqual(-38, e.ErrorCode);
            }

            cl.Insert(record, SDBConst.FLG_INSERT_CONTONDUP);
            record = new BsonDocument();
            record.Add("_id", 2);
            record.Add("num", 2);
            BsonDocument doc = cl.Insert(record, SDBConst.FLG_INSERT_RETURN_OID);
            Assert.AreEqual(2, cl.GetCount(null));
            Assert.AreEqual("{ \"_id\" : 2 }", doc.ToString());
            cs.DropCollection(clName1);
        }
        // BsonDocument Insert(List<BsonDocument> recordList, int flags)
        public void TestInsert16622_2()
        {
            InsertDuplicateKey0();
            InsertDuplicateKey1();
            InsertReturnOID();
        }

        //set the flag is 0
        public void InsertDuplicateKey0()
        {
            DBCollection cl = cs.CreateCollection(clName2);
            try
            {
                cl.Delete(null);
                List<BsonDocument> insertor = GenerateDuplicateData();
                cl.Insert(insertor, 0);
                Assert.Fail("bulkInsert will interrupt when Duplicate key exist");
            }
            catch (BaseException e)
            {
                Assert.AreEqual(e.ErrorCode, -38, e.ErrorCode + e.Message);
            }
            Assert.AreEqual(1, cl.GetCount(null));
            Assert.AreEqual(0, cl.GetCount(new BsonDocument { { "test16622", "test16622" } }));
            cs.DropCollection(clName2);
        }

        //set the flag is SDBConst.FLG_INSERT_CONTONDUP
        public void InsertDuplicateKey1()
        {
            DBCollection cl = cs.CreateCollection(clName2);
            try
            {
                List<BsonDocument> insertor = GenerateDuplicateData();
                cl.Insert(insertor, SDBConst.FLG_INSERT_CONTONDUP);
            }
            catch (BaseException e)
            {
                Assert.Fail("Failed to bulkinsert:", e.ErrorCode + e.Message);
            }
            Assert.AreEqual(2, cl.GetCount(null));
            Assert.AreEqual(1, cl.GetCount(new BsonDocument { { "test16622", "test16622" } }));
            cs.DropCollection(clName2);
        }

        //set the flag is SDBConst.FLG_INSERT_RETURN_OID
        public void InsertReturnOID()
        {
            DBCollection cl = cs.CreateCollection(clName2);
            List<BsonDocument> insertor = new List<BsonDocument>();
            for (int i = 0; i < 2; i++)
            {
                BsonDocument obj = new BsonDocument();
                obj.Add("_id", i).
                            Add("operation", "Insert").
                            Add("name", "zhangsan" + i);
                insertor.Add(obj);
            }
            BsonDocument doc = cl.Insert(insertor, SDBConst.FLG_INSERT_RETURN_OID);
            Assert.AreEqual("{ \"_id\" : [0, 1] }", doc.ToString());
            cs.DropCollection(clName2);
        }

        public List<BsonDocument> GenerateDuplicateData()
        {
            List<BsonDocument> insertor = new List<BsonDocument>();
            for (int i = 0; i < 2; i++)
            {
                BsonDocument obj = new BsonDocument();
                obj.Add("_id", 1).
                            Add("operation", "Insert").
                            Add("name", "zhangsan"+i);
                insertor.Add(obj);
            }

            BsonDocument doc = new BsonDocument();
            doc.Add("test16622", "test16622");
            insertor.Add(doc);
            return insertor;
        }
    }
}
