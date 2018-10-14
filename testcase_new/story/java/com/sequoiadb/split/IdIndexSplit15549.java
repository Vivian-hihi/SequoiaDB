package com.sequoiadb.split;
import java.util.ArrayList;
import java.util.List;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.CommLib;
import com.sequoiadb.testcommon.SdbTestBase;
import com.sequoiadb.testcommon.SdbThreadBase;

/**
 * @FileName: seqDB-15549:切分时，修改AutoIndexId属性
 * 1、向cl中插入数据记录
 * 2、执行split，设置切分条件
 * 3、切分过程中修改AutoIndexId属性
 * 4、查看切分和删除id索引结果 
 * @author zhaoxiaoni
 * @Date 2018/8/17
 * @version 3.00
 *
 */

public class IdIndexSplit15549 extends SdbTestBase {
	private Sequoiadb commSdb = null;
	private String clName = "cl_15549";
	private List<String> groupNames;
	private String srcGroup;
	private String desGroup;
	
	@BeforeClass
	public void setUp(){
		commSdb = new Sequoiadb(coordUrl,"","");
		
		//跳过standAlone和数据组不足的情况
		CommLib commlib = new CommLib();
		if(commlib.isStandAlone(commSdb)){
			throw new SkipException("StandAlone environment!");
		}
		
		groupNames = commlib.getDataGroupNames(commSdb);
		if(groupNames.size()<2){
			throw new SkipException("Current environment less than tow groups! ");
		}
		srcGroup = groupNames.get(0);
		desGroup = groupNames.get(1);
		
		CollectionSpace cs = commSdb.getCollectionSpace(csName);
	    DBCollection cl = cs.createCollection(clName,(BSONObject)JSON
				.parse("{ShardingKey:{sk:1},ShardingType:'range',Group:'" + srcGroup + "'}"));
		
		//写入待分的记录5000
		insertData(cl);
	}
	
	@Test
	public void splitAndAlter(){
		Sequoiadb db = null;
		DBCollection cl = null;
		try{
			//启动切分线程
			Split splitThread = new Split();
			splitThread.start();
			
			//修改AutoIndexId属性
			db = new Sequoiadb(coordUrl, "", "");
			for(int i=0;i<50;i++){
				DBCursor cursor = db.listTasks((BSONObject)JSON.parse("{Name:\"" + csName + "." + clName + "\"}"), null, null, null);
				if(cursor.hasNext()){
					cl = db.getCollectionSpace(csName).getCollection(clName);
					try{
						cl.alterCollection((BSONObject)JSON.parse("{AutoIndexId:false}"));
						Assert.fail("alert_success_ERROR");
					}catch(BaseException e){
						if(e.getErrorCode()!=-334){
							throw e;
						}
					}
					break;
				}
			}
			
			// 等待切分结束
			if (!splitThread.isSuccess()) {
				splitThread.getExceptions().get(0).printStackTrace();
				Assert.fail(splitThread.getErrorMsg());
			}
			
			// 查看索引
			checkIndex(cl);
			
			// 校验源组和目标组的数据 
			checkData(db, 4500, "{sk:{$gte:500,$lt:5000}}", 4500, desGroup);
			checkData(db, 500, "{sk:{$gte:0,$lt:500}}", 500, srcGroup);
			
		}catch(BaseException e){
			e.printStackTrace();
			Assert.fail(e.getMessage()+"\r\n"+SplitUtils.getKeyStack(e,this));
		}finally {
			if (db != null) {
				db.close();
			}
		}
	}
	
	@AfterClass
	public void tearDown() {
		try {
			CollectionSpace cs = commSdb.getCollectionSpace(csName);
			cs.dropCollection(clName);
		} catch (BaseException e) {
			Assert.fail(e.getMessage()+"\r\n"+SplitUtils.getKeyStack(e,this));
		} finally {
			if (commSdb != null) {
				commSdb.close();;
			}
		}
	}
	
	private void insertData(DBCollection cl) {
		// TODO Auto-generated method stub
		List<BSONObject> list = new ArrayList<BSONObject>();
		for(int i = 0; i < 5000; i++){
		    BSONObject obj = (BSONObject)JSON.parse("{sk:" + i + "}");
			list.add(obj);
		}
		cl.insert(list);
	}
	
	private class Split extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			// TODO Auto-generated method stub
			Sequoiadb db = null;
			DBCollection cl = null;
			try{
				db = new Sequoiadb(coordUrl, "", "");
				cl = db.getCollectionSpace(csName).getCollection(clName);
				cl.split(srcGroup, desGroup, 90);
			}catch(BaseException e){
				System.out.println("Split:"+e.getErrorCode());
				throw e;
			}finally {
				if(db!=null){
					db.close();
				}
			}
		}
	}
	
	public void checkIndex(DBCollection cl) {
		DBCursor dbc = null;
		try {
			dbc = commSdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{Name:\"" + csName + "." + clName + "\"}", null, null);	
			if(!(((String)dbc.getNext().get("AttributeDesc")).equals(""))){
				throw new SkipException("id index may be deleted! ");
			}
		} catch (BaseException e) {
			throw e;
		} finally {
			if (dbc != null) {
				dbc.close();
			}
		}
	}
	
	private void checkData(Sequoiadb db, int expectedCount, String macher, int expectTotalCount, String group) {
		Sequoiadb desDataNode = null;
		DBCollection desCL = null;
		try {
			desDataNode = db.getReplicaGroup(group).getMaster().connect();
			desCL = desDataNode.getCollectionSpace(csName).getCollection(clName);	
			long count = desCL.getCount(macher);
            
			String hostandport = desDataNode.getHost()+":"+desDataNode.getPort();
			Assert.assertEquals(count, expectedCount, hostandport);// 目标组应当含有上述查询数据
			Assert.assertEquals(desCL.getCount(), expectTotalCount, hostandport); // 目标组应当含有的数据量
		} catch (BaseException e) {
			e.printStackTrace();
			Assert.fail(e.getMessage()+"\r\n"+SplitUtils.getKeyStack(e,this));
		} finally {
			if (desDataNode != null) {
				desDataNode.close();
			}
		}
	}
}