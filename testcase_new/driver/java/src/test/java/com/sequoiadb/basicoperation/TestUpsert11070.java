package com.sequoiadb.basicoperation;

import java.util.ArrayList;
import java.util.concurrent.atomic.AtomicInteger;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.SkipException;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.ReplicaGroup;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.basicoperation.Commlib;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
* @TestLink: seqDB-11070
* @describe: 副本数为0，带_id条件并发Upsert
* @author wangkexin
* @Date   2019.02.22
* @version 1.00
*/
public class TestUpsert11070 extends SdbTestBase{
	private String clName = "cl11070";
	Sequoiadb sdb = null;
	
	String string1 = "";
	String string2 = "";
	
	private static int[] names = new int[1000];
	private static AtomicInteger v = new AtomicInteger(0);
	
	@BeforeClass
	public void setup() {
		sdb = new Sequoiadb(SdbTestBase.coordUrl, "", "");
		if (Commlib.isStandAlone(sdb)){
			throw new SkipException("is standalone skip testcase");
		}
		ArrayList<String> dataRG = Commlib.getDataGroups(sdb);
		ReplicaGroup dataRg = sdb.getReplicaGroup(dataRG.get(0));
		String rgName = dataRg.getGroupName();
		BSONObject options = new BasicBSONObject("Group", rgName);
        options.put("ReplSize", 0);
		sdb.getCollectionSpace(SdbTestBase.csName).createCollection(clName, options);
		
	}
	@Test()
	public void prepare(){
		for(int i = 0;i <1000; i++){
			names[i] = i;
		}
		
		for(int i = 0; i < 5455;i++ ){
			string1 += "a";
		}
	
		for(int i = 0; i < 5471;i++ ){
			string2 += "a";
		}
		
	}
	
	@Test(invocationCount = 1000, threadPoolSize = 10, dependsOnMethods = "prepare")
	public void test(){
		int name = names[v.getAndIncrement() % names.length]; 
			
		try (Sequoiadb db1 = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
			DBCollection cl1 = db1.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
			BSONObject matcher1 = new BasicBSONObject();
			BSONObject modifer1 = new BasicBSONObject();
			BSONObject value1 = new BasicBSONObject();
			
			matcher1.put("_id", name);
			value1.put("a", string1);
			modifer1.put("$set", value1);
			
			cl1.upsert(matcher1, modifer1, null);
		} catch (BaseException e) {
			System.out.println("name:"+name);
			e.printStackTrace();
			Assert.fail("insert failed",e);
		}
		
		try (Sequoiadb db2 = new Sequoiadb(SdbTestBase.coordUrl, "", "")){
			DBCollection cl2 = db2.getCollectionSpace(SdbTestBase.csName).getCollection(clName);
			BSONObject matcher2 = new BasicBSONObject();
			BSONObject modifer2 = new BasicBSONObject();
			BSONObject value2 = new BasicBSONObject();
			
			matcher2.put("_id", name);
			value2.put("a", string2);
			modifer2.put("$set", value2);
			
			cl2.upsert(matcher2, modifer2, null);
		} catch (BaseException e) {
			System.out.println("name:"+name);
			e.printStackTrace();
			Assert.fail("upsert failed",e);
		}
	}
	
	@AfterClass
	public void tearDown(){
		sdb.getCollectionSpace(csName).dropCollection(clName);
		sdb.close();
	}
}