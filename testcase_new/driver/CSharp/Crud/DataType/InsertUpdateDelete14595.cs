using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SequoiaDB;
using CSharp.TestCommon;
using SequoiaDB.Bson;

namespace CSharp.Crud.DataType
{

    /**
      * description: insert/update/delete string
      *              insert int data ,then update and delete
      * testcase:    14595 
      * author:      chensiqin
      * date:        2018/3/16
     */
    [TestClass]
    public class InsertUpdateDelete14595
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;
        private DBCollection cl = null;
        private string clName = "cl14595";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
            cs = sdb.GetCollecitonSpace(SdbTestBase.csName);
            cl = cs.CreateCollection(clName);

        }

        [TestMethod()]
        public void TestDelete14595()
        {
            InsertDatas(10);
            BsonDocument matcher = new BsonDocument();
            BsonDocument modifer = new BsonDocument(); 
            cl.Update(matcher.Add("str1", new BsonDocument().Add("$et", "zhangsan2")), modifer.Add("$set", new BsonDocument().Add("obj", "test14595")), null);
            DBCursor cursor = cl.Query(matcher.Add("obj", new BsonDocument().Add("$et", "test14595")), null, null, null);
            int i = 0;
            while (cursor.Next() != null)
            {
                i++;
                Console.WriteLine(cursor.Current().ToString());
            }
            cursor.Close();
            Assert.AreEqual(1, i);

            BsonDocument condition = new BsonDocument();
            condition.Add("$ne", "test14595");
            matcher = new BsonDocument();
            Assert.AreEqual(9, cl.GetCount(matcher.Add("obj", condition)));

            matcher = new BsonDocument();
            cl.Delete(matcher.Add("obj", new BsonDocument().Add("$et", "test14595")), null);
            Assert.AreEqual(9, cl.GetCount(null));
            matcher = new BsonDocument();
            Assert.AreEqual(0, cl.GetCount(matcher.Add("obj", new BsonDocument().Add("$et", "test14595"))));
        }

        [TestCleanup()]
        public void TearDown()
        {
            try
            {
                cs.DropCollection(clName);
                Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " end  : " + this.GetType().ToString());
            }
            catch (BaseException e)
            {
                Assert.Fail("Failed to clearup:", e.ErrorCode + e.Message);
            }
            finally
            {
                if (sdb != null)
                {
                    sdb.Disconnect();
                }
            }
        }

        private List<BsonDocument> InsertDatas(int len)
        {
            List<BsonDocument> dataList = new List<BsonDocument>();
            for (int i = 0; i < len; i++)
            {
                //insert  4M  string   data
                string str = "11111111";
                while (System.Text.Encoding.ASCII.GetBytes(str).Length / 1048576 < 2)
                {
                    str += str;
                }
                BsonDocument strObject = new BsonDocument();
                strObject.Add("obj", str);
                strObject.Add("age", 24);
                strObject.Add("str1", "zhangsan"+i);
                dataList.Add(strObject);
            }

            cl.BulkInsert(dataList, 0);
            
            return dataList;
        }

    }

}
