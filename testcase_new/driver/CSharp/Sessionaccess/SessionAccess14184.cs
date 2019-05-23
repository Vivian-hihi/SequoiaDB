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
     * description:  1、coord配置文件中设置preferedInstance值分别为M/S/A 
     *               2、重启coord节点 
     *               3、连接该coord节点，执行查询操作（如查询多条记录） 
     *               4、查看访问节点是否为指定访问实例属性对应节点(执行explain访问计划查看访问节点)
     *               5、执行getSessionAtr（）获取session信息
     * testcase:    14184
     * author:      chensiqin
     * date:        2019/04/02
     */

    [TestClass]
    public class SessionAccess14184
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;
        private DBCollection cl = null;
        private string clName = "cl14184";
        private string rgName = "sessionAccessRG14184";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
        }

        [TestMethod]
        public void Test14184()
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
            BsonDocument options = new BsonDocument("Group", rgName);
            options.Add("ReplSize", 0);
            cl = cs.CreateCollection(clName, options);
            SessionAccessUtil.InsertRecords(cl);
            //TODO:以下代码前面的空格不对，并且文本用例中的前提没有实现
        string[] expectPreferedInstance = new string[] { "M", "S" };
        for (int i = 0; i < 2; i++ )
        {
            string s = expectPreferedInstance[i];
            options = new BsonDocument("PreferedInstance", s);
            sdb.SetSessionAttr(options);
            String hostName = SessionAccessUtil.GetActualDataNodeName(cl);
            if (s.Equals("M"))
            {
                Assert.IsTrue(SessionAccessUtil.IsMaster(sdb, rgName, hostName), "the actual data node name is: " + hostName + ",the current option is " + options.ToString());
            }
            else if (s.Equals("S"))
            {
                Assert.IsFalse(SessionAccessUtil.IsMaster(sdb, rgName, hostName), "the actual data node name is: " + hostName + ",the current option is " + options.ToString());
            }
            options.Add("PreferedInstanceMode", "random").Add("Timeout", -1L);
            BsonDocument act = sdb.GetSessionAttr();
            Assert.AreEqual(act.GetElement("PreferedInstance").Value, options.GetElement("PreferedInstance").Value);
            Assert.AreEqual(act.GetElement("PreferedInstanceMode").Value, options.GetElement("PreferedInstanceMode").Value);
            Assert.AreEqual(act.GetElement("Timeout").Value, options.GetElement("Timeout").Value);
        }
        
        List<string> nodeNames = new List<string>();
        for (int i=0 ; i< nodes.Count(); i++) {
        	BsonDocument node = nodes[i];
        	nodeNames.Add(node.GetElement("nodeName").Value.ToString());
        }
        //设置PreferedInstance为'A'
        options = new BsonDocument("PreferedInstance", "A");
        List<string> actNodeNames = new List<string>();
        for(int i = 0 ; i < 20 ; i++){
        	sdb.SetSessionAttr(options);
        	String actNodeName = SessionAccessUtil.GetActualDataNodeName(cl);
        	Assert.IsTrue(nodeNames.Contains(actNodeName),"The actual Node name is not expected: " + actNodeName);
        	actNodeNames.Add(actNodeName);
        }
        //TODO：此处actNodeNames的大小为20，这里是不是应该去重后再比较
        //如果actNodeNames大小为1时，代表实际操作的节点只有一个，没有随机取值
        Assert.AreNotEqual(actNodeNames.Count(), 1, "When PreferedInstance is 'A', the actual node is unchanged, the node name is:" + actNodeNames[0]);
        //options.Add("PreferedInstanceMode", "random").Add("Timeout", -1L);
        BsonDocument actSessionAttr = sdb.GetSessionAttr();
        Assert.AreEqual("random", actSessionAttr.GetElement("PreferedInstanceMode").Value.ToString());
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
