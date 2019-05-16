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
 * @Description seqDB-18079: the object name include old and new delimiter, than
 *              update the old delimiter to the new delimiter.
 * @author wuyan
 * @Date 2019.04.09
 * @version 1.00
 */
// TODO :@Description中描述为对象名包含新旧分隔符，而文本用例中标题为对象名中包含旧分隔符，两边描述不一致
public class UpdateDelimiter18079 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18079";
	private String keyName = "/test/aa/object18079";
	private int keyNums = 10;
	private String delimiter = "%";
	private List<String> expCommprefixList = new ArrayList<>();
	private AmazonS3 s3Client = null;

	@BeforeClass
	private void setUp() throws IOException {
		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");

		for (int i = 0; i < keyNums; i++) {
			// TODO :文本用例中标题以及步骤中指明上传对象名包含旧分隔符而不是新旧分隔符，请确认下文本用例是否需要修改
			String subKeyName = keyName + "_" + i + "_%test.png";
			s3Client.putObject(bucketName, subKeyName, "context18079" + i);
			String matchDir = keyName + "_" + i + "_%";
			expCommprefixList.add(matchDir);
		}
	}

	@Test
	public void testUpdateDelimiter() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);

		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);

		List<String> expContentList = new ArrayList<>();
		DelimiterUtils.listObjectsWithDelimiter(s3Client, bucketName, delimiter, expCommprefixList, expContentList);
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
