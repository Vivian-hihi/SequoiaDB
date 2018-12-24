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
 * test content: 指定encoding-type查询对象元数据列表 
 * testlink-case: seqDB-16440
 * @author wangkexin
 * @Date 2018.11.22
 * @version 1.00
 */
public class GetObjectList16440 extends S3TestBase {
	private String bucketName = "bucket16440";
	private String[] keyNames = {"%6Ftest!_ST.-(test0|0.txt","%6Ftest!_、/abc*st/ab）|1.txt","%6Ftest!_@#$%~^@><|2.txt"};
	private String prefix = "%6Ftest!_";
	private String delimiter = "|";
	private List<String> expresultList = new ArrayList<String>(3000);//TODO:1、建议修改下变量名，符合实际意义，另外没有必要生气
	private AmazonS3 s3Client = null;
	private boolean runSuccess = false;

	@BeforeClass
	private void setUp() throws Exception {
		s3Client = CommLib.buildS3Client();
		//create bucket
		s3Client.createBucket(new CreateBucketRequest(bucketName));
		
		//put multiple objects
		for(int i = 0 ; i < keyNames.length ; i++){
			String expCommPrefixes =keyNames[i].substring(0, keyNames[i].indexOf(delimiter)+1);
			s3Client.putObject(bucketName, keyNames[i], "object_file16440");
			expresultList.add(expCommPrefixes);
		}
	}
	//TODO:1、没有覆盖包含contents的记录信息，请参考文本用例中列出的测试点。
	@Test
	public void testGetObject() throws Exception {
		ListObjectsV2Request req = new ListObjectsV2Request().withBucketName(bucketName).withEncodingType("url")
						.withPrefix(prefix).withDelimiter(delimiter);
		ListObjectsV2Result result = s3Client.listObjectsV2(req);
		Assert.assertEquals(result.getEncodingType(), "url");
		List<String> commprefixesResult = result.getCommonPrefixes();
		checkListObjectsV2Result(commprefixesResult);
		
		runSuccess =true;
	}

	@AfterClass
	private void tearDown() {
		if (runSuccess) {
			CommLib.deleteAllObjectVersions(s3Client, bucketName);
			s3Client.deleteBucket(bucketName);
		}
	}
	//TODO:2、可提取公共方法使用。
	private void checkListObjectsV2Result(List<String> resultList){
		Collections.sort(expresultList);
		Assert.assertEquals(resultList.size(), expresultList.size(), "The expected results do not match the actual number of returns");
		for( int i = 0;i< resultList.size();i++){
			Assert.assertEquals(resultList.get(i),expresultList.get(i), "commonPrefixes is wrong");
		}
	}
}
