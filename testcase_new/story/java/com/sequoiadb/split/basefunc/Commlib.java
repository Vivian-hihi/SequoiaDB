package com.sequoiadb.split.basefunc;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;

import com.sequoiadb.base.CollectionSpace;
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

	public static ArrayList<String> getDataGroups(Sequoiadb sdb) {
		try {
			groupList = sdb.getReplicaGroupNames();
			groupList.remove("SYSCatalogGroup");
			groupList.remove("SYSCoord");
			groupList.remove("SYSSpare");
		} catch (BaseException e) {
			throw e;
		}
		return groupList;
	}
	
	public static DBCollection createHashCl(Sequoiadb db, String clName, String dataGroupName){
	    CollectionSpace cs = db.getCollectionSpace(csName);
	    BSONObject option = (BSONObject)JSON.parse("{ShardingKey:{a:1}, ShardingType:'hash', Group:'" + dataGroupName + "'}");
	    DBCollection cl = cs.createCollection(clName, option);
	    return cl;
	}
    
    public static void checkSplitOnCoord(DBCollection cl, List<BSONObject> expRecs) {
        try {
            List<BSONObject> actRecs = new ArrayList<BSONObject>();
            DBCursor cursor = cl.query(null, null, "{_id:1}", null);
            while( cursor.hasNext() ) {
                BSONObject obj = cursor.getNext();
                actRecs.add(obj);
            }
            cursor.close();
            if(actRecs.size() != expRecs.size()){
                System.out.println(cl.getCount());
                System.out.println(actRecs.size());
                System.out.println(expRecs.size());
                throw new BaseException("data is different");
            }
            for(int i = 0; i < actRecs.size(); i++){
                if(!actRecs.get(i).equals(expRecs.get(i))){
                    System.out.println(actRecs.get(i));
                    System.out.println(expRecs.get(i));
                    throw new BaseException("data is different");
                }
            }
/*            if(!actRecs.equals(expRecs)){
                throw new BaseException("data is different");
            }*/
        } catch (BaseException e) {
            throw e;
        }
    }
    
    public static Sequoiadb getDataDB(Sequoiadb db, String dataGroupName){ 
        int dataGroupPort = db.getReplicaGroup(dataGroupName).getMaster().getPort();
        return new Sequoiadb(hostName + " : " + dataGroupPort, "", "");
    }
    
    public static void checkSplitOnData(Sequoiadb dataDB, String clName, int expCnt, int offSet) {
        try {
            DBCollection cl = dataDB.getCollectionSpace(csName).getCollection(clName);
            int actCnt = (int)cl.getCount();
            if(Math.abs(actCnt - expCnt) > offSet){
                throw new BaseException("actual count:[" + actCnt + "]" 
                        + "excepted count:[" + expCnt + "]"
                        + "the split result is wrong");
            }
        } catch (BaseException e) {
            throw e;
        } finally {
            dataDB.disconnect();
        }
    }
}