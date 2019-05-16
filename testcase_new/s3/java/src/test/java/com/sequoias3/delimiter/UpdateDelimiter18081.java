package com.sequoias3.delimiter;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;

/**
 * @Description seqDB-18081: current delimiter is delimiter2, delimiter1 is
 *              exists, than update the delimiter2 to the new delimiter.
 * @author wuyan
 * @Date 2019.04.10
 * @version 1.00
 */
public class UpdateDelimiter18081 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18081";
	private String keyName = "aa?%bb?cc?%test1_18081.png";
	private String newDelimiter = "c";
	private String curDelimiter = "cc";
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws IOException {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);

		s3Client.putObject(bucketName, keyName, "updatedelimite18081");
	}

	@Test
	public void testUpdateDelimiter() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, curDelimiter);
		// TODO
		// :1.这里建议增加checkCurrentDelimiteInfo方法验证当前分隔符状态为Normal,是本场景验证的前提，可以与
		// "当前分隔符状态不为Normal，分隔符重建任务正在进行中" 的场景做区分

		// TODO :2.建议这里单独写一下更新分隔符的方法，验证更新失败，而不要用公共方法，不然无法验证更新是否失败
		DelimiterUtils.updateDelimiterSuccessAgain(bucketName, newDelimiter);
		// TODO :3.上面验证失败之后需要再次更新分隔符，这里建议使用公共方法
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, newDelimiter);

		List<String> expCommprefixList = new ArrayList<>();
		expCommprefixList.add("aa?%bb?c");
		List<String> expContentList = new ArrayList<>();
		DelimiterUtils.listObjectsWithDelimiter(s3Client, bucketName, newDelimiter, expCommprefixList, expContentList);

		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
			}
		} finally {
			s3Client.shutdown();
		}
	}
}
