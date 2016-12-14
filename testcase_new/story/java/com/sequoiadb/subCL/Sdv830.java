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
 * FileName: Sdv830.java
 * test content: 多线程并发删除数据的同时做count操作_SD.subCL.01.017 
 * testlink case: seqDB-830
 * @author zengxianquan
 * @date 2016年12月13日
 * @version 1.00
 */
public class Sdv830 extends SdbTestBase{
		
	private Sequoiadb sdb = null;
	private String csName = null;
	
	private CollectionSpace maincs = null;
	private String mainclName = "maincl830";
	
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
					maincs.createCollection("subcl830_"+j);					
			}
		}catch(BaseException e){
			Assert.assertTrue(false, "createSubcl failed "+e.getMessage());
		}	
	}
	
	public void attachSubcls(){	
		DBCollection maincl=null;
		try{
			maincl=maincs.getCollection(mainclName);			
		}catch(BaseException e){
			Assert.fail("failed to get collection "+e.getMessage());
		}
		try{
			for(int j=0;j<5;j++){
				String jsonStr="{'LowBound':{'a':"+j*100+"},UpBound:{'a':"+(j+1)*100+"}}";
				BSONObject options=(BSONObject) JSON.parse(jsonStr);
				maincl.attachCollection(SdbTestBase.csName+".subcl830_"+j, options);
			}
		}catch(BaseException e){
			Assert.assertTrue(false, "attachSubcl  failed "+e.getMessage());
		}
	}
	@DataProvider(name = "diffDataProvider",parallel=true )
	public  Object[][] createData(){
		return new Object[][]{
			new Object[]{"a"},
			new Object[]{"b"},
			new Object[]{"c"},
			new Object[]{"d"},
			new Object[]{"e"},
		};
	}
	public void insertData(DBCollection maincl,String diffData){
		//构造插入的数据
		List <BSONObject> insertor=new ArrayList<>();
		for(int i=0; i<500; i++){
			BSONObject bson=new BasicBSONObject();
			bson.put("a", i);
			bson.put("test", diffData);
			insertor.add(bson);
		}
		try{
			maincl.bulkInsert(insertor, 1);			
		}catch(BaseException e){
			Assert.fail("failed to bulkInsert "+e.getMessage());
		}
	}
	@Test(dataProvider = "diffDataProvider")
	public void test(String diffData){
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
		DBCollection maincl = null;
		try{
			maincl=cs.getCollection(mainclName);
		}catch(BaseException e){
			Assert.fail("failed to get cs "+e.getMessage());	
		}
		insertData(maincl,diffData);
		delete(maincl,diffData);
		//检验数据的数量是否正确
		checkData(maincl, diffData);		
	}
	public void delete(DBCollection maincl,String diffData){
		BSONObject options=new BasicBSONObject();
		BSONObject opt=new BasicBSONObject();
		opt.put("$gte", 100);
		options.put("a", opt);
		options.put("test", diffData);
		try{
			maincl.delete(options);			
		}catch(BaseException e){
			Assert.fail("failed to delete data"+e.getMessage());
		}
	}
	public void checkData(DBCollection maincl,String diffData){
		BSONObject options=new BasicBSONObject();
		options.put("test", diffData);
		long count=0;
		try{
			count=maincl.getCount(options);	
		}catch(BaseException e){
			Assert.fail("failed to get count "+e.getMessage());
		}
		if(count!=100){
			Assert.fail("failed to check count");
		}
	}
}
