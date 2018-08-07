package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.PutMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class BucketDelimiterController {
    private static final Logger logger = LoggerFactory.getLogger(BucketController.class);

    @PutMapping(value = "/{bucketname}", params = RestParamDefine.DELIMITER)
    public String putBucketDelimiter(@PathVariable("bucketname") String bucketName)
            throws S3ServerException {
        logger.info("bucket=" + bucketName + "@delimiter");
        return bucketName + "@delimiter";
    }

    @GetMapping(value = "/{bucketname", params = RestParamDefine.DELIMITER)
    public String getBucketDelimiter(@PathVariable("bucketname") String bucketName) {
        return bucketName + "&versioning";
    }


}
