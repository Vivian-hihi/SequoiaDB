package com.story.consistency;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.Random;

import org.testng.annotations.Test;
import org.testng.annotations.Parameters;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.story.consistency.CommLib;

/**
* TestLink: seqDB-10212
* @author xiaoni huang init
* @Date   2016.10.17
*/

public class Index10212 {
	private CommLib CommLib = new CommLib();
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private String csName = "cs10212";
	private String clName = "cl10212";
	private String idxName = "idx";
	
	
	@BeforeClass
	@Parameters({"coordAddr"})
	public void setUp(String coordAddr){
		//start time
		System.out.println("Begin to run " + this.getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(coordAddr, "", "");
			//clear env
			CommLib.clearCL(sdb, csName, clName);
			CommLib.clearCS(sdb, csName);
			//create cs/cl
			sdb.createCollectionSpace(csName);
			sdb.getCollectionSpace(csName).createCollection(clName);
		}catch(BaseException e){
			Assert.fail("Failed to prepare env at th begining. "
					+ "ErrorMsg:\n" +e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			//clear env
			CommLib.clearCL(sdb, csName, clName);
			CommLib.clearCS(sdb, csName);
		}catch(BaseException e){
			Assert.fail("ErrorMsg:\n" +e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test(invocationCount = 10, threadPoolSize = 10)
	@Parameters({"coordAddr"})
	public void testIdIndex10212(String coordAddr){
		Sequoiadb db  = null;
		
		//-----create index-----
		try{
			db = new Sequoiadb(coordAddr, "", "");
			DBCollection clDB = db.getCollectionSpace(csName).getCollection(clName);

			String name = idxName;
			Random i = new Random();
			BSONObject opt = new BasicBSONObject();
			opt.put("a" + i.nextInt(10000), 1);
			clDB.createIndex(name + i.nextInt(100), opt, false, false);
			//check results
			Index10212 Index10212 = new Index10212();
			Index10212.checkResult(db);
		}catch(BaseException e){
			if(e.getErrorCode() != -247){  //-247:Redefine index
				Assert.fail(e.getMessage());
			}
		}
		
		//-----drop cl-----
		try{
			db.getCollectionSpace(csName).dropCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -23){  //-23:Collection does not exist
				Assert.fail(e.getMessage());
			}
		}
		
		//-----create cl-----
		try{
			db.getCollectionSpace(csName).createCollection(clName);
		}catch(BaseException e){
			if(e.getErrorCode() != -22){  //-22:Collection already exists
				Assert.fail(e.getMessage());
			}
		}finally{
			db.disconnect();
		}
		
	}
	
	public void checkResult(Sequoiadb sdb){
		try{
			if(CommLib.isStandAlone(sdb)){
				DBCollection clDB = sdb.getCollectionSpace(csName).getCollection(clName);
				DBCursor idxInfo = clDB.getIndex("$id");
				if(idxInfo.hasNext()){
					clDB.dropIdIndex();
				}else{
					BSONObject opt = new BasicBSONObject();
					opt.put("SortBufferSize", 128);
					clDB.createIdIndex(opt);
				}
			}else{
				CommLib.checkIndex(sdb, csName, clName);
			}
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}
	}
}