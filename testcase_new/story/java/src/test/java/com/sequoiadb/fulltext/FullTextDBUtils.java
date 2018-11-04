package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.testcommon.CommLib;

public class FullTextDBUtils {

    /**
     * @param cl
     * @param textIndexName
     */ 
     public static String getCappedCLName(DBCollection cl, String textIndexName) {
         String cappedCLName = "";
         DBCursor cur = cl.getIndex(textIndexName);
         cappedCLName = (String) cur.getNext().get("ExtDataName");
    	    
         return cappedCLName;
     }
	
     /**
      * @param db
      * @param csName
      * @param clName
      * @param textIndexName
      */ 
      public static List<DBCollection> getCappedCLs(Sequoiadb db, String csName, String clName, String textIndexName) {
          DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
          String cappedCLName = getCappedCLName( cl, textIndexName ); 
          List<String> groupNames = getCLGroups( db, csName + "." + clName);
          // sort groupname, in order to mapping esIndexNames <-> cappedCLs
          groupNames.sort(null);;
	      
          // get each cappedCL from each group
          List<DBCollection> cappedCLs = new ArrayList<>();
          for(String groupName : groupNames) {
              DBCollection cappedCL = db.getReplicaGroup(groupName).getMaster().connect().
                                        getCollectionSpace(cappedCLName).getCollection(cappedCLName);
              cappedCLs.add(cappedCL);
          }  
          return cappedCLs;
     }
	
     /**
      * @param db
      * @param csName
      * @param clName
      * @param textIndexName
      */ 
      public static List<String> getESIndexNames(Sequoiadb db, String csName, String clName, String textIndexName) {
          DBCollection cl = db.getCollectionSpace(csName).getCollection(clName);
          String cappedCLName = getCappedCLName(cl, textIndexName);
		
          // get es index names
          List<String> esIndexNames = new ArrayList<>();
          List<String> groupNames = getCLGroups(db, csName + "." + clName);
          // sort groupname, in order to mapping esIndexNames <-> cappedCLs
          groupNames.sort(null);
          for(String groupName : groupNames) {
              esIndexNames.add(cappedCLName.toLowerCase() + "_" + groupName);
          }
		     
          // if sharding cl, return all indices
          return esIndexNames;
     }
	
     /**
      * @param cappedCL
      */ 
      public static int getLastLid(DBCollection cappedCL) {
          long lastLogicalID = -1;
          BSONObject sortObj = new BasicBSONObject();
          sortObj.put("_id", 1);
          List<BSONObject> records = getRecordsFromCL(cappedCL, null, null, sortObj, null, 0, -1);
          if(records.size() > 0)
          { 
              BSONObject lastMatch = records.get(records.size()-1);
              lastLogicalID = (long) lastMatch.get("_id");
          } 
         return (int)lastLogicalID;
      }
	
      /**
       * @param cl
       * @param matcher
       * @param selector
       * @param orderBy
       * @param hint
       * @param skip
       * @param limit
       */ 
       public static List<BSONObject> getRecordsFromCL(DBCollection cl, BSONObject matcher, BSONObject selector, BSONObject orderBy, BSONObject hint, long skip, long limit) {
           List<BSONObject> objs = new ArrayList<>();
           DBCursor cur = cl.query(matcher, selector, orderBy,  hint, skip, limit);
           while(cur.hasNext()) {
               BSONObject obj = cur.getNext();
               objs.add(obj);
           }
           return objs;
       }
	
       /**
        * @param db
        * @param clFullName
        */ 
        public static List<String> getCLGroups(Sequoiadb db, String clFullName) {		
            if(CommLib.isStandAlone(db)) {
                return new ArrayList<>();
            }
            List<String> groupNames = new ArrayList<>();
            BSONObject matcher = new BasicBSONObject();
            matcher.put("Name", clFullName);
            DBCursor cur = db.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG , matcher, null, null);
            while(cur.hasNext()) {
                BasicBSONList bsonLists = (BasicBSONList) cur.getNext().get("CataInfo");
                for(int i = 0; i < bsonLists.size(); i++) {
                    BasicBSONObject obj = (BasicBSONObject) bsonLists.get(i);
                    groupNames.add(obj.getString("GroupName"));
                }			
            }
		
            return groupNames;
        }
	
}
