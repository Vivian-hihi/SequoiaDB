package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.MediaType;
import org.springframework.web.bind.annotation.*;

@RestController
public class BucketVersioningController {
    private static final Logger logger = LoggerFactory.getLogger(BucketController.class);

    @PutMapping(value = "/{bucketname:.+}", params = RestParamDefine.VERSIONING)
    public String putBucketVerisoning(@PathVariable("bucketname") String bucketName)
            throws S3ServerException {
        logger.info("bucket=" + bucketName + "@versioning");
        return bucketName + "@versioning";
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.VERSIONING)
    public String getBucketVersioning(@PathVariable("bucketname") String bucketName) {
        return bucketName + "&versioning";
    }

    @GetMapping(value = "/{bucketname:.+}/*/**", params = RestParamDefine.VERSIONID)
    public String getObjectByVersionId(@PathVariable("bucketname") String bucketName) {
        logger.info("get object by versionId");
        return "get object by versionId";
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.VERSIONS, produces = MediaType.APPLICATION_XML_VALUE)
    public String listObjectsVersions(@PathVariable("bucketname") String bucketName,
                                      @RequestParam(RestParamDefine.ListObjectVersionsPara.PREFIX) String prefix,
                                      @RequestParam(RestParamDefine.ListObjectVersionsPara.DELIMITER) String delimiter,
                                      @RequestParam(RestParamDefine.ListObjectVersionsPara.STRAT_AFTER) String startAfter,
                                      @RequestParam(RestParamDefine.ListObjectVersionsPara.MAX_KEYS) String maxKeys) {
        logger.info("listobjectversions. bucket="+bucketName);
        return bucketName + "&versions";
    }

    @DeleteMapping(value = "/{bucketname:.+}/*/**", params = RestParamDefine.VERSIONID, produces = MediaType.APPLICATION_XML_VALUE)
    public String deletObjectByVersionId(@PathVariable("bucketname") String bucketName) {
        logger.info("delete object by versionId");
        return "delete object by versionId";
    }
}
