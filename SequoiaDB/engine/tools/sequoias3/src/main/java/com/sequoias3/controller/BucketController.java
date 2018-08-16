package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.User;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.BucketService;
import com.sequoias3.utils.RestUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import javax.servlet.http.HttpServletRequest;


@RestController
public class BucketController {
    private static final Logger logger = LoggerFactory.getLogger(BucketController.class);

    @Autowired
    RestUtils restUtils;

    @Autowired
    BucketService bucketService;

    @PutMapping(value = "/{bucketname:.+}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity putBucket(@PathVariable("bucketname") String bucketName,
                                    @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                    HttpServletRequest httpServletRequest)
            throws S3ServerException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        logger.info("URI = "+httpServletRequest.getRequestURI()+"  bucketname = "+bucketName);
        logger.info("Create bucket bucketName = " + bucketName+"  operator="+operator.getUserName());
        bucketService.createBucket(operator.getUserId(),bucketName);
        return ResponseEntity.ok()
                .header(RestParamDefine.LOCATION, RestParamDefine.REST_DELIMITER+bucketName)
                .build();
    }

    @GetMapping(produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listBuckets( @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization)
            throws S3ServerException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        logger.info("get bucket:owner="+operator.getUserName());
        return ResponseEntity.ok()
                .body(bucketService.getService(operator));
    }

    @DeleteMapping(value = "/{bucketname:.+}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteBucket(@PathVariable("bucketname") String bucketName,
                               @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization)
            throws S3ServerException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        logger.info("delete bucket:bucket=" + bucketName+"operator="+operator.getUserName());
        bucketService.deleteBucket(operator.getUserId(), bucketName);
        return ResponseEntity.noContent().build();
    }
}
