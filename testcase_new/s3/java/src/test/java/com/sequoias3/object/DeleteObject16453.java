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
 * test content: 带versionId删除对象，该对象为删除标记
 * @author wangkexin
 * @Date 2018.11.28
 * @version 1.00
 */

public class DeleteObject16453 extends S3TestBase {
	private String bucketName = "bucket16453";
	private String keyName = "testkey16453";
	private int deleteVersionNum = 3;
	private String file = "object16453";
	private List<S3VersionSummary> expDeleteMarkersList = new ArrayList<S3VersionSummary>();
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		// create bucket and set bucket status is enabled
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		s3Client.putObject(bucketName, keyName, file);
		
		for(int i = 0 ; i < deleteVersionNum ; i++){
			s3Client.deleteObject(bucketName, keyName);
			S3VersionSummary version = new S3VersionSummary();
			version.setKey(keyName);
			version.setIsDeleteMarker(true);
			version.setVersionId(String.valueOf(deleteVersionNum-i));
			expDeleteMarkersList.add(version);
		}
		expDeleteMarkersList.size();//TODO:1、多余的代码吗？
	}

	@Test
	public void testGetObjectList() throws Exception {
		// delete the latest version of deletemarker
		s3Client.deleteVersion(bucketName, keyName, expDeleteMarkersList.get(0).getVersionId());
		
		// check the object version list
		ListVersionsRequest req = new ListVersionsRequest().withBucketName(bucketName);
		VersionListing versionList = s3Client.listVersions(req);
		List<S3VersionSummary> verList = versionList.getVersionSummaries();
		checkDeleteMarkerResult(verList, 0);//TODO:1、这里的0请给下说明或者定义变量名。
		
		// delete the history version of deletemarker
		s3Client.deleteVersion(bucketName, keyName, "1");
		
		// check the object version list
		req = new ListVersionsRequest().withBucketName(bucketName);
		versionList = s3Client.listVersions(req);
		verList = versionList.getVersionSummaries();
		checkDeleteMarkerResult(verList, expDeleteMarkersList.size()-1);
		
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
	
	private void checkDeleteMarkerResult(List<S3VersionSummary> verList, int removeIndex){
		expDeleteMarkersList.remove(removeIndex);
		Assert.assertEquals(verList.size()-1, expDeleteMarkersList.size());//TODO:2、这里比较为啥要把实际的versionList减去一个？
		for(int i = 1 ; i < verList.size() ; i++){//TODO:3、这里为啥忽略0？
			Assert.assertEquals(verList.get(i).getKey(), expDeleteMarkersList.get(i-1).getKey());
			Assert.assertEquals(verList.get(i).isDeleteMarker(), expDeleteMarkersList.get(i-1).isDeleteMarker());
			Assert.assertEquals(verList.get(i).getVersionId(), expDeleteMarkersList.get(i-1).getVersionId());
		}
	}
}
