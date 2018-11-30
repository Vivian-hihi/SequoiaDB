package com.sequoias3.object;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Random;

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
 * test content: 带分隔符delimiter和maxkeys查询 testlink-case: seqDB-16393
 * 
 * @author wangkexin
 * @Date 2018.11.27
 * @version 1.00
 */
public class GetObjectVersionList16393 extends S3TestBase {
	private String bucketName = "bucket16393";
	private String[] keyName = { "dir1/test1", "dir2/dir2_1/test2", "dir3/dir3_1/test3", "test4", "test5" };
	private String delimiter = "/";
	private int maxKeys = 0;
	private List<String> expCommonPrefixes = new ArrayList<String>();
	private List<String> actCommonPrefixes = new ArrayList<String>();
	private List<String> expVersionsKeyName = new ArrayList<String>();
	private List<String> actVersionsKeyName = new ArrayList<String>();
	private int returnedNum = 0;
	private String file = "object16393";
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		// create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		for (int i = 0; i < keyName.length; i++) {
			if(i<3){
				expCommonPrefixes.add(keyName[i].substring(0, keyName[i].indexOf(delimiter)+1));
			}
			s3Client.putObject(bucketName, keyName[i], file);
		}
		for (int i = 0; i < keyName.length; i++) {
			s3Client.putObject(bucketName, keyName[i], file);
		}
		expVersionsKeyName.add(keyName[3]);
		expVersionsKeyName.add(keyName[3]);
		expVersionsKeyName.add(keyName[4]);
		expVersionsKeyName.add(keyName[4]);
		returnedNum = expCommonPrefixes.size()+expVersionsKeyName.size();
	}

	@Test
	public void testGetObjectList() throws Exception {
		Random rand = new Random();
		maxKeys = rand.nextInt(returnedNum)+1;
		int countTurn = 0;
		ListVersionsRequest req = new ListVersionsRequest().withBucketName(bucketName)
						.withDelimiter(delimiter).withMaxResults(maxKeys);
		VersionListing versionList = s3Client.listVersions(req);
		while(true){
			List<String> commprefixesResult = versionList.getCommonPrefixes();
			for(String s : commprefixesResult){
				actCommonPrefixes.add(s);
			}
			List<S3VersionSummary> verList = versionList.getVersionSummaries();
			for(S3VersionSummary s3VersionSummary : verList){
				actVersionsKeyName.add(s3VersionSummary.getKey());
			}
			
			countTurn++;
			if(versionList.isTruncated()){
				versionList = s3Client.listNextBatchOfVersions(versionList);
			}else{
				break;
			}
		}
		
		//check result
		Assert.assertEquals(countTurn, (int)Math.ceil((double)returnedNum/maxKeys), "The total number of results is incorrect");
		Assert.assertTrue(equals(actCommonPrefixes,expCommonPrefixes), "the number of results returned by commonPrefixes is wrong");
		Assert.assertTrue(equals(actVersionsKeyName,expVersionsKeyName), "the number of results returned by versions is wrong");
		
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
	
	private static boolean equals(List<String> a , List<String> b){
		if(a.size()!=b.size())
			return false;
		
		Collections.sort(a);
		Collections.sort(b);
		
		for(int i = 0 ; i < a.size() ; i++){
			if(!a.get(i).equals(b.get(i)))
				return false;
		}
		return true;
	}
}
