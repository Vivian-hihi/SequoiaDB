package com.sequoiadb.crud.truncate;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @FileName:seqDB-171:split与truncate的并发
 * 插入数据，一条线程执行切分，另一条线程执行truncate
 * @Author linsuqiang
 * @Date 2016-12-06
 * @Version 1.00
 */
public class TestTruncate171 extends SdbTestBase {
	private static Sequoiadb sdb = null;
	private String clName = "cl_171";
	private SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss.S" );
	private String srcGroupName = null;
	private String dstGroupName = null;
	
	private DBCollection createShardCL( Sequoiadb sdb, String csName, String clName ){
		try{
			if (!sdb.isCollectionSpaceExist(csName)){
				sdb.createCollectionSpace(csName);	
			}
		}catch(BaseException e){
			//-33 CS exist,ignore exceptions
			Assert.assertEquals(-33,e.getErrorCode(),e.getMessage());
		}
		DBCollection cl = null;
		String test = "{ShardingKey:{a:1},ShardingType:'hash',ReplSize:0,Compressed:true}";
		BSONObject options =(BSONObject) JSON.parse(test);
		try{
			CollectionSpace cs = sdb.getCollectionSpace(csName);
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}
			cl = cs.createCollection(clName,options);
		}catch(BaseException e){
			Assert.assertTrue(false,"create cl fail "+e.getErrorType()+":"+e.getMessage());
		}
		return cl;
	}
	
	@BeforeClass
	public void setUp() {
		System.out.println(this.getClass().getName()+" begin at "
				+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		try{
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		}catch(BaseException e){			
			Assert.assertTrue(false,"connect %s failed,"+SdbTestBase.coordUrl+e.getMessage());
		}
		if (Commlib.isStandAlone(sdb)){
			throw new SkipException("is standalone skip testcase");
		}
		
		if (Commlib.OneGroupMode(sdb)){
			throw new SkipException("less two groups skip testcase");
		}
		try{
			DBCollection cl = createShardCL( sdb, csName, clName );
			// doing insert
			Commlib.insertData( cl );
			// prepare data for splitting
			srcGroupName = Commlib.getSrcGroupName(sdb, cl);
			dstGroupName = Commlib.getDstGroupName(sdb, srcGroupName);
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
			// check truncate
			Commlib.checkTruncated( db, cl, hostName );
		}catch( BaseException e ){
			Assert.fail( e.getMessage() );
		}finally{
			db.disconnect();
		}
	}
	
	@Test
	public void launchSplit() {
		Sequoiadb db = null;
		DBCollection cl = null;
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl, "", "");
			cl = db.getCollectionSpace(csName).getCollection(clName);
			// doing split
			cl.split( srcGroupName, dstGroupName, 50 );
		}catch( BaseException e ){
			Assert.fail( e.getMessage() );
		}finally{
			db.disconnect();
		}
	}
}