package com.sequoias3.delimiter;

import java.util.ArrayList;
import java.util.Arrays;
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
 * test content: 带分隔符delimiter和maxkeys查询 
 * testlink-case: seqDB-18148
 * @author wangkexin
 * @Date 2019.04.28
 * @version 1.00
 */

public class ListObjectVersionsWithDelimiter18148 extends S3TestBase {
	private String bucketName = "bucket18148";
	private String[] keyNames = {"dir1/test18148_1","dir1/test18148_1","dir1/test18148_1","dir1/dir2/test18148_2",
						"dir1/dir2/test18148_2","dir1/dir2/test18148_2","/dir/test/test18148_3",
						"test18148_4","dir1/test1/test18148_5","dir/log","dir/log","dir/log","dir1/log"};
	
	private String[] multiVerKeys = {"dir1/test18148_1","dir1/dir2/test18148_2","dir/log"};
	private List<String> expCommPrefixes = new ArrayList<>();
	private List<String> expVersionList = new ArrayList<String>();
	private String delimiter = "tes";
	private int versionNum = 3;
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
			s3Client.putObject(bucketName, objectName, "object_file18148");
        }
	}

	@Test
	public void testGetObjectList() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		expCommPrefixes = ObjectUtils.getCommPrefixes(keyNames, "", delimiter);
		expVersionList = ObjectUtils.getKeys(keyNames, "", delimiter);
		Collections.sort(expVersionList);
		//maxKeys 大于匹配记录数
		checkResult(10, expCommPrefixes.size() + expVersionList.size());
		
		//maxKeys 小于匹配记录数
		checkResult(1, 1);
		
		runSuccess = true;
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
	
	private void checkResult(int maxResults, int expOnceReturnedNum){
		ListVersionsRequest req = new ListVersionsRequest().withBucketName(bucketName).withDelimiter(delimiter).withMaxResults(maxResults);
		VersionListing versionList = s3Client.listVersions(req);
		List<String> commonPrefixes = new ArrayList<String>();
		List<KeyAndVersionId> actVersionList = new ArrayList<KeyAndVersionId>();
		while(true){
			int onceReturn = 0;
			commonPrefixes.addAll(versionList.getCommonPrefixes());
			onceReturn += versionList.getCommonPrefixes().size();
			List<S3VersionSummary> verList = versionList.getVersionSummaries();
			for (S3VersionSummary s3VersionSummary : verList) {
				KeyAndVersionId obj = new KeyAndVersionId(s3VersionSummary.getKey(), s3VersionSummary.getVersionId());
				actVersionList.add(obj);
			}

			onceReturn += verList.size();
			Assert.assertEquals(onceReturn, expOnceReturnedNum,"commonPrefixes : " + commonPrefixes.toString() + ", versions : " + printList(actVersionList));
			if (versionList.isTruncated()) {
				versionList = s3Client.listNextBatchOfVersions(versionList);
			} else {
				break;
			}
		}
		
		ObjectUtils.checkListObjectsV2Commprefixes(commonPrefixes, expCommPrefixes);
		
		// check keys of versions
		Assert.assertEquals(actVersionList.size(), expVersionList.size(), " act result is : " + actVersionList.toString() + " , exp result is : "  + expVersionList.toString());

		int versionId = versionNum -1 ;
		List<String> expVersionId = Arrays.asList(multiVerKeys);
		for(int i = 0 ; i < actVersionList.size(); i ++){
			String currentKey = actVersionList.get(i).key;
			String currentVersion = actVersionList.get(i).versionId;
			Assert.assertEquals(currentKey, expVersionList.get(i));
			if(expVersionId.contains(currentKey)){
				Assert.assertEquals(currentVersion, String.valueOf(versionId));
				versionId--;
			}else{
				Assert.assertEquals(currentVersion, "0", " current key is : " + currentKey);
			}
		}
	}
	
	private class KeyAndVersionId{
		private String key = "";
		private String versionId = "";
		public KeyAndVersionId(String key, String versionId){
			this.key = key;
			this.versionId = versionId;
		}
	}
	
	private String printList(List<KeyAndVersionId> versionList){
		String str = "";
		for(KeyAndVersionId obj : versionList){
			str += "[key : " + obj.key + ", value : " + obj.versionId + "]";
		}
		return str;
	}
}
