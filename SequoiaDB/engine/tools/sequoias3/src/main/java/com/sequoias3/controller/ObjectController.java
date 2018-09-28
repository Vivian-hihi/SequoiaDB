package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.*;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.service.ObjectService;
import com.sequoias3.utils.DataFormatUtils;
import com.sequoias3.utils.RestUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import javax.servlet.ServletInputStream;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import java.io.IOException;
import java.util.*;

@RestController
public class ObjectController {
    private final Logger logger = LoggerFactory.getLogger(ObjectController.class);

    @Autowired
    RestUtils restUtils;

    @Autowired
    ObjectService objectService;

    @PutMapping(value="/{bucketname:.+}/*/**", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity putObject(@PathVariable("bucketname") String bucketName,
                                    @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                    @RequestHeader(name = RestParamDefine.PutObjectHeader.CONTENT_MD5, required = false) String contentMD5,
                                    HttpServletRequest httpServletRequest)
            throws S3ServerException, IOException {
        User operator = restUtils.getOperatorByAuthorization(authorization);

        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.debug("put object. bucketname="+bucketName+", objectname="+objectName);

        Map<String, String> requestHeaders = new HashMap<>();
        Map<String, String> xMeta          = new HashMap<>();
        Enumeration names = httpServletRequest.getHeaderNames();
        while (names.hasMoreElements()){
            String name = names.nextElement().toString();
            if (name.startsWith(RestParamDefine.PutObjectHeader.X_AMZ_META_PREFIX)){
                xMeta.put(name,httpServletRequest.getHeader(name));
            }else {
                requestHeaders.put(name, httpServletRequest.getHeader(name));
            }
        }
        ServletInputStream body = httpServletRequest.getInputStream();
        PutDeleteResult result = objectService.putObject(operator.getUserId(),
                bucketName,
                objectName,
                contentMD5,
                requestHeaders,
                xMeta,
                body);

        HttpHeaders headers = new HttpHeaders();
        if (result.geteTag() != null){
            headers.add(RestParamDefine.PutObjectResultHeader.ETAG,
                    result.geteTag());
        }
        if (result.getVersionId() != null){
            headers.add(RestParamDefine.PutObjectResultHeader.VERSION_ID,
                    result.getVersionId());
        }
        return ResponseEntity.ok()
                .headers(headers)
                .build();
    }

    @GetMapping(value="/{bucketname:.+}/*/**", produces = MediaType.APPLICATION_XML_VALUE )
    public ResponseEntity getObject(@PathVariable("bucketname") String bucketName,
                                    @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                    @RequestParam(value = RestParamDefine.VERSIONID, required = false) String versionId,
                                    HttpServletRequest httpServletRequest,
                                    HttpServletResponse response)
            throws S3ServerException, IOException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.debug("get object. bucketname="+bucketName+", objectname="+objectName);

        Map<String,String> requestHeaders = new HashMap<>();
        Enumeration headerNames = httpServletRequest.getHeaderNames();
        while (headerNames.hasMoreElements()){
            String name = headerNames.nextElement().toString();
            requestHeaders.put(name,httpServletRequest.getHeader(name));
        }

        Range range = null;
        if (requestHeaders.containsKey(RestParamDefine.GetObjectReqHeader.REQ_RANGE)){
            logger.info("range:"+requestHeaders.get(RestParamDefine.GetObjectReqHeader.REQ_RANGE));
            range = restUtils.getRange(requestHeaders.get(RestParamDefine.GetObjectReqHeader.REQ_RANGE));
        }

        Map<String, String> requestParas = new HashMap<>();
        Enumeration paraNames = httpServletRequest.getParameterNames();
        while (paraNames.hasMoreElements()){
            String name = paraNames.nextElement().toString();
            requestParas.put(name, httpServletRequest.getParameter(name));
        }

        ServletOutputStream outputStream = response.getOutputStream();
        ObjectMeta result = objectService.getObject(operator.getUserId(), bucketName,
                objectName, null, requestHeaders, range, outputStream);

        HttpHeaders headers = new HttpHeaders();
        HttpStatus  httpStatus = buildHeadersForGetObject(result, requestParas, range, headers);

        return ResponseEntity.status(httpStatus)
                .headers(headers)
                .body(outputStream);
    }

    @DeleteMapping(value = "/{bucketname:.+}/*/**", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteObject(@PathVariable("bucketname") String bucketName,
                                       @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                       HttpServletRequest httpServletRequest)
            throws S3ServerException{

        User operator = restUtils.getOperatorByAuthorization(authorization);
        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.debug("delete object. bucket="+bucketName+", objectname="+objectName);

        objectService.deleteObject(operator.getUserId(), bucketName, objectName, null);
        return ResponseEntity.noContent().build();
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.ListObjectsPara.LIST_TYPE2,
            produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsV2(@PathVariable("bucketname") String bucketName,
                                        @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.PREFIX, required = false) String prefix,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.DELIMITER, required = false) String delimiter,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.STRAT_AFTER, required = false) String startAfter,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.MAX_KEYS, required = false, defaultValue = "1000") Integer maxKeys,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.CONTINUATIONTOKEN, required = false) String continueToken,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.ENCODING_TYPE, required = false) String encodingType,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.FETCH_OWNER, required = false, defaultValue = "false") Boolean fetchOwner)
            throws S3ServerException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        logger.debug("listobjects. bucket={}",bucketName);

        if (null != encodingType) {
            if (!encodingType.equals(RestParamDefine.ENCODING_TYPE_URL)) {
                throw new S3ServerException(S3Error.OBJECT_INVALID_ENCODING_TYPE, "encoding type must be url");
            }
        }

        ListObjectsResult result = objectService.listObjects(operator.getUserId(),
                bucketName, prefix, delimiter, startAfter,
                maxKeys, continueToken, encodingType, fetchOwner);

        return ResponseEntity.ok()
                .body(result);
    }

    private HttpStatus buildHeadersForGetObject(ObjectMeta result, Map<String, String> requestParas,
                                                 Range range, HttpHeaders headers){

        HttpStatus status = HttpStatus.OK;

        headers.add(RestParamDefine.GetObjectResHeader.ETAG, result.geteTag());
        headers.add(RestParamDefine.GetObjectResHeader.LAST_MODIFIED, DataFormatUtils.formateDate2(result.getLastModified()));
        if (null == range){
            headers.add(RestParamDefine.GetObjectResHeader.CONTENT_LENGTH, String.valueOf(result.getSize()));
        }else {
            headers.add(RestParamDefine.GetObjectResHeader.CONTENT_LENGTH, String.valueOf(range.getContentLength()));
            if (range.getContentLength() < result.getSize()){
                headers.add(RestParamDefine.GetObjectResHeader.CONTENT_RANGE,
                        "bytes " + range.getStart() + "-" + range.getEnd() + "/" + result.getSize());
                status = HttpStatus.PARTIAL_CONTENT;
            }
        }

        if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CACHE_CONTROL)){
            headers.add(RestParamDefine.GetObjectResHeader.CACHE_CONTROL,
                    requestParas.get(RestParamDefine.GetObjectReqPara.RES_CACHE_CONTROL).toString());
        }else {
            if (result.getCacheControl() != null){
                headers.add(RestParamDefine.GetObjectResHeader.CACHE_CONTROL, result.getCacheControl());
            }
        }

        if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_DISPOSITION)){
            headers.add(RestParamDefine.GetObjectResHeader.CONTENT_DISPOSITION,
                    requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_DISPOSITION).toString());
        }else {
            if (result.getContentDisposition() != null){
                headers.add(RestParamDefine.GetObjectResHeader.CONTENT_DISPOSITION, result.getContentDisposition());
            }
        }

        if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_ENCODING)){
            headers.add(RestParamDefine.GetObjectResHeader.CONTENT_ENCODING,
                    requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_ENCODING).toString());
        }else {
            if (result.getContentEncoding() != null){
                headers.add(RestParamDefine.GetObjectResHeader.CONTENT_ENCODING, result.getContentEncoding());
            }
        }

        if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_LANGUAGE)){
            headers.add(RestParamDefine.GetObjectResHeader.CONTENT_LANGUAGE,
                    requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_LANGUAGE).toString());
        }else {
            if (result.getContentLanguage() != null){
                headers.add(RestParamDefine.GetObjectResHeader.CONTENT_LANGUAGE, result.getContentLanguage());
            }
        }

        if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_TYPE)){
            headers.add(RestParamDefine.GetObjectResHeader.CONTENT_TYPE,
                    requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_TYPE).toString());
        }else {
            if (result.getContentType() != null){
                headers.add(RestParamDefine.GetObjectResHeader.CONTENT_TYPE, result.getContentType());
            }
        }

        if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_EXPIRES)){
            headers.add(RestParamDefine.GetObjectResHeader.EXPIRES,
                    requestParas.get(RestParamDefine.GetObjectReqPara.RES_EXPIRES).toString());
        }else {
            if (result.getExpires() != null){
                headers.add(RestParamDefine.GetObjectResHeader.EXPIRES, result.getExpires());
            }
        }

        if (null != result.getMetaList()){
            Map metaList = result.getMetaList();
            Iterator it = metaList.entrySet().iterator();
            while (it.hasNext()){
                Map.Entry entry = (Map.Entry)it.next();
                headers.add(entry.getKey().toString(), entry.getValue().toString());
            }
        }

        return status;
    }
}
