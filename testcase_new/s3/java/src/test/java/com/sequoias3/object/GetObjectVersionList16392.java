package com.sequoias3.object;

import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * test content: 带分隔符delimiter查询对象版本列表
 * testlink-case: seqDB-16392
 * @author wangkexin
 * @Date 2018.11.24
 * @version 1.00
 */
public class GetObjectVersionList16392 extends S3TestBase {
	private String bucketName = "bucket16392";
	private String[] keyName = {"dir1/test1","dir1/dir2/test2","test3","test4"};
	private String delimiter = "/";
	private String expPrefix = "dir1/";
	private List<String> expVersionsKeyName = new ArrayList<String>();
	private String file = "object16392";
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		for(int i = 0 ; i < keyName.length ; i ++ ){
			s3Client.putObject(bucketName, keyName[i], file);
		}
		for(int i = 0 ; i < keyName.length ; i ++ ){
			s3Client.putObject(bucketName, keyName[i], file);
		}
		expVersionsKeyName.add(keyName[2]);
		expVersionsKeyName.add(keyName[2]);
		expVersionsKeyName.add(keyName[3]);
		expVersionsKeyName.add(keyName[3]);
	}

	@Test
	public void testGetObjectList() throws Exception {
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter));
		Assert.assertEquals(versionList.getCommonPrefixes().size(), 1, "the number of results returned by commonPrefixes is wrong");
		Assert.assertEquals(versionList.getCommonPrefixes().get(0), expPrefix, "the result of commonPrefixes is wrong");
		List<S3VersionSummary> verList = versionList.getVersionSummaries();
		checkVersionsResult(verList);
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
	
	private void checkVersionsResult(List<S3VersionSummary> verList){
		Assert.assertEquals(verList.size(), expVersionsKeyName.size(), "The number of results returned does not match the expected value");
		for(int i = 0 ; i < verList.size() ; i++){
			Assert.assertEquals(verList.get(i).getKey(), expVersionsKeyName.get(i), "the result of versions is wrong!");
		}
	}
}
