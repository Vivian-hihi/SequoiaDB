package com.sequoias3.object;

import java.text.ParseException;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
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
 * test content: 带前缀prefix查询对象版本列表 
 * testlink-case: seqDB-16390
 * @author wangkexin
 * @Date 2018.11.23
 * @version 1.00
 */
public class GetObjectVersionList16390 extends S3TestBase {
	private String bucketName = "bucket16390";
	private String[] keyName = {"dir1/dir2/test1","dir1/dir2/test2","test3","test4"};
	private String prefix = "dir1";
	private List<String> expEtagList = new ArrayList<String>();
	private PutObjectResult objReault = null;
	private String file = "object16390";
	private Date beforeDate = null;
	private Calendar beforeCalEarliest = null;
	private Calendar beforeCalLatest = null;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		beforeDate= new Date();

		for(int i = 0 ; i < keyName.length ; i ++ ){
			objReault = s3Client.putObject(bucketName, keyName[i], file);
			if(i == 1 || i == 2){
				expEtagList.add(objReault.getETag());
			}
		}
		beforeCalEarliest = Calendar.getInstance();
		beforeCalEarliest.setTime(beforeDate);
		
		//设置偏差范围在5分钟之内
		beforeCalLatest = Calendar.getInstance();
		beforeCalLatest.setTime(beforeDate);
		beforeCalLatest.set(Calendar.MINUTE, beforeCalLatest.get(Calendar.MINUTE)+ 5 );
	}

	@Test
	public void testGetObjectList() throws Exception {
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName).withPrefix(prefix));
		List<S3VersionSummary> verList = versionList.getVersionSummaries();
		checklistVersionsResult(verList);
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}

	private void checklistVersionsResult(List<S3VersionSummary> versions) throws ParseException{
		Assert.assertEquals(versions.size(), expEtagList.size(),"The number of results returned does not match the expected value");
		for( int i = 0; i < expEtagList.size(); i++){
			Assert.assertEquals(versions.get(i).getKey(), keyName[i], "versions' key is wrong");
			Assert.assertEquals(versions.get(i).getVersionId(), "null", "versions' versionid is wrong");
			Assert.assertEquals(versions.get(i).getSize(), file.length(), "versions' size is wrong");
			Assert.assertEquals(versions.get(i).getETag(), expEtagList.get(i), "versions' Etag is wrong");
			
			Calendar actCal = Calendar.getInstance();
			actCal.setTime(versions.get(i).getLastModified());
			actCal.set(Calendar.HOUR, actCal.get(Calendar.HOUR)- 8 );
			System.out.println("act:" + actCal.getTime().toString());
			System.out.println("before earliest :" + beforeCalEarliest.getTime().toString());
			System.out.println("before latest" + beforeCalLatest.getTime().toString());
			if(actCal.before(beforeCalEarliest) || actCal.after(beforeCalLatest)){
				Assert.fail("versions' lastModified is wrong , "
						+ "exp actual time is in :[" + beforeCalEarliest.getTime().toString()+ ", " + beforeCalLatest.getTime().toString() + "] , "
								+ "but actual time is : " + actCal.getTime().toString());
			}
		}
	}
}
