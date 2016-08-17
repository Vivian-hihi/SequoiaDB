package com.sequoiadb.test.db;


import static org.junit.Assert.*;
import com.sequoiadb.test.common.*;

import java.lang.reflect.Field;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import java.util.Map;

import junit.framework.Assert;

import org.bson.BSONObject;
import org.bson.BasicBSONObject;
import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Ignore;
import org.junit.Test;

import com.sequoiadb.base.ClientOptions;
import com.sequoiadb.base.CollectionSpace;
import com.sequoiadb.base.DBCollection;
import com.sequoiadb.base.DBCursor;
import com.sequoiadb.base.DBQuery;
import com.sequoiadb.base.Sequoiadb;
import com.sequoiadb.base.SequoiadbConstants;
import com.sequoiadb.exception.BaseException;
import com.sequoiadb.testdata.*;

public class DBClientCache {
	private static Sequoiadb sdb = null ;
	private static CollectionSpace cs = null;
	private static DBCollection cl = null;
	private static DBCursor cursor = null;
	private static String csName1 = "cs_cache_java1";
	private static String csName2 = "cs_cache_java2";
	private static String csName3 = "cs_cache_java3";
	private static String clName1_1 = "cl_cache_java1_1";
	private static String clName1_2 = "cl_cache_java1_2";
	private static String clName1_3 = "cl_cache_java1_3";
	private static String clName2_1 = "cl_cache_java2_1";
	private static String clName2_2 = "cl_cache_java2_2";
	private static String clName2_3 = "cl_cache_java2_3";
	private static String clName3_1 = "cl_cache_java3_1";
	private static String clName3_2 = "cl_cache_java3_2";
	private static String clName3_3 = "cl_cache_java3_3";
	private static String[] csArr = new String[3];
	private static String[] clArr1 = new String[3];
	private static String[] clArr2 = new String[3];
	private static String[] clArr3 = new String[3];
	

	@BeforeClass
	public static void setUpBeforeClass() throws Exception {
		csArr[0] = csName1;
		csArr[1] = csName2;
		csArr[2] = csName3;
		clArr1[0] = clName1_1;
		clArr1[1] = clName1_2;
		clArr1[2] = clName1_3;
		clArr2[0] = clName2_1;
		clArr2[1] = clName2_2;
		clArr2[2] = clName2_3;
		clArr3[0] = clName3_1;
		clArr3[1] = clName3_2;
		clArr3[2] = clName3_3;
		sdb = new Sequoiadb(Constants.COOR_NODE_CONN,"","");
		assertNotNull(sdb);
	}

	@AfterClass
	public static void tearDownAfterClass() throws Exception {
		sdb.disconnect();
	}

	@Before
	public void setUp() throws Exception {
	}

	@After
	public void tearDown() throws Exception {
	}
	
	/// case1:����initClient�ӿڼ�ClientOptions�Ƿ���Ч
	@Test
	public void testInitClientWithDefaultValue() {
		boolean defaultBoolValue = true;
		long defaultLongValue = 300 * 1000;
		Class<?> c = sdb.getClass();
		try {
			Field f_enableCache = c.getDeclaredField("enableCache");
			Field f_cacheInterval = c.getDeclaredField("cacheInterval");
			f_enableCache.setAccessible(true);
			f_cacheInterval.setAccessible(true);
			try {
				System.out.println("f_enableCache is: " + f_enableCache.getBoolean(c));
				System.out.println("f_cacheInterval is: " + f_cacheInterval.getLong(c));
				Assert.assertEquals(defaultBoolValue, f_enableCache.getBoolean(c));
				Assert.assertEquals(defaultLongValue, f_cacheInterval.getLong(c));
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
				Assert.fail();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
				Assert.fail();
			}
		} catch (SecurityException e) {
			e.printStackTrace();
			Assert.fail();
		} catch (NoSuchFieldException e) {
			e.printStackTrace();
			Assert.fail();
		}
	}
	@Test
	public void testInitClientWithDefinedValue() {
		boolean definedBoolValue = true;
		long definedLongValue = 0 * 1000;
		ClientOptions options = new ClientOptions();
		options.setCacheInterval(definedLongValue);
		options.setEnableCache(definedBoolValue);
		Sequoiadb.initClient(options);
		Sequoiadb db = new Sequoiadb(Constants.COOR_NODE_CONN,"","");
		Class<?> c = db.getClass();
		try {
			// test1: ������ȫ�ֲ���󣬽��������Խ��������ӳ��е�ȫ�ֲ����Ƿ���ȷ
			Field f_enableCache = c.getDeclaredField("enableCache");
			Field f_cacheInterval = c.getDeclaredField("cacheInterval");
			f_enableCache.setAccessible(true);
			f_cacheInterval.setAccessible(true);
			try {
				System.out.println("f_enableCache is: " + f_enableCache.getBoolean(c));
				System.out.println("f_cacheInterval is: " + f_cacheInterval.getLong(c));
				Assert.assertEquals(definedBoolValue, f_enableCache.getBoolean(c));
				Assert.assertEquals(definedLongValue, f_cacheInterval.getLong(c));
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
				Assert.fail();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
				Assert.fail();
			}
			// test2: �ڽ���֮����������ȫ�ֲ���������ӳ��е�ȫ�ֲ����Ƿ���ȷ
			definedBoolValue = false;
			definedLongValue = 60 * 1000;
			options.setEnableCache(definedBoolValue);
			options.setCacheInterval(definedLongValue);
			Sequoiadb.initClient(options);
			try {
				System.out.println("f_enableCache is: " + f_enableCache.getBoolean(c));
				System.out.println("f_cacheInterval is: " + f_cacheInterval.getLong(c));
				Assert.assertEquals(definedBoolValue, f_enableCache.getBoolean(c));
				Assert.assertEquals(definedLongValue, f_cacheInterval.getLong(c));
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
				Assert.fail();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
				Assert.fail();
			}
			// test3: �ڽ���֮���������ô����ȫ�ֲ���������ӳ��е�ȫ�ֲ����Ƿ���ȷ
			definedBoolValue = false;
			definedLongValue = -60 * 1000;
			long expectLongValue = 300 * 1000;
			options.setEnableCache(definedBoolValue);
			options.setCacheInterval(definedLongValue);
			Sequoiadb.initClient(options);
			try {
				System.out.println("f_enableCache is: " + f_enableCache.getBoolean(c));
				System.out.println("f_cacheInterval is: " + f_cacheInterval.getLong(c));
				Assert.assertEquals(definedBoolValue, f_enableCache.getBoolean(c));
				Assert.assertEquals(expectLongValue, f_cacheInterval.getLong(c));
			} catch (IllegalArgumentException e) {
				e.printStackTrace();
				Assert.fail();
			} catch (IllegalAccessException e) {
				e.printStackTrace();
				Assert.fail();
			}
		} catch (SecurityException e) {
			e.printStackTrace();
			Assert.fail();
		} catch (NoSuchFieldException e) {
			e.printStackTrace();
			Assert.fail();
		}
		db.disconnect();
	}
	
	/// case2:����ClientOptions���õ�ʱ���Ƿ���Ч
	// �ֹ����ԣ���Ҫ��getCollectionSpace�����(new Exception()).printStackTrace();
	// ����ӡ���ö�ջ��
	/*
	public CollectionSpace getCollectionSpace(String csName)
			throws BaseException {
		// get cs object from cache
		if (fetchCache(csName)) {
			// TODO: debug
			(new Exception()).printStackTrace();
			return new CollectionSpace(this, csName);
		}
		// get cs object from database
		// we don't need to update or remove cache here,
		// for "isCollectionSpaceExist" has do that
		if (isCollectionSpaceExist(csName)) {
			// TODO: debug
			(new Exception()).printStackTrace();
			return new CollectionSpace(this, csName);
		} else {
			throw new BaseException("SDB_DMS_CS_NOTEXIST", csName);
		}
	} 
	*/
	@Test
	@Ignore
	public void testCacheIntervalWorksOrNot() throws InterruptedException {
		boolean definedBoolValue = true;
		long definedLongValue = 6 * 1000;
		ClientOptions options = new ClientOptions();
		options.setCacheInterval(definedLongValue);
		options.setEnableCache(definedBoolValue);
		Sequoiadb.initClient(options);
		Sequoiadb db = new Sequoiadb(Constants.COOR_NODE_CONN,"","");
		CollectionSpace cs1 = null;
		CollectionSpace cs2 = null;
		cs1 = db.getCollectionSpace(csName1);
		Thread.sleep(definedLongValue - 3000);
		// test1: ��ȡcs����ʱӦ�÷��ʻ���
		cs1 = db.getCollectionSpace(csName1);
		Thread.sleep(3500);
		// test2: ��ȡcs����ʱӦ�÷�����ݿ�
		cs1 = db.getCollectionSpace(csName1);
		db.disconnect();
	}
	
	/// case3:�����뻺����صļ��������ڲ��߼��Ƿ���ȷ
	// upsertCache/removeCache/fetchCache
	@Test
	public void testCacheLogicWithEnableCache() throws SecurityException, NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
		boolean defaultBoolValue = true;
		long defaultLongValue = 60 * 1000;
		ClientOptions options = new ClientOptions();
		options.setEnableCache(defaultBoolValue);
		options.setCacheInterval(defaultLongValue);
		Sequoiadb.initClient(options);
		Sequoiadb db1 = new Sequoiadb(Constants.COOR_NODE_CONN,"","");
		// ��ȡ�����map
		Class<?> c = db1.getClass();
		Field f_nameCache = c.getDeclaredField("nameCache");
		boolean accessFlag = f_nameCache.isAccessible();
		f_nameCache.setAccessible(true);
		@SuppressWarnings("unchecked")
		Map<String, Long> map1 = (Map<String, Long>)(f_nameCache.get(db1));
		
		// create cs
		CollectionSpace[] csObjArr = new CollectionSpace[3];
		for(int i = 0; i < csArr.length; i++) {
			try{
				db1.dropCollectionSpace(csArr[i]);
			}catch(BaseException e) {
			}
		}
		for(int i = 0; i < csArr.length; i++) {
			csObjArr[i] = db1.createCollectionSpace(csArr[i]);
		}
		// test1: ���db������cs�Ļ������
		// TODO:
		System.out.println("point 1: after creating cs, nameCache.size() is: " + map1.size());
		Assert.assertEquals(csArr.length, map1.size());
		for(int i = 0; i < csArr.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[i]));
		}
		// drop one cs
		db1.dropCollectionSpace(csArr[0]);
		// test2: ɾ��һ��cs֮�󣬼��db������cs�Ļ������
		Assert.assertEquals(csArr.length - 1, map1.size());
		Assert.assertFalse(map1.containsKey(csArr[0]));
		for(int i = 1; i < csArr.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[i]));
		}
		// create the drop cs
		csObjArr[0] = db1.createCollectionSpace(csArr[0]);
		Assert.assertEquals(csArr.length, map1.size());
		for(int i = 0; i < csArr.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[i]));
		}
		// create cl
		BSONObject conf = new BasicBSONObject();
		conf.put("ReplSize", 0);
		for(int i = 0; i < clArr1.length; i++) {
			//System.out.println("csObjArr[0] is: " + csObjArr[0].getName() + ", clArr1[x] is: " + clArr1[i]);
			csObjArr[0].createCollection(clArr1[i], conf);
		}
		// test3: ���db������cs,cl�Ļ������
		Assert.assertEquals(csArr.length + clArr1.length, map1.size());
		for(int i = 0; i < csArr.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[i]));
		}
		for(int i = 0; i < clArr1.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[0] + "." + clArr1[i]));
		}
        // drop one cl
		csObjArr[0].dropCollection(clArr1[0]);
		// test4: ���ɾ��cl֮��cs,cl�Ļ������
		Assert.assertEquals(csArr.length + clArr1.length - 1, map1.size());
		for(int i = 0; i < csArr.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[i]));
		}
		Assert.assertFalse(map1.containsKey(csArr[0] + "." + clArr1[0]));
		for(int i = 1; i < clArr1.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[0] + "." + clArr1[i]));
		}
		// drop one cs
		db1.dropCollectionSpace(csArr[0]);
		// test5: ���ɾ��cs֮��cs,cl�Ļ������
		Assert.assertEquals(csArr.length - 1, map1.size());
		Assert.assertFalse(map1.containsKey(csArr[0]));
		for(int i = 1; i < csArr.length; i++) {
			Assert.assertTrue(map1.containsKey(csArr[i]));
		}
		for(int i = 1; i < clArr1.length; i++) {
			Assert.assertFalse(map1.containsKey(csArr[0] + "." + clArr1[i]));
		}
//		for(int i = 0; i < clArr2.length; i++) {
//			System.out.println("csObjArr[1] is: " + csObjArr[1].getName() + ", clArr2[x] is: " + clArr2[i]);
//			csObjArr[1].createCollection(clArr2[i], conf);
//		}
//		for(int i = 0; i < clArr3.length; i++) {
//			System.out.println("csObjArr[2] is: " + csObjArr[2].getName() + ", clArr3[x] is: " + clArr3[i]);
//			csObjArr[2].createCollection(clArr3[i], conf);
//		}
//		// TODO:
//		System.out.println("f_nameCache is: " + ((Object)f_nameCache.get(db)).toString());

		f_nameCache.setAccessible(accessFlag);
		db1.disconnect();
	}
	
	@Test
	public void testCacheLogicWithDisableCache() throws SecurityException, NoSuchFieldException, IllegalArgumentException, IllegalAccessException {
		boolean defaultBoolValue = false;
		long defaultLongValue = 60 * 1000;
		ClientOptions options = new ClientOptions();
		options.setCacheInterval(defaultLongValue);
		options.setEnableCache(defaultBoolValue);
		Sequoiadb.initClient(options);
		Sequoiadb db = new Sequoiadb(Constants.COOR_NODE_CONN,"","");
		// ��ȡ�����map
		Class<?> c = db.getClass();
		Field f_nameCache = c.getDeclaredField("nameCache");
		boolean accessFlag = f_nameCache.isAccessible();
		f_nameCache.setAccessible(true);
		@SuppressWarnings("unchecked")
		Map<String, Long> map = (Map<String, Long>)(f_nameCache.get(db));
		
		// create cs
		CollectionSpace[] csObjArr = new CollectionSpace[3];
		for(int i = 0; i < csArr.length; i++) {
			try{
				db.dropCollectionSpace(csArr[i]);
			}catch(BaseException e) {
			}
		}
		for(int i = 0; i < csArr.length; i++) {
			csObjArr[i] = db.createCollectionSpace(csArr[i]);
		}
		// test1: ���db������cs�Ļ������
		// TODO:
		System.out.println("point 2: after creating cs, nameCache.size() is: " + map.size());
		Assert.assertEquals(0, map.size());
		// drop one cs
		db.dropCollectionSpace(csArr[0]);
		// test2: ɾ��һ��cs֮�󣬼��db������cs�Ļ������
		Assert.assertEquals(0, map.size());
		// create the drop cs
		csObjArr[0] = db.createCollectionSpace(csArr[0]);
		Assert.assertEquals(0, map.size());
		// create cl
		BSONObject conf = new BasicBSONObject();
		conf.put("ReplSize", 0);
		for(int i = 0; i < clArr1.length; i++) {
			//System.out.println("csObjArr[0] is: " + csObjArr[0].getName() + ", clArr1[x] is: " + clArr1[i]);
			csObjArr[0].createCollection(clArr1[i], conf);
		}
		// test3: ���db������cs,cl�Ļ������
		Assert.assertEquals(0, map.size());
        // drop one cl
		csObjArr[0].dropCollection(clArr1[0]);
		// test4: ���ɾ��cl֮��cs,cl�Ļ������
		Assert.assertEquals(0, map.size());
		// drop one cs
		db.dropCollectionSpace(csArr[0]);
		// test5: ���ɾ��cs֮��cs,cl�Ļ������
		Assert.assertEquals(0, map.size());

		f_nameCache.setAccessible(accessFlag);
		db.disconnect();
	}
	
	/// case4�� cache����/�ر�ʱ��ʹ�����ӳ���ҵ��
	
	/// case5: cache����ʱ�������쳣����
	// ��Ҫ�ֹ�
	@Test
	public void testInvalidSituaction() throws SecurityException, NoSuchFieldException, IllegalArgumentException, IllegalAccessException
	{
		ClientOptions options = new ClientOptions();
		options.setEnableCache(true);
		Sequoiadb.initClient(options);
		Sequoiadb db = new Sequoiadb(Constants.COOR_NODE_CONN,"","");
		// ��ȡ�����map
		Class<?> c = db.getClass();
		Field f_nameCache = c.getDeclaredField("nameCache");
		boolean accessFlag = f_nameCache.isAccessible();
		f_nameCache.setAccessible(true);
		@SuppressWarnings("unchecked")
		Map<String, Long> map = (Map<String, Long>)(f_nameCache.get(db));
		
		String csName = "foo_java.";
		String clName = "bar_java.";
		try {
			db.dropCollectionSpace(csName);
		} catch(BaseException e) {
		}
		try {
			db.createCollectionSpace(csName);
			Assert.fail();
		} catch(BaseException e) {
			csName = "foo_java";
		}
		try {
			db.dropCollectionSpace(csName);
		} catch(BaseException e) {
		}
		// check
		Assert.assertEquals(0, map.size());
		CollectionSpace cs = db.createCollectionSpace(csName);
		// check
		Assert.assertEquals(1, map.size());
		Assert.assertTrue(map.containsKey(csName));
		try {
			cs.createCollection(clName);
			Assert.fail();
		} catch(BaseException e) {
			clName = "bar_java";
		}
		// check
		Assert.assertEquals(1, map.size());
		Assert.assertTrue(map.containsKey(csName));
		cs.createCollection(clName);
		// check
		Assert.assertEquals(2, map.size());
		Assert.assertTrue(map.containsKey(csName + "." + clName));
		Assert.assertTrue(map.containsKey(csName));
		
		db.dropCollectionSpace(csName);
		// check
		Assert.assertEquals(0, map.size());
		
		f_nameCache.setAccessible(accessFlag);
		db.disconnect();
	}

	
}
