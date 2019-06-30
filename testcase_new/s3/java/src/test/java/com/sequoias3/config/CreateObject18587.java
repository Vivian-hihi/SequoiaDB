package com.sequoias3.config;

import java.util.ArrayList;
import java.util.List;

import org.json.JSONObject;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ConfigUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * test content: 开启鉴权，执行对象管理操作 testlink-case: seqDB-18587
 * 
 * @author wangkexin
 * @Date 2019.06.21
 * @version 1.00
 */
public class CreateObject18587 extends S3TestBase {
	@DataProvider(name = "authorizationProvider")
	public Object[][] generateAuthorization() {
		return new Object[][] {
				// test a : authorization 为version2版本
				new Object[] { "bucket18587v2", "AWS " + accessKeys[0] + ":signature" },
				// test b : authorization 为version4版本
				new Object[] { "bucket18587v4", UserCommDefind.authValPre + accessKeys[0] + "/" } };
	}

	private boolean runSuccess = false;
	private String userName = "user18587";
	private String roleName = "normal";
	private String[] objectNames = { "dir1?test18587_1", "dir1?Dir2?test18587_2", "?aa?bb?test18587_3",
			"?aa?cc?test18587_4" };
	private String content = "content18587";
	private String delimiter = "?";
	private AmazonS3 s3Client = null;
	private String[] accessKeys = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);
	}

	@Test(dataProvider = "authorizationProvider")
	private void testCreateObject(String bucketName, String authorization) throws Exception {
		// create bucket
		ConfigUtils.createBucket(bucketName, authorization);
		ConfigUtils.putBucketDelimiter(bucketName, delimiter, authorization);
		ConfigUtils.checkCurrentDelimiteInfo(bucketName, delimiter, authorization);

		for (int i = 0; i < objectNames.length; i++) {
			String tmpContent = content + "_" + i;
			ConfigUtils.putObject(bucketName, objectNames[i], tmpContent, authorization);
			String actMd5 = ConfigUtils.getObject(bucketName, objectNames[i], authorization);
			Assert.assertEquals(actMd5, TestTools.getMD5(tmpContent.getBytes()),
					"get object, check etag is wrong, key = " + objectNames[i]);
			ConfigUtils.headObject(bucketName, objectNames[i], authorization);
		}

		// check
		List<String> expContentList = new ArrayList<>();
		List<String> expCommonPrefixes = ObjectUtils.getCommPrefixes(objectNames, "", delimiter);
		JSONObject ListBucketResultObj = ConfigUtils.listObjectsWithDelimiter(bucketName, delimiter, authorization);
		ConfigUtils.checkListObjV2WithDelimiter(ListBucketResultObj, expCommonPrefixes, expContentList);

		ConfigUtils.listOvjectV2(bucketName, authorization);
		clearBucket(bucketName, authorization);
		Assert.assertFalse(s3Client.doesBucketExist(bucketName));
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void clearBucket(String bucketName, String authorization) throws Exception {
		List<String> keyNames = ConfigUtils.listOvjectV2(bucketName, authorization);
		for (String key : keyNames) {
			ConfigUtils.deleteObjet(bucketName, key, authorization);
		}
		ConfigUtils.deleteBucket(bucketName, authorization);
	}
}
