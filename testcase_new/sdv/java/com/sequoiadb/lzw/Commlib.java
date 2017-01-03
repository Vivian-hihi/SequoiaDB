package com.sequoiadb.lzw;

import java.util.ArrayList;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
import org.testng.Assert;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

public class Commlib extends SdbTestBase {
    public static ArrayList<String> groupList;
    
    public static boolean isStandAlone(Sequoiadb sdb) {
        try {
            sdb.listReplicaGroups();
        } catch (BaseException e) {
            if (e.getErrorCode() == -159) {
                System.out.printf("run mode is standalone");
                return true;
            }
        }
        return false;
    }

    public static boolean OneGroupMode(Sequoiadb sdb) {
        if (getDataGroups(sdb).size() < 2) {
            System.out.printf("only one group");
            return true;
        }
        return false;
    }

    public static ArrayList<String> getDataGroups(Sequoiadb sdb) throws BaseException {
        try {
            groupList = sdb.getReplicaGroupNames();
            groupList.remove("SYSCatalogGroup");
            groupList.remove("SYSCoord");
            groupList.remove("SYSSpare");
        } catch (BaseException e) {
            e.printStackTrace();
            throw e;
        }
        return groupList;
    }
    
    public static void checkCompressed(DBCollection cl, String dataGroupName){
        // connect to data node of cl
        Sequoiadb db = cl.getSequoiadb();
        Sequoiadb dataDB = getDataDB(db, dataGroupName);
        checkCompression(dataDB, cl.getName());
    }
    
    public static Sequoiadb getDataDB(Sequoiadb db, String dataGroupName){
        // connect to data node of cl 
        int dataGroupPort = db.getReplicaGroup(dataGroupName).getMaster().getPort();
        return new Sequoiadb(hostName + " : " + dataGroupPort, "", "");
    }
    
    public static void checkCompression(Sequoiadb dataDB, String clName){
        // get details of snapshot
        BSONObject nameBSON = new BasicBSONObject();
        nameBSON.put("Name", csName + "." + clName);
        DBCursor snapshot = dataDB.getSnapshot(4, nameBSON, null, null);
        BasicBSONList details = (BasicBSONList) snapshot.getNext().get("Details");
        BSONObject detail = (BSONObject) details.get(0);
        
        // judge whether data is compressed
        boolean ratioRight = (double)detail.get("CurrentCompressionRatio") < (double)1;
        boolean attrRight = ((String)detail.get("Attribute")).equals("Compressed");
        boolean typeRight = ((String)detail.get("CompressionType")).equals("lzw");
        if(!(ratioRight && attrRight && typeRight)){
            Assert.fail("data is not compressed");
        }
    }
}