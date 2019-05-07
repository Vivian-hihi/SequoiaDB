package com.sequoias3.delimiter;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import org.testng.Assert;
import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.DataProvider;
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
 * test content: 带prefix、Key-marker和delimiter查询对象版本列表 
 * testlink-case: seqDB-18150
 * @author wangkexin
 * @Date 2019.04.28
 * @version 1.00
 */

public class ListObjectVersionsWithDelimiter18150 extends S3TestBase {
	private String bucketName = "bucket18150";
	private List<String> keyNames = Arrays.asList("dir1/test18150_1","dir1/dir2/test18150_2","dir1/aa/bb/test18150_3","dir1/bb/test18150_4");
	private String delimiter = "te";
	private String prefix = "dir1/";
	private int versionNum = 3;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@DataProvider(name = "keyMarKerProvider")
	public Object[][] generatePageSize() {
		Collections.sort(keyNames);
		
		return new Object[][] {
			//test a :指定位置为中间记录
			new Object[] { keyNames.get(keyNames.size()/2), keyNames.size()/2 + 1 },
			//test b :指定第一条记录
			new Object[] { keyNames.get(0), 1 },
			//test c :指定最后一条记录
			new Object[] { keyNames.get(keyNames.size()-1), keyNames.size() },
			//test d :指定匹配最后一条记录
			new Object[] { keyNames.get(keyNames.size()-2), keyNames.size()-1 },
			//test e :指定keyMarKer不在记录中，i)记录小于所有记录 ii)记录在所有记录中间 iii)记录大于所有记录
			new Object[] { "aaa", 0 },
			new Object[] { keyNames.get(keyNames.size()/2)+"a", keyNames.size()/2 + 1 },
			new Object[] { keyNames.get(keyNames.size()-1)+"a", keyNames.size() },
		};
	}
	
	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		
		//put multiple objects
		for (String objectName : keyNames) {
            for (int j = 0; j < versionNum; j++) {
                s3Client.putObject(bucketName, objectName, "object_file18150");
            }
        }
	}

	@Test(dataProvider = "keyMarKerProvider")
	public void testGetObjectList(String KeyMarker, int startIndex) throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter).withPrefix(prefix).withKeyMarker(KeyMarker));
		List<String> commonPrefixes = versionList.getCommonPrefixes();
		List<String> subKeyNames = keyNames.subList(startIndex, keyNames.size());
		String[] objectNames = new String[subKeyNames.size()];
		List<String> expCommPrefixes = ObjectUtils.getCommPrefixes(subKeyNames.toArray(objectNames), prefix, delimiter);
		ObjectUtils.checkListObjectsV2Commprefixes(commonPrefixes, expCommPrefixes);
		
		List<S3VersionSummary> versions = versionList.getVersionSummaries();
		for(S3VersionSummary version : versions){
			System.out.println("versions : " + version.getKey());
		}
		Assert.assertEquals(versionList.getVersionSummaries().size(), 0);
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
