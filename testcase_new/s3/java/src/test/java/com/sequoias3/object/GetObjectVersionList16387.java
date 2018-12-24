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
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.PutObjectResult;
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * test content:  对象存在多个版本，查询对象版本列表
 * testlink-case: seqDB-16387
 * @author wangkexin
 * @Date 2018.11.23
 * @version 1.00
 */
public class GetObjectVersionList16387 extends S3TestBase {
	private String bucketName = "bucket16387";
	private String keyName = "/dir/dir";
	private List<String> expresultList = new ArrayList<String>();
	private int objectTotalNum = 5;
	private String latestVersionId = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		
		//put multiple objects
		for(int i = 0 ; i < objectTotalNum ; i++){
			String currentKeyName = keyName+i+"/16387";
			s3Client.putObject(bucketName, currentKeyName, "object_file16387");
			expresultList.add(currentKeyName);
		}
		
		//put object key = "/dir/dir1/16387" twice again
		String currentKeyName = keyName+"1/16387";//TODO:1、建议修改变量名，这里的变量名和上面的同名变量名时不同的意思，需要区别命名
		s3Client.putObject(bucketName, currentKeyName, "object_file16387");
		expresultList.add(currentKeyName);
		PutObjectResult result = s3Client.putObject(bucketName, currentKeyName, "object_file16387");
		latestVersionId = result.getVersionId();
		expresultList.add(currentKeyName);
	}

	@Test
	public void testGetObjectList() throws Exception {
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName));
		List<S3VersionSummary> verList = versionList.getVersionSummaries();
		checkListObjectsResult(verList);
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}

	private void checkListObjectsResult(List<S3VersionSummary> versions){
		Collections.sort(expresultList);
		Assert.assertEquals(versions.size(), expresultList.size(),"The number of results returned does not match the expected value");
		for( int i = 0;i< versions.size();i++){
			Assert.assertEquals(versions.get(i).getKey(),expresultList.get(i), "commonPrefixes is wrong");
			//TODO:1、多个版本的key名，前面已经定义了变量，这里可以直接用变量，或者补充描述
			if(versions.get(i).getKey().equals("/dir/dir1/16387")){
				if(versions.get(i).getVersionId().equals(latestVersionId)){
					Assert.assertEquals(versions.get(i).isLatest(), true, "isLatest is wrong");
				}else{
					Assert.assertEquals(versions.get(i).isLatest(), false, "isLatest is wrong");
				}
			}else{
				Assert.assertEquals(versions.get(i).isLatest(), true, "isLatest is wrong");
			}
		}
	}
}
