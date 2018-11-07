package com.sequoiadb.cappedcl;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;
import java.util.ArrayList;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* FileName: InsertMoreCappedCL11778.java
* test content:test concurrentcy insert for more cappedCLs
* @author liuxiaoxuan
    * @Date    2017.7.18
*/
public class InsertMoreCappedCL11778 extends SdbTestBase{

        private Sequoiadb sdb = null;
        private String cappedCSName = "story_java_cappedCS_11778";
        private String cappedCLName = "cappedCL_11778";
        private int csNum = 2; //2 CSs
        private int clNum = 2; //each CS has 2 CLs
        private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	
	@BeforeClass
	public void setUp() {
           System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
           sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
           CappedCLUtils.createMoreCappedCL(sdb, cappedCSName, cappedCLName ,csNum,clNum);
	}
	
	@Test
        public void testGreatConcurrencyInsert() {
		StringBuffer strBuffer = new StringBuffer();
		
		InsertThread insertThread = new InsertThread(strBuffer);
		int threadNum = 20;
		insertThread.start(threadNum);
		
		Assert.assertTrue(insertThread.isSuccess(),insertThread.getErrorMsg());
		
		//check primary and slave datas
		sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
		for(int csNo = 1; csNo <= csNum; csNo++) {
         for(int clNo = 1; clNo <= clNum; clNo++) {  
         	DBCollection primaryCL = sdb.getCollectionSpace(cappedCSName + csNo).getCollection(cappedCLName + clNo);
			   Assert.assertTrue(CappedCLUtils.checkLogicalID(primaryCL, strBuffer.length(),this.getClass().getName()));
         }
      }
      System.out.println("-------sueccess to check primary node-------");
		
		sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'S'}"));
		for(int csNo = 1; csNo <= csNum; csNo++) {
         for(int clNo = 1; clNo <= clNum; clNo++) {  
			   DBCollection slaveCL = sdb.getCollectionSpace(cappedCSName + csNo).getCollection(cappedCLName + clNo);
			   Assert.assertTrue(CappedCLUtils.checkLogicalID(slaveCL, strBuffer.length(),this.getClass().getName()));
         }
      }
      System.out.println("-------sueccess to check slave node-------");
	}
	
   @AfterClass
	public void tearDown() {
		try {
			for(int csNo = 1; csNo <= csNum; csNo++) {
				CollectionSpace cs = sdb.getCollectionSpace(cappedCSName + csNo);
				if(cs != null) {
					sdb.dropCollectionSpace(cappedCSName + csNo);
				}
			}
		}catch (BaseException e) {
			e.printStackTrace();
		}finally {
			sdb.close();
			System.out.println(this.getClass().getName()+" end at "+sdf.format(new Date()));
		}
		
	}
	
	private class InsertThread extends SdbThreadBase{
	
        StringBuffer strBuffer = null;
        BSONObject obj = null;
    	
    	public InsertThread(StringBuffer strBuffer) {
    		this.obj = new BasicBSONObject();
    		this.strBuffer = strBuffer;
    		int stringLength = CappedCLUtils.getRandomStringLength();
    	    for(int len = 0; len < stringLength; len++) {    
    	    	strBuffer.append("a");
    	    }
    	    obj.put("a", strBuffer.toString());  
    	}
		
    	@Override
        public void exec() throws BaseException{
            Sequoiadb db = null;
            CollectionSpace cs = null;
            DBCollection cl = null;
            try{
                db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
                
                //insert records in all CLs
                for(int csNo = 1; csNo <= csNum; csNo++) {
                	cs = db.getCollectionSpace(cappedCSName + csNo);
                	for(int clNo = 1; clNo <= clNum; clNo++) {  
         	            cl = cs.getCollection(cappedCLName + clNo);
         	            CappedCLUtils.insertRecords(cl,obj); 
                	}
                }
    
            }catch(BaseException e){
                if(-23 != e.getErrorCode()  || -34 != e.getErrorCode()){
                    throw e;
                }
            }finally{
                db.close();
            }
        }
	}
}
