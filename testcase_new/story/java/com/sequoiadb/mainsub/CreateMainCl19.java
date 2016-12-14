package com.sequoiadb.mainsub;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testcommon.SdbTestBase;

/**
 * FileName: CreateMainCl19.java
 * test content: 创建主表时指定为非分区表 
 * testlink case: seqDB-19
 * @author zengxianquan
 * @date 2016年12月9日
 * @version 1.00
 */
public class CreateMainCl19 extends SdbTestBase{
		
	private String clName = "maincl_19";
	private Sequoiadb sdb = null;
	private CollectionSpace cs = null;
	
	@BeforeClass
	public void setUp(){
		System.out.println(this.getClass().getName()+" begin at "
				+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
		try{
			sdb=new Sequoiadb(SdbTestBase.coordUrl,"","");
		}catch(BaseException e){
			Assert.fail("connect failed,"+SdbTestBase.coordUrl+e.getMessage());
		}	
	}
	
	@AfterClass
	public void tearDown(){		
		try{
			System.out.println(this.getClass().getName()+" end at "
					+new SimpleDateFormat("yyyy-MM-dd HH:mm:ss:S").format(new Date()));
			if(cs.isCollectionExist(clName)){
				cs.dropCollection(clName);
			}
			sdb.disconnect();
		}catch(BaseException e){			
			Assert.assertTrue(false,"clean up failed:"+e.getMessage());
		}
	}	
	@Test
	public void testCreateMainclByNoAppointShardingKey(){
		cs=sdb.getCollectionSpace(SdbTestBase.csName);
		BSONObject options=new BasicBSONObject();
		options.put("IsMainCL", true);
		options.put("ShardingType", "range");
		try{
			cs.createCollection(clName,options);
			Assert.fail("success to create maincl by not appoint shardingkey");
		}catch(BaseException e){
			Assert.assertEquals(e.getErrorCode(), -245,e.getMessage());
		}
	}	
}
