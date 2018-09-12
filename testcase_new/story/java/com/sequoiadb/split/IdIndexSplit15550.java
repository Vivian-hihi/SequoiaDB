package com.sequoiadb.split;

import java.util.ArrayList;
import java.util.List;

import javax.print.attribute.standard.PrinterLocation;

import org.bson.BSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
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
 * @FileName: seqDB-15550:创建id索引与切分并发
 * 1、向cl中插入数据记录
 * 2、执行split，设置切分条件
 * 3、切分过程中创建id索引
 * 4、查看切分和创建id索引结果 
 * @author zhaoxiaoni
 * @Date 2018/8/17
 * @version 3.00
 *
 */

public class IdIndexSplit15550 extends SdbTestBase {

	private Sequoiadb commSdb = null;
	private List<String> groupNames = null;
	private String clName = "cl_15550";
	private String desGroup;
	private String srcGroup;
	
	@BeforeClass
	public void setUp(){
		commSdb = new Sequoiadb(coordUrl,"","");
		//跳过standAlone和数据组不足的情况
		CommLib commLib = new CommLib();
		if(commLib.isStandAlone(commSdb)){
			throw new SkipException("Skip StandAlone");
		}
		groupNames = commLib.getDataGroupNames(commSdb);
		if(groupNames.size()<2)
		{
			throw new SkipException("Only one group!");
		}
		
		srcGroup = groupNames.get(0);
		desGroup = groupNames.get(1);

		CollectionSpace cs = commSdb.getCollectionSpace(csName);
		DBCollection cl = cs.createCollection(clName,(BSONObject)JSON.
				parse("{ShardingKey:{sk:1},ShardingType:'range',AutoIndexId:false,Group:'" + srcGroup + "'}"));
		
		//写入待分的记录5000
		insertData(cl);
	}
	
	@Test
	public void SplitAndCreate(){
		Sequoiadb db = null;
		Split split = null;
		Create create = null;
		DBCollection cl = null;
		try{
			db = new Sequoiadb(coordUrl, "", "");
			cl = db.getCollectionSpace(csName).getCollection(clName);
			
			//启动创建id索引线程
			create = new Create();
			create.start();
			
			//启动切分线程
			split = new Split();
			split.start();
			
			// 等待切分结束
			if(!split.isSuccess()){
				split.getExceptions().get(0).printStackTrace();
				Assert.fail(split.getErrorMsg());
			}
					
		}catch(BaseException e){
			
		}finally{
			if(db!=null){
				db.close();
			}
		}
	}

	private void insertData(DBCollection cl) {
		// TODO Auto-generated method stub
		List<BSONObject> list = new ArrayList<BSONObject>();
		for(int i=0; i<5000; i++){
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
				// 查看索引
				checkIndex(cl);
				// 校验源组和目标组的数据
				checkData(db,4500, "{sk:{$gte:500,$lt:5000}}", 4500, desGroup);
				checkData(db, 500, "{sk:{$gte:0,$lt:500}}", 500, srcGroup);
			}catch(BaseException e){
				System.out.println("Split:"+e.getErrorCode());
				if(e.getErrorCode()==-279){
					System.out.println("Don't has id index!");
				}
			}finally {
				if(db!=null){
					db.close();
				}
			}
		}
	}
	
	private class Create extends SdbThreadBase{

		@Override
		public void exec() throws Exception {
			// TODO Auto-generated method stub
			Sequoiadb db = null;
			DBCollection cl = null;
			try{
				db = new Sequoiadb(coordUrl, "", "");
				cl = db.getCollectionSpace(csName).getCollection(clName);
				cl.createIdIndex((BSONObject)JSON.parse("{SortBufferSize:128}"));
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
	
	private void checkIndex(DBCollection cl) {
		// TODO Auto-generated method stub
		DBCursor dbc = null;
		try{
			dbc = commSdb.getSnapshot(Sequoiadb.SDB_SNAP_CATALOG, "{Name:\"" + csName + "." + clName + "\"}", null, null);
		    if(!(((String)dbc.getNext().get("AttributeDesc")).equals(""))){
		    	throw new SkipException("id index error! ");
		    }
		}catch(BaseException e){
			throw e;
		}finally {
			if(dbc != null){
				dbc.close();
			}
		}
	}
	
	private void checkData(Sequoiadb db,int expectedCount,String matcher,int expectedTotalCount,String group){
		Sequoiadb desDataNode = null;
		DBCollection desCl = null;
		try{
			desDataNode = db.getReplicaGroup(group).getMaster().connect();
			desCl = desDataNode.getCollectionSpace(csName).getCollection(clName);
			long count = desCl.getCount(matcher);
			
			String hostandport = desDataNode.getHost()+":"+desDataNode.getPort();
			Assert.assertEquals(count, expectedCount, hostandport);
			Assert.assertEquals(desCl.getCount(), expectedTotalCount,hostandport);
		}catch(BaseException e){
			e.printStackTrace();
			Assert.fail(e.getMessage()+"\r\n"+SplitUtils.getKeyStack(e, this));
		}finally {
			if(desDataNode != null){
				desDataNode.close();
			}
		}
	}
}
