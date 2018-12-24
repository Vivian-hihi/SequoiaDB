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
import com.amazonaws.services.s3.model.ListObjectsV2Request;
import com.amazonaws.services.s3.model.ListObjectsV2Result;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;

/**
 * test content: 带prefix、start-after、delimiter匹配查询对象元数据列表（多次查询）  
 * testlink-case: seqDB-16433
 * @author wangkexin
 * @Date 2018.11.15
 * @version 1.00
 */
public class GetObjectList16433 extends S3TestBase {
	private String bucketName = "bucket16433";
	private String keyName = "/dir/dir";
	private String prefix = "/dir/";
	private String delimiter = "/";
	private List<String> expresultList = new ArrayList<String>();
	private int objectTotalNum = 3000;
	private int objectOnceQueryNum = 1000;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket and set bucket version status
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");
		
		//put multiple objects
		for(int i = 0 ; i < objectTotalNum ; i++){
			String currentKeyName = keyName+i+"/16433";
			s3Client.putObject(bucketName, currentKeyName, "object_file16433");
			expresultList.add(currentKeyName);
		}
	}

	@Test
	public void testGetObjectList() throws Exception {//TODO:1、startAfter建议指定匹配部分对象记录，如果指定所有的话功能失效无法检测到
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName)
						.withPrefix(prefix).withDelimiter(delimiter).withStartAfter("/dir/123");
		ListObjectsV2Result result; 
		//currentTurn is query times
		int currentTurn = 0;
		
		do{
			currentTurn++;
			result = s3Client.listObjectsV2(req);
			List<String> commprefixesResult = result.getCommonPrefixes();
			checkListObjectsV2Result(commprefixesResult, currentTurn);
			
			String nextContinuationToken = result.getNextContinuationToken();
			req.setContinuationToken(nextContinuationToken);
		}while(result.isTruncated());
		
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
    //TODO:2、检测结果代码太复杂，请简化代码，另外需要比较获取的内容是否正确
	private void checkListObjectsV2Result(List<String> resultList,int currentTurn){
		int lastIndex = 0;
		Collections.sort(expresultList);
		int startKeyNum = (currentTurn - 1)* objectOnceQueryNum;
		if(currentTurn == Math.ceil((double)objectTotalNum/objectOnceQueryNum)){
			if(objectTotalNum % objectOnceQueryNum == 0){
				Assert.assertEquals(resultList.size(), objectOnceQueryNum,"The result of the last round of return is not equal to the expected result");
			}else{
				Assert.assertEquals(resultList.size(), objectTotalNum%objectOnceQueryNum,"The result of the last round of return is not equal to the expected result");
			}
		}else{
			Assert.assertEquals(resultList.size(), objectOnceQueryNum,"The result of return is not equal to the expected result");
		}
		for( int i = 0;i< resultList.size();i++){
			lastIndex = expresultList.get(startKeyNum).lastIndexOf('/');
			Assert.assertEquals(resultList.get(i),expresultList.get(startKeyNum).substring(0, lastIndex+1), "commonPrefixes is wrong");
			startKeyNum++;
		}
	}
}
