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
 * test content: 带前缀prefix和delimiter查询对象版本列表,指定旧delimiter
 * testlink-case: seqDB-18147
 * @author wangkexin
 * @Date 2019.04.28
 * @version 1.00
 */

public class ListObjectVersionsWithDelimiter18147 extends S3TestBase {
	private String bucketName = "bucket18147";
	private String[] keyNames = {"dir1/test18147_1","dir1/?Dir2/?/dir3/test18147_2","dir1/test18147_3",
			"dir1/dir2/aa/test18147_4","dir1/dir2/aa/cc/test18147_5","dir1/dir2/aa/dd/test18147_6","dir18147","testdir18147.txt"};
	private String oldDelimiter = "/";
	private String newDelimiter = "te";
	private String prefix = "dir1/";
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
		for (String objectName : keyNames) {
            for (int j = 0; j < versionNum; j++) {
                s3Client.putObject(bucketName, objectName, "object_file18147");
            }
        }
	}

	@Test
	public void testGetObjectList() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, newDelimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, newDelimiter);
		//查看访问计划索引为对象元数据表索引信息
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(oldDelimiter).withPrefix(prefix));
		List<String> commonPrefixes = versionList.getCommonPrefixes();
		List<String> expCommPrefixes = ObjectUtils.getCommPrefixes(keyNames, prefix, oldDelimiter);
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
			expVersionList.addAll(ObjectUtils.getKeys(keyNames, prefix, oldDelimiter));
		}
		Collections.sort(expVersionList);
		Assert.assertEquals(actVersionList, expVersionList);
		
		//check keys' versionid of versions
		int keyNum = ObjectUtils.getKeys(keyNames, prefix, oldDelimiter).size();
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
