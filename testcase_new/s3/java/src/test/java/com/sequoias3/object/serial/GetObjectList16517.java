package com.sequoias3.object.serial;

import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.amazonaws.services.s3.model.S3ObjectSummary;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 桶中对象数量较大，查询对象元数据列表   
 * testlink-case: seqDB-16517
 * @author wangkexin
 * @Date 2018.11.15
 * @version 1.00
 */
public class GetObjectList16517 extends S3TestBase {
	@DataProvider(name = "numberProvider")
	public Object[][] generateObjectNumber() {
		return new Object[][] {
				// test a : 上传10w个对象
				new Object[] {100000},
				// test b : 上传100w个对象
				new Object[] {1000000},
		};
	}
	private String bucketName = "bucket16517";
	private String keyName = "/dir";
	private List<String> expresultList = new ArrayList<String>();
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		s3Client.createBucket(new CreateBucketRequest(bucketName));
	}

	@Test(dataProvider = "numberProvider")
	public void testGetObjectList(int objectTotalNum) throws Exception {
		//put multiple objects
		for(int i = 0 ; i < objectTotalNum ; i++){
			String currentKeyName = keyName+i+"/16517";
			s3Client.putObject(bucketName, currentKeyName, "file16517");
			expresultList.add(currentKeyName);
		}
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName);
		ListObjectsV2Result result; 
		List<S3ObjectSummary> objectSummaries = new ArrayList<S3ObjectSummary>();
		
		do{
			result = s3Client.listObjectsV2(req);
			objectSummaries.addAll(result.getObjectSummaries());
			
			String nextContinuationToken = result.getNextContinuationToken();
			req.setContinuationToken(nextContinuationToken);
		}while(result.isTruncated());
		
		ObjectUtils.checkListObjectsV2KeyName(objectSummaries, expresultList);
		expresultList.clear();
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
}