using SequoiaDB;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using System;
using SequoiaDB.Bson;

namespace DriverTest
{

    [TestClass()]
    public class BSONTest
    {


        private TestContext testContextInstance;
        private static Config config = null;

        Sequoiadb sdb = null;
        CollectionSpace cs = null;
        DBCollection coll = null;
        DBCursor cur = null;
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
            sdb = new Sequoiadb(config.conf.Coord.Address);
            sdb.Connect(config.conf.UserName, config.conf.Password);
            if (sdb.IsCollectionSpaceExist(csName))
                sdb.DropCollectionSpace(csName);
            cs = sdb.CreateCollectionSpace(csName);
            coll = cs.CreateCollection(cName);
        }

        [TestCleanup()]
        public void MyTestCleanup()
        {
            //cs.DropCollection(cName);
            sdb.DropCollectionSpace(csName);
            sdb.Disconnect();
        }
        #endregion

        [TestMethod()]
        public void Digits_is_long_test()
        {
            string expect1 = "";
            string expect2 = "";
            BsonDocument result = null;
            BsonDocument obj = new BsonDocument();

            obj.Add("a", 0);
            obj.Add("b", int.MaxValue);
            obj.Add("c", int.MinValue);
            obj.Add("e", int.MaxValue + 1L);
            obj.Add("f", int.MinValue - 1L);
            obj.Add("g", long.MaxValue);
            obj.Add("h", long.MinValue);
            coll.Insert(obj);
            cur = coll.Query(null, new BsonDocument().Add("_id", new BsonDocument().Add("$include", 0)), null, null);
            try
            {
                result = cur.Next();
            }
            finally
            {
                cur.Close();
            }

            expect1 = "{ \"a\" : 0, \"b\" : 2147483647, \"c\" : -2147483648, \"e\" : 2147483648, \"f\" : -2147483649, \"g\" : 9223372036854775807, \"h\" : -9223372036854775808 }";
            expect2 = "{ \"a\" : 0, \"b\" : 2147483647, \"c\" : -2147483648, \"e\" : 2147483648, \"f\" : -2147483649, \"g\" : {\"$numberLong\": 9223372036854775807}, \"h\" : {\"$numberLong\": -9223372036854775808} }";
            // case 1:
            Console.WriteLine("case1's result is: {0}", result.ToString());
            Assert.AreEqual(expect1, result.ToString());

            // case 2:
            BsonDefaults.JsCompatibility = true;
            Console.WriteLine("case2's result is: {0}", result.ToString());
            Assert.AreEqual(expect2, result.ToString());
            
            // case 3:
            BsonDefaults.JsCompatibility = false;
            Console.WriteLine("case3's result is: {0}", result.ToString());
            Assert.AreEqual(expect1, result.ToString());

            // case 4:
            BsonDefaults.JsCompatibility = true;
            Console.WriteLine("case4's result is: {0}", result.ToString());
            Assert.AreEqual(expect2, result.ToString());
        }

    }
}
