package com.sequoiadb.cappedCL;

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
* test content:test concurrentcy insert more cappedCL 
* @author liuxiaoxuan
    * @Date    2017.7.18
*/
public class InsertMoreCappedCL11778 extends SdbTestBase{

	private Sequoiadb sdb = null;
	private List<DBCollection> cappedCLs = new ArrayList<DBCollection>();
	private String cappedCSName_11778 = "story_java_cappedCS_11778";
	private String cappedCLName_11778 = "cappedCL_11778";
	private int csNum = 2; //2 cs
	private int clNum = 2; //each cs has 2 cl
	private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	
	@BeforeClass
	public void setUp() {
		System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
		try {
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
			cappedCLs = Commlib.createMoreCappedCL(sdb, cappedCSName_11778, cappedCLName_11778 ,csNum,clNum);
		}catch(BaseException e) {
//			System.out.println("Error message: " + e.getMessage());
			Assert.fail(e.getMessage());
		}
	}
	
	@Test
    public void testGreatConcurrencyInsert() {
		StringBuffer strBuffer = new StringBuffer();
		
		InsertThread insertThread = new InsertThread(strBuffer);
		int threadNum = 10;
		insertThread.start(threadNum);
		
		Assert.assertTrue(insertThread.isSuccess(),insertThread.getErrorMsg());
		
		//check all cappedCLs' logicalID
		for(int clNo = 0; clNo < cappedCLs.size(); clNo++) {
			DBCollection cl = cappedCLs.get(clNo);
			 if(cl != null) {
				 Assert.assertTrue(Commlib.checkLogicalID(sdb, cl, strBuffer.length()));
			 } 
		}

				
	}
	
	@AfterClass
	public void tearDown() {
		try {
			for(int csNo = 1; csNo <= csNum; csNo++) {
				CollectionSpace cs = sdb.getCollectionSpace(cappedCSName_11778 + csNo);
				if(cs != null) {
					sdb.dropCollectionSpace(cappedCSName_11778 + csNo);
				}
			}
		}catch (BaseException e) {
//			System.out.println("11778 teardown error: " + e.getMessage());
			Assert.fail(e.getMessage());
		}finally {
			sdb.close();
//			System.out.println(this.getClass().getName()+" end at "+sdf.format(new Date()));
		}
		
	}
	
	private class InsertThread extends SdbThreadBase{
	
        StringBuffer strBuffer = null;
        BSONObject obj = null;
    	
    	public InsertThread(StringBuffer strBuffer) {
    		this.obj = new BasicBSONObject();
    		this.strBuffer = strBuffer;
    		int stringLength = Commlib.getRandomStringLength();
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
                db.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));               
                
				//2 CSs and 2CLs,total all 4 CLs
                for(int csNo = 1; csNo <= csNum; csNo++) {
                	cs = db.getCollectionSpace(cappedCSName_11778 + csNo);
                	for(int clNo = 1; clNo <= clNum; clNo++) {  
         	            cl = cs.getCollection(cappedCLName_11778 + clNo);
         	            Commlib.insertRecords(cl,strBuffer,obj); 
                	}
                }
    
            }catch(BaseException e){
//            	System.out.println("11778 ERROR_EXEC:"+e);
                if(-23 != e.getErrorCode()  || -34 != e.getErrorCode()){
                    throw e;
                }
            }finally{
                db.close();
            }
        }
	}
}
