package com.sequoiadb.metadataconsistency.cluster;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.annotations.Test;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.AfterClass;
import org.testng.Assert;
import org.testng.SkipException;

import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.metadataconsistency.data.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
* TestLink: seqDB-10223: concurrency[removeRG, alterDomain]
* @author xiaoni huang init
* @Date   2016.10.24
*/

public class Group10223 extends SdbTestBase {
	private SimpleDateFormat dateFm = new SimpleDateFormat("YYYY-MM-dd HH:mm:ss");
	private static Sequoiadb sdb = null;
	private static ArrayList<String> dataGroups = null;
	private String rgName = "rg10223";
	private String domainName = "dm10223";
	private Random random = new Random();
	private int msec = 100;
	
	@BeforeClass
	public void setUp(){
		//start time
		System.out.println("Begin to run " + getClass().getName() 
					+ ", begin in: " + dateFm.format(new Date().getTime()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			//judge the mode and group number
			if(CommLib.isStandAlone(sdb) || CommLib.OneGroupMode(sdb)){
				throw new SkipException("The mode is standlone, or only one group, "
						+ "skip the testCase.");
			}
			CommLib.clearDomain(sdb, domainName);
			CommLib.clearGroup(sdb, rgName);

			dataGroups = CommLib.getDataGroupNames(sdb);
			
			ReplicaGroup rg = sdb.createReplicaGroup(rgName);
			createNode();
			rg.start();
			
			createDomain(sdb);
		}catch(BaseException e){
			sdb.disconnect();
			Assert.fail(e.getMessage());
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			CommLib.clearDomain(sdb, domainName);
			CommLib.clearGroup(sdb, rgName);
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}finally{
			System.out.println("End to run " + getClass().getName() 
						+ ", end in: " + dateFm.format(new Date().getTime()));
			sdb.disconnect();
		}
	}
	
	@Test
	public void test(){
		
		AlterDomain alterDomain = new AlterDomain();
		alterDomain.start();

		RemoveRG removeRG = new RemoveRG();
		CommLib.sleep(random.nextInt(msec));
		removeRG.start();
		
		if( !( removeRG.isSuccess() && alterDomain.isSuccess() ) ){
			Assert.fail(removeRG.getErrorMsg() + alterDomain.getErrorMsg());
		}
		
		//check results
		CommLib.checkRGOfCatalog(rgName);
		CommLib.checkDomainOfCatalog(domainName);
	}

	private class RemoveRG extends SdbThreadBase{
		@Override
		public void exec() throws BaseException{
			Sequoiadb db  = null;
			try
			{
				db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
				db.removeReplicaGroup(rgName);
			}catch(BaseException e){
				//int eCode = e.getErrorCode();
				throw e;
			}finally{
				db.disconnect();
			}
		}
	}

	private class AlterDomain extends SdbThreadBase{
		@Override
		public void exec() throws BaseException{
			Sequoiadb db  = null;
			try
			{
				db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
				
				String[] groups = {dataGroups.get(0)};
				BSONObject opt = new BasicBSONObject();
				opt.put( "Groups", groups );
				sdb.getDomain(domainName).alterDomain(opt);
			}catch(BaseException e){
				int eCode = e.getErrorCode();
				if( eCode != -154 ){
					throw e;
				}
			}
			finally{
				db.disconnect();
			}
		}
	}
	
	public void createNode(){
		try
		{
			for(int i = 0; i < 3; i++){
				CommLib.createNode( sdb, rgName, 
						   SdbTestBase.reservedPortBegin, 
						   SdbTestBase.reservedPortEnd, 
						   SdbTestBase.reservedDir );
			}
		}catch(BaseException e){
			throw e;
		}
	}
	
	public void createDomain(Sequoiadb sdb){
		try
		{
			String[] rgArr = {rgName};
			BSONObject opt = new BasicBSONObject();
			opt.put( "Groups", rgArr );
			sdb.createDomain ( domainName, opt);
		}catch(BaseException e){
			throw e;
		}
	}
	
}