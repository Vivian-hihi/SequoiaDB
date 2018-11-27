package com.sequoiadb.fulltext;

import java.util.ArrayList;
import java.util.List;
import java.util.HashSet;
import org.testng.Assert;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;

import org.elasticsearch.client.*;

public class FullTextUtils {

     public static final int INSERT_NUMS = 200000; // insert 20w datas

    /**
     * @param esClient    
     * @param db
     * @param csName
     * @param clName
     * @param textIndexName
     * @param expectCount
     */ 
     public static void checkFullSyncToES(Client esClient, Sequoiadb db, String csName, String clName, String textIndexName, int expectCount) {
         List<String> esIndexNames = FullTextDBUtils.getESIndexNames(db, csName, clName, textIndexName);
         esIndexNames = removeDuplicateItems(esIndexNames);
         // sort esIndexNames
         FullTextDBUtils.compare(esIndexNames);
         List<DBCollection> cappedCLs = FullTextDBUtils.getCappedCLs(db, csName, clName, textIndexName);

         // check indexnames sync to ES
         for(String esIndexName: esIndexNames) {
             String msg = esIndexName + " is not exist";
             Assert.assertTrue(FullTextESUtils.isExistIndexInES(esClient, esIndexName), msg);
         }
		   
         // check all indices sync to ES
         checkCountInES(esClient, esIndexNames, expectCount);
         checkLidInES(esClient, esIndexNames, cappedCLs);
     }

     /**
     * @param esClient
     * @param db
     * @param csName
     * @param mainCLName
     * @param textIndexName
     * @param expectCount
     */
     public static void checkMainCLFullSyncToES(Client esClient, Sequoiadb db, String csName, String mainCLName, String textIndexName, int expectCount) {
          List<String> subCLFullNames = FullTextDBUtils.getSubCLNames(db, csName + "." + mainCLName);
          
          // get all indexs from maincl 
          List<String> esIndexNames = new ArrayList<>();
          List<DBCollection> cappedCLs = new ArrayList<>();
          for(String subCLFullName: subCLFullNames){
              String subCSName = subCLFullName.split("\\.")[0];
              String subCLName = subCLFullName.split("\\.")[1];

              esIndexNames.addAll(FullTextDBUtils.getESIndexNames(db, subCSName, subCLName, textIndexName));
              cappedCLs.addAll(FullTextDBUtils.getCappedCLs(db, subCSName, subCLName, textIndexName));
          }  
          esIndexNames = removeDuplicateItems(esIndexNames);
          // sort esIndexNames
          FullTextDBUtils.compare(esIndexNames);
        
          // check indexnames sync to ES
          for(String esIndexName: esIndexNames) {
             String msg = esIndexName + " is not exist";
             Assert.assertTrue(FullTextESUtils.isExistIndexInES(esClient, esIndexName), msg);
          }

          // check all indices sync to ES
          checkCountInES(esClient, esIndexNames, expectCount);
          checkLidInES(esClient, esIndexNames, cappedCLs);
     }

     /**
     * @param esClient
     * @param esIndexNames
     */ 
     public static void checkIndexNotExistInES(Client esClient, List<String> esIndexNames) {
        // check indexnames sync to ES
        for(String esIndexName: esIndexNames) {
               String msg = esIndexName + " is exist";
               Assert.assertTrue(FullTextESUtils.isIndexDeletedInES(esClient, esIndexName), msg);
        }                      
     }

     /**
      * @param esClient     
      * @param esIndexNames
      * @param expectCount
      */ 
      public static void checkCountInES(Client esClient, List<String> esIndexNames, int expectCount) {
          boolean isSync = false;
          int timeout = 600;
          int doTimes = 0;
          int interval = 1;  //interval 1s
          int actCount = 0;
		 
          while(true) {
              actCount = 0;
              // Add counts of all indices 
              for(String esIndexName : esIndexNames) {
                  actCount += (FullTextESUtils.getCountFromES(esClient, esIndexName) - 1);
              }
		     		     
              // if expect count < act count, exit
              if(actCount == expectCount) { 
                  isSync = true;
                  break;
              }
              else {
                  doTimes++;
                  try {
                      Thread.sleep(1000);
                  } catch (InterruptedException e) {
                      e.printStackTrace();
                  } 
              }

          }
          // after count fininsh sync
          System.out.println("esIndexNames: " + esIndexNames.toString() + ", doTimes: " + doTimes + ", actCount: " + actCount + ", expectCount: " + expectCount);	  

          // print message while not finish sync
          String msg = "";
          for(String esIndexName : esIndexNames) {  msg += esIndexName + "/";  }
          Assert.assertTrue(isSync, "check " + msg + " count syn to es fail");
     }
	
     /**
      * @param esClient     
      * @param esIndexNames
      * @param cappedCLs
      */ 
      public static void checkLidInES(Client esClient, List<String> esIndexNames, List<DBCollection> cappedCLs) {
          boolean isSync = false;
          int timeout = 600;
          int doTimes = 0;
          int interval = 1;  //interval 1s
		 
          // get all lids from all groups
          List<Integer> lastLogicalIDs = new ArrayList<>();
          for(DBCollection cappedCL:  cappedCLs) {
              lastLogicalIDs.add(FullTextDBUtils.getLastLid(cappedCL));   
          }

          List<Integer> commitIDs = null;
		 
          while(true) {
              // get all commitids from all esIndexNames 
              commitIDs = new ArrayList<>();
              for(int i = 0; i < esIndexNames.size(); i++) { 
                  commitIDs.add(FullTextESUtils.getCommitIDFromES(esClient, esIndexNames.get(i)));  
              }
		    
              for(int i = 0; i < esIndexNames.size(); i++) {	 
                  if(commitIDs.get(i).intValue() != lastLogicalIDs.get(i).intValue()) {  
                      isSync = false;
                      break;
                  }
                  else {
                      isSync = true;
                  }	
              }
		    
              // check sync finish or not
              if(isSync) {
                  break;
              }  
              else {
                  doTimes++;
                  try {
                      Thread.sleep(1000);
                  } catch (InterruptedException e) {
                      e.printStackTrace();
                  } 
              }   

          }

          // after lid finish sync
          System.out.println("esIndexNames: " + esIndexNames.toString() + ", doTimes: " + doTimes + ", commitIDs: " + commitIDs.toString() + ", lastLogicalIDs: " + lastLogicalIDs.toString());
		 
          // print message while not finish sync
          String msg = "";
          for(String esIndexName : esIndexNames) {  msg += esIndexName + "/";  }
          Assert.assertTrue(isSync, "check " + msg + " lid syn to es fail");
     }

     /**
      * @param arrayList
      */ 
      public static List<String> removeDuplicateItems(List<String> arrayList){
          HashSet uniqueSet = new HashSet(arrayList);
          arrayList.clear();
          arrayList.addAll(uniqueSet);
          return arrayList;
      }
}
