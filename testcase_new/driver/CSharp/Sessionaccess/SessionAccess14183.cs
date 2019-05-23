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
     * description:  1、连接该coord节点，设置preferedInstance值为数据节点的instanceid值，设置timeout值 
     *               2、执行查询操作（如查询多条记录），其中查询操作时间分别验证小于timeout时间、超过timeout时间 
     *               3、查看访问节点是否为指定instanceid对应节点(如执行explain访问计划查看连接节点db.wy.wy.find({a:{"$lt":2}}).explain()) 
     *               4、执行getSessionAtr（）获取session信息
     * testcase:    14183
     * author:      chensiqin
     * date:        2019/04/02
     */

    [TestClass]
    public class SessionAccess14183
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;
        private DBCollection cl = null;
        private string clName = "cl14183";
        private string rgName = "sessionAccessRG14183";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
        }

        [TestMethod]
        public void Test14183()
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

            ReplicaGroup rg = sdb.GetReplicaGroup(rgName);
            string slaveNodeName = rg.GetSlave().NodeName;
            int expctId = SessionAccessUtil.GetInstanceidByNodeName(nodes, slaveNodeName);
            BsonDocument expSessionAttr = new BsonDocument("PreferedInstance", expctId).Add("Timeout", 1000L);

            //put lob 
            DBLob lob = cl.CreateLob();
            lob.Write(new byte[1024 * 1024 * 10]);
            ObjectId oid = lob.GetID();
            lob.Close();
            sdb.SetSessionAttr(expSessionAttr);
        
            try {
        	    DBLob openLob = cl.OpenLob(oid);
                openLob.Close();
            } catch (BaseException e) {
        	    Assert.AreEqual(-13, e.ErrorCode);
            }
            expSessionAttr.Remove("Timeout");
            expSessionAttr.Add("Timeout", 20000L);
            sdb.SetSessionAttr(expSessionAttr);
            DBLob openlob = cl.OpenLob(oid);
            openlob.Close();
        
            String actualNodeName = SessionAccessUtil.GetActualDataNodeName(cl);
            Assert.AreEqual(slaveNodeName, actualNodeName);

            BsonDocument actSessionAttr = sdb.GetSessionAttr();
            expSessionAttr.Remove("Timeout");
            expSessionAttr.Add("Timeout", 20000L);//TODO:这两行跟前面设置超时时间为20000L一样，应该不用重新设置了吧？
            Assert.AreEqual(actSessionAttr.GetElement("PreferedInstance").Value, expSessionAttr.GetElement("PreferedInstance").Value);
            Assert.AreEqual(actSessionAttr.GetElement("Timeout").Value, expSessionAttr.GetElement("Timeout").Value);
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
