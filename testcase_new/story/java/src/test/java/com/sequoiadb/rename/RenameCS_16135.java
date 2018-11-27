package com.sequoiadb.rename;

import java.util.Arrays;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @Description RenameCS_16135.java 并发修改cs1、cs2名，cs2指定修改为cs1
 * @author luweikang
 * @date 2018年10月17日
 */
public class RenameCS_16135 extends SdbTestBase{
	
	private String csNameA = "renameCS_16135A";
	private String csNameB = "renameCS_16135B";
	private String newCSNameA = "renameCS_16135A_new";
	private String clNameA = "renameCS_CL_16135A";
	private String clNameB = "renameCS_CL_16135B";
	private Sequoiadb sdb = null;
	private CollectionSpace csA = null;
	private CollectionSpace csB = null;
	private DBCollection clA= null;
	private DBCollection clB= null;
	private int recordNum = 1000;
	
	@BeforeClass
	public void setUp(){
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		csA = sdb.createCollectionSpace(csNameA);
		csB = sdb.createCollectionSpace(csNameB);
		clA = csA.createCollection(clNameA);
		clB = csB.createCollection(clNameB);
		RenameUtil.insertData(clA, recordNum);
		RenameUtil.insertData(clB, recordNum);
	}
	
	@Test
	public void test(){ 
		RenameCSAThread reCSANameThread = new RenameCSAThread();
		RenameCSBThread reCSBNameThread = new RenameCSBThread();
		
		reCSANameThread.start();
		reCSBNameThread.start();
		
		boolean csARename = reCSANameThread.isSuccess();
		boolean csBRename = reCSBNameThread.isSuccess();
		
		Assert.assertTrue(csARename, reCSANameThread.getErrorMsg());
		if(!csBRename){
			Integer[] errnos = { -33, -147 };
			BaseException error = (BaseException)reCSBNameThread.getExceptions().get(0);
			if( !Arrays.asList(errnos).contains(error.getErrorCode()) ){
				Assert.fail(reCSBNameThread.getErrorMsg());
			}
		}
		
		try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
			if(csBRename){
				RenameUtil.checkRenameCSResult(db, csNameB, csNameA, 1);
			}else{
				RenameUtil.checkRenameCSResult(db, csNameA, csNameB, 1);
			}
			RenameUtil.checkRenameCSResult(db, "NotExistCS16135B", newCSNameA, 1);
		}
	}
	
	@AfterClass
	public void tearDown(){
		try {
			CommLib.clearCS(sdb, csNameA);
			CommLib.clearCS(sdb, csNameB);
			CommLib.clearCS(sdb, newCSNameA);
		} finally {
			if(sdb!=null){
				sdb.close();
			}
		}
	}
	
	private class RenameCSAThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			Thread.sleep(5);
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
				db.renameCollectionSpace(csNameA, newCSNameA);
			}
		}
	}
	
	private class RenameCSBThread extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			try( Sequoiadb db = new Sequoiadb(SdbTestBase.coordUrl, "", "") ) {
				db.renameCollectionSpace(csNameB, csNameA);
			}
		}
	}
	
}
