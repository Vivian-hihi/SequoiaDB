package com.sequoiadb.test.bson;

import org.bson.BSON;
import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BSONTimestamp;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.FixMethodOrder;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runners.MethodSorters;

import java.util.Date;
import java.util.regex.Pattern;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.test.common.*;

public class BSONCodeTypeTest {
	
	private static Sequoiadb sdb ;
	private static CollectionSpace cs ;
	private static DBCollection cl ;
	private static DBCursor cursor ;
	
	@BeforeClass
	public static void setConnBeforeClass() throws Exception{
		// sdb
		sdb = new Sequoiadb(Constants.COOR_NODE_CONN,"","");
	}
	
	@AfterClass
	public static void DropConnAfterClass() throws Exception {
		sdb.disconnect();
	}
	
	@Before
	public void setUp() throws Exception {
		// cs
		if(sdb.isCollectionSpaceExist(Constants.TEST_CS_NAME_1)){
			sdb.dropCollectionSpace(Constants.TEST_CS_NAME_1);
		}
	    cs = sdb.createCollectionSpace(Constants.TEST_CS_NAME_1);
		// cl
		BSONObject conf = new BasicBSONObject();
		conf.put("ReplSize", 0);
		cl = cs.createCollection(Constants.TEST_CL_NAME_1, conf);
	}

	@After
	public void tearDown() throws Exception {
		sdb.dropCollectionSpace(Constants.TEST_CS_NAME_1);
	}
	
	@Test
	public void code_type_display_test() {
		if ( !Constants.isCluster() ) {
			return;
		}
		String name = "abc_in_java";
		String code = "function abc_in_java(x, y){return x + y ;}";
		sdb.crtJSProcedure(code);
		BSONObject obj = null;
		try {
			cursor = sdb.listProcedures(new BasicBSONObject().append("name", name));
			try {
				while(cursor.hasNext()) {
					obj = cursor.getNext();
				}
			} finally {
				cursor.close();
			}
			// check
			Assert.assertTrue(obj != null);
			obj.removeField("_id");
			System.out.println("obj is: " + obj);
			String retStr = "{ \"name\" : \"abc_in_java\" , \"func\" : { \"$code\" : \"function abc_in_java(x, y){return x + y ;}\"} , \"funcType\" : 0}";
			Assert.assertEquals(retStr, obj.toString());
		} finally {
			try{
				sdb.rmProcedure(name);
			}catch (BaseException e){
				System.out.println("Failed to remove js procedure");
				System.out.println("Error message is: "+e.getMessage()+e.getErrorCode());
			}
		}
	}

}
