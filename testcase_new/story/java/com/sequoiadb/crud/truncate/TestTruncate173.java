package com.sequoiadb.crud.truncate;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
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
 * @FileName:seqDB-173:insert与truncate的并发
 * 插入数据，一条线程执行insert，另一条线程执行truncate
 * @Author linsuqiang
 * @Date 2016-12-06
 * @Version 1.00
 */
public class TestTruncate173 extends SdbTestBase {
	private static Sequoiadb sdb = null;
	private String clName = "cl_173";
	private SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss.S" );
	private BSONObject record = null;
	
	@BeforeClass
	public void setUp() {
		System.out.println( this.getClass().getName()+" begin at "+sdf.format( new Date() ) );
		try{
			sdb = new Sequoiadb( SdbTestBase.coordUrl, "", "" );
			DBCollection cl = Commlib.createCL( sdb, csName, clName );
			// doing insert
			Commlib.insertData( cl );
			// prepare data for insert(below)
			record = new BasicBSONObject();
			record.put( "key", "123456789" );
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
	
	@Test
	public void launchTruncate() {
		Sequoiadb db = null;
		DBCollection cl = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			cl = db.getCollectionSpace(csName).getCollection(clName);
			// doing truncate
			cl.truncate();
		}catch( BaseException e ){
			Assert.fail( e.getMessage() );
		}finally{
			db.disconnect();
		}
	}
	
	@Test
	public void launchInsert(){
		Sequoiadb db = null;
		DBCollection cl = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			cl = db.getCollectionSpace(csName).getCollection(clName);
			// doing insert
			cl.insert( record );
		}catch( BaseException e ){
			Assert.fail(e.getMessage());
		}finally{
			db.disconnect();
		}
	}
}