package com.sequoias3.object;

import java.io.File;
import java.io.FileOutputStream;

import org.json.JSONObject;
import org.json.XML;
import org.springframework.http.HttpMethod;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.HttpClientErrorException;

import com.amazonaws.services.s3.AmazonS3;
import com.amazonaws.services.s3.model.S3Object;
import com.amazonaws.services.s3.model.S3ObjectInputStream;
import com.sequoias3.testcommon.S3TestBase;
import com.sequoias3.testcommon.TestTools;

public class ObjectUtils extends S3TestBase {
    

    public static String downloadAnObject(AmazonS3 s3Client, File localPath,String bucketName, String keyName) throws Exception {
    	S3Object object = s3Client.getObject(bucketName, keyName);
		S3ObjectInputStream s3is = object.getObjectContent();		
		String downloadPath = TestTools.LocalFile.initDownloadPath(localPath, TestTools.getMethodName(),
				Thread.currentThread().getId());
        FileOutputStream fos = new FileOutputStream(new File(downloadPath), true);
        byte[] read_buf = new byte[1024];
        int read_len = 0;
        while ((read_len = s3is.read(read_buf)) > -1) {
            fos.write(read_buf, 0, read_len);
        }
        s3is.close();
        fos.close();
        
        return downloadPath;
    }   
}
