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
 * test content: 查询桶中对象版本列表 
 * testlink-case: seqDB-16386
 * @author wangkexin
 * @Date 2018.11.23
 * @version 1.00
 */
public class GetObjectVersionList16386 extends S3TestBase {
	private String bucketName = "bucket16386";
	private String keyName = "/dir/dir";
	private List<String> expresultList = new ArrayList<String>();
	private int objectTotalNum = 1500;
	private int objectOnceQueryNum = 1000;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		
		//put multiple objects
		for(int i = 0 ; i < objectTotalNum ; i++){
			String currentKeyName = keyName+i+"/16386";
			s3Client.putObject(bucketName, currentKeyName, "object_file16386");
			expresultList.add(currentKeyName);//TODO:1、建议优化变量名。
		}
	}

	@Test
	public void testGetObjectList() throws Exception {
		VersionListing versionList = s3Client.listVersions(new ListVersionsRequest().withBucketName(bucketName));
		int currentTurn = 0;//TODO:2、建议优化变量名。
		while(true){
			currentTurn++;
			List<S3VersionSummary> verList = versionList.getVersionSummaries();
			checkListObjectsResult(verList,currentTurn);
			if(versionList.isTruncated()){
				versionList = s3Client.listNextBatchOfVersions(versionList);
			}else{
				break;
			}
		}//TODO:3、需要补充所有查询结果数检查
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}

	private void checkListObjectsResult(List<S3VersionSummary> versions,int currentTurn){
		Collections.sort(expresultList);
		int startKeyNum = (currentTurn - 1)* objectOnceQueryNum;
		//TODO:4、这里的逻辑需要简化下，已经有预期结果可以直接用预期结果去判断，分支太多不利于后期代码维护
		if(currentTurn == Math.ceil((double)objectTotalNum/objectOnceQueryNum)){
			if(objectTotalNum % objectOnceQueryNum == 0){
				Assert.assertEquals(versions.size(), objectOnceQueryNum,"The result of the last round of return is not equal to the expected result");
			}else{
				Assert.assertEquals(versions.size(), objectTotalNum%objectOnceQueryNum,"The result of the last round of return is not equal to the expected result");
			}
		}else{
			Assert.assertEquals(versions.size(), objectOnceQueryNum,"The number of results returned does not match the expected value");
		}
		for( int i = 0;i< versions.size();i++){
			Assert.assertEquals(versions.get(i).getKey(),expresultList.get(startKeyNum), "commonPrefixes is wrong");
			Assert.assertEquals(versions.get(i).getVersionId(), "null", "version id is not null");
			startKeyNum++;
		}
	}
}
