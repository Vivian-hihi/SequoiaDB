package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.User;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.utils.RestUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.web.bind.annotation.*;

import javax.servlet.http.HttpServletRequest;
import java.util.Enumeration;

@RestController
public class BucketController {
    private static final Logger logger = LoggerFactory.getLogger(BucketController.class);

    @Autowired
    RestUtils restUtils;

    @PutMapping(value = "/{bucketname}")
    public String putBucket(@PathVariable("bucketname") String bucketName,
                            @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization)
            throws S3ServerException {
        User adminUser = restUtils.getOperatorByAuthorization(authorization);

        logger.info("Create bucket bucketName = " + bucketName);
        return bucketName;
    }

    @GetMapping()
    public String listBuckets() {
        return "Get Service";
    }


    @DeleteMapping(value = "/{bucketname}")
    public String deleteBucket(@PathVariable("bucketname") String bucketName)
            throws S3ServerException {
        logger.info("bucket=" + bucketName);
        return bucketName;
    }

    @DeleteMapping(value = "/{bucketname}", params = "force")
    public String deleteBucketForce(@PathVariable("bucketname") String bucketName)
            throws S3ServerException {
        logger.info("bucket=" + bucketName + "&force");
        return bucketName + "&force";
    }
}
