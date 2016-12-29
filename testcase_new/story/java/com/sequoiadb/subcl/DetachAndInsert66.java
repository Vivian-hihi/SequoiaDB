package com.sequoiadb.subcl;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.util.JSON;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterTest;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;
/**
 * 
 * FileName: DetachAndInsert66
 * test content: detach子表后，验证数据操作 
 * testlink case: seqDB-66
 * @author zengxianquan
 * @date 2016年12月27日
 * @version 1.00
 */
public class DetachAndInsert66 extends SdbTestBase {
	private Sequoiadb sdb1;
    private Sequoiadb sdb2;
    private DBCollection  cl1;
    private DBCollection  cl2;
    private DBCollection  cl3;
    private String mainclName="maincl66";
    private List<String> addressList = null;

    @BeforeClass
	public void setUp(){
	    System.out.println(this.getClass().getName()+" begin at "
	    		+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
	    Sequoiadb tmpdb = null; 
		try{
		    tmpdb = new Sequoiadb(SdbTestBase.coordUrl,"","");
		    if (Commlib.isStandAlone( tmpdb )){
		    	throw new SkipException("is standalone skip testcase");
		    }            
			addressList = Commlib.getNodeAddress(tmpdb, "SYSCoord");            
			sdb1 = new Sequoiadb(addressList.get(0), "", "");
			sdb2 = new Sequoiadb(addressList.get(1), "", "");
	    }catch(BaseException e){
	        Assert.fail(e.getMessage()+e.getMessage());
	    }finally{
	        tmpdb.disconnect();
	    }
	    createCl(sdb1);
	    attach(sdb1);
	}
	
	@AfterTest
	public void tearDown() {
		Sequoiadb tmpdb = null; 
	    CollectionSpace cs = null;
		try{
			tmpdb = new Sequoiadb(SdbTestBase.coordUrl,"","");
			cs = tmpdb .getCollectionSpace(SdbTestBase.csName);
			cs.dropCollection(mainclName);        
		}catch(BaseException e){
			Assert.assertEquals(e.getErrorCode(), -23,e.getMessage());
		}finally{
			sdb1.disconnect();
			sdb2.disconnect();
			tmpdb.disconnect();
			System.out.println("End to run " + this.getClass().getName() 
					+ ", end in: " + new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		}
	}

    @Test
    public void test(){    	 
    	detach(sdb1, cl1);
    	//检验detach后的子表能否做增删查
    	checkCRUD(sdb2);
    	//检验其它子表能否做增删改查
    	checkInsert(sdb2, 100, 300);
    	checkUpdate(sdb2, 100, 300);
    	checkDelete(sdb2, 250, 150);
    }
    
    /**
     * 检验detach后的子表能否做增删查
     * @param db
     */
    public void checkCRUD(Sequoiadb db){
    	CollectionSpace cs = null;
    	DBCollection  maincl = null;
   	    try{
   	    	cs = db.getCollectionSpace(SdbTestBase.csName);
   	    	maincl = cs.getCollection(mainclName);
   	    }catch(BaseException e){
   	    	e.printStackTrace();
   	    	Assert.fail("Fail to check CRUD"+e.getMessage());
   	    }
  	    try{
  	    	maincl.insert("{age:1,name:'xiaohaong'}");   
  	    	Assert.fail("insert success");
   	    }catch(BaseException e){
   	    	Assert.assertEquals(e.getErrorCode(), -135, e.getMessage());
   	    }
  	    try{
  	    	maincl.query("{age:1}", null, null, null);
  	    	Assert.fail("query success");
  	    }catch(BaseException e){
  	    	Assert.assertEquals(e.getErrorCode(), -23, e.getMessage());
  	    }
  	    try{
  	    	maincl.delete("{age:1}");  
  	    	Assert.fail("delete success");
  	    }catch(BaseException e){
  	    	Assert.assertEquals(e.getErrorCode(), -23, e.getMessage());
  	    }
    }
    
    public void createCl(Sequoiadb db){
    	CollectionSpace cs = null;
   	    try{
   	    	cs = db.getCollectionSpace(SdbTestBase.csName);
    	    BSONObject mainOpt=(BSONObject)JSON.parse("{IsMainCL:true,ShardingKey:{age:1},ShardingType:\"range\"}");
            BSONObject subOpt=(BSONObject)JSON.parse("{ShardingKey:{age:1},ShardingType:\"hash\",Partition:1024}");
   	        cs.createCollection(mainclName, mainOpt);	
           
   	        cl1=cs.createCollection("subcl66_1", subOpt);
            cl2=cs.createCollection("subcl66_2", subOpt);
            cl3=cs.createCollection("subcl66_3", subOpt);
   	    }catch(BaseException e){
   	    	Assert.fail("create is faild:"+e.getMessage());
   	    }
   	}
    
    public void attach(Sequoiadb db){
    	CollectionSpace cs = null;
    	DBCollection  maincl = null;
        try{	 
        	cs = db.getCollectionSpace(SdbTestBase.csName);
        	maincl = cs.getCollection(mainclName);
        	
            BSONObject opt1 = (BSONObject)JSON.parse("{LowBound:{age:0},UpBound:{age:100}}");
      	    BSONObject opt2 = (BSONObject)JSON.parse("{LowBound:{age:100},UpBound:{age:200}}");
      	    BSONObject opt3 = (BSONObject)JSON.parse("{LowBound:{age:200},UpBound:{age:300}}");
      	    maincl.attachCollection(cl1.getFullName(), opt1);
      	    maincl.attachCollection(cl2.getFullName(), opt2);
      	    maincl.attachCollection(cl3.getFullName(), opt3);
        }catch(BaseException e){
      	    Assert.fail("attach is error:"+e.getMessage());
        }
    }
    
    public void detach(Sequoiadb db, DBCollection cl){
    	CollectionSpace cs = null;
    	DBCollection  maincl = null;
        try{	 
        	cs = db.getCollectionSpace(SdbTestBase.csName);
        	maincl = cs.getCollection(mainclName);
    	    maincl.detachCollection(cl.getFullName());
    	 }catch(BaseException e){
    	    Assert.fail("detach is faild:"+e.getMessage());
    	 }
     }
    
    public void checkInsert(Sequoiadb db, int lowBound, int upBound){
    	CollectionSpace cs = null;
    	DBCollection maincl = null;
		try{
			cs = db.getCollectionSpace(SdbTestBase.csName);
			maincl = cs.getCollection(mainclName);	
			//检验插入
			List <BSONObject> insertor = new ArrayList<>();
			for(int i = lowBound; i<upBound; i++){
				BSONObject bson = new BasicBSONObject();
				bson.put("age", i);
				bson.put("name", "xiaohong");
				bson.put("test", "test");
				insertor.add(bson);
			}
			maincl.bulkInsert(insertor, DBCollection.FLG_INSERT_CONTONDUP);
			//检验数据的正确性
			BSONObject order = new BasicBSONObject();
			order.put("age", 1);
			DBCursor res = maincl.query(null, null, order, null);
			int i = 0;
			while(res.hasNext()){
				BSONObject dataRes = res.getNext();
				if(!insertor.get(i).equals(dataRes)){
					Assert.fail("failed to query data ");
				}
				i++;
			}
			Assert.assertEquals(maincl.getCount(), upBound-lowBound, "failed to delete data");	
		}catch(BaseException e){
			Assert.fail("failed to insert :"+e.getMessage());
		}
		
    }
    
    public void checkUpdate(Sequoiadb db, int lowBound, int upBound){
    	CollectionSpace cs = null;
    	DBCollection maincl = null;
    	try{
    		cs = db.getCollectionSpace(SdbTestBase.csName);
			maincl = cs.getCollection(mainclName);
			//检验更新
			List <BSONObject> updateList = new ArrayList<>();
			for(int i = lowBound; i<upBound; i++){
				BSONObject bson = new BasicBSONObject();
				bson.put("age", i);
				bson.put("name", "xiaohong");
				bson.put("test", "update");
				updateList.add(bson);
			}		
			maincl.update(null, "{$set:{'test':'update'}}",null);
			BSONObject order = new BasicBSONObject();
			order.put("age", 1);
			DBCursor updateRes = maincl.query(null, null, order, null);
			int i = 0;
			while(updateRes.hasNext()){
				BSONObject dataRes = updateRes.getNext();
				dataRes.removeField("_id");
				if(!updateList.get(i).equals(dataRes)){
					Assert.fail("failed to query data ");
				}
				i++;
			}
			Assert.assertEquals(maincl.getCount(), upBound-lowBound, "failed to delete data");
    	}catch(BaseException e){
			Assert.fail("failed to update :"+e.getMessage());
		}
    }
    
    public void checkDelete(Sequoiadb db, int gte, int count){
    	CollectionSpace cs = null;
    	DBCollection maincl = null;
		try{
			cs = db.getCollectionSpace(SdbTestBase.csName);
			maincl = cs.getCollection(mainclName);	
			//检验删除
			maincl.delete("{age:{$gte:"+gte+"}}");
			Assert.assertEquals(maincl.getCount(),count,"failed to delete data");
			
		}catch(BaseException e){
				Assert.fail("failed to delete :"+e.getMessage());
		}
	}
}
