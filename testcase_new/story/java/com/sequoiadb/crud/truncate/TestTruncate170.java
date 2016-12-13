package com.sequoiadb.crud.truncate;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName:seqDB-170:查询与truncate的并发
 * 插入数据，一条线程执行查询，另一条线程执行truncate
 * @Author linsuqiang
 * @Date 2016-12-06
 * @Version 1.00
 * other:
 * 存在BUG,正在修正，故暂时跳过
 * 对应JIRA问题单：2111
 * 修正后请将本文件中@Test的enabled=false删除
 */
public class TestTruncate170 extends SdbTestBase {
	private static Sequoiadb sdb = null;
	private String clName = "cl_170";
	private SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss.S" );
	
	@BeforeClass
	public void setUp() {
		System.out.println( this.getClass().getName()+" begin at "+sdf.format( new Date() ) );
		try{
			sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
			DBCollection cl = Commlib.createCL( sdb, csName, clName );
			// doing insert
			Commlib.insertData( cl );
		}catch(BaseException e){
			Assert.fail( e.getMessage() );
		}
	}
	
	@AfterClass
	public void tearDown(){
		try{
			CollectionSpace cs = sdb.getCollectionSpace( csName );	
			if( cs.isCollectionExist( clName ) ){
				cs.dropCollection( clName );
			}
			sdb.disconnect();
		}catch( BaseException e ){			
			Assert.fail( e.getMessage() );
		}finally{
			System.out.println( this.getClass().getName()+" end at "+sdf.format( new Date() ) );
		}
	}
	
	// to encounter the occasional problem, set repeat count as 100
	@Test( invocationCount = 100, enabled = false )
	public void launchTruncate() {
		Sequoiadb db = null;
		DBCollection cl = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			cl = db.getCollectionSpace(csName).getCollection(clName);
			// doing truncate
			cl.truncate();
			// check truncate
			Commlib.checkTruncated( db, cl, hostName );
		}catch( BaseException e ){
			Assert.fail( e.getMessage() );
		}finally{
			db.disconnect();
		}
	}
	
	@Test( invocationCount = 100, enabled = false )
	public void launchQuery() {
		Sequoiadb db = null;
		DBCollection cl = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			cl = db.getCollectionSpace(csName).getCollection(clName);
			// doing query
			cl.query();
		}catch( BaseException e ){
			Assert.fail(e.getMessage());
		}finally{
			db.disconnect();
		}
	}
}