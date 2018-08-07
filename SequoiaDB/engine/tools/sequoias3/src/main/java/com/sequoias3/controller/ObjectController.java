package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.web.bind.annotation.*;

import javax.servlet.http.HttpServletRequest;

@RestController
public class ObjectController {
    private final Logger logger = LoggerFactory.getLogger(ObjectController.class);

    @PutMapping("/{bucketname}/**")
    public String putObject(@PathVariable("bucketname") String bucketName,
                            HttpServletRequest httpServletRequest)throws S3ServerException {
        logger.info("object=" + httpServletRequest.getRequestURI());
        logger.info("bucket=" + bucketName + "object");
        return httpServletRequest.getRequestURI();
    }

    @GetMapping("/{bucketname}/**")
    public String getObjectByVersionId(@PathVariable("bucketname") String bucketName){
        logger.info("get object");
        return "get object";
    }

    @GetMapping(value="/{bucketname}/**", params = RestParamDefine.VERSIONID)
    public String getObject(@PathVariable("bucketname") String bucketName){
        logger.info("get object by versionId");
        return "get object by versionId";
    }

    @DeleteMapping("/{bucketname}/**")
    public String deleteObject(@PathVariable("bucketname") String bucketName){
        logger.info("delete object");
        return "delete object";
    }

    @DeleteMapping(value = "/{bucketname}/**", params = RestParamDefine.VERSIONID)
    public String deletObjectByVersionId(@PathVariable("bucketname") String bucketName){
        logger.info("delete object by versionId");
        return "delete object by versionId";
    }
}
