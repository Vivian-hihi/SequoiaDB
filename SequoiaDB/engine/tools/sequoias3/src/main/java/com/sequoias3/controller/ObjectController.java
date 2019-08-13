package com.sequoias3.controller;

import com.sequoias3.common.RestParamDefine;
import com.sequoias3.core.*;
import com.sequoias3.exception.S3Error;
import com.sequoias3.exception.S3ServerException;
import com.sequoias3.model.*;
import com.sequoias3.service.ObjectService;
import com.sequoias3.utils.RestUtils;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.http.HttpHeaders;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import java.io.IOException;
import java.io.InputStream;
import java.util.*;

@RestController
public class ObjectController {
    private final Logger logger = LoggerFactory.getLogger(ObjectController.class);

    @Autowired
    RestUtils restUtils;

    @Autowired
    ObjectService objectService;

    @PutMapping(value="/{bucketname:.+}/**", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity putObject(@PathVariable("bucketname") String bucketName,
                                    @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                    @RequestHeader(name = RestParamDefine.PutObjectHeader.CONTENT_MD5, required = false) String contentMD5,
                                    HttpServletRequest httpServletRequest)
            throws S3ServerException, IOException {
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);

            String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
            logger.debug("put object. bucketName={}, objectName={}", bucketName, objectName);

            Map<String, String> requestHeaders = new HashMap<>();
            Map<String, String> xMeta = new HashMap<>();
            restUtils.getHeaders(httpServletRequest, requestHeaders, xMeta);

            InputStream body = null;
            Long realContenLength = 0L;
            if (httpServletRequest.getHeader("x-amz-decoded-content-length") != null) {
                body = new S3InputStreamReaderChunk(httpServletRequest.getInputStream());
                realContenLength = Long.parseLong(httpServletRequest.getHeader("x-amz-decoded-content-length"));
            } else {
                body = httpServletRequest.getInputStream();
                if (httpServletRequest.getHeader("content-length") != null) {
                    realContenLength = Long.parseLong(httpServletRequest.getHeader("content-length"));
                }
            }

            PutDeleteResult result = objectService.putObject(operator.getUserId(),
                    bucketName,
                    objectName,
                    contentMD5,
                    requestHeaders,
                    xMeta,
                    body,
                    realContenLength);

            HttpHeaders headers = new HttpHeaders();
            if (result.geteTag() != null) {
                headers.add(RestParamDefine.PutObjectResultHeader.ETAG,
                        "\"" + result.geteTag() + "\"");
            }
            if (result.getVersionId() != null) {
                headers.add(RestParamDefine.PutObjectResultHeader.VERSION_ID,
                        result.getVersionId());
            }

            logger.debug("put object success. bucketName={}, objectName={}, versionId={}, eTag={}", bucketName, objectName, result.getVersionId(), result.geteTag());
            return ResponseEntity.ok()
                    .headers(headers)
                    .build();
        }catch (Exception e){
            logger.error("put object failed. bucketName/objectName:" + httpServletRequest.getRequestURI());
            throw e;
        }
    }

    @GetMapping(value="/{bucketname:.+}/**", produces = MediaType.APPLICATION_XML_VALUE )
    public void getObject(@PathVariable("bucketname") String bucketName,
                                    @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                    @RequestParam(value = RestParamDefine.VERSION_ID, required = false) String versionId,
                                    HttpServletRequest httpServletRequest,
                                    HttpServletResponse response)
            throws S3ServerException, IOException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);
            String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
            logger.debug("get object. bucketName={}, objectName={}", bucketName, objectName);

            Map<String, String> requestHeaders = new HashMap<>();
            Enumeration headerNames = httpServletRequest.getHeaderNames();
            while (headerNames.hasMoreElements()) {
                String name = headerNames.nextElement().toString();
                requestHeaders.put(name, httpServletRequest.getHeader(name));
            }

            Range range = null;
            if (requestHeaders.containsKey(RestParamDefine.GetObjectReqHeader.REQ_RANGE)) {
                range = restUtils.getRange(requestHeaders.get(RestParamDefine.GetObjectReqHeader.REQ_RANGE));
            }

            Map<String, String> requestParas = new HashMap<>();
            Enumeration paraNames = httpServletRequest.getParameterNames();
            while (paraNames.hasMoreElements()) {
                String name = paraNames.nextElement().toString();
                requestParas.put(name, httpServletRequest.getParameter(name));
            }

            Boolean nullVersionFlag = null;
            Long cvtVersionId = null;
            if (versionId != null) {
                cvtVersionId = convertVersionId(versionId);
                if (null == cvtVersionId) {
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
            } finally {
                objectService.releaseGetResult(result);
            }

            logger.debug("get object success. bucketName={}, objectName={}", bucketName, objectName);
        }catch (Exception e){
            logger.error("get object failed. bucketName={}, bucketName/objectName={}, versionId={}",
                    bucketName, httpServletRequest.getRequestURI(), versionId);
            throw e;
        }
    }

    @DeleteMapping(value = "/{bucketname:.+}/**", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteObject(@PathVariable("bucketname") String bucketName,
                                       @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                       HttpServletRequest httpServletRequest)
            throws S3ServerException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);
            String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
            logger.debug("delete object. bucketName={}, objectName={}", bucketName, objectName);

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
            logger.debug("delete object success. bucketName={}, objectName={}", bucketName, objectName);
            return ResponseEntity.noContent()
                    .headers(headers)
                    .build();
        }catch (Exception e){
            logger.error("delete object failed. bucketName={}, bucketName/objectName={}", bucketName, httpServletRequest.getRequestURI());
            throw e;
        }
    }

    @DeleteMapping(value = "/{bucketname:.+}/**", params = RestParamDefine.VERSION_ID, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity deleteObjectByVersionId(@PathVariable("bucketname") String bucketName,
                                                  @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                                  @RequestParam(RestParamDefine.VERSION_ID) String versionId,
                                                  HttpServletRequest httpServletRequest)
            throws S3ServerException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);
            String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
            logger.debug("delete object with version id. bucketName={}, objectName={}, versionId={}",
                    bucketName, objectName, versionId);

            Boolean nullVersionFlag = null;
            Long cvtVersionId = convertVersionId(versionId);
            if (null == cvtVersionId) {
                nullVersionFlag = true;
            }

            objectService.deleteObject(operator.getUserId(), bucketName, objectName, cvtVersionId, nullVersionFlag);
            HttpHeaders headers = new HttpHeaders();
            headers.add(RestParamDefine.DeleteObjectResultHeader.VERSION_ID, versionId);

            logger.debug("delete object with version id success. bucketName={}, objectName={}, versionId={}",
                    bucketName, objectName, versionId);
            return ResponseEntity.noContent()
                    .headers(headers)
                    .build();
        }catch (Exception e){
            logger.error("delete object failed. bucketName={}, bucketName/objectName={}, versionId={}",
                    bucketName, httpServletRequest.getRequestURI(), versionId);
            throw e;
        }
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.ListObjectsPara.LIST_TYPE2,
            produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsV2(@PathVariable("bucketname") String bucketName,
                                        @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.PREFIX, required = false) String prefix,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.DELIMITER, required = false) String delimiter,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.START_AFTER, required = false) String startAfter,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.MAX_KEYS, required = false, defaultValue = "1000") Integer maxKeys,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.CONTINUATIONTOKEN, required = false) String continueToken,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.ENCODING_TYPE, required = false) String encodingType,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.FETCH_OWNER, required = false, defaultValue = "false") Boolean fetchOwner)
            throws S3ServerException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);
            logger.debug("list objectsV2. bucketName={}, delimiter={}, prefix={}, startAfter={}",
                    bucketName, delimiter, prefix, startAfter);

            if (null != encodingType) {
                if (!encodingType.equals(RestParamDefine.ENCODING_TYPE_URL)) {
                    throw new S3ServerException(S3Error.OBJECT_INVALID_ENCODING_TYPE, "encoding type must be url");
                }
            }

            if (delimiter != null && delimiter.length() == 0) {
                delimiter = null;
            }

            if (prefix != null && prefix.length() == 0) {
                prefix = null;
            }

            ListObjectsResult result = objectService.listObjects(operator.getUserId(),
                    bucketName, prefix, delimiter, startAfter,
                    maxKeys, continueToken, encodingType, fetchOwner);

            logger.debug("list objectsV2 success. bucketName={}, " +
                            "commonprefix.size={}, " +
                            "content.size={} ",
                    bucketName,
                    result.getCommonPrefixList().size(),
                    result.getCommonPrefixList().size());
            return ResponseEntity.ok()
                    .body(result);
        }catch (Exception e){
            logger.error("List objects v2 failed. bucketName={}, delimiter={}, prefix={}, " +
                    "startAfter={}", bucketName, delimiter, prefix, startAfter);
            throw e;
        }
    }

    @GetMapping(value = "/{bucketname:.+}", produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsV1(@PathVariable("bucketname") String bucketName,
                                        @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                        @RequestParam(value = RestParamDefine.ListObjectsPara.PREFIX, required = false) String prefix,
                                        @RequestParam(value = RestParamDefine.ListObjectsV1Para.DELIMITER, required = false) String delimiter,
                                        @RequestParam(value = RestParamDefine.ListObjectsV1Para.MARKER, required = false) String startAfter,
                                        @RequestParam(value = RestParamDefine.ListObjectsV1Para.MAX_KEYS, required = false, defaultValue = "1000") Integer maxKeys,
                                        @RequestParam(value = RestParamDefine.ListObjectsV1Para.ENCODING_TYPE, required = false) String encodingType)
            throws S3ServerException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);
            logger.debug("list objectsV1 with delimiter={}, marker={}, prefix={}, maxKeys={}, encodingType={}", delimiter, startAfter, prefix, maxKeys, encodingType);

            if (null != encodingType) {
                if (!encodingType.equals(RestParamDefine.ENCODING_TYPE_URL)) {
                    throw new S3ServerException(S3Error.OBJECT_INVALID_ENCODING_TYPE, "encoding type must be url");
                }
            }

            if (delimiter != null && delimiter.length() == 0) {
                delimiter = null;
            }

            if (prefix != null && prefix.length() == 0) {
                prefix = null;
            }

            ListObjectsResultV1 result = objectService.listObjectsV1(operator.getUserId(),
                    bucketName, prefix, delimiter, startAfter,
                    maxKeys, encodingType);

            logger.debug("list objectsV1 success. bucketName={}," +
                            " commonPrefix.size={}," +
                            " content.size={}",
                    bucketName,
                    result.getCommonPrefixList().size(),
                    result.getContentList().size());
            return ResponseEntity.ok()
                    .body(result);
        }catch (Exception e){
            logger.error("list objectsV1 failed. with delimiter={}, marker={}, prefix={}, maxKeys={}, " +
                    "encodingType={}", delimiter, startAfter, prefix, maxKeys, encodingType);
            throw e;
        }
    }

    @GetMapping(value = "/{bucketname:.+}", params = RestParamDefine.VERSIONS, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity listObjectsVersions(@PathVariable("bucketname") String bucketName,
                                              @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.PREFIX, required = false) String prefix,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.DELIMITER, required = false) String delimiter,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.KEY_MARKER, required = false) String keyMarker,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.VERSION_ID_MARKER, required = false) String versionIdMarker,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.ENCODING_TYPE, required = false) String encodingType,
                                              @RequestParam(value = RestParamDefine.ListObjectVersionsPara.MAX_KEYS, required = false, defaultValue = "1000") Integer maxKeys)
            throws S3ServerException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);
            logger.debug("list versions. bucketName={}, delimiter={}, prefix={}, startAfter={}", bucketName, delimiter, prefix, keyMarker);

            if (null != encodingType) {
                if (!encodingType.equals(RestParamDefine.ENCODING_TYPE_URL)) {
                    throw new S3ServerException(S3Error.OBJECT_INVALID_ENCODING_TYPE, "encoding type must be url");
                }
            }

            if (delimiter != null && delimiter.length() == 0) {
                delimiter = null;
            }

            if (prefix != null && prefix.length() == 0) {
                prefix = null;
            }

            ListVersionsResult result = objectService.listVersions(operator.getUserId(),
                    bucketName, prefix, delimiter, keyMarker, versionIdMarker, maxKeys, encodingType);

            logger.debug("list versions success. bucketName={}, " +
                            "commonPrefix.size={}, " +
                            "versions.size={}, " +
                            "deleterMarker.size={}",
                    bucketName,
                    result.getCommonPrefixList().size(),
                    result.getVersionList().size(),
                    result.getDeleteMarkerList().size());
            return ResponseEntity.ok()
                    .body(result);
        }catch (Exception e){
            logger.error("list versions failed. bucketName={}, delimiter={}, prefix={}, startAfter={}",
                    bucketName, delimiter, prefix, keyMarker);
            throw e;
        }
    }

    @RequestMapping(method = RequestMethod.HEAD, value="/{bucketname:.+}/**")
    public void headObject(@PathVariable("bucketname") String bucketName,
                                     @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                                     @RequestParam(value = RestParamDefine.VERSION_ID, required = false) String versionId,
                                     HttpServletRequest httpServletRequest,
                                     HttpServletResponse response)
            throws S3ServerException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);
            String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
            logger.debug("head object. bucketName={}, objectName={}", bucketName, objectName);

            Map<String, String> requestHeaders = new HashMap<>();
            Enumeration headerNames = httpServletRequest.getHeaderNames();
            while (headerNames.hasMoreElements()) {
                String name = headerNames.nextElement().toString();
                requestHeaders.put(name, httpServletRequest.getHeader(name));
            }

            Range range = null;
            if (requestHeaders.containsKey(RestParamDefine.GetObjectReqHeader.REQ_RANGE)) {
                range = restUtils.getRange(requestHeaders.get(RestParamDefine.GetObjectReqHeader.REQ_RANGE));
            }

            Boolean nullVersionFlag = null;
            Long cvtVersionId = null;
            if (versionId != null) {
                cvtVersionId = convertVersionId(versionId);
                if (null == cvtVersionId) {
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
            } finally {
                objectService.releaseGetResult(result);
            }

            logger.debug("head object success. bucketName={}, objectName={}", bucketName, objectName);
        }catch (Exception e){
            logger.error("head object failed. bucketName={}, bucketName/objectName={}",
                    bucketName, httpServletRequest.getRequestURI());
            throw e;
        }
    }

    @PutMapping(value="/{bucketname:.+}/**", headers = RestParamDefine.CopyObjectHeader.X_AMZ_COPY_SOURCE, produces = MediaType.APPLICATION_XML_VALUE)
    public ResponseEntity copyObject(@PathVariable("bucketname") String bucketName,
                           @RequestHeader(value = RestParamDefine.AUTHORIZATION, required = false) String authorization,
                           @RequestHeader(name = RestParamDefine.CopyObjectHeader.X_AMZ_COPY_SOURCE) String copySource,
                           @RequestHeader(name = RestParamDefine.CopyObjectHeader.METADATA_DIRECTIVE, required = false) String directive,
                           HttpServletRequest httpServletRequest,
                           HttpServletResponse response)
            throws S3ServerException, IOException{
        try {
            User operator = restUtils.getOperatorByAuthorization(authorization);

            String objectName = restUtils.getObjectNameByURI(httpServletRequest.getRequestURI());
            logger.debug("copy object. bucketName={}, objectName={}, source={}", bucketName, objectName, copySource);
            Map<String, String> requestHeaders = new HashMap<>();
            Map<String, String> xMeta = new HashMap<>();
            restUtils.getHeaders(httpServletRequest, requestHeaders, xMeta);

            ObjectUri sourceUri = new ObjectUri(copySource);

            boolean copyMeta = directiveCopy(directive);

            CopyObjectResult result = objectService.copyObject(operator.getUserId(),
                    bucketName, objectName, requestHeaders, xMeta, sourceUri,
                    copyMeta, response.getOutputStream());

            HttpHeaders headers = new HttpHeaders();
            if (result.getVersionId() != null) {
                headers.add(RestParamDefine.CopyObjectResultHeader.VERSION_ID,
                        result.getVersionId().toString());
            }
            if (result.getSourceVersionId() != null) {
                headers.add(RestParamDefine.CopyObjectResultHeader.SOURCE_VERSION_ID,
                        result.getVersionId().toString());
            }

            logger.debug("copy object success. bucketName={}, objectName={}, source={}", bucketName, objectName, copySource);

            return ResponseEntity.ok()
                    .headers(headers)
                    .body(result);
        }catch (Exception e){
            logger.error("copy object failed. bucketName={}, bucketName/objectName={}, " +
                    "source={}", bucketName, httpServletRequest.getRequestURI(), copySource);
            throw e;
        }
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
            throw new S3ServerException(S3Error.OBJECT_INVALID_VERSION,
                    "version id is invalid。 version id=" + versionId);
        }catch (Exception e){
            throw new S3ServerException(S3Error.OBJECT_INVALID_VERSION,
                    "versionId is invalid. versionId="+versionId+",e:"+e.getMessage());
        }
    }

    private boolean directiveCopy(String directive) throws S3ServerException{
        if (directive == null){
            return true;
        }
        if (directive.equals(RestParamDefine.REST_DIRECTIVE_COPY)){
            return true;
        }
        if (directive.equals(RestParamDefine.REST_DIRECTIVE_REPLACE)){
            return false;
        }

        throw new S3ServerException(S3Error.OBJECT_COPY_INVALID_DIRECTIVE,
                "invalid directive. directive="+directive);
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
        response.addHeader(RestParamDefine.GetObjectResHeader.ETAG, "\""+objectMeta.geteTag()+"\"");
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
}
