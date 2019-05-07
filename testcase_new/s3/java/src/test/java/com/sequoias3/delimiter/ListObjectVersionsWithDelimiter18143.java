package com.sequoias3.delimiter;

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
import com.amazonaws.services.s3.model.S3VersionSummary;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 带前缀prefix和delimiter查询对象版本列表
 * testlink-case: seqDB-18143
 * @author wangkexin
 * @Date 2019.04.25
 * @version 1.00
 */

public class ListObjectVersionsWithDelimiter18143 extends S3TestBase {
	private String bucketName = "bucket18143";
	private String[] keyName = {"dir1/test18143_1","dir1/?Dir2/?/dir3/test18143_2","dir1/test18143_3",
			"dir1/dir2/aa/test18143_4","dir1/dir2/aa/cc/test18143_5","dir1/dir2/aa/dd/test18143_6","dir18143","testdir18143.txt"};
	private String delimiter = "?";
	private String prefix = "dir1";
	private int versionNum = 4;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		
		//put multiple objects
		for (String objectName : keyName) {
            for (int j = 0; j < versionNum; j++) {
                s3Client.putObject(bucketName, objectName, "object_file18143");
            }
        }
	}

	@Test
	public void testGetObjectList() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);
		
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter).withPrefix(prefix));
		List<String> commonPrefixes = versionList.getCommonPrefixes();
		List<String> expCommPrefixes = ObjectUtils.getCommPrefixes(keyName, prefix, delimiter);
		ObjectUtils.checkListObjectsV2Commprefixes(commonPrefixes, expCommPrefixes);
		
		List<String> actVersionList = new ArrayList<>();
		List<String> actVersionIdList = new ArrayList<>();
		List<S3VersionSummary> verList = versionList.getVersionSummaries();
		for(S3VersionSummary version : verList){
			actVersionList.add(version.getKey());
			actVersionIdList.add(version.getVersionId());
			Assert.assertFalse(version.isDeleteMarker(), "isDeleteMarker is wrong , key = " + version.getKey());
		}
		
		//check keys of versions
		List<String> expVersionList =  new ArrayList<>();
		for(int i = 0; i < versionNum ; i++){
			expVersionList.addAll(ObjectUtils.getKeys(keyName, prefix, delimiter));
		}
		Collections.sort(expVersionList);
		Assert.assertEquals(actVersionList, expVersionList);
		
		//check keys' versionid of versions
		int keyNum = ObjectUtils.getKeys(keyName, prefix, delimiter).size();
		List<String> expVersionIdList = new ArrayList<>();
		for(int i = 0 ; i < keyNum ; i++){
			for(int j = versionNum - 1 ; j >= 0 ; j--){
				expVersionIdList.add(String.valueOf(j));
			}
		}
		
		Assert.assertEquals(actVersionIdList, expVersionIdList, "actVersionIdList : " + actVersionIdList.toString() + " , expVersionIdList : " + expVersionIdList.toString());
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		try{
			if (runSuccess) {
				CommLib.deleteAllObjectVersions(s3Client, bucketName);
				s3Client.deleteBucket(bucketName);
			}
		}finally{
			if (s3Client != null) {
				s3Client.shutdown();
			}
		}
	}
}
