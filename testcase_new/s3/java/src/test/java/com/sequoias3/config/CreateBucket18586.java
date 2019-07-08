package com.sequoias3.config;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.json.JSONArray;
import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.BucketVersioningConfiguration;
import com.sequoias3.region.Region;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.ConfigUtils;
import com.sequoias3.testcommon.s3utils.RegionUtils;
import com.sequoias3.testcommon.s3utils.UserUtils;
import com.sequoias3.user.UserCommDefind;

/**
 * test content: 开启鉴权，执行桶管理操作 testlink-case: seqDB-18586
 * 
 * @author wangkexin
 * @Date 2019.06.20
 * @version 1.00
 */
public class CreateBucket18586 extends S3TestBase {
	@DataProvider(name = "authorizationProvider")
	public Object[][] generateAuthorization() {
		return new Object[][] {
				// test a : authorization 为version2版本
				new Object[] { "bucket18586v2", "AWS " + accessKeys[0] + ":signature" },
				// test b : authorization 为version4版本
				new Object[] { "bucket18586v4", UserCommDefind.authValPre + accessKeys[0] + "/" } };
	}

	private boolean runSuccess = false;
	private String userName = "user18586";
	private int countNum = 5;
	private String roleName = "normal";
	private String keyName = "key18586";
	private String regionName = "region18586";
	private AmazonS3 s3Client = null;
	private String[] accessKeys = null;

	@BeforeClass
	private void setUp() throws Exception {
		CommLib.clearUser(userName);
		RegionUtils.clearRegion(regionName);
		accessKeys = UserUtils.createUser(userName, roleName);
		s3Client = CommLib.buildS3Client(accessKeys[0], accessKeys[1]);

		Region region = new Region();
		region.withName(regionName);
		RegionUtils.putRegion(region);
	}

	@SuppressWarnings("deprecation")
	@Test(dataProvider = "authorizationProvider")
	private void testCreateBucket(String bucketName, String authorization) throws Exception {
		String tmpContent = "content18586" + authorization;
		// create bucket
		ConfigUtils.createBucket(bucketName, regionName, authorization);

		// check head bucket
		ConfigUtils.headBucket(bucketName, authorization);

		List<String> buckets = ConfigUtils.listBuckets(authorization);
		Assert.assertEquals(buckets.size(), 1, "buckets : " + buckets.toString());
		Assert.assertEquals(buckets.get(0), bucketName);
        //TOOD : 1、检查桶是否存在和上面的检查点重复，另外桶的属性信息可以在获取桶时一起验证，如用户、状态和区域等属性
		ConfigUtils.checkCreateBucketResult(s3Client, bucketName, userName);

		// check region info
		String location = ConfigUtils.getBucketLocation(bucketName, authorization);
		Assert.assertEquals(location, regionName);

		ConfigUtils.setBucketVersioning(authorization, bucketName, BucketVersioningConfiguration.ENABLED);
		String status = ConfigUtils.getBucketVersioning(authorization, bucketName);
		Assert.assertEquals(status, BucketVersioningConfiguration.ENABLED);

		List<String> contentList = new ArrayList<>();
		//TODO: 2、countNum变量名定义和实际不相符，建议更新变量名或者增加注释说明
		for (int i = 0; i < countNum; i++) {
			String currentExpContent = tmpContent + "." + i;
			ConfigUtils.putObject(bucketName, keyName, currentExpContent, authorization);
			contentList.add(currentExpContent);
		}
		checkPutObjectResult(authorization, bucketName, contentList);
		clearBucket(authorization, bucketName);
		Assert.assertFalse(s3Client.doesBucketExist(bucketName));
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() throws Exception {
		try {
			if (runSuccess) {
				UserUtils.deleteUser(userName);
				RegionUtils.deleteRegion(regionName);
			}
		} finally {
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}

	private void checkPutObjectResult(String authorization, String bucketName, List<String> contentList)
			throws Exception {
		// Objects in the version list are stored in reverse order by versionId
		Collections.reverse(contentList);
		JSONArray version = ConfigUtils.listVersions(authorization, bucketName);
		// check object content by md5
		for (int i = 0; i < version.length(); i++) {
			Assert.assertEquals(version.getJSONObject(i).getInt("VersionId"), (countNum - 1) - i,
					"versionid is wrong! version : " + version.toString());
			String actMd5 = version.getJSONObject(i).getString("ETag");
			Assert.assertEquals(actMd5, TestTools.getMD5(contentList.get(i).getBytes()),
					"md5 is different! content : " + contentList.get(i) + ", version : " + version.toString());
		}
	}

	private void clearBucket(String authorization, String bucketName) throws Exception {
		JSONArray version = ConfigUtils.listVersions(authorization, bucketName);
		for (int i = 0; i < version.length(); i++) {
			ConfigUtils.deleteVersion(bucketName, version.getJSONObject(i).getString("Key"),
					version.getJSONObject(i).getInt("VersionId"), authorization);
		}
		ConfigUtils.deleteBucket(bucketName, authorization);
	}
}
