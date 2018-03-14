using System;
using System.Text;
using System.Collections.Generic;
using System.Linq;
using Microsoft.VisualStudio.TestTools.UnitTesting;
using SequoiaDB;
using CSharp.TestCommon;
using SequoiaDB.Bson;

namespace CSharp.TestCommon
{
    public class Common
    {
        public static int CompareBson(BsonDocument x, BsonDocument y)
        {
            return x.CompareTo(y);
        }

        public static bool isStandalone(Sequoiadb sdb)
        {
            try
            {
                sdb.ListReplicaGroups();
            }
            catch (BaseException e)
            {
                if (e.ErrorCode == -159) // -159: The operation is for coord node only
                    return true;
            }
            return false;
        }

        /// <summary>
        /// judge whether two BsonDocuments equal. 
        /// this func can adapt disorder key.
        /// </summary>
        public static bool isEquals(BsonDocument a, BsonDocument b)
        {
            if (a.ElementCount != b.ElementCount)
                return false;
            for (int i = 0; i < a.ElementCount; ++i)
            {
                String name = a.ElementAt(i).Name;
                if (!b.Contains(name))
                    return false;
                if (!a.GetElement(name).Equals(b.GetElement(name)))
                    return false;
            }
            return true;
        }
    }
}
