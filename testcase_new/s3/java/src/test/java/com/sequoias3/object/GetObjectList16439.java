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
 * test content: 多次查询结果在commprefix中有相同记录
 * testlink-case: seqDB-16439
 * @author wangkexin
 * @Date 2018.11.22
 * @version 1.00
 */
public class GetObjectList16439 extends S3TestBase {
	private String bucketName = "bucket16439";
	private String keyName = "/dir";
	private String prefix = "/";
	private String delimiter = "/";
	private int maxKeys = 2;
	private List<String> expresultList = new ArrayList<String>();
	private int samePrefixObjNum = 4;
	private int sameDirNum = 3;
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		String currentKeyName = null;
		String expCurrentKeyName = null;
		//put multiple objects
		for(int i = 0 ; i < samePrefixObjNum ; i++){
			for(int j = 0 ; j < sameDirNum ; j++){
				currentKeyName = keyName+i+"/subdir"+j+"/16439";
				s3Client.putObject(bucketName, currentKeyName, "object_file16439");
			}
			expCurrentKeyName = keyName + i +"/";
			expresultList.add(expCurrentKeyName);
		}
	}

	@Test
	public void testGetObjectList() throws Exception {
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName)
						.withPrefix(prefix).withDelimiter(delimiter).withMaxKeys(maxKeys);
		ListObjectsV2Result result; 
		//currentTurn is query times
		int currentTurn = 0;
		
		do{
			currentTurn++;
			result = s3Client.listObjectsV2(req);
			List<String> commprefixesResult = result.getCommonPrefixes();
			if(currentTurn == Math.ceil((double)samePrefixObjNum/maxKeys)){
				if(samePrefixObjNum%maxKeys==0){
					Assert.assertEquals(result.getKeyCount(), maxKeys, "The expected results do not match the actual number of returns");
				}else{
					Assert.assertEquals(result.getKeyCount(), samePrefixObjNum%maxKeys, "The expected results do not match the actual number of returns");
				}
				//SEQUOIADBMAINSTREAM-3987
				Assert.assertFalse(result.isTruncated(), "last turn result.isTruncated should be false!");
			}else{
				Assert.assertEquals(result.getKeyCount(), maxKeys, "The expected results do not match the actual number of returns");
				Assert.assertTrue(result.isTruncated(), "when it is not last turn result.isTruncated should be true!");
			}
			checkListObjectsV2Result(commprefixesResult, (currentTurn-1)*maxKeys);
			
			String NextContinuationToken = result.getNextContinuationToken();
			req.setContinuationToken(NextContinuationToken);
			
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

	private void checkListObjectsV2Result(List<String> resultList,int startKeyNum){
		Collections.sort(expresultList);
		for( int i = 0;i< resultList.size();i++){
			Assert.assertEquals(resultList.get(i),expresultList.get(startKeyNum), "commonPrefixes is wrong");
			startKeyNum++;
		}
	}
}
