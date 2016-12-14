package com.sequoiadb.subCL;


import java.lang.reflect.Method;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Random;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
/**
 * FileName: Sdv829.java
 * test content: 多线程并发同时在主表插入大量数据_SD.subCL.01.015
 * testlink case: seqDB-829
 * @author zengxianquan
 * @date 2016年12月12日
 * @version 1.00
 */
public class Sdv829 extends SdbTestBase{
	private Sequoiadb sdb = null;
	private CollectionSpace maincs = null;
	private String mainclName = "maincl829";
	private DBCollection maincl = null;
	
	@BeforeClass
	public void setUp(){
		System.out.println(this.getClass().getName()+" begin at "
				+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		try{
			sdb=new Sequoiadb(SdbTestBase.coordUrl,"","");
		}catch(BaseException e){
			Assert.assertTrue(false, "connect  failed,"+SdbTestBase.coordUrl+e.getMessage());
		}
		init();
	}
	@AfterClass
	public void tearDown(){
		try{
			if(maincs.isCollectionExist(mainclName)){
				maincs.dropCollection(mainclName);
			}
		}catch(BaseException e){
			Assert.fail("failed to drop cl"+"ErrorMsg:\n" +e.getMessage());
		}finally{
			System.out.println("End to run " + this.getClass().getName() 
						+ ", end in: " + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
			sdb.disconnect();
		}
	}	
	public void init(){
		 maincs=sdb.getCollectionSpace(SdbTestBase.csName);
		 createMaincl();
		 createSubcls();
		 attachSubcls();
		 insertData();
	}
	
	public void createMaincl(){	
		BSONObject options=new BasicBSONObject();	
		options.put("IsMainCL", true);
		options.put("ShardingType", "range");
		BSONObject opt=new BasicBSONObject();
		opt.put("a", 1);
		options.put("ShardingKey", opt);
		try{			
			maincs.createCollection(mainclName,options);
		}catch(BaseException e){
			Assert.assertTrue(false, "createMaincl failed "+e.getMessage());
		}
	}
	
	public void createSubcls(){
		try{
			for(int j=0;j<5;j++){
					maincs.createCollection("subcl829_"+j);					
			}
		}catch(BaseException e){
			Assert.assertTrue(false, "createSubcl failed "+e.getMessage());
		}	
	}
	
	public void attachSubcls(){	
		try{
			maincl=maincs.getCollection(mainclName);			
		}catch(BaseException e){
			Assert.fail("failed to get collection "+e.getMessage());
		}
		try{
			for(int j=0;j<5;j++){
				String jsonStr="{'LowBound':{'a':"+j*100+"},UpBound:{'a':"+(j+1)*100+"}}";
				BSONObject options=(BSONObject) JSON.parse(jsonStr);
				maincl.attachCollection(SdbTestBase.csName+".subcl829_"+j, options);
			}
		}catch(BaseException e){
			Assert.assertTrue(false, "attachSubcl  failed "+e.getMessage());
		}
	}
	
	public void insertData(){
		//构造插入的数据
		List <BSONObject> insertor=new ArrayList<>();
		for(int i=0;i<10;i++){
			for(int j=0; j<500; j++){
				BSONObject bson=new BasicBSONObject();
				bson.put("a", j);
				bson.put("test", "test");
				bson.put("name", "name"+i);
				insertor.add(bson);
			}
		}
		try{
			maincl.bulkInsert(insertor, 1);			
		}catch(BaseException e){
			Assert.fail("failed to bulkInsert "+e.getMessage());
		}
	}
	@DataProvider(name = "updateDataProvider",parallel=true )
	public  Object[][] createData(){
		return new Object[][]{
			new Object[]{"update0","name0"},
			new Object[]{"update1","name1"},
			new Object[]{"update2","name2"},
			new Object[]{"update3","name3"},
			new Object[]{"update4","name4"},
			new Object[]{"update5","name5"},
			new Object[]{"update6","name6"},
			new Object[]{"update7","name7"},
			new Object[]{"update8","name8"},
			new Object[]{"update9","name9"},
		};
	}
	
	
	
	@Test(dataProvider = "updateDataProvider")
	public void test(String updateStr,String diffData){
		Sequoiadb db=null;
		try{
			db=new Sequoiadb(SdbTestBase.coordUrl,"","");			
		}catch(BaseException e){
			Assert.fail("failed to connect sdb "+e.getMessage());
		}	
		
		CollectionSpace cs=null;
		try{
			cs=db.getCollectionSpace(SdbTestBase.csName);	
		}catch(BaseException e){
			Assert.fail("failed to get cs "+e.getMessage());
		}
		
		DBCollection maincl=null;
		try{
			maincl=cs.getCollection(mainclName);
		}catch(BaseException e){
			Assert.fail("failed to get cs "+e.getMessage());	
		}
		//更新数据
		BSONObject matcher=new BasicBSONObject();
		matcher.put("name", diffData);
		BSONObject updateBson=new BasicBSONObject();
		BSONObject update=new BasicBSONObject();
		update.put("test", updateStr);
		updateBson.put("$set", update);
		try{
			maincl.update(matcher, updateBson, null);		
		}catch(BaseException e){
			Assert.fail("failed to update "+e.getMessage());
		}
		//检验数据是否正确
		checkData(cs, updateStr, diffData);		
	}
	public void checkData(CollectionSpace cs,String updateStr,String diffData){
		DBCollection subcl=null;
		for(int i=0;i<5;i++){
			try{
				subcl=cs.getCollection("subcl829_"+i);				
			}catch(BaseException e){
				Assert.fail("failed to get cl "+e.getMessage());
			}
			BSONObject matcher=new BasicBSONObject();
			matcher.put("name", diffData);
			BSONObject order=new BasicBSONObject();
			order.put("a", 1);
			DBCursor cursor=null;
			try{
				cursor=subcl.query(matcher,null,order,null);				
			}catch(BaseException e){
				Assert.fail("failed to query data "+e.getMessage());
			}
			int j=100*i;
			BSONObject res;
			while(cursor.hasNext()){
				res=cursor.getNext();
				if((!res.get("a").equals(j))
						||(!res.get("test").equals(updateStr))
						||(!res.get("name").equals(diffData))){
					Assert.fail("The data is not same");
				}
				j++;
			}			
		}	
	}
}
