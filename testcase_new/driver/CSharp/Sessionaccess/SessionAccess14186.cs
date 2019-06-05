using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SequoiaDB.Bson;
using SequoiaDB;
using CSharp.TestCommon;

namespace CSharp.Sessionaccess
{
    /**
     * description:   sequoiadb集群运行正常 2、系统中存在cl，且cl中有数据 3、数据节点已配置instanceid值，如数据组为三个节点a、b、c，设置a为12
     *                1、驱动端执行getSessionAttr()获取session信息
     *                2、驱动端连接coord，设置preferedInstance值为数据节点的instanceid值
     *                3、执行getSessionAtr（）获取session信息
     *                4、再次设置session属性，设置其它instanceid
     *                5、重复执行getSessionAttr()获取session信息
     *                6、连接coord节点重启，恢复后，再次执行getSessionAttr()获取session信息
     * testcase:    14186
     * author:      chensiqin
     * date:        2019/04/03
     */

    [TestClass]
    public class SessionAccess14186
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;
        private DBCollection cl = null;
        private string clName = "cl14186";
        private string rgName = "sessionAccessRG14186";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
        }

        [TestMethod]
        public void Test14186()
        {
            if (Common.IsStandalone(sdb) == true)
            {
                return;
            }
            cs = sdb.GetCollecitonSpace(SdbTestBase.csName);
            if (cs.IsCollectionExist(clName))
            {
                cs.DropCollection(clName);
            }
            List<BsonDocument> nodes = SessionAccessUtil.CreateRG(sdb, rgName);
            BsonDocument option = new BsonDocument();
            option.Add("Group", rgName);
            option.Add("ReplSize", 0);
            cl = cs.CreateCollection(clName, option);
            SessionAccessUtil.InsertRecords(cl);
            List<int> instanceidList = SessionAccessUtil.getInstanceidList(nodes);

            BsonDocument doc = sdb.GetSessionAttr();
            Assert.AreEqual("M", doc.GetElement("PreferedInstance").Value.ToString());

            BsonDocument options = new BsonDocument()
                                     .Add("PreferedInstance", "S");
            sdb.SetSessionAttr(options);
            doc = sdb.GetSessionAttr();
            Assert.AreEqual("S", doc.GetElement("PreferedInstance").Value.ToString());

            options = new BsonDocument()
                                     .Add("PreferedInstance", instanceidList[0]);
            sdb.SetSessionAttr(options);
            doc = sdb.GetSessionAttr();
            Assert.AreEqual(instanceidList[0]+"", doc.GetElement("PreferedInstance").Value.ToString());
            doc = sdb.GetSessionAttr();
            Assert.AreEqual(instanceidList[0] + "", doc.GetElement("PreferedInstance").Value.ToString());

            ReplicaGroup rg = sdb.GetReplicaGroup(rgName);
            bool flag1 = rg.Stop();
            bool flag2 = rg.Start();
            if (flag1 && flag2)
            {
                doc = sdb.GetSessionAttr();
                Assert.AreEqual(instanceidList[0] + "", doc.GetElement("PreferedInstance").Value.ToString());
            }
        }

        [TestCleanup()]
        public void TearDown()
        {
            if (cs.IsCollectionExist(clName))
            {
                cs.DropCollection(clName);
            }
            if (sdb.IsReplicaGroupExist(rgName))
            {
                sdb.RemoveReplicaGroup(rgName);
            }
            if (sdb != null)
            {
                sdb.Disconnect();
            }
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " end  : " + this.GetType().ToString());
        }
    }
}
