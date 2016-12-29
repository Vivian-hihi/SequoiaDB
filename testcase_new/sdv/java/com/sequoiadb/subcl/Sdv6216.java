package com.sequoiadb.subcl;


import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BasicBSONList;
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
import com.sequoiadb.testcommon.SdbTestBase;
/**
 * FileName: MainSub6216.java
 * test content: 子表创建索引后在主表创建相同索引_SD.subCL.01.024 
 * testlink case: seqDB-6216
 * @author zengxianquan
 * @date 2016年12月28日
 * @version 1.00
 */
public class Sdv6216 extends SdbTestBase{
		
	private Sequoiadb db = null;
	private CollectionSpace maincs = null;
	private String mainclName = "maincl6216";
	private DBCollection subcl1 = null;
	private DBCollection subcl2 = null;
	private DBCollection maincl = null;
	private String indexName = "index_1";
	
	@BeforeClass
	public void setUp(){
		System.out.println(this.getClass().getName()+" begin at "
				+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		try{
			db = new Sequoiadb(SdbTestBase.coordUrl,"","");
			maincs = db.getCollectionSpace(SdbTestBase.csName);
		}catch(BaseException e){
			Assert.fail(e.getMessage()+e.getMessage());
		}
        if (Commlib.isStandAlone(db)){
            throw new SkipException("is standalone skip testcase");
        }
		createMaincl();
		//创建普通子表，每个子表还不存在索引，并挂载到主表中
		createAndAttachSubcls();
	}

	@AfterClass
	public void tearDown(){
		try{		
			maincs.dropCollection(mainclName);
		}catch(BaseException e){
			Assert.assertEquals(e.getErrorCode(), -23, e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
			db.disconnect();
		}
	}
	
	@Test
	public void testCreateIndex(){
		//覆盖非唯一索引
		boolean isUnique = false;
		//subcl1创建索引
		createIndexInSub(isUnique);
		//在主表创建索引之前，检验每个子表的索引情况
		checkIndexBeforeMainclCreateIndex();
		//主表创建一个相同的索引
		createIndexInMaincl(isUnique);
		//在主表创建索引之后，检验每个子表的索引情况
		checkIndexAfterMainclCreateIndex();
		insertData();
		checkQueryExplain();
		//覆盖唯一索引
		//先把前面的创建的索引删除
		dropIndex();
		isUnique = true;
		//subcl1创建索引
		createIndexInSub(isUnique);
		//在主表创建索引之前，检验每个子表的索引情况
		checkIndexBeforeMainclCreateIndex();
		//主表创建一个相同的索引
		createIndexInMaincl(isUnique);
		//在主表创建索引之后，检验每个子表的索引情况
		checkIndexAfterMainclCreateIndex();
		insertData();
		checkQueryExplain();
	}

	public void dropIndex(){
		DBCursor cursor = null;
		try{
			maincl.dropIndex(indexName);
			cursor = maincl.getIndexes();
			if(cursor.hasNext()){
				Assert.fail("dropIndex is error");
			}
		}catch(BaseException e){
			e.printStackTrace();
			Assert.fail(e.getMessage());
		}finally{
			if(cursor != null){
				cursor.close();
			}
		}
	}
	
	public void createMaincl(){	
		BSONObject options = new BasicBSONObject();	
		options.put("IsMainCL", true);
		options.put("ShardingType", "range");
		BSONObject opt = new BasicBSONObject();
		opt.put("time", 1);
		options.put("ShardingKey", opt);
		try{			
			maincl = maincs.createCollection(mainclName,options);
		}catch(BaseException e){
			Assert.assertTrue(false, "createMaincl failed "+e.getMessage());
		}
	}
	
	/**
	 * 创建普通的不存在索引的表，并挂载到主表中
	 */
	public void createAndAttachSubcls(){	
		BSONObject options = new BasicBSONObject();
		options.put("AutoIndexId", false);	
		String jsonStr1 = "{'LowBound':{'time':0},UpBound:{'time':100}}";
		String jsonStr2 = "{'LowBound':{'time':100},UpBound:{'time':200}}";	
		BSONObject options1 = (BSONObject) JSON.parse(jsonStr1);
		BSONObject options2 = (BSONObject) JSON.parse(jsonStr2);
		try{
			maincl = maincs.getCollection(mainclName);
			subcl1 = maincs.createCollection("subcl626_1",options);					
			subcl2 = maincs.createCollection("subcl626_2",options);
			maincl.attachCollection(SdbTestBase.csName+"."+"subcl626_1", options1);
			maincl.attachCollection(SdbTestBase.csName+"."+"subcl626_2", options2);
		}catch(BaseException e){
			e.printStackTrace();
			Assert.fail( "createAndAttachCl failed "+e.getMessage());
		}
	}
	
	private void checkIndexBeforeMainclCreateIndex(){
		String expectIndexName = "index_1";
		BSONObject expectIndexKey =  (BSONObject) JSON.parse("{'time':1}");
		
		checkIndex(subcl1, expectIndexName, expectIndexKey);
		checkIndex(maincl, expectIndexName, expectIndexKey);	
		checkIndex(subcl2, null	, null);
	}
	
	private void checkIndexAfterMainclCreateIndex(){
		String expectIndexName = "index_1";
		BSONObject expectIndexKey =  (BSONObject) JSON.parse("{'time':1}");
		
		checkIndex(subcl1, expectIndexName, expectIndexKey);
		checkIndex(maincl, expectIndexName, expectIndexKey);	
		checkIndex(subcl2, expectIndexName, expectIndexKey);
	}
	
	public void createIndexInSub(boolean isUnique){
		BSONObject keyBson = new BasicBSONObject();
		keyBson.put("time", 1);
		try{
			subcl1.createIndex(indexName, keyBson, isUnique, false);
		}catch(BaseException e){
			Assert.fail("failed to create index");
		}		
	}

	public void createIndexInMaincl(boolean isUnique){
		BSONObject keyBson = new BasicBSONObject();
		keyBson.put("time", 1);
		try{
			maincl.createIndex(indexName, keyBson, isUnique, false);
		}catch(BaseException e){
			Assert.fail("failed to create index"+e.getMessage());
		}	
	}
	
	public void checkIndex(DBCollection cl, String expectIndexName, BSONObject expectIndexKey){
		DBCursor cursor = null;
		BSONObject indexDetail = null;
		BSONObject indexDef = null;
		BSONObject key = null;
		String name = null;
		try{
			//判断不存在索引的情况
			cursor = cl.getIndexes();
			if(cursor.hasNext() == false && (expectIndexName != null || expectIndexKey != null)){
				Assert.fail("index message is error");
			}
			//判断存在索引的情况
			while(cursor.hasNext()){
				indexDetail = cursor.getNext();
				indexDef = (BSONObject) indexDetail.get("IndexDef");
				name =  (String) indexDef.get("name");
				key = (BSONObject) indexDef.get("key");
				if((!name.equals(expectIndexName))||(!key.equals(expectIndexKey))){
					Assert.fail("index is error");
				}
			}
		}catch(BaseException e){
			e.printStackTrace();
			Assert.fail(e.getMessage());
		}finally{
			if(cursor != null){
				cursor.close();				
			}
		}
	}
	public void insertData(){
		//构造插入的数据
		List <BSONObject> insertor = new ArrayList<>();
		for(int i = 0; i < 200; i++){
			BSONObject bson = new BasicBSONObject();
			bson.put("time", i);
			bson.put("test", "test");
			insertor.add(bson);
		}	
		try{
			//插入数据
			maincl.bulkInsert(insertor, 1);		
		}catch(BaseException e){
			Assert.fail(e.getMessage());
		}	
	}
	
	public void checkQueryExplain(){
		DBCursor expRes = null;
		BSONObject expDet = null;
		BasicBSONList expDets = null ;
		BSONObject queryDetail = null;
		try{
			BSONObject matcher = (BSONObject) JSON.parse("{time:{$gt:10}}");
			BSONObject options = (BSONObject) JSON.parse("{Run:true}");
			BSONObject query = (BSONObject) JSON.parse("{$and:[{time:{$gt:10}}]}");
			expRes = maincl.explain(matcher, null, null, null, 0, -1, 0, options);			
			while(expRes.hasNext()){
				expDets=(BasicBSONList) expRes.getNext().get("SubCollections");
				expDet=(BSONObject) expDets.get(0);
				String scanType	= (String) expDet.get("ScanType");
				String name = (String) expDet.get("IndexName");
				queryDetail = (BSONObject) JSON.parse(expDet.get("Query").toString());
				if(!scanType.equals("ixscan")||!indexName.equals(name)||!queryDetail.equals(query)){
					Assert.fail("query().explain() has false");
				}
			}
		}catch(BaseException e){
			e.printStackTrace();
			Assert.fail(e.getMessage());
		}finally{
			if(expRes != null){
				expRes.close();
			}
		}
	}
}
