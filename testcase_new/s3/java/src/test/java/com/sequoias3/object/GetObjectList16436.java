package com.sequoias3.object;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * test content: 带start-after和maxkeys查询对象元数据列表
 * testlink-case: seqDB-16436
 * @author wangkexin
 * @Date 2018.11.16
 * @version 1.00
 */
public class GetObjectList16436 extends S3TestBase {
	private String bucketName = "bucket16436";
	private String keyName = "/dir/dir";
	private List<String> expresultList = new ArrayList<String>(10);
	private int objectTotalNum = 10;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		// create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));

		// put multiple objects
		for (int i = 0; i < objectTotalNum; i++) {
			String currentKeyName = keyName + i + "/16436";
			s3Client.putObject(bucketName, currentKeyName, "object_file16436");
			expresultList.add(currentKeyName);//TODO:4、变量名建议优化下，如keyNameList
		}
	}

	@Test
	public void testGetObjectList() throws Exception {
		int startAfterNextIndex = 0 + 1;//TODO:1、这里用0+1有啥特殊意义吗？如果是第一个直接赋值1。
		int maxKeys = 2;
		//startAfter match the first record
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName)
				.withStartAfter("/dir/dir0/16436").withMaxKeys(maxKeys);
		ListObjectsV2Result result = s3Client.listObjectsV2(req);
		List<S3ObjectSummary> objectSummaries = result.getObjectSummaries();
		//TODO:2、查询未结束会导致游标不会关闭。另外这里只校验一次查询的结果，这里是要多次查询才结束，应该每个查询的maxkeys
		checkListObjectsV2Result(objectSummaries, startAfterNextIndex, maxKeys);

		//startAfter match the last record
		startAfterNextIndex = (objectTotalNum-1) + 1;//TODO:3、建议直接赋值，可以加注释描述
		maxKeys = 5;
		ListObjectsV2Request req2 = new ListObjectsV2Request().withBucketName(bucketName)
				.withStartAfter("/dir/dir"+ (objectTotalNum-1) +"/16436").withMaxKeys(maxKeys);
		ListObjectsV2Result result2 = s3Client.listObjectsV2(req2);
		List<S3ObjectSummary> objectSummaries2 = result2.getObjectSummaries();
		Assert.assertEquals(objectSummaries2.size(), 0, "The number of returned results is wrong");

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}

	private void checkListObjectsV2Result(List<S3ObjectSummary> objectSummaries, int startAfterNextIndex , int expMaxKeys){
		Assert.assertEquals(objectSummaries.size(), expMaxKeys, "The number of returned results is wrong");
		Collections.sort(expresultList);
		for (int i = 0; i < objectSummaries.size(); i++) {
			Assert.assertEquals(objectSummaries.get(i).getKey(), expresultList.get(startAfterNextIndex), "commonPrefixes is wrong");
			startAfterNextIndex++;
		}
	}
}
