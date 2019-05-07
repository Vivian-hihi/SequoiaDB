package com.sequoias3.delimiter;

import java.io.File;
import java.io.IOException;

import org.testng.annotations.AfterClass;
import org.testng.annotations.BeforeClass;
import org.testng.annotations.Test;

import com.amazonaws.services.s3.AmazonS3;
import com.sequoias3.testcommon.CommLib;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;
import com.sequoias3.testcommon.s3utils.DelimiterUtils;

/**
 * @Description seqDB-18076: the object name include old and new delimiter, than
 *              update the old delimiter to the new delimiter.
 * @author wuyan
 * @Date 2019.04.09
 * @version 1.00
 */
public class UpdateDelimiter18076 extends S3TestBase {
	private boolean runSuccess = false;
	private String bucketName = "bucket18076";
	private String keyName = "aa%test/maa%/bb%/object18076";
	private String deleteTagkeyName = "aa%delete/aa%/test%/object18076";
	private String delimiter = "%";
	private AmazonS3 s3Client = null;
	private int fileSize = 1024 * 30;
	private File localPath = null;
	private String filePath = null;

	@BeforeClass
	private void setUp() throws IOException {
		localPath = new File(S3TestBase.workDir + File.separator + TestTools.getClassName());
		filePath = localPath + File.separator + "localFile_" + fileSize + ".txt";
		TestTools.LocalFile.removeFile(localPath);
		TestTools.LocalFile.createDir(localPath.toString());
		TestTools.LocalFile.createFile(filePath, fileSize);

		s3Client = CommLib.buildS3Client();
		CommLib.clearBucket(s3Client, bucketName);
		s3Client.createBucket(bucketName);
		CommLib.setBucketVersioning(s3Client, bucketName, "Enabled");

		// add a deleteTag key
		s3Client.deleteObject(bucketName, deleteTagkeyName);
		s3Client.putObject(bucketName, keyName, new File(filePath));
	}

	@Test
	public void testUpdateDelimiter() throws Exception {
		DelimiterUtils.putBucketDelimiter(bucketName, delimiter);
		DelimiterUtils.checkCurrentDelimiteInfo(bucketName, delimiter);
		runSuccess = true;
	}

	@AfterClass
	private void tearDown() {
		try {
			if (runSuccess) {
				CommLib.clearBucket(s3Client, bucketName);
				TestTools.LocalFile.removeFile(localPath);
			}
		} finally {
			s3Client.shutdown();
		}
	}

}
