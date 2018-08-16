package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.exception.S3ServerException;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import javax.servlet.http.HttpServletRequest;

@RestController
public class ObjectController {
    private final Logger logger = LoggerFactory.getLogger(ObjectController.class);

    @PutMapping(value="/{bucketname:.+}/*/**", produces = MediaType.APPLICATION_XML_VALUE)
    public String putObject(@PathVariable("bucketname") String bucketName,
                            HttpServletRequest httpServletRequest) throws S3ServerException {
        logger.info("create object:object=" + httpServletRequest.getRequestURI());
        return httpServletRequest.getRequestURI();
    }

    @GetMapping(value="/{bucketname:.+}/*/**", produces = MediaType.APPLICATION_XML_VALUE)
    public String getObject(@PathVariable("bucketname") String bucketName,
                                       HttpServletRequest httpServletRequest) {
        logger.info("get object:"+httpServletRequest.getRequestURL());
        return "get object";
    }

    @GetMapping(value = "/{bucketname:.+}/*/**", params = RestParamDefine.VERSIONID)
    public String getObjectByVersionId(@PathVariable("bucketname") String bucketName) {
        logger.info("get object by versionId");
        return "get object by versionId";
    }

    @GetMapping(value="/{bucketname:.+}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsVersion1(@PathVariable("bucketname") String bucketName,
                            HttpServletRequest httpServletRequest) {
        logger.info("listObjectsVersion1"+httpServletRequest.getRequestURL());
        return ResponseEntity.ok().build();
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.LIST_TYPE2, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsVersion2(@PathVariable("bucketname") String bucketName,
                                              @RequestParam(value = RestParamDefine.PREFIX, required = false) String prefix,
                                              @RequestParam(value = RestParamDefine.DELIMITER, required = false) String delimiter,
                                              @RequestParam(value = RestParamDefine.STRAT_AFTER, required = false) String startAfter,
                                              @RequestParam(value = RestParamDefine.MAX_KEYS, required = false) String maxKeys) {
        logger.info("listobjects. bucket="+bucketName);
        return ResponseEntity.ok().build();
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.VERSIONS, produces = MediaType.APPLICATION_XML_VALUE)
    public String listObjectsVersions(@PathVariable("bucketname") String bucketName,
                                      @RequestParam(RestParamDefine.PREFIX) String prefix,
                                      @RequestParam(RestParamDefine.DELIMITER) String delimiter,
                                      @RequestParam(RestParamDefine.STRAT_AFTER) String startAfter,
                                      @RequestParam(RestParamDefine.MAX_KEYS) String maxKeys) {
        logger.info("listobjectversions. bucket="+bucketName);
        return bucketName + "&versions";
    }

    @DeleteMapping(value = "/{bucketname:.+}/*/**", produces = MediaType.APPLICATION_XML_VALUE)
    public String deleteObject(@PathVariable("bucketname") String bucketName) {
        logger.info("delete object. bucket="+bucketName);
        return "delete object";
    }

    @DeleteMapping(value = "/{bucketname:.+}/*/**", params = RestParamDefine.VERSIONID, produces = MediaType.APPLICATION_XML_VALUE)
    public String deletObjectByVersionId(@PathVariable("bucketname") String bucketName) {
        logger.info("delete object by versionId");
        return "delete object by versionId";
    }
}
