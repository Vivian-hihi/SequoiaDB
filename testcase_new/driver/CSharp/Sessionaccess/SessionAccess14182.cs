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
     * description:   1、连接coord执行db.setSessionAttr()，设置PreferedInstance会话实例为instanceid，其中instanceid包含8/9/10，如设置为【8,9,11】或【9/10/11】 
     *                2、连接该coord节点，执行查询操作 
     *                3、查看访问节点情况
     * testcase:    14182
     * author:      chensiqin
     * date:        2019/04/03
     */

    [TestClass]
    public class SessionAccess14182
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;
        private DBCollection cl = null;
        private string clName = "cl14182";
        private string rgName = "sessionAccessRG14182";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
        }
        
        [TestMethod]
        public void Test14182()
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
            arr.Add(instanceidList[0]);
            arr.Add(instanceidList[1]);
            arr.Add(instanceidList[2]+1);
            BsonDocument options = new BsonDocument()
                                     .Add("PreferedInstance", arr);
            sdb.SetSessionAttr(options);
            String actualNodeName = SessionAccessUtil.GetActualDataNodeName(cl);
            if ( !actualNodeName.Equals(nodes[0].GetElement("nodeName").Value.ToString()) 
                && !actualNodeName.Equals(nodes[1].GetElement("nodeName").Value.ToString()))
            {
                Assert.Fail(" node expected is " + nodes[0].GetElement("nodeName").Value.ToString() + "or" + nodes[1].GetElement("nodeName").Value.ToString() + "but found :" + actualNodeName);
            }
        }

        [TestCleanup()]
        public void TearDown()
        {
            if (cs.IsCollectionExist(clName))
            {
                cs.DropCollection(clName);
            }
            sdb.RemoveReplicaGroup(rgName);
            if (sdb != null)
            {
                sdb.Disconnect();
            }
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " end  : " + this.GetType().ToString());
        }
    }
}
