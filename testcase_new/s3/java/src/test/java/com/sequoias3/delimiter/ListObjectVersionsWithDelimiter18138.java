package com.sequoias3.delimiter;

import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.CreateBucketRequest;
import com.amazonaws.services.s3.model.ListVersionsRequest;
import com.amazonaws.services.s3.model.VersionListing;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;
import com.sequoias3.testcommon.s3utils.ObjectUtils;

/**
 * test content: 带分隔符查询桶中对象版本列表，匹配所有记录
 * testlink-case: seqDB-18138
 * @author wangkexin
 * @Date 2019.04.24
 * @version 1.00
 */

public class ListObjectVersionsWithDelimiter18138 extends S3TestBase {
	private String bucketName = "bucket18138";
	private String keyName = "dir";
	private String delimiter = "?";
	private String repeatedKeyName = "dir1?test18138";
	private String[] objectNames = new String[200];
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		
		//put multiple objects
		for(int i = 0 ; i < objectNames.length ; i++){
			String currentKeyName = keyName + i + delimiter + "test18138";
			s3Client.putObject(bucketName, currentKeyName, "object_file18138");
			objectNames[i] = currentKeyName;
		}
		
		//put object key = "dir1?test18138" twice again		
		s3Client.putObject(bucketName, repeatedKeyName, "object_file18138");
		s3Client.putObject(bucketName, repeatedKeyName, "object_file18138");
	}

	@Test
	public void testGetObjectList() throws Exception {
		// 将分隔符设置为?（默认为'/'）
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);
		
		//db端查看访问计划显示索引为目录表索引
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter));
		List<String> commonPrefixes = versionList.getCommonPrefixes();
		
		List<String> expresultList = ObjectUtils.getCommPrefixes(objectNames, "", delimiter);
		ObjectUtils.checkListObjectsV2Commprefixes(commonPrefixes, expresultList);
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
