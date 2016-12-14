package com.sequoiadb.subcl;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.Iterator;
import java.util.List;

import org.bson.BSONObject;
import org.bson.types.BasicBSONList;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * @TestName: 数据操作同时对子表进行删除
 * @author ouyangzhongnan
 * @version 1.00
 */
public class CrudDeleteSubclTest10483 extends SdbTestBase{
	
	private static Sequoiadb sdb = null;
	private static Sequoiadb sdb_other = null;
	private ArrayList<String> replicaGroupNames = null;
	private CollectionSpace cs = null;
	private DBCollection maincl;
	private DBCollection[] subcls = {null,null,null};
	private String mainclName = "maincl_10483";
	private String[] subclNames = {"maincl_10483_subcl_0","maincl_10483_subcl_1","maincl_10483_subcl_2"};
	private SimpleDateFormat sdf = new SimpleDateFormat( "yyyy-MM-dd HH:mm:ss.S" );
	
	@BeforeClass
	public void setUp() {
		System.out.println( this.getClass().getName()+" begin at "+sdf.format( new Date() ) );
		try {
			sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		} catch (BaseException e) {
			Assert.assertTrue(false, "connect %s failed,"
					+ SdbTestBase.coordUrl + e.getMessage());
		}
		
		try {
			replicaGroupNames = sdb.getReplicaGroupNames();
			List<String> exclude = new ArrayList<>();
			exclude.add("SYSCoord");
			exclude.add("SYSCatalogGroup");
			replicaGroupNames.removeAll(exclude);
			if (replicaGroupNames.size() < 3) {
				Assert.assertTrue(false,"At least three data sets are required !");
			}			
		} catch (BaseException e) {
			Assert.assertTrue(false,"judge at least three data sets failed "+e.getMessage());
		}

		
		try{
			ReplicaGroup coordRG = sdb.getReplicaGroup("SYSCoord");
			BSONObject coordRGBson = (BSONObject)coordRG.getDetail();
			BasicBSONList coordList = (BasicBSONList) coordRGBson.get("Group");
			Iterator<Object> iterator = coordList.iterator();
			while(iterator.hasNext()) {
				BSONObject bsonObject = (BSONObject) iterator.next();
				String coordHostName = (String) bsonObject.get("HostName");
				if (!coordHostName.equals(SdbTestBase.hostName)) {
					sdb_other = new Sequoiadb(coordHostName+":"+SdbTestBase.serviceName, "", "");
					break;
				}
			}			
		} catch (BaseException e) {
			Assert.assertTrue(false,"get other coord and to connection it failed "+e.getMessage());
		}
		
		createCL();
		initData();
	}
	
	@Test
	public void crud() {
		try {
			int a = 0;
			for (int i = 600; i < 3000; i++) {
				a = i%300;
				maincl.insert((BSONObject) JSON.parse(" {name_:'"+i+"',a:"+a+"} "));
			}
			for (int i = 0; i < 300; i++) {
				maincl.upsert((BSONObject) JSON.parse(" {name:'name_"+i+"'} "), 
						(BSONObject) JSON.parse(" {$set:{name:'updatename_"+i+"'}} "), null);
			}
			for (int i = 300; i < 600; i++) {
				maincl.delete((BSONObject) JSON.parse(" {name:'name_"+i+"'} "));
			} 
			Assert.assertTrue(false, "crud data faild");
		} catch (BaseException e) {
			Assert.assertEquals(e.getErrorCode(), -135, "crud data faild: "+e.getMessage());
		} catch (Exception e) {
			Assert.assertTrue(false, "crud data faild");
		}
	}
	
	@Test
	public void deleteSubcl() {
		try {
			sdb_other.getCollectionSpace(SdbTestBase.csName).dropCollection(subclNames[2]);
		} catch (Exception e) {
			Assert.assertTrue(false,"delete subcl faild: "+ e.getMessage());
		} 
	}
	
	@AfterClass
	public void tearDown() {
		checkData();
		try{
			CollectionSpace cs = sdb.getCollectionSpace( csName );	
			for (String subclName : subclNames) {
				if( cs.isCollectionExist( subclName ) ){
					cs.dropCollection( subclName );
				}
			}
			if( cs.isCollectionExist( mainclName ) ){
				cs.dropCollection( mainclName );
			}			
			sdb.disconnect();
		}catch( BaseException e ){			
			Assert.fail( e.getMessage() );
		}finally{
			System.out.println( this.getClass().getName()+" end at "+sdf.format( new Date() ) );
		}
	}
	
	private void createCL() {
		try {
			if (!sdb.isCollectionSpaceExist(SdbTestBase.csName)) {
				sdb.createCollectionSpace(SdbTestBase.csName);
			}
		} catch (BaseException e) {
			Assert.assertEquals(-33, e.getErrorCode(), e.getMessage());
		}
		try {
			String mainclOptions = "{IsMainCL:true, ShardingKey:{a:1},ShardingType:'range',ReplSize:0,Compressed:true}";
			String[] subclOptions = {
					"{Group:'"+replicaGroupNames.get(0)+"'}",
					"{Group:'"+replicaGroupNames.get(1)+"',ShardingKey:{a:1},ShardingType:'range',ReplSize:0,Compressed:true}",
					"{Group:'"+replicaGroupNames.get(2)+"',ShardingKey:{a:1},ShardingType:'hash',ReplSize:0,Compressed:true,Partition:16}"
					};
			cs = sdb.getCollectionSpace(SdbTestBase.csName);
			maincl = cs.createCollection(mainclName, (BSONObject) JSON.parse(mainclOptions));
			subcls[0] = cs.createCollection(subclNames[0], (BSONObject) JSON.parse(subclOptions[0]));
			subcls[1] = cs.createCollection(subclNames[1], (BSONObject) JSON.parse(subclOptions[1]));
			subcls[2] = cs.createCollection(subclNames[2], (BSONObject) JSON.parse(subclOptions[2]));
			maincl.attachCollection(SdbTestBase.csName+"."+subclNames[0], (BSONObject) JSON.parse("{ LowBound:{a:0},UpBound:{a:100} }"));
			maincl.attachCollection(SdbTestBase.csName+"."+subclNames[1], (BSONObject) JSON.parse("{ LowBound:{a:100},UpBound:{a:200} }"));
			maincl.attachCollection(SdbTestBase.csName+"."+subclNames[2], (BSONObject) JSON.parse("{ LowBound:{a:200},UpBound:{a:300} }"));
		} catch (BaseException e) {
			Assert.assertTrue(false,"create collections faild: "+e.getMessage());
		}	
	}	
	
	private void initData() {
		try {
			List<BSONObject> initData = new ArrayList<>();
			int a = 0;
			for (int i = 0; i < 600; i++) {
				a = i%300;
				initData.add((BSONObject)JSON.parse(" {name:'name_"+i+"', a:"+a+"} "));
			}
			maincl.bulkInsert(initData, 0);
			subcls[1].split(replicaGroupNames.get(1), replicaGroupNames.get(2), 50);
			subcls[2].split(replicaGroupNames.get(2), replicaGroupNames.get(1), 50);
		} catch (BaseException e) {
			Assert.assertTrue(false,"init data faild: "+e.getMessage());
		}
	}
	
	private void checkData() {
		boolean flag = true;
		try {
			//check delete subcl record
			DBCursor query = maincl.query("{a:250}", null, null, null);
			if (!query.hasNext()) {
				//check subcl and group
				DBCursor explain_A50 = maincl.explain((BSONObject)JSON.parse(" {a:50} "), null, null, null, 0, -1, DBQuery.FLG_QUERY_STRINGOUT, null);
				DBCursor explain_A149 = maincl.explain((BSONObject)JSON.parse(" {a:149} "), null, null, null, 0, -1, DBQuery.FLG_QUERY_STRINGOUT, null);
				DBCursor explain_A150 = maincl.explain((BSONObject)JSON.parse(" {a:150} "), null, null, null, 0, -1, DBQuery.FLG_QUERY_STRINGOUT, null);
				BSONObject[] objArr =  { explain_A50.getNext(), explain_A149.getNext(), explain_A150.getNext()};
				
				if ( (explain_A50.getNext() == null || explain_A149.getNext() == null || explain_A150.getNext() == null) &&
						( ((BasicBSONList)objArr[0].get("SubCollections")).size() == 1 && ((BasicBSONList)objArr[1].get("SubCollections")).size() == 1 && ((BasicBSONList)objArr[2].get("SubCollections")).size() == 1)	
						) {
					String subcl_A50  = (String)((BSONObject)((BasicBSONList)objArr[0].get("SubCollections")).get(0)).get("Name");
					if ( !(replicaGroupNames.get(0).equals(objArr[0].get("GroupName")) && 
							(SdbTestBase.csName+"."+subclNames[0]).equals(subcl_A50)) ) {
						System.out.println("true");
						flag = false;
					}
					String subcl_A149  = (String)((BSONObject)((BasicBSONList)objArr[1].get("SubCollections")).get(0)).get("Name");
					if ( !(replicaGroupNames.get(1).equals(objArr[1].get("GroupName")) && 
							(SdbTestBase.csName+"."+subclNames[1]).equals(subcl_A149)) ) {
						System.out.println("true");
						flag = false;
					}
					String subcl_A150  = (String)((BSONObject)((BasicBSONList)objArr[2].get("SubCollections")).get(0)).get("Name");
					if ( !(replicaGroupNames.get(2).equals(objArr[2].get("GroupName")) && 
							(SdbTestBase.csName+"."+subclNames[1]).equals(subcl_A150)) ) {
						System.out.println("true");
						flag = false;
					}
				} else {
					flag = false;
				}
			} else {
				flag = false;
			}
		} catch (BaseException e) {
			Assert.assertTrue(false,"check data faild: "+e.getMessage());
		}
		Assert.assertTrue(flag,"check data not expected");
	}	
	
}
