package com.sequoias3.object;

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

/**
 * test content: 禁用版本控制存在删除标记的对象，查询对象版本列表 
 * testlink-case: seqDB-16389
 * @author wangkexin
 * @Date 2018.11.23
 * @version 1.00
 */
public class GetObjectVersionList16389 extends S3TestBase {
	private String bucketName = "bucket16389";
	private int objectTotalNum = 5;
	private String[] keyName = new String[objectTotalNum];
	private List<String> expDeleteMarKersList = new ArrayList<String>();
	private List<String> expVersionsList = new ArrayList<String>();
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		
		for(int i = 0 ; i < objectTotalNum ; i ++ ){
			keyName[i] = "/dir"+i+"/16389";
		}
		//put multiple objects
		for(int i = 0 ; i < objectTotalNum ; i++){
			s3Client.putObject(bucketName, keyName[i], "object_file16389");
			expVersionsList.add(keyName[i]);
		}
		//delete object key = "/dir1/16389" and "/dir4/16389"
		s3Client.deleteObject(bucketName, keyName[1]);
		expDeleteMarKersList.add(keyName[1]);
		
		s3Client.deleteObject(bucketName, keyName[4]);
		expDeleteMarKersList.add(keyName[4]);
		
		//set bucket version status is Suspended
		CommLib.setBucketVersioning(s3Client, bucketName, "Suspended");
		//delete object key = "/dir0/16389"
		s3Client.deleteObject(bucketName, keyName[0]);
		expDeleteMarKersList.add(keyName[0]);
	}

	@Test
	public void testGetObjectList() throws Exception {
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName));
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

	private void checklistVersionsResult(List<S3VersionSummary> versions){
		Collections.sort(expVersionsList);
		Collections.sort(expDeleteMarKersList);
		Assert.assertEquals(versions.size(), expVersionsList.size()+ expDeleteMarKersList.size(),"The number of results returned does not match the expected value");
		for( int i = 0; i < expVersionsList.size(); i++){
			Assert.assertEquals(versions.get(i).getKey(), expVersionsList.get(i), "versions is wrong");
			Assert.assertEquals(versions.get(i).isDeleteMarker(),false, "isdeleteMarKer is wrong");
		}
		for(int i = 0 ; i < expDeleteMarKersList.size() ; i++){
			Assert.assertEquals(versions.get(expVersionsList.size() + i).getKey(), expDeleteMarKersList.get(i), "deleteMarKerList key is wrong");
			Assert.assertEquals(versions.get(expVersionsList.size() + i).isDeleteMarker(),true, "isdeleteMarKer is wrong");
			if(versions.get(expVersionsList.size() + i).getKey().equals(keyName[0])){
				Assert.assertEquals(versions.get(expVersionsList.size() + i).getVersionId(),"null", "isdeleteMarKer is wrong");
			}else{
				Assert.assertNotEquals(versions.get(expVersionsList.size() + i).getVersionId(),"null", "isdeleteMarKer is wrong");
			}
		}
	}
}
