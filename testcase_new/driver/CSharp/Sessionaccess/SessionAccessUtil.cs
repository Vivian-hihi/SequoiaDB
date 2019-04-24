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

    public class SessionAccessUtil
    {
        public static void InsertRecords(DBCollection localcl)
        {
            for (int i = 0; i < 1000; i++)
            {
                BsonDocument obj = new BsonDocument();
                obj.Add("a", i);
                localcl.Insert(obj);
            }
        }
        public static List<BsonDocument> CreateRG(Sequoiadb localdb, string rgName)
        {
            int reservedPortBegin = SdbTestBase.reservedPortBegin;
            if (localdb.IsReplicaGroupExist(rgName))
            {
                localdb.RemoveReplicaGroup(rgName);
            }
            ReplicaGroup rg = localdb.CreateReplicaGroup(rgName);
            int[][] param = new int[3][] { new int[] { reservedPortBegin + 10, 7 }, new int[] { reservedPortBegin + 20, 8 }, new int[] { reservedPortBegin + 30, 9 } };
            string hostName=localdb.GetReplicaGroup("SYSCatalogGroup").GetMaster().HostName;
        
            List<BsonDocument> nodes = new List<BsonDocument>();

            foreach (int[] ints in param)
            {
                BsonDocument config = new BsonDocument();
                config.Add("instanceid", ints[1]);
                rg.CreateNode(hostName, ints[0], SdbTestBase.reservedDir + "/" + ints[0], config);

                BsonDocument node = new BsonDocument();
                node.Add("nodeName", hostName + ":" + ints[0]);
                node.Add("instanceid", ints[1]);
                nodes.Add(node);
            }
            rg.Start();
            return nodes;
        }
        public static int GetInstanceidByNodeName(List<BsonDocument> nodes, string nodeName)
        {
            for (int i = 0; i < nodes.Count(); i++)
            {
                BsonDocument node = nodes[i];
                if (nodeName.Equals(node.GetElement("nodeName").Value.ToString()))
                {
                    return node.GetElement("instanceid").Value.ToInt32();
                }
            }
            return -1;
        }
        public static string GetActualDataNodeName(DBCollection localcl)
        {
            DBCursor cur = localcl.Explain(null, null, null, null, 0, 10, 0, new BsonDocument("Run", true));
            BsonDocument doc = cur.Next();
            string nodeName = doc.GetElement("NodeName").Value.ToString();
            cur.Close();
            return nodeName;
        }
        public static bool IsMaster(Sequoiadb db, string groupName, string nodeName)
        {
            ReplicaGroup rg = db.GetReplicaGroup(groupName);
            if (nodeName.Equals(rg.GetMaster().NodeName.Trim()))
            {
                return true;
            }
            else
            {
                return false;
            }
        }

        public static List<int> getInstanceidList(List<BsonDocument> nodes)
        {
            List<int> instanceidList = new List<int>();
            for (int i = 0; i < nodes.Count(); i++)
            {
                BsonDocument node = nodes[i];
                instanceidList.Add(node.GetElement("instanceid").Value.ToInt32());
            }
            return instanceidList;
        }
    }
}
