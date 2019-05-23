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
     * description:  1、sequoiadb集群运行正常 2、系统中存在cl，且cl中有数据 3、数据节点已配置instanceid值，如数据组为三个节点a、b、c，设置a为7、b为100、c为255
     *                1、连接coord执行db.setSessionAttr()，设置PreferedInstance会话实例为instanceid和【M/S/A】,访问模式为顺序模式 ，分别验证如下场景：
     *                   1）、指定多个instanceid实例和“M”、“m”
     *               4、查看访问节点情况（如执行explain查看）
     * testcase:    14187
     * author:      chensiqin
     * date:        2019/04/03
     */

    [TestClass]
    public class SessionAccess14187
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;
        private DBCollection cl = null;
        private string clName = "cl14187";
        private string rgName = "sessionAccessRG14187";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
        }
        //TODO:此用例的文本用例需要修改，步骤序号乱了
        [TestMethod]
        public void Test14187()
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
            BsonArray arr = new BsonArray();
            arr.Add(instanceidList[1]);
            arr.Add(instanceidList[0]);
            arr.Add(instanceidList[2]);
            arr.Add("m");

            BsonDocument options = new BsonDocument().Add("PreferedInstance", arr)
                                                     .Add("PreferedInstanceMode", "ordered");
            sdb.SetSessionAttr(options);
            String actualNodeName = SessionAccessUtil.GetActualDataNodeName(cl);
           
            BsonDocument expNode = nodes[1];
            Assert.AreEqual( expNode.GetElement("nodeName").Value.ToString(), actualNodeName);
            //assert getSessionAttr
            BsonDocument actual = sdb.GetSessionAttr();
            Assert.AreEqual(actual.GetElement("PreferedInstance").Value, options.GetElement("PreferedInstance").Value);

            arr = new BsonArray();
            arr.Add(instanceidList[1]);
            arr.Add(instanceidList[0]);
            arr.Add(instanceidList[2]);
            arr.Add("M");
            options = new BsonDocument().Add("PreferedInstance", arr)
                                                     .Add("PreferedInstanceMode", "ordered");
            sdb.SetSessionAttr(options);
            actualNodeName = SessionAccessUtil.GetActualDataNodeName(cl);
            Assert.IsTrue(SessionAccessUtil.IsMaster(sdb, rgName, actualNodeName));
            //assert getSessionAttr
            actual = sdb.GetSessionAttr();
            Assert.AreEqual(actual.GetElement("PreferedInstance").Value, options.GetElement("PreferedInstance").Value);
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
