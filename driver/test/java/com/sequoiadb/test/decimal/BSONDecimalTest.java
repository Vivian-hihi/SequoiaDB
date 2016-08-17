package com.sequoiadb.test.decimal;

import static org.junit.Assert.*;
import static org.junit.Assert.assertEquals;

import java.math.BigDecimal;
import java.math.BigInteger;
import java.math.MathContext;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.bson.types.BSONDecimal;
import org.bson.types.BSONTimestamp;
import org.bson.types.ObjectId;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Assert;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.base.SequoiadbConstants;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.test.common.*;
import com.sun.org.apache.bcel.internal.classfile.ConstantCP;
import com.sequoiadb.test.common.*;
/**
 * @author tanzhaobo
 * @brief 测试对外的BSONDecimal类型的对外接口
 */
public class BSONDecimalTest {

	private static Sequoiadb sdb;
	private static CollectionSpace cs;
	private static DBCollection cl;
	private static DBCursor cur;

	@BeforeClass
	public static void beforeClass() throws Exception {
		// sdb
		sdb = new Sequoiadb(Constants.COOR_NODE_CONN, "", "");
		// cs
		if (sdb.isCollectionSpaceExist(Constants.TEST_CS_NAME_1)) {
			sdb.dropCollectionSpace(Constants.TEST_CS_NAME_1);
			cs = sdb.createCollectionSpace(Constants.TEST_CS_NAME_1);
		} else {
			cs = sdb.createCollectionSpace(Constants.TEST_CS_NAME_1);
		}
		// cl
		cl = cs.createCollection(Constants.TEST_CL_NAME_1, 
				new BasicBSONObject().append("ReplSize", 0));
	}

	@AfterClass
	public static void afterClass() throws Exception {
		try {
			sdb.dropCollectionSpace(Constants.TEST_CS_NAME_1);
		} catch (BaseException e) {
			e.printStackTrace();
		}
		sdb.disconnect();
	}

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
//		cl.truncate();
	}
	
	/**
	 * 用户构建BSONDecimal对象后，对象持有的内容是否正确。
	 */
	@Test
	public void buildBSONDecimalTest() {
		BSONObject obj = null;
		BSONDecimal decimal = null;
		BSONDecimal retDecimal = null;
		String value = null;
		String retValue = null;
		String expectValue = null;
		int precision = 0;
		int retPrecision = 0;
		int scale = 0;
		int retScale = 0;
		
		// case 1: specify string value, precision, scale
        value = "123456789.0987654321";
		expectValue = "123456789.098765432100000";
        precision = 30;
        scale = 15;
        decimal = new BSONDecimal(value, precision, scale);
        obj = new BasicBSONObject().append("case1", decimal);
        cl.insert(obj);
        cur = cl.query(new BasicBSONObject().append("case1", new BasicBSONObject("$exists",1)), 
        		new BasicBSONObject("case1", ""), null, null);
        assertTrue(cur.hasNext());
        obj = cur.getNext();
        retDecimal = (BSONDecimal)obj.get("case1");
        retValue = retDecimal.getValue();
        retPrecision = retDecimal.getPrecision();
        retScale = retDecimal.getScale();
        Assert.assertEquals(expectValue, retValue);
        Assert.assertEquals(precision, retPrecision);
        Assert.assertEquals(scale, retScale);

		// case 2: specify string value
        value = "1.234567890987654321";
		expectValue = value;
        decimal = new BSONDecimal(value);
        obj = new BasicBSONObject().append("case2", decimal);
        cl.insert(obj);
        cur = cl.query(new BasicBSONObject().append("case2", new BasicBSONObject("$exists",1)), 
        		new BasicBSONObject("case2", ""), null, null);
        assertTrue(cur.hasNext());
        obj = cur.getNext();
        retDecimal = (BSONDecimal)obj.get("case2");
        retValue = retDecimal.getValue();
        retPrecision = retDecimal.getPrecision();
        retScale = retDecimal.getScale();
        Assert.assertEquals(expectValue, retValue);
        Assert.assertEquals(-1, retPrecision);
        Assert.assertEquals(-1, retScale);
		
		// case 3: specify BigDecimal
        value = "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890";
        expectValue = "123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890.1234567890";
        MathContext context = new MathContext(75);
        
        BigInteger bigInt = new BigInteger(value);
        BigDecimal big = new BigDecimal(bigInt, 10, context);
        decimal = new BSONDecimal(big);
        Assert.assertEquals(-1, decimal.getPrecision());
        Assert.assertEquals(-1, decimal.getScale());
        obj = new BasicBSONObject().append("case3", decimal);
        cl.insert(obj);
        cur = cl.query(new BasicBSONObject().append("case3", new BasicBSONObject("$exists",1)), 
        		new BasicBSONObject("case3", ""), null, null);
        assertTrue(cur.hasNext());
        obj = cur.getNext();
        retDecimal = (BSONDecimal)obj.get("case3");
        retValue = retDecimal.getValue();
        retPrecision = retDecimal.getPrecision();
        retScale = retDecimal.getScale();
        Assert.assertEquals(-1, retPrecision);
        Assert.assertEquals(-1, retScale);
        
		// case 4: toBigDecimal
		BigDecimal big2 = retDecimal.toBigDecimal();
		Assert.assertEquals(0, big.compareTo(big2));
		
		// case 5: getValue
		Assert.assertEquals(big.toString(), decimal.getValue());
		Assert.assertEquals(big.toPlainString(), retDecimal.getValue());
		
	}

}
