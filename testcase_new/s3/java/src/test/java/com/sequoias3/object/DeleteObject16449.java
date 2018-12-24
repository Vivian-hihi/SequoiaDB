package com.sequoias3.object;

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.AmazonS3Exception;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.GetObjectRequest;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

/**
 * test content:  开启版本控制，带versionId删除历史版本对象 
 * @author wangkexin
 * @Date 2018.11.28
 * @version 1.00
 */
public class DeleteObject16449 extends S3TestBase {
	private String bucketName = "bucket16449";
	private String keyName = "testkey16449";
	private List<S3VersionSummary> expVersionList = new ArrayList<>();
	private int oneObjVersionNum = 3;
	private String file = "object16449";
	private File localPath = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		
		s3Client = CommLib.buildS3Client();
		// create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");

		for (int i = 0; i < oneObjVersionNum; i++) {
			s3Client.putObject(bucketName, keyName, file + "." + i);
			//TODO:1、可直接按版本号规则存入版本号旧可以了，如versionId分别为0/1/2
			if (i < oneObjVersionNum - 1) {
				S3VersionSummary version = new S3VersionSummary();
				version.setKey(keyName);
				version.setVersionId(String.valueOf((oneObjVersionNum - 1) - i));
				expVersionList.add(version);//TODO:2、expVersionList只存了两个版本的对象，建议增加描述信息
			}
		}
	}

	@Test
	public void testGetObjectList() throws Exception {
		// delete object with latest version id
		//TODO:3、直接参考版本号规则给出预期版本号，如historyVersionId = '0';
		String historyVersionId = String.valueOf(0);
		s3Client.deleteVersion(bucketName, keyName, historyVersionId);

		try {
			s3Client.getObject(new GetObjectRequest(bucketName, keyName, historyVersionId));
			Assert.fail("the object still exist!");
		} catch (AmazonS3Exception e) {
			Assert.assertEquals(e.getErrorCode(), "NoSuchVersion");
		}

		// check the object version list
		ListVersionsRequest req = new ListVersionsRequest().withBucketName(bucketName);
		VersionListing versionList = s3Client.listVersions(req);
		List<S3VersionSummary> verList = versionList.getVersionSummaries();		
		Assert.assertEquals(verList.size(), expVersionList.size());
		for (int i = 0; i < verList.size(); i++) {
			Assert.assertEquals(verList.get(i).getKey(), expVersionList.get(i).getKey());
			Assert.assertEquals(verList.get(i).getVersionId(), expVersionList.get(i).getVersionId());
		}

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
}
