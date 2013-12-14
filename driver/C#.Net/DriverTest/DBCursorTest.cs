using SequoiaDB;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using SequoiaDB.Bson;

namespace DriverTest
{

    [TestClass()]
    public class DBCursorTest
    {


        private TestContext testContextInstance;
        private static Config config = null;

        Sequoiadb sdb = null;
        CollectionSpace cs = null;
        DBCollection coll = null;
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
            if ( config == null )
                config = new Config();
        }

        [TestInitialize()]
        public void MyTestInitialize()
        {
            sdb = new Sequoiadb(config.conf.Coord.Address);
            sdb.Connect(config.conf.UserName, config.conf.Password);
            cs = sdb.GetCollecitonSpace(csName);
            if (cs != null)
                sdb.DropCollectionSpace(csName);
            cs = sdb.CreateCollectionSpace(csName);
            coll = cs.CreateCollection(cName);
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            cs.DropCollection(cName);
            sdb.DropCollectionSpace(csName);
            sdb.Disconnect();
        }
        #endregion
    /*    
        [TestMethod()]
        public void UpdateCurrentTest()
        {
            BsonDocument insertor = new BsonDocument();
            insertor.Add("Last Name", "Lin");
            insertor.Add("First Name", "Hetiu");
            insertor.Add("Address", "SYSU");
            BsonDocument sInsertor = new BsonDocument();
            sInsertor.Add("Phone", "10086");
            sInsertor.Add("EMail", "hetiu@yahoo.com.cn");
            insertor.Add("Contact", sInsertor);
            coll.Insert(insertor);

            BsonDocument dummy = new BsonDocument();
            BsonDocument updater = new BsonDocument();
            BsonDocument matcher = new BsonDocument();
            BsonDocument modifier = new BsonDocument();
            updater.Add("Age", 25);
            modifier.Add("$group", updater);
            matcher.Add("First Name", "Hetiu");
            DBCursor cursor = coll.Query(matcher, dummy, dummy, dummy);
            Assert.IsNotNull(cursor);
            BsonDocument bson = cursor.Next();
            Assert.IsNotNull(bson);

            cursor.UpdateCurrent(modifier);
            bson = cursor.Current();
            Assert.IsNotNull(bson);
            Assert.IsTrue(bson["First Name"].AsString.Equals("Hetiu"));
            Assert.IsTrue(bson["Age"].AsInt32.Equals(25));
        }

        [TestMethod()]
        public void DeleteCurrentTest()
        {
            BsonDocument insertor1 = new BsonDocument();
            insertor1.Add("Last Name", "Lin");
            insertor1.Add("First Name", "Hetiu");
            coll.Insert(insertor1);
            BsonDocument insertor2 = new BsonDocument();
            insertor2.Add("Last Name", "Wang");
            insertor2.Add("First Name", "Tao");
            coll.Insert(insertor2);

            BsonDocument dummy = new BsonDocument();
            BsonDocument matcher = new BsonDocument();
            matcher.Add("First Name", "Hetiu");
            DBCursor cursor = coll.Query(matcher, dummy, dummy, dummy);
            Assert.IsNotNull(cursor);
            BsonDocument bson = cursor.Next();
            Assert.IsNotNull(bson);
            cursor.DeleteCurrent();
            Assert.IsNull(cursor.Current());

            BsonDocument matcher2 = new BsonDocument();
            matcher2.Add("First Name", "Tao");
            DBCursor cursor2 = coll.Query(matcher2, dummy, dummy, dummy);
            Assert.IsNotNull(cursor2);
            BsonDocument bson2 = cursor2.Next();
            Assert.IsNotNull(bson2);
            Assert.IsTrue(coll.GetCount(dummy) == 1);
        }
  */
    }
}
