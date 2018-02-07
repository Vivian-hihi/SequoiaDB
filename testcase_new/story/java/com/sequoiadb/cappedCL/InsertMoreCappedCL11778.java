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
* test content:test concurrentcy insert for more cappedCLs
* @author liuxiaoxuan
    * @Date    2017.7.18
*/
public class InsertMoreCappedCL11778 extends SdbTestBase{

	private Sequoiadb sdb = null;
	private List<DBCollection> cappedCLs = new ArrayList<DBCollection>();
	private String cappedCSName_11778 = "story_java_cappedCS_11778";
	private String cappedCLName_11778 = "cappedCL_11778";
	private int csNum = 2; //2 CSs
	private int clNum = 2; //each CS has 2 CLs
	private SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.S");
	
	@BeforeClass
	public void setUp() {
		System.out.println(this.getClass().getName()+" begin at "+sdf.format(new Date()));
		try {
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "","");
			sdb.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));
			cappedCLs = CappedCLUtils.createMoreCappedCL(sdb, cappedCSName_11778, cappedCLName_11778 ,csNum,clNum);
		}catch(BaseException e) {
			Assert.fail(e.getMessage());
		}
	}
	
	@Test
    public void testGreatConcurrencyInsert() {
		StringBuffer strBuffer = new StringBuffer();
		
		InsertThread insertThread = new InsertThread(strBuffer);
		int threadNum = 20;
		insertThread.start(threadNum);
		
		Assert.assertTrue(insertThread.isSuccess(),insertThread.getErrorMsg());
		
		//check all cappedCLs' logicalID
		for(DBCollection cl : cappedCLs) {
			if(cl != null) {
				 Assert.assertTrue(CappedCLUtils.checkLogicalID(cl, strBuffer.length() ,this.getClass().getName()));
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
			Assert.fail(e.getMessage());
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
                db.setSessionAttr((BSONObject)JSON.parse("{PreferedInstance:'M'}"));               
                
                //insert records in all CLs
                for(int csNo = 1; csNo <= csNum; csNo++) {
                	cs = db.getCollectionSpace(cappedCSName_11778 + csNo);
                	for(int clNo = 1; clNo <= clNum; clNo++) {  
         	            cl = cs.getCollection(cappedCLName_11778 + clNo);
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
