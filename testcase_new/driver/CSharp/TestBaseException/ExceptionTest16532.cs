using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SequoiaDB.Bson;
using SequoiaDB;
using CSharp.TestCommon;

namespace CSharp.TestBaseException
{
    /**
     * description:  使用无效更新符更新记录，查看getLastErrObj返回的错误对象
     * testcase:     seqDB-16532
     * author:       chensiqin
     * date:         2018/11/13
    */
    [TestClass]
    public class ExceptionTest16532
    {

        private Sequoiadb sdb = null;
        private CollectionSpace cs;
        private DBCollection cl;
        private string clName = "cl16532";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
        }

        [TestMethod]
        public void Test16532()
        {
            createCL();
            DBQuery query = new DBQuery();
            BsonDocument modifier = new BsonDocument();
            BsonDocument obj = new BsonDocument();
            obj.Add("a", 2);
            modifier.Add("$seta", obj);
            query.Modifier = modifier;
            try
            {
                cl.Update(query);
            }
            catch (BaseException e)
            {
                Assert.AreEqual(-6, e.ErrorCode);
                Assert.IsNotNull(e.ErrorObject);
            }
            cs.DropCollection(clName);
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

        public void createCL()
        {
            try
            {
                if (!sdb.IsCollectionSpaceExist(SdbTestBase.csName))
                {
                    sdb.CreateCollectionSpace(SdbTestBase.csName);
                }
            }
            catch (BaseException e)
            {
                Assert.AreEqual(-33, e.ErrorCode);
            }
            try
            {

                cs = sdb.GetCollecitonSpace(SdbTestBase.csName);
                cl = cs.CreateCollection(clName);
            }
            catch (BaseException e)
            {
                Assert.IsTrue(false, "create cl fail " + e.Message);
            }
        }

    }
}
