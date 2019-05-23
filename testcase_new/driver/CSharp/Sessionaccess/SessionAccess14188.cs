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
     * description:   1、 coord连接上使用db.setSessionAttr()进行timeout设置
     *                2、  连接该coord节点，分别执行如下操作：
     *                    1）创建、删除cs、cl；挂载cl、修改cl
     *                    2）执行插入、更新、删除操作
     *                    3）执行切分操作
     *                3.操作耗时较长，超过timeout值
     * testcase:    14188
     * author:      chensiqin
     * date:        2019/04/03
     */

    [TestClass]
    public class SessionAccess14188
    {
        private Sequoiadb sdb = null;
        private CollectionSpace cs = null;
        private DBCollection cl = null;
        private string mainCSName = "mainCS14188";
        private string subCSName = "subCS14188";
        private string localCSName = "cs14188";
        private string clName1 = "cl14188_1";
        private string clName2 = "cl14188_2";
        private string rgName = "sessionAccessRG14188";

        [TestInitialize()]
        public void SetUp()
        {
            Console.WriteLine(DateTime.Now.ToString("yyyy-MM-dd hh:mm:ss:fff") + " begin: " + this.GetType().ToString());
            sdb = new Sequoiadb(SdbTestBase.coordUrl);
            sdb.Connect();
        }

        [TestMethod]
        public void Test14188()
        {
            if (Common.IsStandalone(sdb) == true)
            {
                return;
            }
            cs = sdb.GetCollecitonSpace(SdbTestBase.csName);
            if (cs.IsCollectionExist(clName1))
            {
                cs.DropCollection(clName1);
            }
            List<BsonDocument> nodes = SessionAccessUtil.CreateRG(sdb, rgName);
            BsonDocument option = new BsonDocument();
            option.Add("Group", rgName);
            option.Add("ReplSize", 0);
            cl = cs.CreateCollection(clName1, option);
            SessionAccessUtil.InsertRecords(cl);
            
            sdb.SetSessionAttr(new BsonDocument("Timeout", 1000L));
            try
            {
                sdb.CreateCollectionSpace(localCSName).CreateCollection(clName2);
            }
            catch (BaseException e)
            {
                if (e.ErrorCode != -13)
                    throw e;
            }
            finally
            {
                sdb.SetSessionAttr(new BsonDocument("Timeout", -1L));
                sdb.DropCollectionSpace(localCSName);
            }

            sdb.GetCollecitonSpace(SdbTestBase.csName).CreateCollection(clName2);
            sdb.SetSessionAttr(new BsonDocument("Timeout", 1000L));
            try
            {
                sdb.GetCollecitonSpace(SdbTestBase.csName).DropCollection(clName2);
            }
            catch (BaseException e)
            {
                if (e.ErrorCode != -13)
                    throw e;
            }
            finally
            {
                sdb.SetSessionAttr(new BsonDocument("Timeout", -1L));
                if (sdb.GetCollecitonSpace(SdbTestBase.csName).IsCollectionExist(clName2))
                {
                    sdb.GetCollecitonSpace(SdbTestBase.csName).DropCollection(clName2);
                }
            }

            //挂载、修改cl
            //create mainCL and subCL
            string mainCLName = "mainCL14188";
            BsonDocument mainCLOption = new BsonDocument();
            BsonDocument mainShardingKey = new BsonDocument();
            mainShardingKey.Add("a", 1);
            mainCLOption.Add("ShardingKey", mainShardingKey);
            mainCLOption.Add("ShardingType", "range");
            mainCLOption.Add("ReplSize", 0);
            mainCLOption.Add("IsMainCL", true);
            mainCLOption.Add("Compressed", true);

            string subCLName = "subCL14188";
            BsonDocument subCLOption = new BsonDocument();
            BsonDocument subShardingKey = new BsonDocument();
            subShardingKey.Add("b", 1);
            subCLOption.Add("ShardingKey", subShardingKey);

            CollectionSpace mainCS = sdb.CreateCollectionSpace(mainCSName);
            CollectionSpace subCS = sdb.CreateCollectionSpace(subCSName);
            DBCollection mainCL = mainCS.CreateCollection(mainCLName, mainCLOption);
            subCS.CreateCollection(subCLName, subCLOption);

            //attach
            BsonDocument attachOption = new BsonDocument();
            BsonDocument attachLowObj = new BsonDocument();
            BsonDocument attachUpObj = new BsonDocument();
            attachOption.Add("a", 1);//TODO:这里应该是attachLowObj.Add("a",1)吧？
            attachOption.Add("LowBound", attachLowObj);
            attachUpObj.Add("a", 100);
            attachOption.Add("UpBound", attachUpObj);

            sdb.SetSessionAttr(new BsonDocument("Timeout", 1000L));
            try
            {
                mainCL.AttachCollection(subCSName + "." + subCLName, attachOption);
            }
            catch (BaseException e)
            {
                if (e.ErrorCode != -13)
                    throw e;
            }
            finally
            {
                sdb.SetSessionAttr(new BsonDocument("Timeout", -1L));
                sdb.DropCollectionSpace(mainCSName);
                sdb.DropCollectionSpace(subCSName);
            }

            //修改 cl
            sdb.GetCollecitonSpace(SdbTestBase.csName).CreateCollection(clName2);
            sdb.SetSessionAttr(new BsonDocument("Timeout", 1000L));
            try
            {
                BsonDocument opt = new BsonDocument();
                opt.Add("ReplSize", -1);
                sdb.GetCollecitonSpace(SdbTestBase.csName).GetCollection(clName2).Alter(opt);
            }
            catch (BaseException e)
            {
                if (e.ErrorCode != -13)
                    throw e;
            }
            finally
            {
                sdb.SetSessionAttr(new BsonDocument("Timeout", -1L));
                sdb.GetCollecitonSpace(SdbTestBase.csName).DropCollection(clName2);
            }

            //执行插入、更新、删除操作
            sdb.SetSessionAttr(new BsonDocument("Timeout", 1000L));
            try
            {
                for (int i = 0; i < 100; i++)
                {
                    BsonDocument record = new BsonDocument();
                    record.Add("a", "test14188_" + i);
                    cl.Insert(record);
                }
            }
            catch (BaseException e)
            {
                if (e.ErrorCode != -13)
                    throw e;
            }
            finally
            {
                sdb.SetSessionAttr(new BsonDocument("Timeout", -1L));
            }

            BsonDocument modifier = new BsonDocument();
            BsonDocument modifyObj = new BsonDocument();
            BsonDocument match = new BsonDocument();
            modifyObj.Add("a", "14188");
            modifier.Add("$set", modifyObj);
            sdb.SetSessionAttr(new BsonDocument("Timeout", 1000L));
            try
            {
                cl.Update(match, modifier, null);
            }
            catch (BaseException e)
            {
                if (e.ErrorCode != -13)
                    throw e;
            }
            finally
            {
                sdb.SetSessionAttr(new BsonDocument("Timeout", -1L));
            }

            sdb.SetSessionAttr(new BsonDocument("Timeout", 1000L));
            try
            {
                BsonDocument matcher = new BsonDocument();
                matcher.Add("a", "test14188_10");
                cl.Delete(matcher);
            }
            catch (BaseException e)
            {
                if (e.ErrorCode != -13)
                    throw e;
            }
            finally
            {
                sdb.SetSessionAttr(new BsonDocument("Timeout", -1L));
            }
        }

        [TestCleanup()]
        public void TearDown()
        {
            if (cs.IsCollectionExist(clName1))
            {
                cs.DropCollection(clName1);
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
