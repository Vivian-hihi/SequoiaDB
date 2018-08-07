package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.bind.annotation.*;

import com.sequoias3.exception.S3ServerException;

@RestController
public class BucketController {
    private static final Logger logger = LoggerFactory.getLogger(BucketController.class);

    @PutMapping(value="/{bucketname}")
    public String putBucket(@PathVariable("bucketname") String bucketName)
            throws S3ServerException {
        logger.info("bucket=" + bucketName);
        return bucketName;
    }

    @GetMapping()
    public String listBuckets(){
        return "Get Service";
    }

    @GetMapping(value = "/{bucketname", params = RestParamDefine.LIST_TYPE2)
    public String listObjects(@PathVariable("bucketname") String bucketName,
                              @RequestParam(RestParamDefine.PREFIX) String prefix,
                              @RequestParam(RestParamDefine.DELIMITER) String delimiter,
                              @RequestParam(RestParamDefine.STRAT_AFTER) String startAfter,
                              @RequestParam(RestParamDefine.MAX_KEYS) String maxKeys){
        return bucketName + "list-type=2";
    }

    @GetMapping(value = "/{bucketname", params = RestParamDefine.VERSIONS)
    public String listObjectsVersions(@PathVariable("bucketname") String bucketName,
                                      @RequestParam(RestParamDefine.PREFIX) String prefix,
                                      @RequestParam(RestParamDefine.DELIMITER) String delimiter,
                                      @RequestParam(RestParamDefine.STRAT_AFTER) String startAfter,
                                      @RequestParam(RestParamDefine.MAX_KEYS) String maxKeys){
        return bucketName + "&versions";
    }

    @DeleteMapping(value="/{bucketname}")
    public String deleteBucket(@PathVariable("bucketname") String bucketName)
            throws S3ServerException {
        logger.info("bucket=" + bucketName);
        return bucketName;
    }

    @DeleteMapping(value="/{bucketname}", params = "force")
    public String deleteBucketForce(@PathVariable("bucketname") String bucketName)
            throws S3ServerException {
        logger.info("bucket=" + bucketName + "&force");
        return bucketName + "&force";
    }
}
