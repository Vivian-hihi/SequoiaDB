package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.*;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.model.GetResult;
import com.sequoias3.model.ListObjectsResult;
import com.sequoias3.model.ListVersionsResult;
import com.sequoias3.model.PutDeleteResult;
import com.sequoias3.service.ObjectService;
import com.sequoias3.utils.RestUtils;
import org.apache.commons.codec.binary.Hex;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import sun.misc.BASE64Decoder;

import javax.servlet.ServletInputStream;
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
        logger.debug("put object. bucket name={}, object name={}", bucketName, objectName);

        String validMD5 = null;
        if (contentMD5 != null){
            validMD5 = getMD5(contentMD5);
        }
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
                validMD5,
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
    public void getObject(@PathVariable("bucketname") String bucketName,
                                    @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                    @RequestParam(value = RestParamDefine.VERSION_ID, required = false) String versionId,
                                    HttpServletRequest httpServletRequest,
                                    HttpServletResponse response)
            throws S3ServerException, IOException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.debug("get object. bucketname={}, objectname={}", bucketName, objectName);

        Map<String,String> requestHeaders = new HashMap<>();
        Enumeration headerNames = httpServletRequest.getHeaderNames();
        while (headerNames.hasMoreElements()){
            String name = headerNames.nextElement().toString();
            requestHeaders.put(name,httpServletRequest.getHeader(name));
        }

        Range range = null;
        if (requestHeaders.containsKey(RestParamDefine.GetObjectReqHeader.REQ_RANGE)){
            range = restUtils.getRange(requestHeaders.get(RestParamDefine.GetObjectReqHeader.REQ_RANGE));
        }

        Map<String, String> requestParas = new HashMap<>();
        Enumeration paraNames = httpServletRequest.getParameterNames();
        while (paraNames.hasMoreElements()){
            String name = paraNames.nextElement().toString();
            requestParas.put(name, httpServletRequest.getParameter(name));
        }

        Boolean nullVersionFlag = null;
        Long cvtVersionId = null;
        if (versionId != null) {
            cvtVersionId = convertVersionId(versionId);
            if (null == cvtVersionId){
                nullVersionFlag = true;
            }
        }

        GetResult result = objectService.getObject(operator.getUserId(), bucketName,
                objectName, cvtVersionId, nullVersionFlag, requestHeaders, range);

        try {
            if (result.getMeta().getDeleteMarker()) {
                buildDeleteMarkerResponseHeader(result.getMeta(), response);
                if (null == versionId) {
                    throw new S3ServerException(S3Error.OBJECT_NO_SUCH_KEY, "no object. object:" + objectName);
                } else {
                    throw new S3ServerException(S3Error.METHOD_NOT_ALLOWED, "no object. object:" + objectName);
                }
            } else {
                buildHeadersForGetObject(result.getMeta(), requestParas, range, response);
                objectService.readObjectData(result.getData(), response.getOutputStream(), range);
            }
        }finally {
            objectService.releaseGetResult(result);
        }
    }

    @DeleteMapping(value = "/{bucketname:.+}/*/**", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteObject(@PathVariable("bucketname") String bucketName,
                                       @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                       HttpServletRequest httpServletRequest)
            throws S3ServerException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.debug("delete object. bucket={}, object={}", bucketName, objectName);

        PutDeleteResult result = objectService.deleteObject(operator.getUserId(), bucketName, objectName);
        HttpHeaders headers = new HttpHeaders();
        if (result != null) {
            if (result.getDeleteMarker() != null) {
                headers.add(RestParamDefine.DeleteObjectResultHeader.DELETE_MARKER,
                        result.getDeleteMarker().toString());
            }
            if (result.getVersionId() != null) {
                headers.add(RestParamDefine.DeleteObjectResultHeader.VERSION_ID,
                        result.getVersionId());
            }
        }
        return ResponseEntity.noContent()
                .headers(headers)
                .build();
    }

    @DeleteMapping(value = "/{bucketname:.+}/*/**", params = RestParamDefine.VERSION_ID, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteObjectByVersionId(@PathVariable("bucketname") String bucketName,
                                                  @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                                  @RequestParam(RestParamDefine.VERSION_ID) String versionId,
                                                  HttpServletRequest httpServletRequest)
            throws S3ServerException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.debug("delete object with version id. bucket={}, objectname={}, versionId={}",
                bucketName, objectName, versionId);

        Boolean nullVersionFlag = null;
        Long cvtVersionId = convertVersionId(versionId);
        if (null == cvtVersionId){
            nullVersionFlag = true;
        }

        objectService.deleteObject(operator.getUserId(), bucketName, objectName, cvtVersionId, nullVersionFlag);
        HttpHeaders headers = new HttpHeaders();
        headers.add(RestParamDefine.DeleteObjectResultHeader.VERSION_ID, versionId);
        return ResponseEntity.noContent()
                .headers(headers)
                .build();
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.ListObjectsPara.LIST_TYPE2,
            produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsV2(@PathVariable("bucketname") String bucketName,
                                        @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.PREFIX, required = false) String prefix,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.DELIMITER, required = false) String delimiter,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.START_AFTER, required = false) String startAfter,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.MAX_KEYS, required = false, defaultValue = "1000") Integer maxKeys,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.CONTINUATIONTOKEN, required = false) String continueToken,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.ENCODING_TYPE, required = false) String encodingType,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.FETCH_OWNER, required = false, defaultValue = "false") Boolean fetchOwner)
            throws S3ServerException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        logger.debug("list objectsV2. bucket={}",bucketName);

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

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.VERSIONS, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsVersions(@PathVariable("bucketname") String bucketName,
                                              @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.PREFIX, required = false) String prefix,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.DELIMITER, required = false) String delimiter,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.KEY_MARKER, required = false) String keyMarker,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.VERSION_ID_MARKER, required = false) String versionIdMarker,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.ENCODING_TYPE, required = false) String encodingType,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.MAX_KEYS, required = false, defaultValue = "1000") Integer maxKeys)
            throws S3ServerException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        logger.debug("list versions. bucket={}",bucketName);

        if (null != encodingType) {
            if (!encodingType.equals(RestParamDefine.ENCODING_TYPE_URL)) {
                throw new S3ServerException(S3Error.OBJECT_INVALID_ENCODING_TYPE, "encoding type must be url");
            }
        }

        ListVersionsResult result = objectService.listVersions(operator.getUserId(),
                bucketName, prefix, delimiter, keyMarker, versionIdMarker, maxKeys, encodingType);

        return ResponseEntity.ok()
                .body(result);
    }

    @RequestMapping(method = RequestMethod.HEAD, value="/{bucketname:.+}/*/**")
    public ResponseEntity headObject(@PathVariable("bucketname") String bucketName,
                                     @RequestHeader(RestParamDefine.AUTHORIZATION) String authorization,
                                     @RequestParam(value = RestParamDefine.VERSION_ID, required = false) String versionId,
                                     HttpServletRequest httpServletRequest,
                                     HttpServletResponse response)
            throws S3ServerException{
        User operator = restUtils.getOperatorByAuthorization(authorization);
        String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
        logger.debug("get object. bucketname={}, objectname={}", bucketName, objectName);

        Map<String,String> requestHeaders = new HashMap<>();
        Enumeration headerNames = httpServletRequest.getHeaderNames();
        while (headerNames.hasMoreElements()){
            String name = headerNames.nextElement().toString();
            requestHeaders.put(name,httpServletRequest.getHeader(name));
        }

        Range range = null;
        if (requestHeaders.containsKey(RestParamDefine.GetObjectReqHeader.REQ_RANGE)){
            range = restUtils.getRange(requestHeaders.get(RestParamDefine.GetObjectReqHeader.REQ_RANGE));
        }

        Boolean nullVersionFlag = null;
        Long cvtVersionId = null;
        if (versionId != null) {
            cvtVersionId = convertVersionId(versionId);
            if (null == cvtVersionId){
                nullVersionFlag = true;
            }
        }

        GetResult result = objectService.getObject(operator.getUserId(), bucketName,
                objectName, cvtVersionId, nullVersionFlag, requestHeaders, range);

        try {
            if (result.getMeta().getDeleteMarker()) {
                throw new S3ServerException(S3Error.OBJECT_NO_SUCH_KEY, "no object. object:" + objectName);
            } else {
                buildHeadersForGetObject(result.getMeta(), null, range, response);
            }
        }finally {
            objectService.releaseGetResult(result);
        }

        return ResponseEntity.ok().build();
    }

    private Long convertVersionId(String versionId)
            throws S3ServerException{
        try {
            if (versionId.equals(ObjectMeta.NULL_VERSION_ID)) {
                return null;
            } else {
                return Long.parseLong(versionId);
            }
        }catch (NumberFormatException e) {
            throw new S3ServerException(S3Error.INVALID_ARGUMENT,
                    "version id is invalid。 version id=" + versionId);
        }catch (Exception e){
            throw new S3ServerException(S3Error.INVALID_ARGUMENT, "versionId is invalid. versionId="+versionId);
        }
    }

    private void buildDeleteMarkerResponseHeader(ObjectMeta objectMeta, HttpServletResponse response) {
        response.addHeader(RestParamDefine.GetObjectResHeader.DELETE_MARKER, objectMeta.getDeleteMarker().toString());
        if (objectMeta.getNoVersionFlag()){
            response.addHeader(RestParamDefine.GetObjectResHeader.VERSION_ID, ObjectMeta.NULL_VERSION_ID);
        }else {
            response.addHeader(RestParamDefine.GetObjectResHeader.VERSION_ID, String.valueOf(objectMeta.getVersionId()));
        }
    }

    private void buildHeadersForGetObject(ObjectMeta objectMeta, Map<String, String> requestParas,
                                          Range range, HttpServletResponse response){
        response.addHeader(RestParamDefine.GetObjectResHeader.ETAG, objectMeta.geteTag());
        response.addDateHeader(RestParamDefine.GetObjectResHeader.LAST_MODIFIED, objectMeta.getLastModified());
        if (objectMeta.getNoVersionFlag()) {
            response.addHeader(RestParamDefine.GetObjectResHeader.VERSION_ID, "null");
        }else {
            response.addHeader(RestParamDefine.GetObjectResHeader.VERSION_ID, String.valueOf(objectMeta.getVersionId()));
        }
        response.addHeader(RestParamDefine.GetObjectResHeader.ACCEPT_RANGES, "bytes");

        if (null != objectMeta.getMetaList()){
            Map metaList = objectMeta.getMetaList();
            Iterator it = metaList.entrySet().iterator();
            while (it.hasNext()){
                Map.Entry entry = (Map.Entry)it.next();
                response.addHeader(entry.getKey().toString(), entry.getValue().toString());
            }
        }

        if (requestParas != null) {
            if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CACHE_CONTROL)) {
                response.addHeader(RestParamDefine.GetObjectResHeader.CACHE_CONTROL,
                        requestParas.get(RestParamDefine.GetObjectReqPara.RES_CACHE_CONTROL));
            } else {
                if (objectMeta.getCacheControl() != null) {
                    response.addHeader(RestParamDefine.GetObjectResHeader.CACHE_CONTROL, objectMeta.getCacheControl());
                }
            }

            if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_DISPOSITION)) {
                response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_DISPOSITION,
                        requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_DISPOSITION));
            } else {
                if (objectMeta.getContentDisposition() != null) {
                    response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_DISPOSITION, objectMeta.getContentDisposition());
                }
            }

            if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_ENCODING)) {
                response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_ENCODING,
                        requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_ENCODING));
            } else {
                if (objectMeta.getContentEncoding() != null) {
                    response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_ENCODING, objectMeta.getContentEncoding());
                }
            }

            if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_LANGUAGE)) {
                response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_LANGUAGE,
                        requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_LANGUAGE));
            } else {
                if (objectMeta.getContentLanguage() != null) {
                    response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_LANGUAGE, objectMeta.getContentLanguage());
                }
            }

            if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_CONTENT_TYPE)) {
                response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_TYPE,
                        requestParas.get(RestParamDefine.GetObjectReqPara.RES_CONTENT_TYPE));
            } else {
                if (objectMeta.getContentType() != null) {
                    response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_TYPE, objectMeta.getContentType());
                }
            }

            if (requestParas.containsKey(RestParamDefine.GetObjectReqPara.RES_EXPIRES)) {
                response.addHeader(RestParamDefine.GetObjectResHeader.EXPIRES,
                        requestParas.get(RestParamDefine.GetObjectReqPara.RES_EXPIRES));
            } else {
                if (objectMeta.getExpires() != null) {
                    response.addHeader(RestParamDefine.GetObjectResHeader.EXPIRES, objectMeta.getExpires());
                }
            }
        }

        if (null == range){
            response.setContentLengthLong(objectMeta.getSize());
        }else if (range.getContentLength() >= objectMeta.getSize()){
            response.setContentLengthLong(objectMeta.getSize());
        }else {
            response.addHeader(RestParamDefine.GetObjectResHeader.CONTENT_RANGE,
                    "bytes " + range.getStart() + "-" + range.getEnd() + "/" + objectMeta.getSize());
            response.setContentLengthLong(range.getContentLength());
            response.setStatus(HttpServletResponse.SC_PARTIAL_CONTENT);
        }
    }

    private String getMD5(String contentMd5) throws S3ServerException{
        try {
            if(contentMd5.length() % 4 != 0){
                throw new S3ServerException(S3Error.OBJECT_INVALID_DIGEST,
                        "decode md5 failed, contentMd5:"+contentMd5);
            }
            BASE64Decoder decoder = new BASE64Decoder();
            return new String(Hex.encodeHex(decoder.decodeBuffer(contentMd5)));
        }catch (Exception e){
            throw new S3ServerException(S3Error.OBJECT_INVALID_DIGEST,
                    "decode md5 failed, contentMd5:"+contentMd5);
        }
    }
}
